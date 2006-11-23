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
#include "xliff.h"

#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtXml>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

/**
 * Implementation of XLIFF file format for Linguist
 */
static const char *restypeContext = "x-trolltech-linguist-context";
static const char *restypePlurals = "x-trolltech-linguist-plurals";
static const char *dataTypeUIFile = "x-trolltech-designer-ui";
static const char *XLIFFnamespaceURI = "urn:oasis:names:tc:xliff:document:1.1";

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

struct CharMnemonic
{
    char ch;
    char escape;
    char *mnemonic;
};

static const CharMnemonic charCodeMnemonics[] = {
    {0x07, 'a', "bel"},
    {0x08, 'b', "bs"},
    {0x09, 't', "tab"},
    {0x0a, 'n', "lf"},
    {0x0b, 'v', "vt"},
    {0x0c, 'f', "ff"},
    {0x0d, 'r', "cr"}
};

static char charFromEscape(char escape)
{
    for (int i = 0; i < sizeof(charCodeMnemonics)/sizeof(CharMnemonic); ++i) {
        CharMnemonic cm =  charCodeMnemonics[i];
        if (cm.escape == escape) return cm.ch;
    }
    Q_ASSERT(0);
    return escape;
}

static QString numericEntity( int ch )
{
    QString name;
    char escapechar;
    
    // ### This needs to be reviewed, to reflect the updated XLIFF-PO spec.
    if (ch >= 7 && ch <= 0x0d)
    {
        CharMnemonic cm = charCodeMnemonics[int(ch) - 7];
        name = QLatin1String(cm.mnemonic);
        escapechar = cm.escape;
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
            result += QLatin1String( "&quot;" );
            break;
        case '&':
            result += QLatin1String( "&amp;" );
            break;
        case '>':
            result += QLatin1String( "&gt;" );
            break;
        case '<':
            result += QLatin1String( "&lt;" );
            break;
        case '\'':
            result += QLatin1String( "&apos;" );
            break;
        default:
            if ( (uchar) str[k] < 0x20 && str[k] != '\n' )
                result += numericEntity( (uchar) str[k] );
            else
                result += QLatin1Char(str[k]);
        }
    }
    return result;
}

static void writeLineNumber(QTextStream *t, const MetaTranslatorMessage &msg, int indent)
{
    if (msg.lineNumber() != -1) {
        writeIndent(t, indent);
        (*t) << QLatin1String("<context-group name=\"lineNo\" purpose=\"location\"><context context-type=\"linenumber\">") 
            << msg.lineNumber() << QLatin1String("</context></context-group>\n");
    }
}

static void writeComment(QTextStream *t, const MetaTranslatorMessage &msg, int indent)
{
    if (msg.comment() && qstrlen(msg.comment())) {
        writeIndent(t, indent);
        (*t) << QLatin1String("<note>") << msg.comment() << QLatin1String("</note>\n");
    }
}

static void writeTransUnit(QTextStream *t, const MetaTranslatorMessage &msg, int msgid, 
                           int indent, const QString &translation = QString())
{
    static int plural = 0;
    static int prevMsgId = -1;
    writeIndent(t, indent);
    (*t) << QLatin1String("<trans-unit id=\"msg");
    QString strid;
    QByteArray transl;
    if (msg.isPlural()) {
        if (prevMsgId != msgid) 
            plural = 0;
        strid = QString::fromAscii("%1[%2]").arg(msgid).arg(plural);
        ++plural;
        transl = translation.toUtf8();
    } else {
        strid = QString::fromAscii("%1").arg(msgid);
        plural = 0;
        transl = msg.translation().toUtf8();
    }
    prevMsgId = msgid;
    (*t) << strid << QLatin1String("\"");
    QString state;
    indent+=2;
    if (msg.type() == MetaTranslatorMessage::Obsolete) {
        (*t) << QLatin1String(" translate=\"no\"");
    } else {
        state = msg.type() == MetaTranslatorMessage::Finished 
            ? QLatin1String("final") : QLatin1String("new");
        state = QString::fromAscii(" state=\"%1\"").arg(state);
    }
    (*t) << ">\n";
    writeIndent(t, indent);
    (*t) << "<source>" << protect(msg.sourceText()) << "</source>\n";
    
    writeIndent(t, indent);
    (*t) << QLatin1String("<target") << state << QLatin1String(">") << protect(transl) << QLatin1String("</target>\n");
    // ### In XLIFF 1.1, name is marked as required, and it must be unique
    // This is questionable behaviour, and was brought up at the xliff-comments mailinglist.
    if (!msg.isPlural()) {
        writeLineNumber(t, msg, indent);
        writeComment(t, msg, indent);
    }
    indent-=2;
    writeIndent(t, indent);
    (*t) << QLatin1String("</trans-unit>\n");
}

static void writeMessage(QTextStream *t, const MetaTranslatorMessage &msg, int indent, 
                         const QString &languageCode)
{
    static int msgid = 1;
    if (msg.isPlural()) {
        writeIndent(t, indent);
        (*t) << QLatin1String("<group restype=\"") << restypePlurals << QLatin1String("\">\n");
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
        (*t) << QLatin1String("</group>\n");
    } else {
        writeTransUnit(t, msg, msgid, indent);
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
        QString location = msg.fileName() + QLatin1String(msg.context()) + msg.lineNumber();
        mtSortByFileName.insertMulti(location, msg);
        ++m;
    }

    t.setFieldAlignment(QTextStream::AlignRight);
    t << QLatin1String("<?xml version=\"1.0\"");
    t << QLatin1String(" encoding=\"utf-8\"?>\n");
    t << QLatin1String("<xliff version=\"1.1\" xmlns=\"") << XLIFFnamespaceURI << QLatin1String("\">\n");
    currentindent += indent;
    QMap<QString, MetaTranslatorMessage>::iterator mi = mtSortByFileName.begin();
    MetaTranslatorMessage msg;
    QByteArray ctx;
    QString fn;
    bool ctxdiffer = false;
    bool filediffer = false;
    while (mi != mtSortByFileName.end()) {
        msg = mi.value();
        ctxdiffer = msg.context() != ctx;
        filediffer = msg.fileName() != fn;

        if (ctxdiffer || filediffer) {
            if (!ctx.isEmpty()) {
            writeIndent(&t, currentindent);
                t << QLatin1String("</group>\n");
                currentindent -= indent;
            }
        }

        if (filediffer) {
            if (!fn.isEmpty()) {
                writeIndent(&t, currentindent);
                t << QLatin1String("</body></file>\n");
                currentindent -= indent;
            }
            fn = msg.fileName();

            writeIndent(&t, currentindent);
            t << QLatin1String("<file original=\"") << fn << QLatin1String("\"") +
                QLatin1String(" datatype=\"") << dataType(msg) << QLatin1String("\"") +
                QLatin1String(" source-language=\"") + QLatin1String("en") + QLatin1String("\"") +
                QLatin1String(" target-language=\"") + languageCode() + QLatin1String("\"") +
                QLatin1String("><body>\n");
            currentindent += indent;

        }

        if (ctxdiffer || filediffer) {
            ctx = msg.context();
            writeIndent(&t, currentindent);
            t << QLatin1String("<group restype=\"") << restypeContext << QLatin1String("\"") +
                QLatin1String(" resname=\"") << protect(ctx) << QLatin1String("\"") +
                QLatin1String(">\n");
            currentindent += indent;
        }

        writeMessage(&t, msg, currentindent, m_language);
        ++mi;
    }
    currentindent-=indent;
    writeIndent(&t, currentindent);
    t << QLatin1String("</group>\n");
    currentindent-=indent;
    writeIndent(&t, currentindent);
    t << QLatin1String("</body></file>\n");
    t << QLatin1String("</xliff>\n");

    f.close();
    return true;
}

XLIFFHandler::XLIFFHandler( MetaTranslator *translator )
    : tor( translator ), 
      m_type( MetaTranslatorMessage::Finished ),
      inMessage( false ), 
      m_inContextGroup(false), 
      m_lineNumber(-1), 
      ferrorCount( 1 ), 
      contextIsUtf8( false ),
      messageIsUtf8( false ), 
      m_isPlural(false), 
      m_URI(QLatin1String(XLIFFnamespaceURI))
{ 
    
}

bool XLIFFHandler::startElement( const QString& namespaceURI,
                           const QString& localName, const QString& /*qName*/,
                           const QXmlAttributes& atts )
{
    if (namespaceURI == m_URI) {
        if (localName == QLatin1String("file")) {
            m_fileName = atts.value(QLatin1String("original"));
            m_language = atts.value(QLatin1String("target-language"));
        } else if (localName == QLatin1String("group")) {
            if (atts.value(QLatin1String("restype")) == QLatin1String(restypeContext)) {
                m_context = atts.value(QLatin1String("resname"));
            } else {
                m_isPlural = atts.value(QLatin1String("restype")) 
                                  == QLatin1String(restypePlurals);
                m_comment.clear();
            }
        } else if (localName == QLatin1String("trans-unit")) {
            inMessage = true;
            if (atts.value(QLatin1String("translate")) == QLatin1String("no")) {
                m_type = MetaTranslatorMessage::Obsolete;
            }
            m_comment.clear();
        } else if (localName == QLatin1String("target")) {
            QString state = atts.value(QLatin1String("state"));
            if (state == QLatin1String("new")) {
                m_type = MetaTranslatorMessage::Unfinished;
            } else if (state == QLatin1String("final")) {
                m_type = MetaTranslatorMessage::Finished;
            }
        } else if (localName == QLatin1String("context-group")) {
            QString purpose = atts.value(QLatin1String("purpose"));
            if (purpose == QLatin1String("location")) {
                m_inContextGroup = true;
            }
        } else if (m_inContextGroup && localName == QLatin1String("context")) {
            m_inContextGroup = atts.value(QLatin1String("context-type")) 
                                == QLatin1String("linenumber");
        } else if (localName == QLatin1String("ph")) {
            QString ctype = atts.value(QLatin1String("ctype"));
            if (ctype.startsWith(QLatin1String("x-ch-"))) {
                m_ctype = ctype.right(2);
            }
        }
    }
    accum.clear();
    return true;
}

bool XLIFFHandler::endElement(const QString& namespaceURI,
                              const QString& localName, const QString& /*qName*/ )
{
    if (namespaceURI == m_URI) {
        if (localName == QLatin1String("source")) {
            m_source = accum;
        } else if (localName == QLatin1String("target")) {
            translations.append(accum);
        } else if (localName == QLatin1String("context-group")) {
            m_inContextGroup = false;
        } else if (m_inContextGroup && localName == QLatin1String("context")) {
            bool ok;
            m_lineNumber = accum.trimmed().toInt(&ok);
            if (!ok)
                m_lineNumber = -1;
        } else if (localName == QLatin1String("note")) {
            m_comment = accum;
        } else if (localName == QLatin1String("ph")) {
            m_ctype.clear();
        } else if (localName == QLatin1String("trans-unit")) {
            if (!m_isPlural) {
                tor->insert( MetaTranslatorMessage(m_context.toAscii(), m_source.toAscii(),
                                                m_comment.toAscii(), m_fileName, m_lineNumber, 
                                                translations, false, m_type, m_isPlural) );
                translations.clear();
                m_lineNumber = -1;
            }
        } else if (localName == QLatin1String("group")) {
            if (m_isPlural) {
                tor->insert( MetaTranslatorMessage(m_context.toAscii(), m_source.toAscii(),
                                                m_comment.toAscii(), m_fileName, m_lineNumber, 
                                                translations, false, m_type, m_isPlural) );
                m_isPlural = false;
                translations.clear();
                m_lineNumber = -1;
            }
        }
    }
    return true;
}

bool XLIFFHandler::characters( const QString& ch )
{
    QString t = ch;
    if (m_ctype.isEmpty()) {
        t.replace( QLatin1String("\r"), QLatin1String("") );
        accum += t;
    } else {
        // handle the content of <ph> elements
        for (int i = 0; i < ch.count(); ++i) {
            QChar chr  = ch.at(i);
            if (accum.endsWith(QLatin1Char('\\'))) {
                accum[accum.size() - 1] = QLatin1Char(charFromEscape(chr.toAscii()));
            } else {
                accum.append(chr);
            }
        }
    }
    return true;
}

bool XLIFFHandler::endDocument()
{
    tor->setLanguageCode(m_language);
    return true;
}

bool XLIFFHandler::fatalError( const QXmlParseException& exception )
{
    QString msg;
    msg.sprintf( "Parse error at line %d, column %d (%s).",
                 exception.lineNumber(), exception.columnNumber(),
                 exception.message().toLatin1().data() );
    if ( qApp == 0 )
        fprintf( stderr, "XML error: %s\n", msg.toLatin1().data() );
    else
        QMessageBox::information(0,
                                  QObject::tr("Qt Linguist"), msg );

    return false;
}
