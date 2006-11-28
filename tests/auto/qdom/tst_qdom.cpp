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

public:
    tst_QDom();
    virtual ~tst_QDom();


public slots:
    void init();
    void cleanup();
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

private:
    int hasAttributesHelper( const QDomNode& node );
    bool compareDocuments( const QDomDocument &doc1, const QDomDocument &doc2 );
    bool compareNodes( const QDomNode &node1, const QDomNode &node2, bool deep );
    QDomNode findDomNode( const QDomDocument &doc, const QList<QVariant> &pathToNode );
};

Q_DECLARE_METATYPE(QList<QVariant>)

tst_QDom::tst_QDom()
{
}

tst_QDom::~tst_QDom()
{
}

void tst_QDom::init()
{
}

void tst_QDom::cleanup()
{
}

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

/*
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
//	QVERIFY( node.attributes().count() > 0 );
    } else {
    	if (node.attributes().count() != 0)
	    return -1;
//	QVERIFY( node.attributes().count() == 0 );
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
	OWNERDOCUMENT_CREATE_TEST( QDomAttr,		domDoc.createAttribute( "foo" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomAttr,		domDoc.createAttributeNS( "foo", "bar" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomCDATASection,	domDoc.createCDATASection( "foo" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomComment,		domDoc.createComment( "foo" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomDocumentFragment,domDoc.createDocumentFragment() );
	OWNERDOCUMENT_CREATE_TEST( QDomElement,		domDoc.createElement( "foo" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomElement,		domDoc.createElementNS( "foo", "bar" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomEntityReference,	domDoc.createEntityReference( "foo" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomProcessingInstruction, domDoc.createProcessingInstruction( "foo", "bar" ) );
	OWNERDOCUMENT_CREATE_TEST( QDomText,		domDoc.createTextNode( "foo" ) );
    }

    // test importNode()
    {
	QDomDocument doc2;
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomAttr,		doc2.createAttribute( "foo" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomAttr,		doc2.createAttributeNS( "foo", "bar" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomCDATASection,	doc2.createCDATASection( "foo" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomComment,		doc2.createComment( "foo" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomDocumentFragment,	doc2.createDocumentFragment() );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomElement,		doc2.createElement( "foo" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomElement,		doc2.createElementNS( "foo", "bar" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomEntityReference,	doc2.createEntityReference( "foo" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomProcessingInstruction, doc2.createProcessingInstruction( "foo", "bar" ) );
	OWNERDOCUMENT_IMPORTNODE_TEST( QDomText,		doc2.createTextNode( "foo" ) );
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
    QDomDocument domDoc;
    QVERIFY( domDoc.setContent( doc ) );
    QDomNode node = findDomNode( domDoc, pathToNode );
    QVERIFY(!node.isNull());

    // test QDomDocument::create...()
    {
	PARENTNODE_CREATE_TEST( QDomAttr,		domDoc.createAttribute( "foo" ) );
	PARENTNODE_CREATE_TEST( QDomAttr,		domDoc.createAttributeNS( "foo", "bar" ) );
	PARENTNODE_CREATE_TEST( QDomCDATASection,	domDoc.createCDATASection( "foo" ) );
	PARENTNODE_CREATE_TEST( QDomComment,		domDoc.createComment( "foo" ) );
	PARENTNODE_CREATE_TEST( QDomDocumentFragment,	domDoc.createDocumentFragment() );
	PARENTNODE_CREATE_TEST( QDomElement,		domDoc.createElement( "foo" ) );
	PARENTNODE_CREATE_TEST( QDomElement,		domDoc.createElementNS( "foo", "bar" ) );
	PARENTNODE_CREATE_TEST( QDomEntityReference,	domDoc.createEntityReference( "foo" ) );
	PARENTNODE_CREATE_TEST( QDomProcessingInstruction, domDoc.createProcessingInstruction( "foo", "bar" ) );
	PARENTNODE_CREATE_TEST( QDomText,		domDoc.createTextNode( "foo" ) );
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


/*
    Returns TRUE if \a doc1 and \a doc2 represent the same XML document, i.e.
    they have the same informational content. Otherwise, this function returns
    FALSE.
*/
bool tst_QDom::compareDocuments( const QDomDocument &doc1, const QDomDocument &doc2 )
{
    // ### this test should be improved, since the ordering of attributes does
    // not matter in XML, e.g.
    return doc1.toString() == doc2.toString();
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
//	QVERIFY( !node.isNull() );
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
    QCOMPARE(map.item(1).nodeName(), QString()); // Make sure we don't assert

    QDomNodeList list = doc.elementsByTagName("foo");
    QCOMPARE(list.item(0).nodeName(), QString("foo"));
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

QTEST_MAIN(tst_QDom)
#include "tst_qdom.moc"
