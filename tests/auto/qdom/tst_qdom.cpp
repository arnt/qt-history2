/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>
#include <qxml.h>

#include <qlist.h>
#include <qvariant.h>
#include <qcoreapplication.h>

//TESTED_CLASS=
//TESTED_FILES=xml/qdom.h xml/qdom.cpp

class QDomDocument;
class QDomNode;

class tst_QDom : public QObject
{
    Q_OBJECT

private slots:
    void setContent_data();
    void setContent();
    void toString_01_data();
    void toString_01();
    void toString_02_data();
    void toString_02();
    void hasAttributes_data();
    void hasAttributes();
    void save_data();
    void save();
    void saveWithSerialization() const;
    void saveWithSerialization_data() const;
    void cloneNode_data();
    void cloneNode();
    void ownerDocument_data();
    void ownerDocument();
    void ownerDocumentTask27424_data();
    void ownerDocumentTask27424();
    void parentNode_data();
    void parentNode();
    void documentCreationTask27424_data();
    void documentCreationTask27424();
    void browseElements();
    void ownerElementTask45192_data();
    void ownerElementTask45192();
    void domNodeMapAndList();

    void nullDocument();
    void invalidName_data();
    void invalidName();
    void invalidQualifiedName_data();
    void invalidQualifiedName();
    void invalidCharData_data();
    void invalidCharData();

    void appendChild();

    void checkWarningOnNull() const;
    void roundTripAttributes() const;
    void normalizeEndOfLine() const;
    void normalizeAttributes() const;
    void serializeWeirdEOL() const;
    void reparentAttribute() const;
    void serializeNamespaces() const;
    void flagInvalidNamespaces() const;
    void flagUndeclaredNamespace() const;

    void initTestCase();
    void indentComments() const;
    void checkLiveness() const;

private:
    static int hasAttributesHelper( const QDomNode& node );
    static bool compareDocuments( const QDomDocument &doc1, const QDomDocument &doc2 );
    static bool compareNodes( const QDomNode &node1, const QDomNode &node2, bool deep );
    static QDomNode findDomNode( const QDomDocument &doc, const QList<QVariant> &pathToNode );
    static QString onNullWarning(const char *const functionName);
    static bool isDeepEqual(const QDomNode &n1, const QDomNode &n2);
    static bool isFakeXMLDeclaration(const QDomNode &node);

    QList<QByteArray> m_excludedCodecs;
};

Q_DECLARE_METATYPE(QList<QVariant>)

void tst_QDom::setContent_data()
{
    const QString doc01(
        "<!DOCTYPE a1 [ <!ENTITY blubber 'and'> ]>\n"
        "<a1>\n"
        " <b1>\n"
        "  <c1>foo</c1>\n"
        "  <c2>bar</c2>\n"
        "  <c3>foo &amp; bar</c3>\n"
        "  <c4>foo &blubber; bar</c4>\n"
        " </b1>\n"
        " <b2> </b2>\n"
        " <b3>\n"
        "  <c1/>\n"
        " </b3>\n"
        "</a1>\n"
        );

    QTest::addColumn<QString>("doc");
    QTest::addColumn<QStringList>("featuresTrue");
    QTest::addColumn<QStringList>("featuresFalse");
    QTest::addColumn<QString>("res");

/*    QTest::newRow( "01" ) << doc01
                       << QStringList()
                       << QString("http://trolltech.com/xml/features/report-whitespace-only-CharData").split(' ')
                       << QString("<!DOCTYPE a1>\n"
                                   "<a1>\n"
                                   "    <b1>\n"
                                   "        <c1>foo</c1>\n"
                                   "        <c2>bar</c2>\n"
                                   "        <c3>foo &amp; bar</c3>\n"
                                   "        <c4>foo and bar</c4>\n"
                                   "    </b1>\n"
                                   "    <b2/>\n"
                                   "    <b3>\n"
                                   "        <c1/>\n"
                                   "    </b3>\n"
                                   "</a1>\n");

    QTest::newRow( "02" ) << doc01
                       << QString("http://trolltech.com/xml/features/report-whitespace-only-CharData").split(' ')
                       << QStringList()
                       << QString("<!DOCTYPE a1>\n"
                                   "<a1>\n"
                                   " <b1>\n"
                                   "  <c1>foo</c1>\n"
                                   "  <c2>bar</c2>\n"
                                   "  <c3>foo &amp; bar</c3>\n"
                                   "  <c4>foo and bar</c4>\n"
                                   " </b1>\n"
                                   " <b2> </b2>\n"
                                   " <b3>\n"
                                   "  <c1/>\n"
                                   " </b3>\n"
                                   "</a1>\n");

    QTest::newRow( "03" ) << doc01
                       << QString("http://trolltech.com/xml/features/report-start-end-entity").split(' ')
                       << QString("http://trolltech.com/xml/features/report-whitespace-only-CharData").split(' ')
                       << QString("<!DOCTYPE a1 [\n"
                                   "<!ENTITY blubber \"and\">\n"
                                   "]>\n"
                                   "<a1>\n"
                                   "    <b1>\n"
                                   "        <c1>foo</c1>\n"
                                   "        <c2>bar</c2>\n"
                                   "        <c3>foo &amp; bar</c3>\n"
                                   "        <c4>foo &blubber; bar</c4>\n"
                                   "    </b1>\n"
                                   "    <b2/>\n"
                                   "    <b3>\n"
                                   "        <c1/>\n"
                                   "    </b3>\n"
                                   "</a1>\n");

    QTest::newRow( "04" ) << doc01
                       << QString("http://trolltech.com/xml/features/report-whitespace-only-CharData http://trolltech.com/xml/features/report-start-end-entity").split(' ')
                       << QStringList()
                       << QString("<!DOCTYPE a1 [\n"
                                   "<!ENTITY blubber \"and\">\n"
                                   "]>\n"
                                   "<a1>\n"
                                   " <b1>\n"
                                   "  <c1>foo</c1>\n"
                                   "  <c2>bar</c2>\n"
                                   "  <c3>foo &amp; bar</c3>\n"
                                   "  <c4>foo &blubber; bar</c4>\n"
                                   " </b1>\n"
                                   " <b2> </b2>\n"
                                   " <b3>\n"
                                   "  <c1/>\n"
                                   " </b3>\n"
                                   "</a1>\n");

  */   QTest::newRow("05") << QString("<message>\n"
                                "    <body>&lt;b&gt;foo&lt;/b&gt;>]]&gt;</body>\n"
                                "</message>\n")
                     << QStringList() << QStringList()
                     << QString("<message>\n"
                                "    <body>&lt;b>foo&lt;/b>>]]&gt;</body>\n"
                                "</message>\n");

}

void tst_QDom::setContent()
{
    QFETCH( QString, doc );

    QXmlInputSource source;
    source.setData( doc );

    QFETCH( QStringList, featuresTrue );
    QFETCH( QStringList, featuresFalse );
    QXmlSimpleReader reader;
    QStringList::Iterator it;
    for ( it = featuresTrue.begin(); it != featuresTrue.end(); ++it ) {
        QVERIFY( reader.hasFeature( *it ) );
        reader.setFeature( *it, TRUE );
    }
    for ( it = featuresFalse.begin(); it != featuresFalse.end(); ++it ) {
        QVERIFY( reader.hasFeature( *it ) );
        reader.setFeature( *it, FALSE );
    }

    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( &source, &reader ) );

    QString eRes;
    QTextStream ts( &eRes, QIODevice::WriteOnly );
    domDoc.save( ts, 4 );

    QTEST( eRes, "res" );

    // make sure that if we parse our output again, we get the same document
    QDomDocument domDoc1;
    QDomDocument domDoc2;
    QVERIFY( domDoc1.setContent( doc ) );
    QVERIFY( domDoc2.setContent( eRes ) );
    QVERIFY( compareDocuments( domDoc1, domDoc2 ) );
}

void tst_QDom::appendChild()
{
    QDomImplementation impl;
    const QDomDocumentType type(impl.createDocumentType("Name", QString(), "bla.dtd"));
    QDomDocument doc(impl.createDocument("ns", "Name", type));

    const QDomElement root(doc.createElement("MyMl"));
    const QDomNode result(doc.appendChild(root));

    QVERIFY(result.isNull());
    QCOMPARE(int(doc.childNodes().length()), 1);
}

void tst_QDom::toString_01_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow( "01" ) << QString("testdata/toString_01/doc01.xml");
    QTest::newRow( "02" ) << QString("testdata/toString_01/doc02.xml");
    QTest::newRow( "03" ) << QString("testdata/toString_01/doc03.xml");
    QTest::newRow( "04" ) << QString("testdata/toString_01/doc04.xml");
    QTest::newRow( "05" ) << QString("testdata/toString_01/doc05.xml");

    QTest::newRow( "euc-jp" ) << QString("testdata/toString_01/doc_euc-jp.xml");
    QTest::newRow( "iso-2022-jp" ) << QString("testdata/toString_01/doc_iso-2022-jp.xml");
    QTest::newRow( "little-endian" ) << QString("testdata/toString_01/doc_little-endian.xml");
    QTest::newRow( "utf-16" ) << QString("testdata/toString_01/doc_utf-16.xml");
    QTest::newRow( "utf-8" ) << QString("testdata/toString_01/doc_utf-8.xml");

}

/*! \internal

  This function tests that the QDomDocument::toString() function results in the
  same XML document. The meaning of "same" in this context means that the
  "information" in the resulting XML file is the same as in the original, i.e.
  we are not intrested in different formatting, etc.

  To achieve this, the XML document of the toString() function is parsed again
  and the two QDomDocuments are compared.
*/
void tst_QDom::toString_01()
{
    QFETCH( QString, fileName );

    QFile f(fileName);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorCol;

    QVERIFY(doc.setContent( &f, &errorMsg, &errorLine, &errorCol )); /*,
        QString("QDomDocument::setContent() failed: %1 in line %2, column %3")
                        .arg( errorMsg ).arg( errorLine ).arg( errorCol )); */
    // test toString()'s invariant with different indenting depths
    for ( int i=0; i<5; i++ ) {
        QString toStr = doc.toString( i );

        QDomDocument res;
        QVERIFY( res.setContent( toStr ) );

        QVERIFY( compareDocuments( doc, res ) );
    }
}

void tst_QDom::toString_02_data()
{
    save_data();
}

/*
  Tests the new QDomDocument::toString(int) overload (basically the same test
  as save()).
*/
void tst_QDom::toString_02()
{
    QFETCH( QString, doc );
    QFETCH( int, indent );

    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );
    QTEST( domDoc.toString(indent), "res" );
}


void tst_QDom::hasAttributes_data()
{
    QTest::addColumn<int>("visitedNodes");
    QTest::addColumn<QByteArray>("xmlDoc");

    QByteArray doc1("<top>Make a <blubb>stupid</blubb>, useless test sentence.</top>");
    QByteArray doc2("<top a=\"a\">Make a <blubb a=\"a\">stupid</blubb>, useless test sentence.</top>");
    QByteArray doc3("<!-- just a useless comment -->\n"
                    "<?pi foo bar?>\n"
                    "<foo>\n"
                    "<bar fnord=\"snafu\" hmpf=\"grmpf\">\n"
                    "<foobar/>\n"
                    "</bar>\n"
                    "<bar>blubber</bar>\n"
                    "more text, pretty unintresting, though\n"
                    "<hmpfl blubber=\"something\" />\n"
                    "<![CDATA[ foo bar @!<>] ]]>\n"
                    "</foo>\n"
                    "<!-- just a useless comment -->\n"
                    "<?pi foo bar?>\n");

    QTest::newRow( "01" ) << 6 << doc1;
    QTest::newRow( "02" ) << 6 << doc2;
    QTest::newRow( "03" ) << 13 << doc3;
}

/*
  This function tests that QDomNode::hasAttributes() returns TRUE if and only
  if the node has attributes (i.e. QDomNode::attributes() returns a list with
  attributes in it).
*/
void tst_QDom::hasAttributes()
{
    QFETCH( QByteArray, xmlDoc );

    QDomDocument doc;
    QVERIFY( doc.setContent( xmlDoc ) );

    int visitedNodes = hasAttributesHelper( doc );
    QTEST( visitedNodes, "visitedNodes" );
}

int tst_QDom::hasAttributesHelper( const QDomNode& node )
{
    int visitedNodes = 1;
    if ( node.hasAttributes() ) {
            if (node.attributes().count() == 0)
            return -1;
//        QVERIFY( node.attributes().count() > 0 );
    } else {
            if (node.attributes().count() != 0)
            return -1;
//        QVERIFY( node.attributes().count() == 0 );
    }

    QDomNodeList children = node.childNodes();
    for ( int i=0; i<children.count(); i++ ) {
            int j = hasAttributesHelper( children.item(i) );
        if (j < 0)
            return -1;
        visitedNodes += j;
    }
    return visitedNodes;
}


void tst_QDom::save_data()
{
    const QString doc01(
            "<a1>\n"
            " <b1>\n"
            "  <c1>\n"
            "   <d1/>\n"
            "  </c1>\n"
            "  <c2/>\n"
            " </b1>\n"
            " <b2/>\n"
            " <b3>\n"
            "  <c1/>\n"
            " </b3>\n"
            "</a1>\n"
            );

    QTest::addColumn<QString>("doc");
    QTest::addColumn<int>("indent");
    QTest::addColumn<QString>("res");

    QTest::newRow( "01" ) << doc01 << 0 << QString(doc01).replace( QRegExp(" "), "" );
    QTest::newRow( "02" ) << doc01 << 1 << doc01;
    QTest::newRow( "03" ) << doc01 << 2 << QString(doc01).replace( QRegExp(" "), "  " );
    QTest::newRow( "04" ) << doc01 << 10 << QString(doc01).replace( QRegExp(" "), "          " );
}

void tst_QDom::save()
{
    QFETCH( QString, doc );
    QFETCH( int, indent );

    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );

    QString eRes;
    QTextStream ts( &eRes, QIODevice::WriteOnly );
    domDoc.save( ts, indent );

    QTEST( eRes, "res" );
}

void tst_QDom::initTestCase()
{
    QFile file("testdata/excludedCodecs.txt");
    QVERIFY(file.open(QIODevice::ReadOnly));

    QByteArray codecName;

    m_excludedCodecs = file.readAll().split('\n');
}

void tst_QDom::saveWithSerialization() const
{
    QFETCH(QString, fileName);

    QFile f(fileName);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QDomDocument doc;

    // Read the document
    QVERIFY(doc.setContent(&f));

    const QList<QByteArray> codecs(QTextCodec::availableCodecs());
    QByteArray codec;

    foreach(codec, codecs) {

        /* Avoid codecs that can't handle the files we have. */
        if(m_excludedCodecs.contains(codec))
            continue;

        /* Write out doc in the specified codec. */
        QByteArray storage;
        QBuffer writeDevice(&storage);
        QVERIFY(writeDevice.open(QIODevice::WriteOnly));

        QTextStream s(&writeDevice);
        s.setCodec(QTextCodec::codecForName(codec));

        doc.save(s, 0, QDomNode::EncodingFromTextStream);
        s.flush();
        writeDevice.close();

        QBuffer readDevice(&storage);
        QVERIFY(readDevice.open(QIODevice::ReadOnly));

        QDomDocument result;

        QString msg;
        int line = 0;
        int column = 0;

        QVERIFY2(result.setContent(&readDevice, &msg, &line, &column),
                 qPrintable(QString::fromLatin1("Failed for codec %1: line %2, column %3: %4")
                                                .arg(codec.constData())
                                                .arg(line)
                                                .arg(column)
                                                .arg(msg)));
        QVERIFY2(compareDocuments(doc, result),
                 qPrintable(QString::fromLatin1("Failed for codec %1").arg(codec.constData())));
    }
}

void tst_QDom::saveWithSerialization_data() const
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("doc01.xml") << QString("testdata/toString_01/doc01.xml");
    QTest::newRow("doc01.xml") << QString("testdata/toString_01/doc01.xml");
    QTest::newRow("doc02.xml") << QString("testdata/toString_01/doc02.xml");
    QTest::newRow("doc03.xml") << QString("testdata/toString_01/doc03.xml");
    QTest::newRow("doc04.xml") << QString("testdata/toString_01/doc04.xml");
    QTest::newRow("doc05.xml") << QString("testdata/toString_01/doc05.xml");

    QTest::newRow("doc_euc-jp.xml") << QString("testdata/toString_01/doc_euc-jp.xml");
    QTest::newRow("doc_iso-2022-jp.xml") << QString("testdata/toString_01/doc_iso-2022-jp.xml");
    QTest::newRow("doc_little-endian.xml") << QString("testdata/toString_01/doc_little-endian.xml");
    QTest::newRow("doc_utf-16.xml") << QString("testdata/toString_01/doc_utf-16.xml");
    QTest::newRow("doc_utf-8.xml") << QString("testdata/toString_01/doc_utf-8.xml");
}

void tst_QDom::cloneNode_data()
{
    const QString doc01(
            "<a1>\n"
            " <b1>\n"
            "  <c1>\n"
            "   <d1/>\n"
            "  </c1>\n"
            "  <c2/>\n"
            " </b1>\n"
            " <b2/>\n"
            " <b3>\n"
            "  <c1/>\n"
            " </b3>\n"
            "</a1>\n"
            );
    QList<QVariant> nodeB1;
    nodeB1 << 0;

    QList<QVariant> nodeC1;
    nodeC1 << 0 << 0;

    QList<QVariant> nodeC2;
    nodeC2 << 0 << 1;

    QTest::addColumn<QString>("doc");
    QTest::addColumn<QList<QVariant> >("pathToNode");
    QTest::addColumn<bool>("deep");

    QTest::newRow( "noDeep_01" ) << doc01 << nodeB1 << (bool)FALSE;
    QTest::newRow( "noDeep_02" ) << doc01 << nodeC1 << (bool)FALSE;
    QTest::newRow( "noDeep_03" ) << doc01 << nodeC2 << (bool)FALSE;

    QTest::newRow( "deep_01" ) << doc01 << nodeB1 << (bool)TRUE;
    QTest::newRow( "deep_02" ) << doc01 << nodeC1 << (bool)TRUE;
    QTest::newRow( "deep_03" ) << doc01 << nodeC2 << (bool)TRUE;
}

void tst_QDom::cloneNode()
{
    QFETCH( QString, doc );
    QFETCH( QList<QVariant>, pathToNode );
    QFETCH( bool, deep );
    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );
    QDomNode node = findDomNode( domDoc, pathToNode );
    QVERIFY(!node.isNull());

    QDomNode clonedNode = node.cloneNode( deep );
    QVERIFY( compareNodes( node, clonedNode, deep ) );

    QDomNode parent = node.parentNode();
    if ( !parent.isNull() ) {
        node = parent.replaceChild( clonedNode, node ); // swap the nodes
        QVERIFY( !node.isNull() );
        QVERIFY( compareNodes( node, clonedNode, deep ) );
    }
}


void tst_QDom::ownerElementTask45192_data()
{
    const QString doc(
        "<root>\n"
        " <item name=\"test\" >\n"
        " </item>\n"
        "</root>"
    );

    QTest::addColumn<QString>("doc");
    QTest::newRow("doc") << doc;
}

void tst_QDom::ownerElementTask45192()
{
    QFETCH( QString, doc );
    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );

    QDomNode item = domDoc.documentElement().firstChild();
    QDomNode clone = item.cloneNode(false);

    QVERIFY( clone == clone.attributes().namedItem("name").toAttr().ownerElement() );
}

void tst_QDom::ownerDocument_data()
{
    cloneNode_data();
}

#define OWNERDOCUMENT_CREATE_TEST( t, x ) \
{ \
    t n = x; \
    QVERIFY( n.ownerDocument() == domDoc ); \
}

#define OWNERDOCUMENT_IMPORTNODE_TEST( t, x ) \
{ \
    QDomNode importedNode; \
    t n = x; \
    QVERIFY( n.ownerDocument() != domDoc ); \
    importedNode = domDoc.importNode( n, deep ); \
    QVERIFY( n.ownerDocument() != domDoc ); \
    QVERIFY( importedNode.ownerDocument() == domDoc ); \
}

void tst_QDom::ownerDocument()
{
    QFETCH( QString, doc );
    QFETCH( QList<QVariant>, pathToNode );
    QFETCH( bool, deep );
    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );
    QDomNode node = findDomNode( domDoc, pathToNode );
    QVERIFY(!node.isNull());

    QVERIFY( node.ownerDocument() == domDoc );

    // Does cloneNode() keep the ownerDocument()?
    {
        QDomNode clonedNode = node.cloneNode( deep );
        QVERIFY( node.ownerDocument() == domDoc );
        QVERIFY( clonedNode.ownerDocument() == domDoc );
    }

    // If the original DOM node is replaced with the cloned node, does this
    // keep the ownerDocument()?
    {
        QDomNode clonedNode = node.cloneNode( deep );
        QDomNode parent = node.parentNode();
        if ( !parent.isNull() ) {
            node = parent.replaceChild( clonedNode, node ); // swap the nodes
            QVERIFY( node.ownerDocument() == domDoc );
            QVERIFY( clonedNode.ownerDocument() == domDoc );
        }
    }

    // test QDomDocument::create...()
    {
        OWNERDOCUMENT_CREATE_TEST( QDomAttr,                    domDoc.createAttribute( "foo" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomAttr,                    domDoc.createAttributeNS( "foo", "bar" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomCDATASection,            domDoc.createCDATASection( "foo" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomComment,                 domDoc.createComment( "foo" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomDocumentFragment,        domDoc.createDocumentFragment() );
        OWNERDOCUMENT_CREATE_TEST( QDomElement,                 domDoc.createElement( "foo" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomElement,                 domDoc.createElementNS( "foo", "bar" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomEntityReference,         domDoc.createEntityReference( "foo" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomProcessingInstruction,   domDoc.createProcessingInstruction( "foo", "bar" ) );
        OWNERDOCUMENT_CREATE_TEST( QDomText,                    domDoc.createTextNode( "foo" ) );
    }

    // test importNode()
    {
        QDomDocument doc2;
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomAttr,                    doc2.createAttribute( "foo" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomAttr,                    doc2.createAttributeNS( "foo", "bar" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomCDATASection,            doc2.createCDATASection( "foo" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomComment,                 doc2.createComment( "foo" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomDocumentFragment,        doc2.createDocumentFragment() );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomElement,                 doc2.createElement( "foo" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomElement,                 doc2.createElementNS( "foo", "bar" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomEntityReference,         doc2.createEntityReference( "foo" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomProcessingInstruction,   doc2.createProcessingInstruction( "foo", "bar" ) );
        OWNERDOCUMENT_IMPORTNODE_TEST( QDomText,                    doc2.createTextNode( "foo" ) );
    }
}

void tst_QDom::ownerDocumentTask27424_data()
{
    QTest::addColumn<bool>("insertLevel1AfterCstr");
    QTest::addColumn<bool>("insertLevel2AfterCstr");
    QTest::addColumn<bool>("insertLevel3AfterCstr");

    QTest::newRow( "000" ) << (bool)FALSE << (bool)FALSE << (bool)FALSE;
    QTest::newRow( "001" ) << (bool)FALSE << (bool)FALSE << (bool)TRUE;
    QTest::newRow( "010" ) << (bool)FALSE << (bool)TRUE  << (bool)FALSE;
    QTest::newRow( "011" ) << (bool)FALSE << (bool)TRUE  << (bool)TRUE;
    QTest::newRow( "100" ) << (bool)TRUE  << (bool)FALSE << (bool)FALSE;
    QTest::newRow( "101" ) << (bool)TRUE  << (bool)FALSE << (bool)TRUE;
    QTest::newRow( "110" ) << (bool)TRUE  << (bool)TRUE  << (bool)FALSE;
    QTest::newRow( "111" ) << (bool)TRUE  << (bool)TRUE  << (bool)TRUE;
}

void tst_QDom::ownerDocumentTask27424()
{
    QFETCH( bool, insertLevel1AfterCstr );
    QFETCH( bool, insertLevel2AfterCstr );
    QFETCH( bool, insertLevel3AfterCstr );

    QDomDocument doc("TestXML");

    QDomElement level1 = doc.createElement("Level_1");
    QVERIFY( level1.ownerDocument() == doc );

    if ( insertLevel1AfterCstr ) {
        doc.appendChild(level1);
        QVERIFY( level1.ownerDocument() == doc );
    }

    QDomElement level2 = level1.ownerDocument().createElement("Level_2");
    QVERIFY( level1.ownerDocument() == doc );
    QVERIFY( level2.ownerDocument() == doc );

    if ( insertLevel2AfterCstr ) {
        level1.appendChild(level2);
        QVERIFY( level1.ownerDocument() == doc );
        QVERIFY( level2.ownerDocument() == doc );
    }

    QDomElement level3 = level2.ownerDocument().createElement("Level_3");
    QVERIFY( level1.ownerDocument() == doc );
    QVERIFY( level2.ownerDocument() == doc );
    QVERIFY( level3.ownerDocument() == doc );

    if ( insertLevel3AfterCstr ) {
        level2.appendChild(level3);
        QVERIFY( level1.ownerDocument() == doc );
        QVERIFY( level2.ownerDocument() == doc );
        QVERIFY( level3.ownerDocument() == doc );
    }

    QDomNode level4 = level3.ownerDocument().createTextNode("This_is_a_value!");
    QVERIFY( level4.ownerDocument() == doc );

    level3.appendChild(level4);
    QVERIFY( level1.ownerDocument() == doc );
    QVERIFY( level2.ownerDocument() == doc );
    QVERIFY( level3.ownerDocument() == doc );
    QVERIFY( level4.ownerDocument() == doc );

    if ( !insertLevel3AfterCstr ) {
        level2.appendChild(level3);
        QVERIFY( level1.ownerDocument() == doc );
        QVERIFY( level2.ownerDocument() == doc );
        QVERIFY( level3.ownerDocument() == doc );
        QVERIFY( level4.ownerDocument() == doc );
    }

    if ( !insertLevel2AfterCstr ) {
        level1.appendChild(level2);
        QVERIFY( level1.ownerDocument() == doc );
        QVERIFY( level2.ownerDocument() == doc );
        QVERIFY( level3.ownerDocument() == doc );
        QVERIFY( level4.ownerDocument() == doc );
    }

    if ( !insertLevel1AfterCstr ) {
        doc.appendChild(level1);
        QVERIFY( level1.ownerDocument() == doc );
        QVERIFY( level2.ownerDocument() == doc );
        QVERIFY( level3.ownerDocument() == doc );
        QVERIFY( level4.ownerDocument() == doc );
    }
}

void tst_QDom::parentNode_data()
{
    cloneNode_data();
}

#define PARENTNODE_CREATE_TEST( t, x ) \
{ \
    t n = x; \
    QVERIFY( n.parentNode().isNull() ); \
}

void tst_QDom::parentNode()
{
    QFETCH( QString, doc );
    QFETCH( QList<QVariant>, pathToNode );
    QFETCH( bool, deep );
    Q_UNUSED( deep );
    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );
    QDomNode node = findDomNode( domDoc, pathToNode );
    QVERIFY(!node.isNull());

    // test QDomDocument::create...()
    {
        PARENTNODE_CREATE_TEST( QDomAttr,                   domDoc.createAttribute( "foo" ) );
        PARENTNODE_CREATE_TEST( QDomAttr,                   domDoc.createAttributeNS( "foo", "bar" ) );
        PARENTNODE_CREATE_TEST( QDomCDATASection,           domDoc.createCDATASection( "foo" ) );
        PARENTNODE_CREATE_TEST( QDomComment,                domDoc.createComment( "foo" ) );
        PARENTNODE_CREATE_TEST( QDomDocumentFragment,       domDoc.createDocumentFragment() );
        PARENTNODE_CREATE_TEST( QDomElement,                domDoc.createElement( "foo" ) );
        PARENTNODE_CREATE_TEST( QDomElement,                domDoc.createElementNS( "foo", "bar" ) );
        PARENTNODE_CREATE_TEST( QDomEntityReference,        domDoc.createEntityReference( "foo" ) );
        PARENTNODE_CREATE_TEST( QDomProcessingInstruction,  domDoc.createProcessingInstruction( "foo", "bar" ) );
        PARENTNODE_CREATE_TEST( QDomText,                   domDoc.createTextNode( "foo" ) );
    }
}


void tst_QDom::documentCreationTask27424_data()
{
    QTest::addColumn<bool>("insertLevel1AfterCstr");
    QTest::addColumn<bool>("insertLevel2AfterCstr");
    QTest::addColumn<bool>("insertLevel3AfterCstr");

    QTest::newRow( "000" ) << (bool)FALSE << (bool)FALSE << (bool)FALSE;
    QTest::newRow( "001" ) << (bool)FALSE << (bool)FALSE << (bool)TRUE;
    QTest::newRow( "010" ) << (bool)FALSE << (bool)TRUE  << (bool)FALSE;
    QTest::newRow( "011" ) << (bool)FALSE << (bool)TRUE  << (bool)TRUE;
    QTest::newRow( "100" ) << (bool)TRUE  << (bool)FALSE << (bool)FALSE;
    QTest::newRow( "101" ) << (bool)TRUE  << (bool)FALSE << (bool)TRUE;
    QTest::newRow( "110" ) << (bool)TRUE  << (bool)TRUE  << (bool)FALSE;
    QTest::newRow( "111" ) << (bool)TRUE  << (bool)TRUE  << (bool)TRUE;
}

void tst_QDom::documentCreationTask27424()
{
    QFETCH( bool, insertLevel1AfterCstr );
    QFETCH( bool, insertLevel2AfterCstr );
    QFETCH( bool, insertLevel3AfterCstr );

    QDomDocument docRes;
    QVERIFY( docRes.setContent( QString(
                "<!DOCTYPE TestXML>\n"
                "<Level_1>\n"
                " <Level_2>\n"
                "  <Level_3>This_is_a_value!</Level_3>\n"
                " </Level_2>\n"
                "</Level_1>"
                ) ) );

    QDomDocument doc("TestXML");

    QDomElement level1 = doc.createElement("Level_1");
    if ( insertLevel1AfterCstr )
        doc.appendChild(level1);

    QDomElement level2 = level1.ownerDocument().createElement("Level_2");
    if ( insertLevel2AfterCstr )
        level1.appendChild(level2);

    QDomElement level3 = level2.ownerDocument().createElement("Level_3");
    if ( insertLevel3AfterCstr )
        level2.appendChild(level3);

    QDomNode level4 = level3.ownerDocument().createTextNode("This_is_a_value!");
    level3.appendChild(level4);

    if ( !insertLevel3AfterCstr )
        level2.appendChild(level3);
    if ( !insertLevel2AfterCstr )
        level1.appendChild(level2);
    if ( !insertLevel1AfterCstr )
        doc.appendChild(level1);

    QVERIFY( compareDocuments( doc, docRes ) );
}


bool tst_QDom::isFakeXMLDeclaration(const QDomNode &node)
{
    return node.isProcessingInstruction() &&
           node.nodeName() == QLatin1String("xml");
}

bool tst_QDom::isDeepEqual(const QDomNode &n1, const QDomNode &n2)
{
    const QDomNode::NodeType nt = n1.nodeType();

    if(nt != n2.nodeType())
        return false;

    if(n1.nodeName() != n2.nodeName()
       || n1.namespaceURI() != n2.namespaceURI()
       || n1.nodeValue() != n2.nodeValue())
        return false;

    /* Check the children. */
    const QDomNodeList children1(n1.childNodes());
    const QDomNodeList children2(n2.childNodes());
    uint len1 = children1.length();
    uint len2 = children2.length();
    uint i1 = 0;
    uint i2 = 0;

    if(len1 != 0 && isFakeXMLDeclaration(children1.at(0)))
            ++i1;

    if(len2 != 0 && isFakeXMLDeclaration(children2.at(0)))
            ++i2;

    if(len1 - i1 != len2 - i2)
        return false;

    // We jump over the first to skip the processing instructions that
    // are (incorrectly) used as XML declarations.
    for(; i1 < len1; ++i1)
    {
        if(!isDeepEqual(children1.at(i1), children2.at(i2)))
            return false;

        ++i2;
    }

    return true;
}

/*
    Returns TRUE if \a doc1 and \a doc2 represent the same XML document, i.e.
    they have the same informational content. Otherwise, this function returns
    FALSE.
*/
bool tst_QDom::compareDocuments( const QDomDocument &doc1, const QDomDocument &doc2 )
{
    return isDeepEqual(doc1, doc2);
}

/*
    Returns TRUE if \a node1 and \a node2 represent the same XML node, i.e.
    they have the same informational content. Otherwise, this function returns
    FALSE.

    If \a deep is TRUE, children of the nodes are also tested. If \a deep is
    FALSE, only \a node1 and \a node 2 are compared.
*/
bool tst_QDom::compareNodes( const QDomNode &node1, const QDomNode &node2, bool deep )
{
    if ( deep ) {
        QString str1;
        {
            QTextOStream stream( &str1 );
            stream << node1;
        }
        QString str2;
        {
            QTextOStream stream( &str2 );
            stream << node2;
        }
        return str1 == str2;
    }

    if ( node1.isNull() && node2.isNull() )
        return TRUE;
    // ### I am not sure if this test is complete
    bool equal =     node1.nodeName() == node2.nodeName();
    equal = equal && node1.nodeType() == node2.nodeType();
    equal = equal && node1.localName() == node2.localName();
    equal = equal && node1.nodeValue() == node2.nodeValue();
    equal = equal && node1.prefix() == node2.prefix();

    return equal;
}

/*
    \a pathToNode is a list of indices to wanted node in \a doc. Returns the
    wanted node.
*/
QDomNode tst_QDom::findDomNode( const QDomDocument &doc, const QList<QVariant> &pathToNode )
{
    QDomNode node = doc;
    QList<QVariant>::const_iterator it;
    for ( it = pathToNode.begin(); it != pathToNode.end(); ++it ) {
        QDomNodeList children = node.childNodes();
        node = children.item( (*it).toInt() );
//        QVERIFY( !node.isNull() );
    }
    return node;
}

void tst_QDom::browseElements()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("foo");
    doc.appendChild(root);
    root.appendChild(doc.createElement("bar"));
    root.appendChild(doc.createElement("bop"));
    root.appendChild(doc.createElement("bar"));
    root.appendChild(doc.createElement("bop"));

    QVERIFY(doc.firstChildElement("ding").isNull());
    QDomElement foo = doc.firstChildElement("foo");
    QVERIFY(!foo.isNull());
    QVERIFY(foo.firstChildElement("ding").isNull());
    QVERIFY(foo.nextSiblingElement("foo").isNull());
    QVERIFY(foo.previousSiblingElement("bar").isNull());
    QVERIFY(foo.nextSiblingElement().isNull());
    QVERIFY(foo.previousSiblingElement().isNull());

    QDomElement bar = foo.firstChildElement("bar");
    QVERIFY(!bar.isNull());
    QVERIFY(bar.previousSiblingElement("bar").isNull());
    QVERIFY(bar.previousSiblingElement().isNull());
    QVERIFY(bar.nextSiblingElement("bar").tagName() == "bar");
    QVERIFY(bar.nextSiblingElement("bar").nextSiblingElement("bar").isNull());

    QDomElement bop = foo.firstChildElement("bop");
    QVERIFY(!bop.isNull());
    QVERIFY(bar.nextSiblingElement() == bop);
    QVERIFY(bop.nextSiblingElement("bop") == foo.lastChildElement("bop"));
    QVERIFY(bop.previousSiblingElement("bar") == foo.firstChildElement("bar"));
    QVERIFY(bop.previousSiblingElement("bar") == foo.firstChildElement());
}

void tst_QDom::domNodeMapAndList()
{
    QString xml_str = QString::fromLatin1("<foo ding='dong'></foo>");

    QDomDocument doc;
    QVERIFY(doc.setContent(xml_str));

    QDomNamedNodeMap map = doc.documentElement().attributes();
    QCOMPARE(map.item(0).nodeName(), QString("ding"));

    QTest::ignoreMessage(QtDebugMsg, "Function nodeName() does nothing on a null "
                                     "node(see QDomNode::isNull()). Create non-null "
                                     "nodes with the factory functions such as "
                                     "QDomDocument::createElement().");
    QCOMPARE(map.item(1).nodeName(), QString()); // Make sure we don't assert

    QDomNodeList list = doc.elementsByTagName("foo");
    QCOMPARE(list.item(0).nodeName(), QString("foo"));

    QTest::ignoreMessage(QtDebugMsg, "Function nodeName() does nothing on a null "
                                     "node(see QDomNode::isNull()). Create non-null "
                                     "nodes with the factory functions such as "
                                     "QDomDocument::createElement().");
    QCOMPARE(list.item(1).nodeName(), QString()); // Make sure we don't assert
}

// Verifies that a default-constructed QDomDocument is null, and that calling
// any of the factory functions causes it to be non-null.
#define TEST_NULL_DOCUMENT(func) \
{ \
    QDomDocument doc; \
    QVERIFY(doc.isNull()); \
    QVERIFY(!doc.func.isNull()); \
    QVERIFY(!doc.isNull()); \
}

void tst_QDom::nullDocument()
{
    TEST_NULL_DOCUMENT(createAttribute("foo"))
    TEST_NULL_DOCUMENT(createAttributeNS("http://foo/", "bar"))
    TEST_NULL_DOCUMENT(createCDATASection("foo"))
    TEST_NULL_DOCUMENT(createComment("foo"))
    TEST_NULL_DOCUMENT(createDocumentFragment())
    TEST_NULL_DOCUMENT(createElement("foo"))
    TEST_NULL_DOCUMENT(createElementNS("http://foo/", "foo"))
    TEST_NULL_DOCUMENT(createEntityReference("foo"))
    TEST_NULL_DOCUMENT(createProcessingInstruction("foo", "bar"))
    TEST_NULL_DOCUMENT(createTextNode("foo"))
    QDomDocument doc2;
    QDomElement elt = doc2.createElement("foo");
    doc2.appendChild(elt);
    TEST_NULL_DOCUMENT(importNode(elt, true))
}

#undef TEST_NULL_DOCUMENT

void tst_QDom::invalidName_data()
{
    QTest::addColumn<QString>("in_name");
    QTest::addColumn<bool>("ok_AcceptInvalidChars");
    QTest::addColumn<bool>("ok_DropInvalidChars");
    QTest::addColumn<bool>("ok_ReturnNullNode");
    QTest::addColumn<QString>("out_name");

    QTest::newRow( "foo" )     << QString("foo")     << true  << true  << true  << QString("foo");
    QTest::newRow( "_f.o-o:" ) << QString("_f.o-o:") << true  << true  << true  << QString("_f.o-o:");
    QTest::newRow( "...:." )   << QString("...:.")   << true  << true  << false << QString(":.");
    QTest::newRow( "empty" )   << QString()          << false << false << false << QString();
    QTest::newRow( "~f~o~o~" ) << QString("~f~o~o~") << true  << true  << false << QString("foo");
    QTest::newRow( "~" )       << QString("~")       << true  << false << false << QString();
    QTest::newRow( "..." )     << QString("...")     << true  << false << false << QString();
}

void tst_QDom::invalidName()
{
    QFETCH( QString, in_name );
    QFETCH( bool, ok_AcceptInvalidChars );
    QFETCH( bool, ok_DropInvalidChars );
    QFETCH( bool, ok_ReturnNullNode );
    QFETCH( QString, out_name );

    QDomImplementation impl;
    QDomDocument doc;

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::AcceptInvalidChars);

    {
        QDomElement elt = doc.createElement(in_name);
        QDomElement elt_ns = doc.createElementNS("foo", "foo:" + in_name);
        QDomAttr attr = doc.createAttribute(in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo",  "foo:" + in_name);
        QDomEntityReference ref = doc.createEntityReference(in_name);

        QCOMPARE(!elt.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!elt_ns.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!attr.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!attr_ns.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!ref.isNull(), ok_AcceptInvalidChars);

        if (ok_AcceptInvalidChars) {
            QCOMPARE(elt.tagName(), in_name);
            QCOMPARE(elt_ns.tagName(), in_name);
            QCOMPARE(attr.name(), in_name);
            QCOMPARE(attr_ns.name(), in_name);
            QCOMPARE(ref.nodeName(), in_name);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::DropInvalidChars);

    {
        QDomElement elt = doc.createElement(in_name);
        QDomElement elt_ns = doc.createElementNS("foo", "foo:" + in_name);
        QDomAttr attr = doc.createAttribute(in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo", "foo:" + in_name);
        QDomEntityReference ref = doc.createEntityReference(in_name);

        QCOMPARE(!elt.isNull(), ok_DropInvalidChars);
        QCOMPARE(!elt_ns.isNull(), ok_DropInvalidChars);
        QCOMPARE(!attr.isNull(), ok_DropInvalidChars);
        QCOMPARE(!attr_ns.isNull(), ok_DropInvalidChars);
        QCOMPARE(!ref.isNull(), ok_DropInvalidChars);

        if (ok_DropInvalidChars) {
            QCOMPARE(elt.tagName(), out_name);
            QCOMPARE(elt_ns.tagName(), out_name);
            QCOMPARE(attr.name(), out_name);
            QCOMPARE(attr_ns.name(), out_name);
            QCOMPARE(ref.nodeName(), out_name);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::ReturnNullNode);

    {
        QDomElement elt = doc.createElement(in_name);
        QDomElement elt_ns = doc.createElementNS("foo", "foo:" + in_name);
        QDomAttr attr = doc.createAttribute(in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo", "foo:" + in_name);
        QDomEntityReference ref = doc.createEntityReference(in_name);

        QCOMPARE(!elt.isNull(), ok_ReturnNullNode);
        QCOMPARE(!elt_ns.isNull(), ok_ReturnNullNode);
        QCOMPARE(!attr.isNull(), ok_ReturnNullNode);
        QCOMPARE(!attr_ns.isNull(), ok_ReturnNullNode);
        QCOMPARE(!ref.isNull(), ok_ReturnNullNode);

        if (ok_ReturnNullNode) {
            QCOMPARE(elt.tagName(), in_name);
            QCOMPARE(elt_ns.tagName(), in_name);
            QCOMPARE(attr.name(), in_name);
            QCOMPARE(attr_ns.name(), in_name);
            QCOMPARE(ref.nodeName(), in_name);
        }
    }
}

void tst_QDom::invalidQualifiedName_data()
{
    QTest::addColumn<QString>("in_name");
    QTest::addColumn<bool>("ok_AcceptInvalidChars");
    QTest::addColumn<bool>("ok_DropInvalidChars");
    QTest::addColumn<bool>("ok_ReturnNullNode");
    QTest::addColumn<QString>("out_name");

    QTest::newRow( "foo" )     << QString("foo")      << true  << true  << true  << QString("foo");
    QTest::newRow( "foo:bar" ) << QString("foo:bar")  << true  << true  << true  << QString("foo:bar");
    QTest::newRow( "bar:" )    << QString("bar:")     << false << false << false << QString();
    QTest::newRow( ":" )       << QString(":")        << false << false << false << QString();
    QTest::newRow( "empty" )   << QString()           << false << false << false << QString();
    QTest::newRow("foo:...:.") << QString("foo:...:.")<< true  << true  << false << QString("foo::.");
    QTest::newRow("foo:~")     << QString("foo:~")    << true  << false << false << QString();
    QTest::newRow("foo:.~")    << QString("foo:.~")   << true  << false << false << QString();
}

void tst_QDom::invalidQualifiedName()
{
    QFETCH( QString, in_name );
    QFETCH( bool, ok_AcceptInvalidChars );
    QFETCH( bool, ok_DropInvalidChars );
    QFETCH( bool, ok_ReturnNullNode );
    QFETCH( QString, out_name );

    QDomImplementation impl;
    QDomDocument doc;

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::AcceptInvalidChars);

    {
        QDomElement elt_ns = doc.createElementNS("foo", in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo", in_name);
        QDomDocumentType doctype = impl.createDocumentType(in_name, "foo", "bar");
        QDomDocument doc2 = impl.createDocument("foo", in_name, doctype);

        QCOMPARE(!elt_ns.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!attr_ns.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!doctype.isNull(), ok_AcceptInvalidChars);
        QCOMPARE(!doc2.isNull(), ok_AcceptInvalidChars);

        if (ok_AcceptInvalidChars) {
            QCOMPARE(elt_ns.nodeName(), in_name);
            QCOMPARE(attr_ns.nodeName(), in_name);
            QCOMPARE(doctype.name(), in_name);
            QCOMPARE(doc2.documentElement().nodeName(), in_name);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::DropInvalidChars);

    {
        QDomElement elt_ns = doc.createElementNS("foo", in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo", in_name);
        QDomDocumentType doctype = impl.createDocumentType(in_name, "foo", "bar");
        QDomDocument doc2 = impl.createDocument("foo", in_name, doctype);

        QCOMPARE(!elt_ns.isNull(), ok_DropInvalidChars);
        QCOMPARE(!attr_ns.isNull(), ok_DropInvalidChars);
        QCOMPARE(!doctype.isNull(), ok_DropInvalidChars);
        QCOMPARE(!doc2.isNull(), ok_DropInvalidChars);

        if (ok_DropInvalidChars) {
            QCOMPARE(elt_ns.nodeName(), out_name);
            QCOMPARE(attr_ns.nodeName(), out_name);
            QCOMPARE(doctype.name(), out_name);
            QCOMPARE(doc2.documentElement().nodeName(), out_name);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::ReturnNullNode);

    {
        QDomElement elt_ns = doc.createElementNS("foo", in_name);
        QDomAttr attr_ns = doc.createAttributeNS("foo", in_name);
        QDomDocumentType doctype = impl.createDocumentType(in_name, "foo", "bar");
        QDomDocument doc2 = impl.createDocument("foo", in_name, doctype);

        QCOMPARE(!elt_ns.isNull(), ok_ReturnNullNode);
        QCOMPARE(!attr_ns.isNull(), ok_ReturnNullNode);
        QCOMPARE(!doctype.isNull(), ok_ReturnNullNode);
        QCOMPARE(!doc2.isNull(), ok_ReturnNullNode);

        if (ok_ReturnNullNode) {
            QCOMPARE(elt_ns.nodeName(), in_name);
            QCOMPARE(attr_ns.nodeName(), in_name);
            QCOMPARE(doctype.name(), in_name);
            QCOMPARE(doc2.documentElement().nodeName(), in_name);
        }
    }
}

void tst_QDom::invalidCharData_data()
{
    QTest::addColumn<QString>("in_text");
    QTest::addColumn<bool>("ok_AcceptInvalidChars");
    QTest::addColumn<bool>("ok_DropInvalidChars");
    QTest::addColumn<bool>("ok_ReturnNullNode");
    QTest::addColumn<QString>("out_text");

    QTest::newRow( "foo" )     << QString("foo")       << true  << true  << true  << QString("foo");
    QTest::newRow( "f<o&o" )   << QString("f<o&o")     << true  << true  << true  << QString("f<o&o");
    QTest::newRow( "empty" )   << QString()            << true  << true  << true  << QString();
    QTest::newRow("f\\x07o\\x02")<< QString("f\x07o\x02")<< true  << true  << false << QString("fo");
}

void tst_QDom::invalidCharData()
{
    QFETCH( QString, in_text );
    QFETCH( bool, ok_AcceptInvalidChars );
    QFETCH( bool, ok_DropInvalidChars );
    QFETCH( bool, ok_ReturnNullNode );
    QFETCH( QString, out_text );

    QDomDocument doc;

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::AcceptInvalidChars);

    {
        QDomText text_elt = doc.createTextNode(in_text);
        QCOMPARE(!text_elt.isNull(), ok_AcceptInvalidChars);
        if (ok_AcceptInvalidChars) {
            QCOMPARE(text_elt.nodeValue(), in_text);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::DropInvalidChars);

    {
        QDomText text_elt = doc.createTextNode(in_text);
        QCOMPARE(!text_elt.isNull(), ok_DropInvalidChars);
        if (ok_DropInvalidChars) {
            QCOMPARE(text_elt.nodeValue(), out_text);
        }
    }

    QDomImplementation::setInvalidDataPolicy(QDomImplementation::ReturnNullNode);

    {
        QDomText text_elt = doc.createTextNode(in_text);
        QCOMPARE(!text_elt.isNull(), ok_ReturnNullNode);
        if (ok_ReturnNullNode) {
            QCOMPARE(text_elt.nodeValue(), in_text);
        }
    }
}

static const char *s_onNullMessage = 0;
static QtMsgType s_onNullMessageType = QtSystemMsg;

static void myOnNullMsgHandler(QtMsgType type, const char *msg)
{
    s_onNullMessage = msg;
    s_onNullMessageType = type;
}

QString tst_QDom::onNullWarning(const char *const functionName)
{
        return QString::fromLatin1("Function %1() does nothing on a null node(see "
                                   "QDomNode::isNull()). Create non-null nodes with the factory functions "
                                   "such as QDomDocument::createElement().").arg(QString::fromLatin1(functionName));
}

void tst_QDom::checkWarningOnNull() const
{
    qInstallMsgHandler(myOnNullMsgHandler);

    QDomNode n;

    n.nodeName();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("nodeName")));

    n.nodeValue();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("nodeValue")));

    n.setNodeValue(QLatin1String("a value"));
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("setNodeValue")));

    n.parentNode();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("parentNode")));

    n.childNodes();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("childNodes")));

    n.firstChild();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("firstChild")));

    n.lastChild();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("lastChild")));

    n.previousSibling();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("previousSibling")));

    n.nextSibling();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("nextSibling")));

    n.attributes();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("attributes")));

    n.ownerDocument();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("ownerDocument")));

    n.cloneNode();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("cloneNode")));

    n.normalize();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("normalize")));

    n.namespaceURI();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("namespaceURI")));

    n.prefix();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("prefix")));

    n.setPrefix(QLatin1String("aprefix"));
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("setPrefix")));

    n.localName();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("localName")));

    n.hasAttributes();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("hasAttributes")));

    n.insertBefore(QDomNode(), QDomNode());
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("insertBefore")));

    n.insertAfter(QDomNode(), QDomNode());
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("insertAfter")));

    n.replaceChild(QDomNode(), QDomNode());
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("replaceChild")));

    n.removeChild(QDomNode());
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("removeChild")));

    n.appendChild(QDomNode());
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("appendChild")));

    n.hasChildNodes();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("hasChildNodes")));

    n.clear();
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("clear")));

    n.namedItem(QLatin1String("aName"));
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("namedItem")));

    QTextStream stream;
    n.save(stream, 3);
    QCOMPARE(s_onNullMessage, qPrintable(onNullWarning("save")));
    
    qInstallMsgHandler(0);
}

void tst_QDom::roundTripAttributes() const
{
    /* Create an attribute via the QDom API with weird whitespace content. */
    QDomImplementation impl;

    QDomDocument doc(impl.createDocument("", "localName", QDomDocumentType()));

    QDomElement e(doc.documentElement());

    QString ws;
    ws.reserve(8);
    ws.append(QChar(0x20));
    ws.append(QChar(0x20));
    ws.append(QChar(0x20));
    ws.append(QChar(0xD));
    ws.append(QChar(0xA));
    ws.append(QChar(0x9));
    ws.append(QChar(0x20));
    ws.append(QChar(0x20));

    e.setAttribute("attr", ws);

    QByteArray serialized;
    QBuffer buffer(&serialized);
    buffer.open(QIODevice::WriteOnly);
    QTextStream stream(&buffer);

    doc.save(stream, 0);
    stream.flush();

    const QByteArray expected("<localName xmlns=\"\" attr=\"   &#xd;&#xa;&#x9;  \" />\n");
    QCOMPARE(QString::fromLatin1(serialized.constData()), QString::fromLatin1(expected.constData()));
}

void tst_QDom::normalizeEndOfLine() const
{
    QByteArray input("<a>\r\nc\rc\ra\na</a>");

    QBuffer buffer(&input);
    QVERIFY(buffer.open(QIODevice::ReadOnly));

    QDomDocument doc;
    QVERIFY(doc.setContent(&buffer, true));

    const QString expected(QLatin1String("<a>\nc\nc\na\na</a>"));

    // Qt 5 ### Fix this, if we keep QDom at all.
    QEXPECT_FAIL("", "The parser doesn't perform newline normalization. Fixing that would change behavior.", Continue);
    QCOMPARE(doc.documentElement().text(), expected);
}

void tst_QDom::normalizeAttributes() const
{
    QByteArray data("<element attribute=\"a\na\"/>");
    QBuffer buffer(&data);

    QVERIFY(buffer.open(QIODevice::ReadOnly));

    QDomDocument doc;
    QVERIFY(doc.setContent(&buffer, true));

    // Qt 5 ### Fix this, if we keep QDom at all.
    QEXPECT_FAIL("", "The parser doesn't perform Attribute Value Normalization. Fixing that would change behavior.", Continue);
    QCOMPARE(doc.documentElement().attribute(QLatin1String("attribute")), QString::fromLatin1("a a"));
}

void tst_QDom::serializeWeirdEOL() const
{
    QDomImplementation impl;

    QDomDocument doc(impl.createDocument("", "name", QDomDocumentType()));
    QDomElement ele(doc.documentElement());
    ele.appendChild(doc.createTextNode(QLatin1String("\r\nasd\nasd\rasd\n")));

    QByteArray output;
    QBuffer writeBuffer(&output);
    QVERIFY(writeBuffer.open(QIODevice::WriteOnly));
    QTextStream stream(&writeBuffer);

    const QByteArray expected("<name xmlns=\"\">&#xd;\nasd\nasd&#xd;asd\n</name>\n");
    doc.save(stream, 0);
    QCOMPARE(QString::fromLatin1(output.constData()), QString::fromLatin1(expected.constData()));
}

void tst_QDom::reparentAttribute() const
{
    QDomImplementation impl;
    QDomDocument doc(impl.createDocument("", "localName", QDomDocumentType()));

    QDomElement ele(doc.documentElement());
    QDomAttr attr(doc.createAttribute("localName"));
    ele.setAttributeNode(attr);

    QVERIFY(attr.ownerElement() == ele);
    QVERIFY(attr.parentNode() == ele);
}

void tst_QDom::serializeNamespaces() const
{
    const char *const input = "<doc xmlns:b='http://example.com/'>"
                              "<b:element b:name=''/>"
                              "</doc>";

    QByteArray ba(input);
    QBuffer buffer(&ba);

    QVERIFY(buffer.open(QIODevice::ReadOnly));

    QXmlInputSource source(&buffer);
    QXmlSimpleReader reader;
    reader.setFeature("http://xml.org/sax/features/namespaces", true);
    reader.setFeature("http://xml.org/sax/features/namespace-prefixes", false);

    QDomDocument doc;
    QVERIFY(doc.setContent(&source, &reader));

    const QByteArray serialized(doc.toByteArray());

    QDomDocument doc2;
    QVERIFY(doc2.setContent(doc.toString(), true));

    /* Here we test that it roundtrips. */
    QVERIFY(isDeepEqual(doc2, doc));

    QDomDocument doc3;
    QVERIFY(doc3.setContent(QString::fromLatin1(serialized.constData()), true));

    QVERIFY(isDeepEqual(doc3, doc));
}

void tst_QDom::flagInvalidNamespaces() const
{
    const char *const input = "<doc>"
                              "<b:element xmlns:b='http://example.com/' b:name='' xmlns:b='http://example.com/'/>"
                              "</doc>";

    QDomDocument doc;
    QVERIFY(!doc.setContent(QString::fromLatin1(input, true)));
    QEXPECT_FAIL("", "The parser doesn't flag identical qualified attribute names. Fixing this would change behavior.", Continue);
    QVERIFY(!doc.setContent(QString::fromLatin1(input)));
}

void tst_QDom::flagUndeclaredNamespace() const
{
    /* Note, prefix 'a' is not declared. */
    const char *const input = "<a:doc xmlns:b='http://example.com/'>"
                              "<b:element b:name=''/>"
                              "</a:doc>";

    QByteArray ba(input);
    QBuffer buffer(&ba);

    QVERIFY(buffer.open(QIODevice::ReadOnly));

    QXmlInputSource source(&buffer);
    QXmlSimpleReader reader;
    reader.setFeature("http://xml.org/sax/features/namespaces", true);
    reader.setFeature("http://xml.org/sax/features/namespace-prefixes", false);

    QDomDocument doc;
    QEXPECT_FAIL("", "The parser doesn't flag not declared prefixes. Fixing this would change behavior.", Continue);
    QVERIFY(!doc.setContent(&source, &reader));
}

void tst_QDom::indentComments() const
{
    /* We test that:
     *
     * - Whitespace is not added if a text node appears after a comment.
     * - Whitespace is not added if a text node appears before a comment.
     * - Indentation depth is linear with level depth.
     */
    const char *const input = "<e>"
                                  "<!-- A Comment -->"
                                  "<b><!-- deep --></b>"
                                  "textNode"
                                  "<!-- Another Comment -->"
                                  "<!-- Another Comment2 -->"
                                  "textNode2"
                              "</e>";
    const char *const expected = "<e>\n"
                                 "     <!-- A Comment -->\n"
                                 "     <b>\n"
                                 "          <!-- deep -->\n"
                                 "     </b>"
                                 "textNode"
                                 "<!-- Another Comment -->\n"
                                 "     <!-- Another Comment2 -->"
                                 "textNode2"
                                 "</e>\n";
    QDomDocument doc;
    QVERIFY(doc.setContent(QString::fromLatin1(input)));

    const QString serialized(doc.toString(5));

    QCOMPARE(serialized, QString::fromLatin1(expected));
}

void tst_QDom::checkLiveness() const
{
    QDomImplementation impl;

    QDomDocument doc(impl.createDocument(QString(), "doc", QDomDocumentType()));
    QDomElement ele(doc.documentElement());

    const QDomElement e1(doc.createElement("name"));
    const QDomElement e2(doc.createElement("name"));
    const QDomText t1(doc.createTextNode("content"));

    ele.appendChild(e1);
    ele.appendChild(t1);
    ele.appendChild(e2);

    const QDomNodeList children(ele.childNodes());
    QCOMPARE(children.count(), 3);

    ele.removeChild(e1);

    QCOMPARE(children.count(), 2);
    QCOMPARE(children.at(0), static_cast<const QDomNode &>(t1));
    QCOMPARE(children.at(1), static_cast<const QDomNode &>(e2));
}

QTEST_MAIN(tst_QDom)
#include "tst_qdom.moc"
