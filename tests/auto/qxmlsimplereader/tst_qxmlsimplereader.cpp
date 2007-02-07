/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qthread.h>
#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <QtTest/QtTest>
#include <qfile.h>
#include <qstring.h>
#include <qdir.h>
#include <qbuffer.h>
#include "parser/parser.h"

static const char *inputString = "<!DOCTYPE inferno [<!ELEMENT inferno (circle+)><!ELEMENT circle (#PCDATA)>]><inferno><circle /><circle /></inferno>";
static const char *refString = "setDocumentLocator(locator={columnNumber=1, lineNumber=1})\nstartDocument()\nstartDTD(name=\"inferno\", publicId=\"\", systemId=\"\")\nendDTD()\nstartElement(namespaceURI=\"\", localName=\"inferno\", qName=\"inferno\", atts=[])\nstartElement(namespaceURI=\"\", localName=\"circle\", qName=\"circle\", atts=[])\nendElement(namespaceURI=\"\", localName=\"circle\", qName=\"circle\")\nstartElement(namespaceURI=\"\", localName=\"circle\", qName=\"circle\", atts=[])\nendElement(namespaceURI=\"\", localName=\"circle\", qName=\"circle\")\nendElement(namespaceURI=\"\", localName=\"inferno\", qName=\"inferno\")\nendDocument()\n";

//TESTED_CLASS=QXmlSimpleReader
//TESTED_FILES=qxml.cpp qxml.h

#define TEST_PORT 1088

class XmlServer : public QThread
{
    Q_OBJECT
public:
    XmlServer();
    bool quit_soon;

protected:
    virtual void run();
};

XmlServer::XmlServer()
{
    quit_soon = false;
}

#define CHUNK_SIZE 1

void XmlServer::run()
{
    QTcpServer srv;

    if (!srv.listen(QHostAddress::Any, TEST_PORT))
        return;

    for (;;) {
        srv.waitForNewConnection(100);

        if (QTcpSocket *sock = srv.nextPendingConnection()) {
            QByteArray fileName;
            for (;;) {
                char c;
                if (sock->getChar(&c)) {
                    if (c == '\n')
                        break;
                    fileName.append(c);
                } else {
                    if (!sock->waitForReadyRead(-1))
                        break;
                }
            }

            QFile file(QString::fromLocal8Bit(fileName));
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "XmlServer::run(): could not open" << fileName;
                sock->abort();
                delete sock;
                continue;
            }

            QByteArray data = file.readAll();
            for (int i = 0; i < data.size();) {
//                sock->putChar(data.at(i));
                int cnt = qMin(CHUNK_SIZE, data.size() - i);
                sock->write(data.constData() + i, cnt);
                i += cnt;
                sock->flush();
                QTest::qSleep(1);
                
                if (quit_soon) {
                    sock->abort();
                    break;
                }
            }

            sock->disconnectFromHost();
            delete sock;
        }

        if (quit_soon)
            break;
    }

    srv.close();
}

class tst_QXmlSimpleReader : public QObject
{
    Q_OBJECT

    public:
	tst_QXmlSimpleReader();
	~tst_QXmlSimpleReader();

    private slots:

	void testGoodXmlFile();
	void testGoodXmlFile_data();
	void testBadXmlFile();
	void testBadXmlFile_data();
	void testIncrementalParsing();
	void testIncrementalParsing_data();
        void setDataQString();
        void inputFromQIODevice();
        void inputFromString();
        void inputFromSocket_data();
        void inputFromSocket();

        void idsInParseException1();
        void idsInParseException2();
    private:
        XmlServer *server;
};

tst_QXmlSimpleReader::tst_QXmlSimpleReader()
{
    server = new XmlServer();
    server->start();
    QTest::qSleep(1000);
}

tst_QXmlSimpleReader::~tst_QXmlSimpleReader()
{
    server->quit_soon = true;
    server->wait();
}

class MyErrorHandler : public QXmlErrorHandler
{
public:
    QString publicId;
    QString systemId;

    virtual bool error(const QXmlParseException &)
    {
        return false;
    }

    virtual QString errorString() const
    {
        return QString();
    }

    virtual bool fatalError(const QXmlParseException &exception)
    {
        qDebug() << "fatalError() CALLED";
        publicId = exception.publicId();
        systemId = exception.systemId();
        return true;
    }

    virtual bool warning(const QXmlParseException &)
    {
        return true;
    }

};

void tst_QXmlSimpleReader::idsInParseException1()
{
    MyErrorHandler handler;
    QXmlSimpleReader reader;

    reader.setErrorHandler(&handler);

    /* A non-wellformed XML document with PUBLIC and SYSTEM. */
    QByteArray input("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
                     "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
                     "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">"
                     "<head>"
                         "<a/><a/><a/>"
                     "<head/>");

    QBuffer buff(&input);
    QXmlInputSource source(&buff);

    /* Yes, parsing should be reported as a failure. */
    QVERIFY(!reader.parse(source));

    QCOMPARE(handler.publicId, QString::fromLatin1("-//W3C//DTD XHTML 1.0 Strict//EN"));
    QCOMPARE(handler.systemId, QString::fromLatin1("http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"));
}

void tst_QXmlSimpleReader::idsInParseException2()
{
    MyErrorHandler handler;
    QXmlSimpleReader reader;

    reader.setErrorHandler(&handler);

    /* A non-wellformed XML document with only SYSTEM. */
    QByteArray input("<!DOCTYPE html SYSTEM \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
                      "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">"
                      "<head>"
                          "<a/><a/><a/>"
                      "<head/>");

    QBuffer buff(&input);
    QXmlInputSource source(&buff);

    /* Yes, parsing should be reported as a failure. */
    QVERIFY(!reader.parse(source));

    QCOMPARE(handler.publicId, QString());
    QCOMPARE(handler.systemId, QString::fromLatin1("http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"));
}

static QStringList findXmlFiles(QString dir_name)
{
    QStringList result;

    QDir dir(dir_name);
    QFileInfoList file_list = dir.entryInfoList(QStringList("*.xml"), QDir::Files, QDir::Name);

    QFileInfoList::const_iterator it = file_list.begin();
    for (; it != file_list.end(); ++it) {
	const QFileInfo &file_info = *it;
	result.append(file_info.filePath());
    }

    return result;
}


void tst_QXmlSimpleReader::testGoodXmlFile_data()
{
    const char * const good_data_dirs[] = {
	"xmldocs/valid/sa",
	"xmldocs/valid/not-sa",
	"xmldocs/valid/ext-sa",
	0
    };
    const char * const *d = good_data_dirs;

    QStringList good_file_list;
    for (; *d != 0; ++d)
	good_file_list += findXmlFiles(*d);

    QTest::addColumn<QString>("file_name");
    QStringList::const_iterator it = good_file_list.begin();
    for (; it != good_file_list.end(); ++it)
	QTest::newRow((*it).toLatin1()) << *it;
}

void tst_QXmlSimpleReader::testGoodXmlFile()
{
    QFETCH(QString, file_name);
    QFile file(file_name);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();
    file.close();
    QVERIFY(file.open(QIODevice::ReadOnly));
    Parser parser;

//    static int i = 0;
//    qWarning("Test nr: " + QString::number(i)); ++i;
    QEXPECT_FAIL("xmldocs/valid/sa/089.xml", "", Continue);
    QVERIFY(parser.parseFile(&file));

    QFile ref_file(file_name + ".ref");
    QVERIFY(ref_file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream ref_stream(&ref_file);
    ref_stream.setEncoding(QTextStream::UnicodeUTF8);
    QString ref_file_contents = ref_stream.read();

    QCOMPARE(parser.result(), ref_file_contents);
}

void tst_QXmlSimpleReader::testBadXmlFile_data()
{
    const char * const bad_data_dirs[] = {
	"xmldocs/not-wf/sa",
	0
    };
    const char * const *d = bad_data_dirs;

    QStringList bad_file_list;
    for (; *d != 0; ++d)
	bad_file_list += findXmlFiles(*d);

    QTest::addColumn<QString>("file_name");
    QStringList::const_iterator it = bad_file_list.begin();
    for (; it != bad_file_list.end(); ++it)
	QTest::newRow((*it).toLatin1()) << *it;
}

void tst_QXmlSimpleReader::testBadXmlFile()
{
    QFETCH(QString, file_name);
    QFile file(file_name);
    QVERIFY(file.open(QIODevice::ReadOnly));
    Parser parser;

//    static int i = 0;
//    qWarning("Test nr: " + QString::number(++i));
    QEXPECT_FAIL("xmldocs/not-wf/sa/030.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/031.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/032.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/033.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/038.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/072.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/073.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/074.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/076.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/077.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/078.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/085.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/086.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/087.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/101.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/102.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/104.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/116.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/117.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/119.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/122.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/132.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/142.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/143.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/144.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/145.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/146.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/160.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/162.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/168.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/169.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/170.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/180.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/181.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/182.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/185.xml", "", Continue);
    QEXPECT_FAIL("xmldocs/not-wf/sa/186.xml", "", Continue);
    QVERIFY(!parser.parseFile(&file));

    QFile ref_file(file_name + ".ref");
    QVERIFY(ref_file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream ref_stream(&ref_file);
    ref_stream.setEncoding(QTextStream::UnicodeUTF8);
    QString ref_file_contents = ref_stream.read();

    QCOMPARE(parser.result(), ref_file_contents);
}

void tst_QXmlSimpleReader::testIncrementalParsing_data()
{
    QTest::addColumn<QString>("file_name");
    QTest::addColumn<int>("chunkSize");

    const char * const good_data_dirs[] = {
	"xmldocs/valid/sa",
	"xmldocs/valid/not-sa",
	"xmldocs/valid/ext-sa",
	0
    };
    const char * const *d = good_data_dirs;

    QStringList good_file_list;
    for (; *d != 0; ++d)
	good_file_list += findXmlFiles(*d);

    for (int i=1; i<10; ++i) {
	QStringList::const_iterator it = good_file_list.begin();
	for (; it != good_file_list.end(); ++it) {
	    if ( *it == "xmldocs/valid/sa/089.xml" )
		continue; // fails at the moment -- don't bother
	    if ( i==1 && (
			*it == "xmldocs/valid/sa/049.xml" ||
			*it == "xmldocs/valid/sa/050.xml" ||
			*it == "xmldocs/valid/sa/051.xml" ||
			*it == "xmldocs/valid/sa/052.xml" ) ) {
		continue; // fails at the moment -- don't bother
	    }
	    QTest::newRow(QString("%1 %2").arg(*it).arg(i).toLatin1()) << *it << i;
	}
    }
}

void tst_QXmlSimpleReader::testIncrementalParsing()
{
    QFETCH(QString, file_name);
    QFETCH(int, chunkSize);

    QFile file(file_name);
    QVERIFY(file.open(QIODevice::ReadOnly));

    Parser parser;
    QXmlInputSource source;
    bool first = true;
    while (!file.atEnd()) {
        source.setData(file.read(chunkSize));
        if(first) {
            QVERIFY(parser.parse(&source, true));
            first = false;
        } else {
            QVERIFY(parser.parseContinue());
	}
    }
    // detect end of document
    QVERIFY(parser.parseContinue());
    // parsing should fail after the end of the document was reached
    QVERIFY(!parser.parseContinue());

    QFile ref_file(file_name + ".ref");
    QVERIFY(ref_file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream ref_stream(&ref_file);
    ref_stream.setEncoding(QTextStream::UnicodeUTF8);
    QString ref_file_contents = ref_stream.readAll();

    QCOMPARE(parser.result(), ref_file_contents);
}

void tst_QXmlSimpleReader::setDataQString()
{
    QString input = inputString;
    QString ref = refString;

    QXmlInputSource source;
    Parser parser;

    source.setData(input);
    QVERIFY(parser.parse(&source,false));

    QBuffer resultBuffer;
    resultBuffer.setData(parser.result().toLatin1());

    QBuffer refBuffer;
    refBuffer.setData(ref.toLatin1());

    resultBuffer.open(QIODevice::ReadOnly);
    refBuffer.open(QIODevice::ReadOnly);

    bool success = true;
    while (resultBuffer.canReadLine()) {
        if (!refBuffer.canReadLine()) {
            success = false; break ;
        }
        if (resultBuffer.readLine().simplified() != refBuffer.readLine().simplified()) {
            success = false; break ;
        }
    }
    QVERIFY(success);
}

void tst_QXmlSimpleReader::inputFromQIODevice()
{
    QBuffer inputBuffer;
    inputBuffer.setData(inputString);

    QXmlInputSource source(&inputBuffer);
    Parser parser;

    QVERIFY(parser.parse(&source,false));

    QBuffer resultBuffer;
    resultBuffer.setData(parser.result().toLatin1());

    QBuffer refBuffer;
    refBuffer.setData(refString);

    resultBuffer.open(QIODevice::ReadOnly);
    refBuffer.open(QIODevice::ReadOnly);

    bool success = true;
    while (resultBuffer.canReadLine()) {
        if (!refBuffer.canReadLine()) {
            success = false; break ;
        }
        if (resultBuffer.readLine().simplified() != refBuffer.readLine().simplified()) {
            success = false; break ;
        }
    }
    QVERIFY(success);
}

void tst_QXmlSimpleReader::inputFromString()
{
    QString str = "<foo><bar>kake</bar><bar>ja</bar></foo>";
    QBuffer buff;
    buff.setData((char*)str.utf16(), str.size()*sizeof(ushort));

    QXmlInputSource input(&buff);

    QXmlSimpleReader reader;
    QXmlDefaultHandler handler;
    reader.setContentHandler(&handler);

    QVERIFY(reader.parse(&input));
}

void tst_QXmlSimpleReader::inputFromSocket_data()
{
    QStringList files = findXmlFiles(QLatin1String("encodings"));
    QVERIFY(files.count() > 0);

    QTest::addColumn<QString>("file_name");

    foreach (const QString &file_name, files)
        QTest::newRow(file_name.toLatin1()) << file_name;
}

void tst_QXmlSimpleReader::inputFromSocket()
{
    QFETCH(QString, file_name);

    QTcpSocket sock;
    sock.connectToHost("localhost", TEST_PORT);
    QVERIFY(sock.waitForConnected());

    sock.write(file_name.toLocal8Bit() + "\n");
    QVERIFY(sock.waitForBytesWritten());

    QXmlInputSource input(&sock);

    QXmlSimpleReader reader;
    QXmlDefaultHandler handler;
    reader.setContentHandler(&handler);

    QVERIFY(reader.parse(&input));

//    qDebug() << "tst_QXmlSimpleReader::inputFromSocket(): success" << file_name;
}

QTEST_MAIN(tst_QXmlSimpleReader)
#include "tst_qxmlsimplereader.moc"
