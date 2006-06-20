/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qxml.h>


//TESTED_CLASS=
//TESTED_FILES=qxml.h

class tst_QXml : public QObject
{
Q_OBJECT

public:
    tst_QXml();
    virtual ~tst_QXml();

private slots:
    void getSetCheck();
};

tst_QXml::tst_QXml()
{
}

tst_QXml::~tst_QXml()
{
}

class MyXmlEntityResolver : public QXmlEntityResolver
{
public:
    MyXmlEntityResolver() : QXmlEntityResolver() {}
    QString errorString() const { return QString(); } 
    bool resolveEntity(const QString &, const QString &, QXmlInputSource *&) { return false; }
};

class MyXmlContentHandler : public QXmlContentHandler
{
public:
    MyXmlContentHandler() : QXmlContentHandler() {}
    bool characters(const QString &) { return false; }
    bool endDocument() { return false; }
    bool endElement(const QString &, const QString &, const QString &) { return false; }
    bool endPrefixMapping(const QString &) { return false; }
    QString errorString() const { return QString(); }
    bool ignorableWhitespace(const QString &) { return false; }
    bool processingInstruction(const QString &, const QString &) { return false; }
    void setDocumentLocator(QXmlLocator *) { }
    bool skippedEntity(const QString &) { return false; }
    bool startDocument() { return false; }
    bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &) { return false; }
    bool startPrefixMapping(const QString &, const QString &) { return false; }
};

class MyXmlErrorHandler : public QXmlErrorHandler
{
public:
    MyXmlErrorHandler() : QXmlErrorHandler() {}
    QString errorString() const { return QString(); } 
    bool error(const QXmlParseException &) { return false; }
    bool fatalError(const QXmlParseException &) { return false; }
    bool warning(const QXmlParseException &) { return false; }
};

class MyXmlLexicalHandler : public QXmlLexicalHandler
{
public:
    MyXmlLexicalHandler() : QXmlLexicalHandler() {}
    bool comment(const QString &) { return false; }
    bool endCDATA() { return false; }
    bool endDTD() { return false; }
    bool endEntity(const QString &) { return false; }
    QString errorString() const { return QString(); }
    bool startCDATA() { return false; }
    bool startDTD(const QString &, const QString &, const QString &) { return false; }
    bool startEntity(const QString &) { return false; }
};

class MyXmlDeclHandler : public QXmlDeclHandler
{
public:
    MyXmlDeclHandler() : QXmlDeclHandler() {}
    bool attributeDecl(const QString &, const QString &, const QString &, const QString &, const QString &) { return false; }
    QString errorString() const { return QString(); }
    bool externalEntityDecl(const QString &, const QString &, const QString &) { return false; }
    bool internalEntityDecl(const QString &, const QString &) { return false; }
};

// Testing get/set functions
void tst_QXml::getSetCheck()
{
    QXmlSimpleReader obj1;
    // QXmlEntityResolver* QXmlSimpleReader::entityResolver()
    // void QXmlSimpleReader::setEntityResolver(QXmlEntityResolver*)
    MyXmlEntityResolver *var1 = new MyXmlEntityResolver;
    obj1.setEntityResolver(var1);
    QCOMPARE(var1, obj1.entityResolver());
    obj1.setEntityResolver((QXmlEntityResolver *)0);
    QCOMPARE((QXmlEntityResolver *)0, obj1.entityResolver());
    delete var1;

    // QXmlContentHandler* QXmlSimpleReader::contentHandler()
    // void QXmlSimpleReader::setContentHandler(QXmlContentHandler*)
    MyXmlContentHandler *var2 = new MyXmlContentHandler;
    obj1.setContentHandler(var2);
    QCOMPARE(var2, obj1.contentHandler());
    obj1.setContentHandler((QXmlContentHandler *)0);
    QCOMPARE((QXmlContentHandler *)0, obj1.contentHandler());
    delete var2;

    // QXmlErrorHandler* QXmlSimpleReader::errorHandler()
    // void QXmlSimpleReader::setErrorHandler(QXmlErrorHandler*)
    MyXmlErrorHandler *var3 = new MyXmlErrorHandler;
    obj1.setErrorHandler(var3);
    QCOMPARE(var3, obj1.errorHandler());
    obj1.setErrorHandler((QXmlErrorHandler *)0);
    QCOMPARE((QXmlErrorHandler *)0, obj1.errorHandler());
    delete var3;

    // QXmlLexicalHandler* QXmlSimpleReader::lexicalHandler()
    // void QXmlSimpleReader::setLexicalHandler(QXmlLexicalHandler*)
    MyXmlLexicalHandler *var4 = new MyXmlLexicalHandler;
    obj1.setLexicalHandler(var4);
    QCOMPARE(var4, obj1.lexicalHandler());
    obj1.setLexicalHandler((QXmlLexicalHandler *)0);
    QCOMPARE((QXmlLexicalHandler *)0, obj1.lexicalHandler());
    delete var4;

    // QXmlDeclHandler* QXmlSimpleReader::declHandler()
    // void QXmlSimpleReader::setDeclHandler(QXmlDeclHandler*)
    MyXmlDeclHandler *var5 = new MyXmlDeclHandler;
    obj1.setDeclHandler(var5);
    QCOMPARE(var5, obj1.declHandler());
    obj1.setDeclHandler((QXmlDeclHandler *)0);
    QCOMPARE((QXmlDeclHandler *)0, obj1.declHandler());
    delete var5;
}

QTEST_MAIN(tst_QXml)
#include "tst_qxml.moc"
