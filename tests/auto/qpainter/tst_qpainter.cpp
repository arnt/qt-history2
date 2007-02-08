/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qpainter.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qfontmetrics.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qprinter.h>
#include <limits.h>
#include <q3painter.h>
#include <qpaintengine.h>
#include <qdesktopwidget.h>
#include <qpixmap.h>

#include <qpainter.h>

#include <qlabel.h>

Q_DECLARE_METATYPE(QLine)
Q_DECLARE_METATYPE(QRect)
Q_DECLARE_METATYPE(QSize)
Q_DECLARE_METATYPE(QPoint)

//TESTED_CLASS=
//TESTED_FILES=gui/painting/qpainter.h gui/painting/qpainter.cpp

class tst_QPainter : public QObject
{
Q_OBJECT

public:
    tst_QPainter();
    virtual ~tst_QPainter();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void qt_format_text_clip();
    void qt_format_text_boundingRect();
    void drawPixmap_comp_data();
    void drawPixmap_comp();
    void saveAndRestore_data();
    void saveAndRestore();

    void drawLine_data();
    void drawLine();
    void drawLine_task121143();

    void drawRect_data() { fillData(); }
    void drawRect();

    void drawEllipse_data();
    void drawEllipse();
    void drawClippedEllipse_data();
    void drawClippedEllipse();

    void drawRoundRect_data() { fillData(); }
    void drawRoundRect();

    void qimageFormats_data();
    void qimageFormats();
    void textOnTransparentImage();

    void initFrom();

    void setWindow();

    void combinedMatrix();
    void renderHints();

private:
    void fillData();
    QColor baseColor( int k, int intensity=255 );
    QImage getResImage( const QString &dir, const QString &addition, const QString &extension );
    QBitmap getBitmap( const QString &dir, const QString &filename, bool mask );
};

// Testing get/set functions
void tst_QPainter::getSetCheck()
{
    QImage img(QSize(10, 10), QImage::Format_ARGB32_Premultiplied);
    QPainter obj1;
    obj1.begin(&img);
    // CompositionMode QPainter::compositionMode()
    // void QPainter::setCompositionMode(CompositionMode)
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_SourceOver));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_SourceOver), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_DestinationOver));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_DestinationOver), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_Clear));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_Clear), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_Source));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_Source), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_Destination));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_Destination), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_SourceIn));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_SourceIn), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_DestinationIn));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_DestinationIn), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_SourceOut));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_SourceOut), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_DestinationOut));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_DestinationOut), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_SourceAtop));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_SourceAtop), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_DestinationAtop));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_DestinationAtop), obj1.compositionMode());
    obj1.setCompositionMode(QPainter::CompositionMode(QPainter::CompositionMode_Xor));
    QCOMPARE(QPainter::CompositionMode(QPainter::CompositionMode_Xor), obj1.compositionMode());

    // const QFont & QPainter::font()
    // void QPainter::setFont(const QFont &)
    QFont var2("New Times Roman", 72);
    obj1.setFont(var2);
    QCOMPARE(var2, obj1.font());
    obj1.setFont(QFont());
    QCOMPARE(QFont(), obj1.font());

    // const QPen & QPainter::pen()
    // void QPainter::setPen(const QPen &)
    QPen var3(Qt::red);
    obj1.setPen(var3);
    QCOMPARE(var3, obj1.pen());
    obj1.setPen(QPen());
    QCOMPARE(QPen(), obj1.pen());

    // const QBrush & QPainter::brush()
    // void QPainter::setBrush(const QBrush &)
    QBrush var4(Qt::red);
    obj1.setBrush(var4);
    QCOMPARE(var4, obj1.brush());
    obj1.setBrush(QBrush());
    QCOMPARE(QBrush(), obj1.brush());

    // const QBrush & QPainter::background()
    // void QPainter::setBackground(const QBrush &)
    QBrush var5(Qt::yellow);
    obj1.setBackground(var5);
    QCOMPARE(var5, obj1.background());
    obj1.setBackground(QBrush());
    QCOMPARE(QBrush(), obj1.background());

    // bool QPainter::matrixEnabled()
    // void QPainter::setMatrixEnabled(bool)
    obj1.setMatrixEnabled(false);
    QCOMPARE(false, obj1.matrixEnabled());
    obj1.setMatrixEnabled(true);
    QCOMPARE(true, obj1.matrixEnabled());

    // bool QPainter::viewTransformEnabled()
    // void QPainter::setViewTransformEnabled(bool)
    obj1.setViewTransformEnabled(false);
    QCOMPARE(false, obj1.viewTransformEnabled());
    obj1.setViewTransformEnabled(true);
    QCOMPARE(true, obj1.viewTransformEnabled());
}

Q_DECLARE_METATYPE(QPixmap)
Q_DECLARE_METATYPE(QPolygon)
Q_DECLARE_METATYPE(QBrush)
Q_DECLARE_METATYPE(QPen)
Q_DECLARE_METATYPE(QFont)
Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(QRegion)

tst_QPainter::tst_QPainter()
{
    // QtTestCase sets this to false, but this turns off alpha pixmaps on Unix.
    QApplication::setDesktopSettingsAware(TRUE);
}

tst_QPainter::~tst_QPainter()
{
}

void tst_QPainter::init()
{
}

void tst_QPainter::cleanup()
{
}

/* tests the clipping operations in qt_format_text, making sure
   the clip rectangle after the call is the same as before
*/
void tst_QPainter::qt_format_text_clip()
{
    QVERIFY(1);
    QSKIP( "Needs fixing...", SkipAll);

    QWidget *w = new QWidget( 0 );

    int modes[] = { Qt::AlignVCenter|Qt::TextSingleLine,
		   Qt::AlignVCenter|Qt::TextSingleLine|Qt::TextDontClip,
		   Qt::AlignVCenter|Qt::TextWordWrap,
		   Qt::AlignVCenter|Qt::TextWordWrap|Qt::TextDontClip,
		   0
    };

    int *m = modes;
    while( *m ) {
	{
	    QPainter p( w );
	    QRegion clipreg = p.clipRegion();
	    bool hasClipping = p.hasClipping();
	    qreal tx = p.matrix().dx();
	    qreal ty = p.matrix().dy();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );

	    p.setClipRect( QRect( 5, 5, 50, 50 ) );
	    clipreg = p.clipRegion();
	    hasClipping = p.hasClipping();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );
	}
	{
	    QPainter p( w );
	    p.setMatrix( QMatrix( 2, 1, 3, 4, 5, 6 ) );
	    QRegion clipreg = p.clipRegion();
	    bool hasClipping = p.hasClipping();
	    qreal tx = p.matrix().dx();
	    qreal ty = p.matrix().dy();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );

	    p.setClipRect( QRect( 5, 5, 50, 50 ) );
	    clipreg = p.clipRegion();
	    hasClipping = p.hasClipping();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );
	}
	{
	    QPainter p( w );
	    QRegion clipreg = p.clipRegion();
	    bool hasClipping = p.hasClipping();
	    qreal tx = p.matrix().dx();
	    qreal ty = p.matrix().dy();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );

	    p.setClipRect( QRect( 5, 5, 50, 50 ));
	    clipreg = p.clipRegion();
	    hasClipping = p.hasClipping();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );
	}
	{
	    QPainter p( w );
	    p.setMatrix( QMatrix( 2, 1, 3, 4, 5, 6 ) );
	    QRegion clipreg = p.clipRegion();
	    bool hasClipping = p.hasClipping();
	    qreal tx = p.matrix().dx();
	    qreal ty = p.matrix().dy();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );

	    p.setClipRect(QRect( 5, 5, 50, 50 ));
	    clipreg = p.clipRegion();
	    hasClipping = p.hasClipping();

	    p.drawText( 10, 10, 100, 100, *m,
			"fooo" );

	    QVERIFY( clipreg == p.clipRegion() );
	    QVERIFY( hasClipping == p.hasClipping() );
	    QCOMPARE( tx, p.matrix().dx() );
	    QCOMPARE( ty, p.matrix().dy() );
	}
	++m;
    }
    delete w;
}

/* tests the bounding rect calculations in qt_format_text, making sure
   the bounding rect has a reasonable value.
*/
void tst_QPainter::qt_format_text_boundingRect()
{
    QVERIFY(1);
    QSKIP( "Needs fixing...", SkipAll);

    {
	const char * strings[] = {
	    "a\n\nb",
	    "abc",
	    "a\n \nb",
	    "this is a longer string",
	    "\327\222\327\223\327\233\327\223\327\222\327\233\327\222\327\223\327\233",
	    "aa\327\222\327\233aa",
	    "\327\222\327\223\327\233\327\223\327\222\327\233\327\222\327\223\327\233",
	    "\327\222\327\233aa",
	    "linebreakatend\n",
	    "some text longer than 30 chars with a line break at the end\n",
	    "some text\nwith line breaks\nin the middle\nand at the end\n",
	    "foo\n\n\nfoo",
	    0
	};

	int modes[] = { Qt::AlignVCenter|Qt::TextSingleLine,
			Qt::AlignVCenter|Qt::TextSingleLine|Qt::TextDontClip,
			Qt::AlignVCenter|Qt::TextWordWrap,
			Qt::AlignVCenter|Qt::TextWordWrap|Qt::TextDontClip,
			Qt::AlignLeft,
			Qt::AlignCenter,
			Qt::AlignRight,
			0
	};

	QFont f;
	for(int i = 5; i < 15; ++i) {
	    f.setPointSize(i);
	    QFontMetrics fm(f);
	    const char **str = strings;
	    while( *str ) {
		int *m = modes;
		while( *m ) {
		    QRect br = fm.boundingRect( 0, 0, 2000, 100, *m, QString::fromUtf8( *str ) );
		    QVERIFY( br.width() < 800 );

		    QRect br2 = fm.boundingRect( br.x(), br.y(), br.width(), br.height(), *m, QString::fromUtf8( *str ) );
		    QCOMPARE( br, br2 );
#if 0
		    {
			QPrinter printer;
			printer.setOutputToFile(TRUE);
			printer.setOutputFileName("tmp.prn");
			QPainter p(&printer);
			QRect pbr = p.fontMetrics().boundingRect( 0, 0, 2000, 100, *m, QString::fromUtf8( *str ) );
			QCOMPARE(pbr, br);
		    }
#endif
		    {
			QPrinter printer(QPrinter::HighResolution);
			if (printer.printerName().isEmpty()) {
			    QSKIP( "No printers installed, skipping bounding rect test",
				  SkipSingle );
			    break;
			}

			printer.setOutputToFile(TRUE);
			printer.setOutputFileName("tmp.prn");
			QPainter p(&printer);
			QRect pbr = p.fontMetrics().boundingRect( 0, 0, 12000, 600, *m, QString::fromUtf8( *str ) );
			QVERIFY(pbr.width() > 2*br.width());
			QVERIFY(pbr.height() > 2*br.height());
		    }
		    ++m;
		}
		++str;
	    }
	}
    }

    {
	const char * strings[] = {
	    "a",
	    "a\nb",
	    "a\n\nb",
	    "abc",
//	    "a\n \nb",
	    "this is a longer string",
//	    "\327\222\327\223\327\233\327\223\327\222\327\233\327\222\327\223\327\233",
//	    "aa\327\222\327\233aa",
//	    "\327\222\327\223\327\233\327\223\327\222\327\233\327\222\327\223\327\233",
//	    "\327\222\327\233aa",
//	    "linebreakatend\n",
//	    "some text longer than 30 chars with a line break at the end\n",
//	    "some text\nwith line breaks\nin the middle\nand at the end\n",
	    "foo\n\n\nfoo",
	    "a\n\n\n\n\nb",
	    "a\n\n\n\n\n\nb",
//	    "\347\231\273\351\214\262\346\203\205\345\240\261\343\201\214\350\246\213\343\201\244\343\201\213\343\202\211\343\201\252\343\201\204\343\201\213\347\204\241\345\212\271\343\201\252\343\201\237\343\202\201\343\200\201\nPhotoshop Album \343\202\222\350\265\267\345\213\225\343\201\247\343\201\215\343\201\276\343\201\233\343\202\223\343\200\202\345\206\215\343\202\244\343\203\263\343\202\271\343\203\210\343\203\274\343\203\253\343\201\227\343\201\246\343\201\217\343\201\240\343\201\225\343\201\204\343\200\202"
//	    "\347\231\273\351\214\262\346\203\205\345\240\261\343\201\214\350\246\213\343\201\244\343\201\213\343\202\211\343\201\252\343\201\204\343\201\213\347\204\241\345\212\271\343\201\252\343\201\237\343\202\201\343\200\201\n\343\202\222\350\265\267\345\213\225\343\201\247\343\201\215\343\201\276\343\201\233\343\202\223\343\200\202\345\206\215\343\202\244\343\203\263\343\202\271\343\203\210\343\203\274\343\203\253\343\201\227\343\201\246\343\201\217\343\201\240\343\201\225\343\201\204\343\200\202",
	    0
	};

	int modes[] = { Qt::AlignVCenter,
			Qt::AlignLeft,
			Qt::AlignCenter,
			Qt::AlignRight,
			0
	};


	QFont f;
	for(int i = 5; i < 15; ++i) {
	    f.setPointSize(i);
	    QFontMetrics fm(f);
	    const char **str = strings;
	    while( *str ) {
		int *m = modes;
		while( *m ) {
		    QString s = QString::fromUtf8(*str);
		    QRect br = fm.boundingRect(0, 0, 1000, 1000, *m, s );
		    int lines =
		    s.count("\n");
		    int expectedHeight = fm.height()+lines*fm.lineSpacing();
		    QCOMPARE(br.height(), expectedHeight);
		    ++m;
		}
		++str;
	    }
	    QRect br = fm.boundingRect(0, 0, 100, 0, Qt::TextWordWrap,
		 "A paragraph with gggggggggggggggggggggggggggggggggggg in the middle.");
	    QVERIFY(br.height() >= fm.height()+2*fm.lineSpacing());
	}
    }
}


static const char* const maskSource_data[] = {
"16 13 6 1",
". c None",
"d c #000000",
"# c #999999",
"c c #cccccc",
"b c #ffff00",
"a c #ffffff",
"...#####........",
"..#aaaaa#.......",
".#abcbcba######.",
".#acbcbcaaaaaa#d",
".#abcbcbcbcbcb#d",
"#############b#d",
"#aaaaaaaaaa##c#d",
"#abcbcbcbcbbd##d",
".#abcbcbcbcbcd#d",
".#acbcbcbcbcbd#d",
"..#acbcbcbcbb#dd",
"..#############d",
"...ddddddddddddd"};

static const char* const maskResult_data[] = {
"16 13 6 1",
". c #ff0000",
"d c #000000",
"# c #999999",
"c c #cccccc",
"b c #ffff00",
"a c #ffffff",
"...#####........",
"..#aaaaa#.......",
".#abcbcba######.",
".#acbcbcaaaaaa#d",
".#abcbcbcbcbcb#d",
"#############b#d",
"#aaaaaaaaaa##c#d",
"#abcbcbcbcbbd##d",
".#abcbcbcbcbcd#d",
".#acbcbcbcbcbd#d",
"..#acbcbcbcbb#dd",
"..#############d",
"...ddddddddddddd"};


void tst_QPainter::drawPixmap_comp_data()
{
    if (qApp->desktop()->depth() < 24) {
        QSKIP("Test only works on 32 bit displays", SkipAll);
        return;
    }

    QTest::addColumn<uint>("dest");
    QTest::addColumn<uint>("source");

    QTest::newRow("0% on 0%, 1")           << 0x00000000u<< 0x00000000u;
    QTest::newRow("0% on 0%, 2")           << 0x00007fffu << 0x00ff007fu;

    QTest::newRow("50% on a=0%")           << 0x00000000u << 0x7fff0000u;
    QTest::newRow("50% on a=50%")          << 0x7f000000u << 0x7fff0000u;
    QTest::newRow("50% on deadbeef")      << 0xdeafbeefu <<  0x7fff0000u;
    QTest::newRow("deadbeef on a=0%")      << 0x00000000u << 0xdeadbeefu;
    QTest::newRow("deadbeef on a=50%")     << 0x7f000000u << 0xdeadbeefu;
    QTest::newRow("50% blue on 50% red")   << 0x7fff0000u << 0x7f0000ffu;
    QTest::newRow("50% blue on 50% green") << 0x7f00ff00u << 0x7f0000ffu;
    QTest::newRow("50% red on 50% green")  << 0x7f00ff00u << 0x7fff0000u;
    QTest::newRow("0% on 50%")             << 0x7fff00ffu << 0x00ffffffu;
    QTest::newRow("100% on deadbeef")      << 0xdeafbeefu << 0xffabcdefu;
    QTest::newRow("100% on a=0%")           << 0x00000000u << 0xffabcdefu;
}

QRgb qt_compose_alpha(QRgb source, QRgb dest)
{
    int r1 = qRed(dest), g1 = qGreen(dest), b1 = qBlue(dest), a1 = qAlpha(dest);
    int r2 = qRed(source), g2 = qGreen(source), b2 = qBlue(source), a2 = qAlpha(source);

    int alpha = qMin(a2 + ((255 - a2) * a1 + 127) / 255, 255);
    if (alpha == 0)
        return qRgba(0, 0, 0, 0);

    return qRgba(
        qMin((r2 * a2 + (255 - a2) * r1 * a1 / 255) / alpha, 255),
        qMin((g2 * a2 + (255 - a2) * g1 * a1 / 255) / alpha, 255),
        qMin((b2 * a2 + (255 - a2) * b1 * a1 / 255) / alpha, 255),
        alpha);
}

/* Tests that drawing masked pixmaps works
*/
void tst_QPainter::drawPixmap_comp()
{
#ifdef Q_WS_MAC
    QSKIP("Mac has other ideas about alpha composition", SkipAll);
#endif

    QFETCH(uint, dest);
    QFETCH(uint, source);

    QRgb expected = qt_compose_alpha(source, dest);

    QColor c1(qRed(dest), qGreen(dest), qBlue(dest), qAlpha(dest));
    QColor c2(qRed(source), qGreen(source), qBlue(source), qAlpha(source));

    QPixmap destPm(10, 10), srcPm(10, 10);
    destPm.fill(c1);
    srcPm.fill(c2);

#if defined(Q_WS_X11)
    if (!destPm.x11PictureHandle())
        QSKIP("Requires XRender support", SkipAll);
#endif

    QPainter p(&destPm);
    p.drawPixmap(0, 0, srcPm);
    p.end();

    QImage result = destPm.toImage().convertToFormat(QImage::Format_ARGB32);
    bool different = false;
    for (int y=0; y<result.height(); ++y)
        for (int x=0; x<result.width(); ++x) {
	    bool diff;
            if (qAlpha(expected) == 0) {
                diff = qAlpha(result.pixel(x, y)) != 0;
            } else {
                // Compensate for possible roundoff / platform fudge
                int off = 1;
                QRgb pix = result.pixel(x, y);
                diff = (qAbs(qRed(pix) - qRed(expected)) > off)
                             || (qAbs(qGreen(pix) - qGreen(expected)) > off)
                             || (qAbs(qBlue(pix) - qBlue(expected)) > off)
                             || (qAbs(qAlpha(pix) - qAlpha(expected)) > off);
            }
	    if (diff && !different)
		qDebug( "Different at %d,%d pixel [%d,%d,%d,%d] expected [%d,%d,%d,%d]", x, y,
                        qRed(result.pixel(x, y)), qGreen(result.pixel(x, y)),
                        qBlue(result.pixel(x, y)), qAlpha(result.pixel(x, y)),
                        qRed(expected), qGreen(expected), qBlue(expected), qAlpha(expected));
	    different |= diff;
        }

    QVERIFY(!different);
}

void tst_QPainter::saveAndRestore_data()
{
    QVERIFY(1);

    QTest::addColumn<QFont>("font");
    QTest::addColumn<QPen>("pen");
    QTest::addColumn<QBrush>("brush");
    QTest::addColumn<QColor>("backgroundColor");
    QTest::addColumn<int>("backgroundMode");
    QTest::addColumn<QPoint>("brushOrigin");
    QTest::addColumn<QRegion>("clipRegion");
    QTest::addColumn<QRect>("window");
    QTest::addColumn<QRect>("viewport");

    QPixmap pixmap(1, 1);
    QPainter p(&pixmap);
    QFont font = p.font();
    QPen pen = p.pen();
    QBrush brush = p.brush();
    QColor backgroundColor = p.background().color();
    Qt::BGMode backgroundMode = p.backgroundMode();
    QPoint brushOrigin = p.brushOrigin();
    QRegion clipRegion = p.clipRegion();
    QRect window = p.window();
    QRect viewport = p.viewport();

    QTest::newRow("Original") << font << pen << brush << backgroundColor << int(backgroundMode)
	    << brushOrigin << clipRegion << window << viewport;

    QFont font2 = font;
    font2.setPointSize( 24 );
    QTest::newRow("Modified font.pointSize, brush, backgroundColor, backgroundMode")
            << font2 << pen << QBrush(Qt::red) << QColor(Qt::blue) << int(Qt::TransparentMode)
	    << brushOrigin << clipRegion << window << viewport;

    font2 = font;
    font2.setPixelSize( 20 );
    QTest::newRow("Modified font.pixelSize, brushOrigin, pos")
            << font2 << pen << brush << backgroundColor << int(backgroundMode)
	    << QPoint( 50, 32 ) << clipRegion << window << viewport;

    QTest::newRow("Modified clipRegion, window, viewport")
            << font << pen << brush << backgroundColor << int(backgroundMode)
	    << brushOrigin << clipRegion.subtracted(QRect(10,10,50,30))
	    << QRect(-500, -500, 500, 500 ) << QRect( 0, 0, 50, 50 );
}

void tst_QPainter::saveAndRestore()
{
    QFETCH( QFont, font );
    QFETCH( QPen, pen );
    QFETCH( QBrush, brush );
    QFETCH( QColor, backgroundColor );
    QFETCH( int, backgroundMode );
    QFETCH( QPoint, brushOrigin );
    QFETCH( QRegion, clipRegion );
    QFETCH( QRect, window );
    QFETCH( QRect, viewport );

    QPixmap pixmap(1, 1);
    QPainter painter(&pixmap);

    QFont font_org = painter.font();
    QPen pen_org = painter.pen();
    QBrush brush_org = painter.brush();
    QColor backgroundColor_org = painter.background().color();
    Qt::BGMode backgroundMode_org = painter.backgroundMode();
    QPoint brushOrigin_org = painter.brushOrigin();
    QRegion clipRegion_org = painter.clipRegion();
    QRect window_org = painter.window();
    QRect viewport_org = painter.viewport();

    painter.save();
    painter.setFont( font );
    painter.setPen( QPen(pen) );
    painter.setBrush( brush );
    painter.setBackground( backgroundColor );
    painter.setBackgroundMode( (Qt::BGMode)backgroundMode );
    painter.setBrushOrigin( brushOrigin );
    painter.setClipRegion( clipRegion );
    painter.setWindow( window );
    painter.setViewport( viewport );
    painter.restore();

    QCOMPARE( painter.font(), font_org );
    QCOMPARE( painter.font().pointSize(), font_org.pointSize() );
    QCOMPARE( painter.font().pixelSize(), font_org.pixelSize() );
    QCOMPARE( painter.pen(), pen_org );
    QCOMPARE( painter.brush(), brush_org );
    QCOMPARE( painter.background().color(), backgroundColor_org );
    QCOMPARE( painter.backgroundMode(), backgroundMode_org );
    QCOMPARE( painter.brushOrigin(), brushOrigin_org );
    QCOMPARE( painter.clipRegion(), clipRegion_org );
    QCOMPARE( painter.window(), window_org );
    QCOMPARE( painter.viewport(), viewport_org );
}

/*
   Helper functions
*/

QColor tst_QPainter::baseColor( int k, int intensity )
{
    int r = ( k & 1 ) * intensity;
    int g = ( (k>>1) & 1 ) * intensity;
    int b = ( (k>>2) & 1 ) * intensity;
    return QColor( r, g, b );
}

QImage tst_QPainter::getResImage( const QString &dir, const QString &addition, const QString &extension )
{
    QImage res;
    QString resFilename  = dir + QString( "/res_%1." ).arg( addition ) + extension;
    if ( !res.load( resFilename ) ) {
	QWARN( "Could not load result data " + resFilename );
	return QImage();
    }
    return res;
}

QBitmap tst_QPainter::getBitmap( const QString &dir, const QString &filename, bool mask )
{
    QBitmap bm;
    QString bmFilename = dir + QString( "/%1.xbm" ).arg( filename );
    if ( !bm.load( bmFilename ) ) {
	QWARN( QString("Could not load bitmap '%1'").arg(bmFilename) );
	return QBitmap();
    }
    if ( mask ) {
	QBitmap mask;
	QString maskFilename = dir + QString( "/%1-mask.xbm" ).arg( filename );
	if ( !mask.load( maskFilename ) ) {
	    QWARN( QString("Could not load mask '%1'").arg(maskFilename) );
	    return QBitmap();
	}
	bm.setMask( mask );
    }
    return bm;
}

static QRect getPaintedSize(const QPixmap &pm, const QColor &background)
{
    // not the fastest but at least it works..
    QImage image = pm.toImage();
    int xmin = image.width() + 1;
    int xmax = -1;
    int ymin = image.height() +1;
    int ymax = -1;

    uint color = background.rgba();

    for ( int y = 0; y < image.height(); ++y ) {
	for ( int x = 0; x < image.width(); ++x ) {
	    if ( image.pixel( x, y ) != color && x < xmin )
		xmin = x;
	    if ( image.pixel( x, y ) != color && x > xmax )
		xmax = x;
	    if ( image.pixel( x, y ) != color && y < ymin )
		ymin = y;
	    if ( image.pixel( x, y ) != color && y > ymax )
		ymax = y;
	}
    }

    return QRect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
}

void tst_QPainter::initFrom()
{
    QWidget *widget = new QWidget();
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Foreground, QColor(255, 0, 0));
    pal.setBrush(QPalette::Background, QColor(0, 255, 0));
    widget->setPalette(pal);

    QFont font = widget->font();
    font.setPointSize(26);
    font.setItalic(true);
    widget->setFont(font);

    QPixmap pm(100, 100);
    QPainter p(&pm);
    p.initFrom(widget);

    QCOMPARE(p.font(), font);
    QCOMPARE(p.pen().color(), pal.color(QPalette::Foreground));
    QCOMPARE(p.background(), pal.background());

    delete widget;
}

void tst_QPainter::drawLine_data()
{
    QTest::addColumn<QLine>("line");

    QTest::newRow("0-45") << QLine(0, 20, 100, 0);
    QTest::newRow("45-90") << QLine(0, 100, 20, 0);
    QTest::newRow("90-135") << QLine(20, 100, 0, 0);
    QTest::newRow("135-180") << QLine(100, 20, 0, 0);
    QTest::newRow("180-225") << QLine(100, 0, 0, 20);
    QTest::newRow("225-270") << QLine(20, 0, 0, 100);
    QTest::newRow("270-315") << QLine(0, 0, 20, 100);
    QTest::newRow("315-360") << QLine(0, 0, 100, 20);
}

void tst_QPainter::drawLine()
{
    const int offset = 5;
    const int epsilon = 1; // allow for one pixel difference

    QFETCH(QLine, line);

    QPixmap pixmapUnclipped(qMin(line.x1(), line.x2())
                            + 2*offset + qAbs(line.dx()),
                            qMin(line.y1(), line.y2())
                            + 2*offset + qAbs(line.dy()));

    { // unclipped
        pixmapUnclipped.fill(Qt::white);
        QPainter p(&pixmapUnclipped);
        p.translate(offset, offset);
        p.setPen(QPen(Qt::black));
        p.drawLine(line);
        p.end();

        const QRect painted = getPaintedSize(pixmapUnclipped, Qt::white);

        QLine l = line;
        l.translate(offset, offset);
        QVERIFY(qAbs(painted.width() - qAbs(l.dx())) <= epsilon);
        QVERIFY(qAbs(painted.height() - qAbs(l.dy())) <= epsilon);
        QVERIFY(qAbs(painted.top() - qMin(l.y1(), l.y2())) <= epsilon);
        QVERIFY(qAbs(painted.left() - qMin(l.x1(), l.x2())) <= epsilon);
        QVERIFY(qAbs(painted.bottom() - qMax(l.y1(), l.y2())) <= epsilon);
        QVERIFY(qAbs(painted.right() - qMax(l.x1(), l.x2())) <= epsilon);
    }

    QPixmap pixmapClipped(qMin(line.x1(), line.x2())
                          + 2*offset + qAbs(line.dx()),
                          qMin(line.y1(), line.y2())
                          + 2*offset + qAbs(line.dy()));
    { // clipped
        const QRect clip = QRect(line.p1(), line.p2()).normalized();

        pixmapClipped.fill(Qt::white);
        QPainter p(&pixmapClipped);
        p.translate(offset, offset);
        p.setClipRect(clip);
        p.setPen(QPen(Qt::black));
        p.drawLine(line);
        p.end();
    }

    const QImage unclipped = pixmapUnclipped.toImage();
    const QImage clipped = pixmapClipped.toImage();
    QCOMPARE(unclipped, clipped);
}

void tst_QPainter::drawLine_task121143()
{
    QPen pen(Qt::black);

    QImage image(5, 5, QImage::Format_ARGB32_Premultiplied);
    image.fill(0xffffffff);
    QPainter p(&image);
    p.setPen(pen);
    p.drawLine(QLine(0, 0+4, 0+4, 0));
    p.end();

    QImage expected(5, 5, QImage::Format_ARGB32_Premultiplied);
    expected.fill(0xffffffff);
    for (int x = 0; x < 5; ++x)
        expected.setPixel(x, 5-x-1, pen.color().rgb());

    QCOMPARE(image, expected);
}

void tst_QPainter::drawRect()
{
    QFETCH(QRect, rect);
    QFETCH(bool, usePen);

    QPixmap pixmap(rect.x() + rect.width() + 10,
                   rect.y() + rect.height() + 10);
    {
        pixmap.fill(Qt::white);
        QPainter p(&pixmap);
        p.setPen(usePen ? QPen(Qt::black) : QPen(Qt::NoPen));
        p.setBrush(Qt::black);
        p.drawRect(rect);
        p.end();

        int increment = usePen ? 1 : 0;

        const QRect painted = getPaintedSize(pixmap, Qt::white);
        QCOMPARE(painted.width(), rect.width() + increment);
        QCOMPARE(painted.height(), rect.height() + increment);
    }

    {
        if (usePen && (rect.width() < 2 || rect.height() < 2))
            return;
        pixmap.fill(Qt::white);
        Q3Painter p(&pixmap);
        p.setPen(usePen ? QPen(Qt::black) : QPen(Qt::NoPen));
        p.setBrush(Qt::black);
        p.drawRect(rect);
        p.end();

        const QRect painted = getPaintedSize(pixmap, Qt::white);

        QCOMPARE(painted.width(), rect.width());
        QCOMPARE(painted.height(), rect.height());
    }
}

void tst_QPainter::drawEllipse_data()
{
    QTest::addColumn<QSize>("size");
    QTest::addColumn<bool>("usePen");

    // The current drawEllipse algorithm (drawEllipse_midpoint_i in
    // qpaintengine_raster.cpp) draws ellipses that are too wide if the
    // ratio between width and hight is too large/small (task 114874). Those
    // ratios are therefore currently avoided.
    for (int w = 10; w < 128; w += 7) {
        for (int h = w/2; h < qMin(2*w, 128); h += 13) {
            QString s = QString("%1x%2").arg(w).arg(h);
            QTest::newRow(s + " with pen") << QSize(w, h) << true;
            QTest::newRow(s + " no pen") << QSize(w, h) << false;
        }
    }
}

void tst_QPainter::drawEllipse()
{
    QFETCH(QSize, size);
    QFETCH(bool, usePen);

    const int offset = 10;
    QRect rect(QPoint(offset, offset), size);

    QImage image(size.width() + 2 * offset, size.height() + 2 * offset,
                 QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(Qt::white).rgb());

    QPainter p(&image);
    p.setPen(usePen ? QPen(Qt::black) : QPen(Qt::NoPen));
    p.setBrush(Qt::black);
    p.drawEllipse(rect);
    p.end();

    QPixmap pixmap = QPixmap::fromImage(image);

    const QRect painted = getPaintedSize(pixmap, Qt::white);

    QCOMPARE(painted.x(), rect.x());
    QCOMPARE(painted.y(), rect.y() + (usePen ? 0 : 1));
    QCOMPARE(painted.width(), size.width() + (usePen ? 1 : 0));
    QCOMPARE(painted.height(), size.height() + (usePen ? 1 : -1));
}

void tst_QPainter::drawClippedEllipse_data()
{
    QTest::addColumn<QRect>("rect");

    for (int w = 20; w < 128; w += 7) {
        for (int h = w/2; h < qMin(2*w, 128); h += 13) {
            QString s = QString("%1x%2").arg(w).arg(h);
            QTest::newRow(s + " top") << QRect(0, -h/2, w, h);
            QTest::newRow(s + " topright") << QRect(w/2, -h/2, w, h);
            QTest::newRow(s + " right") << QRect(w/2, 0, w, h);
            QTest::newRow(s + " bottomright") << QRect(w/2, h/2, w, h);
            QTest::newRow(s + " bottom") << QRect(0, h/2, w, h);
            QTest::newRow(s + " bottomleft") << QRect(-w/2, h/2, w, h);
            QTest::newRow(s + " left") << QRect(-w/2, 0, w, h);
            QTest::newRow(s + " topleft") << QRect(-w/2, -h/2, w, h);
        }
    }
}

void tst_QPainter::drawClippedEllipse()
{
    QFETCH(QRect, rect);

    QImage image(rect.width() + 1, rect.height() + 1,
                 QImage::Format_ARGB32_Premultiplied);
    QRect expected = QRect(rect.x(), rect.y(), rect.width()+1, rect.height()+1)
                     & QRect(0, 0, image.width(), image.height());


    image.fill(QColor(Qt::white).rgb());
    QPainter p(&image);
    p.drawEllipse(rect);
    p.end();

    QPixmap pixmap = QPixmap::fromImage(image);
    const QRect painted = getPaintedSize(pixmap, Qt::white);

    QCOMPARE(painted.x(), expected.x());
    QCOMPARE(painted.y(), expected.y());
    QCOMPARE(painted.width(), expected.width());
    QCOMPARE(painted.height(), expected.height());

}

void tst_QPainter::drawRoundRect()
{
    QFETCH(QRect, rect);
    QFETCH(bool, usePen);

    QPixmap pixmap(rect.x() + rect.width() + 10,
                   rect.y() + rect.height() + 10);
    {
        pixmap.fill(Qt::white);
        QPainter p(&pixmap);
        p.setPen(usePen ? QPen(Qt::black) : QPen(Qt::NoPen));
        p.setBrush(Qt::black);
        p.drawRoundRect(rect);
        p.end();

        int increment = usePen ? 1 : 0;

        const QRect painted = getPaintedSize(pixmap, Qt::white);
        QCOMPARE(painted.width(), rect.width() + increment);
        QCOMPARE(painted.height(), rect.height() + increment);
    }

    {
        pixmap.fill(Qt::white);
        Q3Painter p(&pixmap);
        p.setPen(usePen ? QPen(Qt::black) : QPen(Qt::NoPen));
        p.setBrush(Qt::black);
        p.drawRoundRect(rect);
        p.end();

        const QRect painted = getPaintedSize(pixmap, Qt::white);

        QCOMPARE(painted.width(), rect.width());
        QCOMPARE(painted.height(), rect.height());
    }
}

Q_DECLARE_METATYPE(QImage::Format)

void tst_QPainter::qimageFormats_data()
{
    QTest::addColumn<QImage::Format>("format");
    QTest::newRow("QImage::Format_RGB32") << QImage::Format_RGB32;
    QTest::newRow("QImage::Format_ARGB32") << QImage::Format_ARGB32;
    QTest::newRow("QImage::Format_ARGB32_Premultiplied") << QImage::Format_ARGB32_Premultiplied;
}

/*
    Tests that QPainter can paint on various QImage formats.
*/
void tst_QPainter::qimageFormats()
{
    QFETCH(QImage::Format, format);

    const QSize size(100, 100);
    QImage image(size, format);
    image.fill(0);

    const QColor testColor(Qt::red);
    QPainter p(&image);
    QVERIFY(p.isActive());
    p.setBrush(QBrush(testColor));
    p.drawRect(QRect(QPoint(0,0), size));
    QCOMPARE(image.pixel(50, 50), testColor.rgb());
}

void tst_QPainter::fillData()
{
    QTest::addColumn<QRect>("rect");
    QTest::addColumn<bool>("usePen");

    for (int w = 3; w < 100; w += 7) {
        for (int h = 3; h < 100; h += 11) {
            int x = w/2 + 5;
            int y = h/2 + 5;
            QTest::newRow(QString("rect(%1, %2, %3, %4) with pen").arg(x).arg(y).arg(w).arg(h))
                << QRect(x, y, w, h) << true;
            QTest::newRow(QString("rect(%1, %2, %3, %4) no pen").arg(x).arg(y).arg(w).arg(h))
                << QRect(x, y, w, h) << false;
        }
    }
}

/*
    Test that drawline works properly after setWindow has been called.
*/
void tst_QPainter::setWindow()
{
    QPixmap pixmap(600, 600);
    pixmap.fill(QColor(Qt::white));

    QPainter painter(&pixmap);
    painter.setWindow(0, 0, 3, 3);
    painter.drawLine(1, 1, 2, 2);

    const QRect painted = getPaintedSize(pixmap, Qt::white);
    QVERIFY(195 < painted.y() && painted.y() < 205); // correct value is around 200
    QVERIFY(195 < painted.height() && painted.height() < 205); // correct value is around 200
}

void tst_QPainter::combinedMatrix()
{
    QPixmap pm(64, 64);

    QPainter p(&pm);
    p.setWindow(0, 0, 1, 1);
    p.setViewport(32, 0, 32, 32);

    p.translate(0.5, 0.5);

    QMatrix cm = p.combinedMatrix();

    QPointF pt = QPointF(0, 0) * cm;

    QCOMPARE(pt.x(), 48.0);
    QCOMPARE(pt.y(), 16.0);
}

void tst_QPainter::textOnTransparentImage()
{
    bool foundPixel = false;
    QImage image(10, 10, QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(0, 0, 0, 0)); // transparent
    {
        QPainter painter(&image);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(10, 10, "W");
    }
    for (int x = 0; x < image.width(); ++x)
        for (int y = 0; y < image.height(); ++y)
            if (image.pixel(x, y) != 0)
                foundPixel = true;
    QVERIFY(foundPixel);
}

void tst_QPainter::renderHints()
{
    QImage img(1, 1, QImage::Format_RGB32);

    QPainter p(&img);

    // Turn off all...
    p.setRenderHints(QPainter::RenderHints(0xffffffff), false);
    QCOMPARE(p.renderHints(), QPainter::RenderHints(0));

    // Single set/get
    p.setRenderHint(QPainter::Antialiasing);
    QVERIFY(p.renderHints() & QPainter::Antialiasing);

    p.setRenderHint(QPainter::Antialiasing, false);
    QVERIFY(!(p.renderHints() & QPainter::Antialiasing));

    // Multi set/get
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QVERIFY(p.renderHints() & (QPainter::Antialiasing | QPainter::SmoothPixmapTransform));

    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, false);
    QVERIFY(!(p.renderHints() & (QPainter::Antialiasing | QPainter::SmoothPixmapTransform)));
}

QTEST_MAIN(tst_QPainter)
#include "tst_qpainter.moc"
