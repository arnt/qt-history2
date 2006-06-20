/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qpicture.h>
#include <qpainter.h>
#include <qimage.h>
#include <qdesktopwidget.h>
#include <qapplication.h>

//TESTED_CLASS=
//TESTED_FILES=gui/image/qpicture.h gui/image/qpicture.cpp

class tst_QPicture : public QObject
{
    Q_OBJECT

public:
    tst_QPicture();

private slots:
    void getSetCheck();
    void devType();
    void paintingActive();
    void boundingRect();
    void operator_lt_lt();

    void save_restore();
};

// Testing get/set functions
void tst_QPicture::getSetCheck()
{
    QPictureIO obj1;
    // const QPicture & QPictureIO::picture()
    // void QPictureIO::setPicture(const QPicture &)
#if QT_VERSION >= 0x040200
    // Compare operator for QPicture will be added for Qt 4.2
    QPicture var1;
    obj1.setPicture(var1);
    QCOMPARE(var1, obj1.picture());
    obj1.setPicture(QPicture());
    QCOMPARE(QPicture(), obj1.picture());
#endif
    // const char * QPictureIO::format()
    // void QPictureIO::setFormat(const char *)
    char *var2 = "PNG";
    obj1.setFormat(var2);
    QCOMPARE(var2, obj1.format());
    obj1.setFormat((char *)0);
    // The format is stored internally in a QString, so return is always a valid char *
    QVERIFY(QString(obj1.format()).isEmpty());

    // const char * QPictureIO::parameters()
    // void QPictureIO::setParameters(const char *)
    char *var3 = "Bogus data";
    obj1.setParameters(var3);
    QCOMPARE(var3, obj1.parameters());
    obj1.setParameters((char *)0);
    // The format is stored internally in a QString, so return is always a valid char *
    QVERIFY(QString(obj1.parameters()).isEmpty());
}

tst_QPicture::tst_QPicture()
{
}

void tst_QPicture::devType()
{
    QPicture p;
    QCOMPARE( p.devType(), (int)QInternal::Picture );
}

void tst_QPicture::paintingActive()
{
    // actually implemented in QPainter but QPicture is a good
    // example of an external paint device
    QPicture p;
    QVERIFY( !p.paintingActive() );
    QPainter pt( &p );
    QVERIFY( p.paintingActive() );
    pt.end();
    QVERIFY( !p.paintingActive() );
}

void tst_QPicture::boundingRect()
{
    QPicture p1;
    // default value
    QVERIFY( !p1.boundingRect().isValid() );

#if QT_VERSION >= 0x030100
    QRect r1( 20, 30, 5, 15 );
    p1.setBoundingRect( r1 );
    QCOMPARE( p1.boundingRect(), r1 );
    p1.setBoundingRect(QRect());
#endif

    QPainter pt( &p1 );
    pt.drawLine( 10, 20, 110, 80 );
    pt.end();

    // assignment and copy constructor
    QRect r2( 10, 20, 100, 60 );
    QCOMPARE( p1.boundingRect(), r2 );
    QPicture p2( p1 );
    QCOMPARE( p1.boundingRect(), r2 );
    QPicture p3;
    p3 = p1;
    QCOMPARE( p1.boundingRect(), r2 );

    {
        QPicture p4;
        QPainter p(&p4);
        p.drawLine(0, 0, 5, 0);
        p.drawLine(0, 0, 0, 5);
        p.end();

        QRect r3(0, 0, 5, 5);
        QCOMPARE(p4.boundingRect(), r3);
    }
}

// operator<< and operator>>
void tst_QPicture::operator_lt_lt()
{
    // streaming of null pictures
    {
	QPicture pic1, pic2;
	QByteArray ba( 100, 0 );
	QDataStream str1( &ba, QIODevice::WriteOnly );
	str1 << pic1;
	QDataStream str2( &ba, QIODevice::ReadOnly );
	str2 >> pic2;
 	QVERIFY( pic2.isNull() );
    }

    // picture with a simple line, checking bitwise equality
    {
	QPicture pic1, pic2;
	QPainter p( &pic1 );
	p.drawLine( 10, 20, 30, 40 );
	p.end();
	QByteArray ba( 10 * pic1.size(), 0 );
	QDataStream str1( &ba, QIODevice::WriteOnly );
	str1 << pic1;
	QDataStream str2( &ba, QIODevice::ReadOnly );
	str2 >> pic2;
	QCOMPARE( pic1.size(), pic2.size() );
	QVERIFY( memcmp( pic1.data(), pic2.data(), pic1.size() ) == 0 );
    }
}

static QPointF scalePoint(const QPointF &point, QPaintDevice *sourceDevice, QPaintDevice *destDevice)
{
    return QPointF(point.x() * qreal(destDevice->logicalDpiX()) / qreal(sourceDevice->logicalDpiX()),
                   point.y() * qreal(destDevice->logicalDpiY()) / qreal(sourceDevice->logicalDpiY()));
}

static QRectF scaleRect(const QRectF &rect, QPaintDevice *sourceDevice, QPaintDevice *destDevice)
{
    return QRectF(rect.left() * qreal(destDevice->logicalDpiX()) / qreal(sourceDevice->logicalDpiX()),
                  rect.top() * qreal(destDevice->logicalDpiY()) / qreal(sourceDevice->logicalDpiY()),
                  rect.width() * qreal(destDevice->logicalDpiX()) / qreal(sourceDevice->logicalDpiX()),
                  rect.height() * qreal(destDevice->logicalDpiY()) / qreal(sourceDevice->logicalDpiY()));
}

static void paintStuff(QPainter *p)
{
    QPaintDevice *screenDevice = QApplication::desktop();
    p->drawRect(scaleRect(QRectF(100, 100, 100, 100), screenDevice, p->device()));
    p->save();
    p->translate(scalePoint(QPointF(10, 10), screenDevice, p->device()));
    p->restore();
    p->drawRect(scaleRect(QRectF(100, 100, 100, 100), screenDevice, p->device()));
}

/* See task: 41469
   Problem is that the state is not properly restored if the basestate of
   the painter is different when the picture data was created compared to
   the base state of the painter when it is played back.
 */
void tst_QPicture::save_restore()
{
    QPicture pic;
    QPainter p;
    p.begin(&pic);
    paintStuff(&p);
    p.end();

    QPixmap pix1(300, 300);
    pix1.fill(Qt::white);
    p.begin(&pix1);
    p.drawPicture(50, 50, pic);
    p.end();

    QPixmap pix2(300, 300);
    pix2.fill(Qt::white);
    p.begin(&pix2);
    p.translate(50, 50);
    paintStuff(&p);
    p.end();

    QVERIFY( pix1.toImage() == pix2.toImage() );
}

QTEST_MAIN(tst_QPicture)
#include "tst_qpicture.moc"
