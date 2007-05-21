/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QtTest/QtTest>

//TESTED_FILES=corelib/io/qiodevice.h corelib/io/qiodevice.cpp

class tst_QIODevice : public QObject
{
    Q_OBJECT

public:
    tst_QIODevice();
    virtual ~tst_QIODevice();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void constructing_QTcpSocket();
    void constructing_QFile();
    void read_QByteArray();
    void unget();
#if QT_VERSION >= 0x040100
    void peek();
#endif // QT_VERSION
    void getch();
    void putch();

    void readLine_data();
    void readLine();
};

// Testing get/set functions
void tst_QIODevice::getSetCheck()
{
    // OpenMode QIODevice::openMode()
    // void QIODevice::setOpenMode(OpenMode)
    class MyIODevice : public QIODevice {
    public:
        void setOpenMode(OpenMode openMode) { QIODevice::setOpenMode(openMode); }
    };
    QTcpSocket var1;
    MyIODevice *obj1 = reinterpret_cast<MyIODevice*>(&var1);
    obj1->setOpenMode(QIODevice::OpenMode(QIODevice::NotOpen));
    QCOMPARE(QIODevice::OpenMode(QIODevice::NotOpen), obj1->openMode());
    obj1->setOpenMode(QIODevice::OpenMode(QIODevice::ReadWrite));
    QCOMPARE(QIODevice::OpenMode(QIODevice::ReadWrite), obj1->openMode());
}

tst_QIODevice::tst_QIODevice()
{
}

tst_QIODevice::~tst_QIODevice()
{

}

void tst_QIODevice::init()
{
}

void tst_QIODevice::cleanup()
{
}

//----------------------------------------------------------------------------------
void tst_QIODevice::constructing_QTcpSocket()
{
    QTcpSocket socket;
    QIODevice *device = &socket;

    QVERIFY(!device->isOpen());

    socket.connectToHost("imap", 143);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(device->isOpen());

    while (!device->canReadLine())
        QVERIFY(device->waitForReadyRead(5000));

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    qlonglong lineLength = device->readLine(buf, sizeof(buf));
    QVERIFY(lineLength > 0);
    QCOMPARE(socket.pos(), qlonglong(0));

    socket.close();
    socket.connectToHost("imap", 143);
    QVERIFY(socket.waitForConnected(5000));

    while (!device->canReadLine())
        QVERIFY(device->waitForReadyRead(5000));

    char buf2[1024];
    memset(buf2, 0, sizeof(buf2));
    QCOMPARE(socket.readLine(buf2, sizeof(buf2)), lineLength);

    char *c1 = buf;
    char *c2 = buf2;
    while (*c1 && *c2) {
        QCOMPARE(*c1, *c2);
        ++c1;
        ++c2;
    }
    QCOMPARE(*c1, *c2);
}

//----------------------------------------------------------------------------------
void tst_QIODevice::constructing_QFile()
{
    QFile file;
    QIODevice *device = &file;

    QVERIFY(!device->isOpen());

    file.setFileName("tst_qiodevice.cpp");
    QVERIFY(file.open(QFile::ReadOnly));
    QVERIFY(device->isOpen());
    QCOMPARE((int) device->openMode(), (int) QFile::ReadOnly);

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    qlonglong lineLength = device->readLine(buf, sizeof(buf));
    QVERIFY(lineLength > 0);
    QCOMPARE(file.pos(), lineLength);

    file.seek(0);
    char buf2[1024];
    memset(buf2, 0, sizeof(buf2));
    QCOMPARE(file.readLine(buf2, sizeof(buf2)), lineLength);

    char *c1 = buf;
    char *c2 = buf2;
    while (*c1 && *c2) {
        QCOMPARE(*c1, *c2);
        ++c1;
        ++c2;
    }
    QCOMPARE(*c1, *c2);
}


void tst_QIODevice::read_QByteArray()
{
    QFile f("./tst_qiodevice.cpp");
    f.open(QIODevice::ReadOnly);

    QByteArray b = f.read(10);
    QCOMPARE(b.length(), 10);

    b = f.read(256);
    QCOMPARE(b.length(), 256);

    b = f.read(0);
    QCOMPARE(b.length(), 0);
}

//--------------------------------------------------------------------
void tst_QIODevice::unget()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    buffer.write("ZXCV");
    buffer.seek(0);
    QCOMPARE(buffer.read(4), QByteArray("ZXCV"));
    QCOMPARE(buffer.pos(), qint64(4));

    buffer.ungetChar('a');
    buffer.ungetChar('b');
    buffer.ungetChar('c');
    buffer.ungetChar('d');

    QCOMPARE(buffer.pos(), qint64(0));

    char buf[6];
    QCOMPARE(buffer.readLine(buf, 5), qint64(4));
    QCOMPARE(buffer.pos(), qint64(4));
    QCOMPARE(static_cast<const char*>(buf), "dcba");

    buffer.ungetChar('a');
    buffer.ungetChar('b');
    buffer.ungetChar('c');
    buffer.ungetChar('d');

    QCOMPARE(buffer.pos(), qint64(0));
    
    for (int i = 0; i < 5; ++i) {
        buf[0] = '@';
        buf[1] = '@';
        QTest::ignoreMessage(QtWarningMsg,
                              "QIODevice::readLine: Called with maxSize < 2");
        QCOMPARE(buffer.readLine(buf, 1), qint64(-1));
        QCOMPARE(buffer.readLine(buf, 2), qint64(i < 4 ? 1 : -1));
        switch (i) {
        case 0: QCOMPARE(buf[0], 'd'); break;
        case 1: QCOMPARE(buf[0], 'c'); break;
        case 2: QCOMPARE(buf[0], 'b'); break;
        case 3: QCOMPARE(buf[0], 'a'); break;
        case 4: QCOMPARE(buf[0], '\0'); break;
        }
        QCOMPARE(buf[1], i < 4 ? '\0' : '@');
    }

    buffer.ungetChar('\n');
    QCOMPARE(buffer.readLine(), QByteArray("\n"));

    buffer.seek(1);
    buffer.readLine(buf, 3);
    QCOMPARE(static_cast<const char*>(buf), "XC");

    buffer.seek(4);
    buffer.ungetChar('Q');
    QCOMPARE(buffer.readLine(buf, 3), qint64(1));

    for (int i = 0; i < 2; ++i) {
        QTcpSocket socket;
	QIODevice *dev;
	QByteArray result;
	const char *lineResult;
	if (i == 0) {
            dev = &buffer;
            result = QByteArray("ZXCV");
            lineResult = "ZXCV";
        } else {
            socket.connectToHost("www.trolltech.com", 80);
            socket.write("GET / HTTP/1.0\r\n\r\n");
            QVERIFY(socket.waitForReadyRead());
            dev = &socket;
            result = QByteArray("HTTP");
            lineResult = "Cont";
	}
	char ch, ch2;
	dev->seek(0);
	dev->getChar(&ch);
	dev->ungetChar(ch);
	QCOMPARE(dev->peek(4), result);
	dev->getChar(&ch);
	dev->getChar(&ch2);
	dev->ungetChar(ch2);
	dev->ungetChar(ch);
	QCOMPARE(dev->read(1), result.left(1));
	QCOMPARE(dev->read(3), result.right(3));

        if (i == 0)
	    dev->seek(0);
        else
            dev->readLine();
        dev->getChar(&ch);
        dev->ungetChar(ch);
        dev->readLine(buf, 5);
        QCOMPARE(static_cast<const char*>(buf), lineResult);

        if (i == 1)
            socket.close();
    }
}

//--------------------------------------------------------------------
#if QT_VERSION >= 0x040100
void tst_QIODevice::peek()
{
    QBuffer buffer;
    QFile::remove("peektestfile");
    QFile file("peektestfile");

    for (int i = 0; i < 2; ++i) {
	QIODevice *device = i ? (QIODevice *)&file : (QIODevice *)&buffer;

	device->open(QBuffer::ReadWrite);
	device->write("ZXCV");

	device->seek(0);
	QCOMPARE(device->peek(4), QByteArray("ZXCV"));
	QCOMPARE(device->pos(), qint64(0));
	device->write("ABCDE");
	device->seek(3);
	QCOMPARE(device->peek(1), QByteArray("D"));
	QCOMPARE(device->peek(5), QByteArray("DE"));
	device->seek(0);
	QCOMPARE(device->read(4), QByteArray("ABCD"));
	QCOMPARE(device->pos(), qint64(4));
    
	device->seek(0);
	device->write("ZXCV");
	device->seek(0);
	char buf[5];
	buf[4] = 0;
	device->peek(buf, 4);
	QCOMPARE(static_cast<const char *>(buf), "ZXCV");
	QCOMPARE(device->pos(), qint64(0));
	device->read(buf, 4);
	QCOMPARE(static_cast<const char *>(buf), "ZXCV");
	QCOMPARE(device->pos(), qint64(4));
    }
}
#endif // QT_VERSION

void tst_QIODevice::getch()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    buffer.write("\xff\x7f\x80\x00", 4);
    buffer.reset();
    QCOMPARE(buffer.getch(), 0xff);
    QCOMPARE(buffer.getch(), 0x7f);
    QCOMPARE(buffer.getch(), 0x80);
    QCOMPARE(buffer.getch(), 0x00);

    buffer.ungetch(0x00);
    buffer.ungetch(0x80);
    buffer.ungetch(0x7f);
    buffer.ungetch(0xff);

    QCOMPARE(buffer.getch(), 0xff);
    QCOMPARE(buffer.getch(), 0x7f);
    QCOMPARE(buffer.getch(), 0x80);
    QCOMPARE(buffer.getch(), 0x00);
}

void tst_QIODevice::putch()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    buffer.putch(0xff);
    buffer.putch(0x7f);
    buffer.putch(0x80);
    buffer.putch(0x00);
    buffer.reset();
    QCOMPARE(buffer.getch(), 0xff);
    QCOMPARE(buffer.getch(), 0x7f);
    QCOMPARE(buffer.getch(), 0x80);
    QCOMPARE(buffer.getch(), 0x00);
}

void tst_QIODevice::readLine_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("0") << QByteArray("\nAA");
    QTest::newRow("1") << QByteArray("A\nAA");

    QByteArray data(9000, 'A');
    data[8193] = '\n';
    QTest::newRow("8194") << data;
    data[8193] = 'A';
    data[8192] = '\n';
    QTest::newRow("8193") << data;
    data[8192] = 'A';
    data[8191] = '\n';
    QTest::newRow("8192") << data;
    data[8191] = 'A';
    data[8190] = '\n';
    QTest::newRow("8191") << data;

    data[5999] = '\n';
    QTest::newRow("6000") << data;

    data[4095] = '\n';
    QTest::newRow("4096") << data;

    data[4094] = '\n';
    data[4095] = 'A';
    QTest::newRow("4095") << data;
}

void tst_QIODevice::readLine()
{
    QFETCH(QByteArray, data);
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::ReadWrite));
    QVERIFY(buffer.canReadLine());

    int linelen = data.indexOf('\n') + 1;
    QByteArray line;
    line.reserve(linelen + 100);

    int result = buffer.readLine(line.data(), linelen + 100);
    QCOMPARE(result, linelen);

    // try the exact length of the line (plus terminating \0)
    QVERIFY(buffer.seek(0));
    result = buffer.readLine(line.data(), linelen + 1);
    QCOMPARE(result, linelen);

#if QT_VERSION >= 0x0402000
    // try with a line length limit
    QVERIFY(buffer.seek(0));
    line = buffer.readLine(linelen + 100);
    QCOMPARE(line.size(), linelen);

    // try without a length limit
    QVERIFY(buffer.seek(0));
    line = buffer.readLine();
    QCOMPARE(line.size(), linelen);
#endif
}

QTEST_MAIN(tst_QIODevice)
#include "tst_qiodevice.moc"
