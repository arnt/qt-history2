/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "phrase.h"

#include <qapplication.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qxml.h>

static QByteArray protect(const QByteArray& str)
{
    QByteArray p = str;
    p.replace("\"", "&quot;");
    p.replace("&", "&amp;");
    p.replace(">", "&gt;");
    p.replace("<", "&lt;");
    p.replace("'", "&apos;");
    return p;
}

Phrase::Phrase(const QString &source, const QString &target,
               const QString &definition, int sc)
    : shrtc(sc), s(source), t(target), d(definition)
{
}

bool operator==(const Phrase &p, const Phrase &q)
{
    return p.source() == q.source() && p.target() == q.target() &&
        p.definition() == q.definition();
}

class QphHandler : public QXmlDefaultHandler
{
public:
    QphHandler(PhraseBook *phraseBook)
        : pb(phraseBook), ferrorCount(0) { }

    virtual bool startElement(const QString &namespaceURI,
        const QString &localName, const QString &qName,
        const QXmlAttributes &atts);
    virtual bool endElement(const QString &namespaceURI,
        const QString &localName, const QString &qName);
    virtual bool characters(const QString &ch);
    virtual bool fatalError(const QXmlParseException &exception);

private:
    PhraseBook *pb;
    QString source;
    QString target;
    QString definition;

    QString accum;
    int ferrorCount;
};

bool QphHandler::startElement(const QString & /* namespaceURI */,
                              const QString & /* localName */,
                              const QString &qName,
                              const QXmlAttributes & /* atts */)
{
    if (qName == QString("phrase")) {
        source.truncate(0);
        target.truncate(0);
        definition.truncate(0);
    }
    accum.truncate(0);
    return true;
}

bool QphHandler::endElement(const QString & /* namespaceURI */,
                            const QString & /* localName */,
                            const QString &qName)
{
    if (qName == QString("source"))
        source = accum;
    else if (qName == QString("target"))
        target = accum;
    else if (qName == QString("definition"))
        definition = accum;
    else if (qName == QString("phrase"))
        pb->append(Phrase(source, target, definition));
    return true;
}

bool QphHandler::characters(const QString &ch)
{
    accum += ch;
    return true;
}

bool QphHandler::fatalError(const QXmlParseException &exception)
{
    if (ferrorCount++ == 0) {
        QString msg;
        msg.sprintf("Parse error at line %d, column %d (%s).",
            exception.lineNumber(), exception.columnNumber(),
            exception.message().toLatin1().constData());
        QMessageBox::information(0,
            QObject::tr("Qt Linguist"), msg);
    }
    return false;
}

bool PhraseBook::load(const QString &filename)
{
    fn = filename;
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QXmlInputSource in(&f);
    QXmlSimpleReader reader;
    // don't click on these!
    reader.setFeature("http://xml.org/sax/features/namespaces", false);
    reader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
    reader.setFeature("http://trolltech.com/xml/features/report-whitespace"
        "-only-CharData", false);
    QXmlDefaultHandler *hand = new QphHandler(this);
    reader.setContentHandler(hand);
    reader.setErrorHandler(hand);

    bool ok = reader.parse(in);
    reader.setContentHandler(0);
    reader.setErrorHandler(0);
    delete hand;
    f.close();
    if (!ok)
        clear();
    return ok;
}

bool PhraseBook::save(const QString &filename) const
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QTextStream t(&f);
    t << "<!DOCTYPE QPH><QPH>\n";
    ConstIterator p;
    for (p = begin(); p != end(); ++p) {
        t << "<phrase>\n";
        t << "    <source>" << protect( (*p).source().toUtf8() ) << "</source>\n";
        t << "    <target>" << protect( (*p).target().toUtf8() ) << "</target>\n";
        if (!(*p).definition().isEmpty())
            t << "    <definition>" << protect( (*p).definition().toUtf8())
              << "</definition>\n";
        t << "</phrase>\n";
    }
    t << "</QPH>\n";
    f.close();
    return true;
}
