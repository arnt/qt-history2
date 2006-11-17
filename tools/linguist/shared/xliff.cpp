/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metatranslator.h"
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>

/**
 * Implementation of XLIFF file format for Linguist
 */
static const char *restypeContext = "x-trolltech-linguist-context";
static const char *restypePlurals = "x-trolltech-linguist-plurals";
static const char *dataTypeUIFile = "x-trolltech-designer-ui";

static QString dataType(const MetaTranslatorMessage &m)
{
    QString fileName = m.fileName();
    if (fileName.endsWith(QLatin1String(".cpp")))
        return QLatin1String("cpp");
    else
        return QLatin1String(dataTypeUIFile);        //### form?

}

static void writeIndent(QTextStream *t, int indent)
{
    // ### slow (?)
    for (int i = 0 ; i < indent; ++i) {
        (*t) << " ";
    }
}

static QString numericEntity( int ch )
{
    QString name;
    char escapechar;
    switch (ch) {
        case '\a':  //0x07
            name = QLatin1String("bel");
            escapechar = 'a';
            break;
        case '\b':  //0x08
            name = QLatin1String("bs");
            escapechar = 'b';
            break;
        case '\t':  //0x09
            name = QLatin1String("tab");
            escapechar = 't';
            break;
        case '\n':  //0x0a
            name = QLatin1String("lf");
            escapechar = 'n';
            break;
        case '\v':  //0x0b
            name = QLatin1String("vt");
            escapechar = 'v';
            break;
        case '\f':  //0x0c
            name = QLatin1String("ff");
            escapechar = 'f';
            break;
        case '\r':  //0x0d
            name = QLatin1String("cr");
            escapechar = 'r';
            break;
        default:
            // write the numerical value if not all the others did match ???
            break;
    }
    static int id = 0;
    return QString::fromAscii("<ph id=\"ph%1\" ctype=\"x-ch-%2\">\\%3</ph>")
                            .arg(++id)
                            .arg(name)
                            .arg(escapechar);
}

static QString protect( const QByteArray& str )
{
    QString result;
    int len = (int) str.length();
    for ( int k = 0; k < len; k++ ) {
        switch( str[k] ) {
        case '\"':
            result += QString( "&quot;" );
            break;
        case '&':
            result += QString( "&amp;" );
            break;
        case '>':
            result += QString( "&gt;" );
            break;
        case '<':
            result += QString( "&lt;" );
            break;
        case '\'':
            result += QString( "&apos;" );
            break;
        default:
            if ( (uchar) str[k] < 0x20 && str[k] != '\n' )
                result += numericEntity( (uchar) str[k] );
            else
                result += str[k];
        }
    }
    return result;
}

static void writeLineNumber(QTextStream *t, const MetaTranslatorMessage &msg, int indent)
{
    if (msg.lineNumber() != -1) {
        writeIndent(t, indent);
        (*t) << "<context-group name=\"lineNo\" purpose=\"location\"><context context-type=\"linenumber\">" 
            << msg.lineNumber() << "</context></context-group>\n";
    }
}

static void writeComment(QTextStream *t, const MetaTranslatorMessage &msg, int indent)
{
    if (msg.comment()) {
        writeIndent(t, indent);
        (*t) << "<note>" << msg.comment() << "</note>\n";
    }
}

static void writeTransUnit(QTextStream *t, const MetaTranslatorMessage &msg, int msgid, 
                           int indent, const QString &translation)
{
    static int plural = 0;
    static int prevMsgId = -1;
    writeIndent(t, indent);
    (*t) << "<trans-unit id=\"msg";
    QString strid;
    if (msg.isPlural()) {
        if (prevMsgId != msgid) 
            plural = 0;
        strid = QString::fromAscii("%1[%2]").arg(msgid).arg(plural);
        ++plural;
    } else {
        strid = QString::fromAscii("%1").arg(msgid);
        plural = 0;
    }
    prevMsgId = msgid;
    (*t) << strid << "\"";
    QString state;
    indent+=2;
    if (msg.type() == MetaTranslatorMessage::Obsolete) {
        (*t) << " translate=\"no\"";
    } else {
        state = msg.type() == MetaTranslatorMessage::Finished 
            ? QLatin1String("final") : QLatin1String("new");
        state = QString::fromAscii(" state=\"%1\"").arg(state);
    }
    (*t) << ">\n";
    writeIndent(t, indent);
    (*t) << "<source>" << protect(msg.sourceText()) << "</source>\n";
    
    QByteArray transl = translation.toUtf8();
    writeIndent(t, indent);
    (*t) << "<target" << state << ">" << protect(transl) << "</target>\n";
    // ### In XLIFF 1.1, name is marked as required, and it must be unique
    // This is questionable behaviour, and was brought up at the xliff-comments mailinglist.
    if (!msg.isPlural()) {
        writeLineNumber(t, msg, indent);
        writeComment(t, msg, indent);
    }
    indent-=2;
    writeIndent(t, indent);
    (*t) << "</trans-unit>\n";
}

static void writeMessage(QTextStream *t, const MetaTranslatorMessage &msg, int indent, 
                         const QString &languageCode)
{
    static int msgid = 1;
    if (msg.isPlural()) {
        writeIndent(t, indent);
        (*t) << "<group restype=\"" << restypePlurals << "\">\n";
        indent+=2;
        writeLineNumber(t, msg, indent);
        writeComment(t, msg, indent);
        
        QLocale::Language l;
        QLocale::Country c;
        MetaTranslator::languageAndCountry(languageCode, &l, &c);
        QStringList translns = MetaTranslator::normalizedTranslations(msg, l, c);
        for (int j = 0; j < qMax(1, translns.count()); ++j) {
            writeTransUnit(t, msg, msgid, indent, translns.at(j));
        }
        indent-=2;
        writeIndent(t, indent);
        (*t) << "</group>\n";
    } else {
        writeTransUnit(t, msg, msgid, indent, 0);
    }
    ++msgid;
}

bool MetaTranslator::saveXLIFF( const QString& filename) const
{
    QFile f( filename );
    if ( !f.open(QIODevice::WriteOnly | QIODevice::Text) )
        return false;

    int indent = 2;
    int currentindent = 0;

    QTextStream t( &f );
    t.setCodec( QTextCodec::codecForName("ISO-8859-1") );

    QMap<QString, MetaTranslatorMessage> mtSortByFileName;
    TMM::ConstIterator m = mm.begin();
    while ( m != mm.end() ) {
        MetaTranslatorMessage msg = m.key();
        QString location = msg.fileName() + msg.context() + msg.lineNumber();
        mtSortByFileName.insert(location, msg);
        ++m;
    }

    t.setFieldAlignment(QTextStream::AlignRight);
    t << "<?xml version=\"1.0\"";
    t << " encoding=\"utf-8\"?>\n";
    t << "<xliff version=\"1.1\" xmlns=\"urn:oasis:names:tc:xliff:document:1.1\">\n";
    currentindent += indent;
    QMap<QString, MetaTranslatorMessage>::iterator mi = mtSortByFileName.begin();
    MetaTranslatorMessage msg;
    QByteArray ctx;
    QString fn;
    bool ctxdiffer = false;
    while (mi != mtSortByFileName.end()) {
        msg = mi.value();
        ctxdiffer = msg.context() != ctx;
        if (ctxdiffer) {
            if (!ctx.isEmpty()) {
            writeIndent(&t, currentindent);
                t << "</group>\n";
                currentindent -= indent;
            }
        }

        if (msg.fileName() != fn) {
            if (!fn.isEmpty()) {
                writeIndent(&t, currentindent);
                t << "</body></file>\n";
                currentindent -= indent;
            }
            fn = msg.fileName();

            writeIndent(&t, currentindent);
            t << "<file original=\"" << fn << "\""
                " datatype=\"" << dataType(msg) << "\""
                " source-language=\"" + QLatin1String("en") + "\""
                " target-language=\"" + languageCode() + "\""
                "><body>\n";
            currentindent += indent;

        }

        if (ctxdiffer) {
            ctx = msg.context();
            writeIndent(&t, currentindent);
            t << "<group restype=\"" << restypeContext << "\""
                " resname=\"" << protect(ctx) << "\""
                ">\n";
            currentindent += indent;
        }

        writeMessage(&t, msg, currentindent, m_language);
        ++mi;
    }
    currentindent-=indent;
    writeIndent(&t, currentindent);
    t << "</group>\n";
    currentindent-=indent;
    writeIndent(&t, currentindent);
    t << "</body></file>\n";
    t << "</xliff>\n";

    f.close();
    return true;
}



