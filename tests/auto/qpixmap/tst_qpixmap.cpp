/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qmatrix.h>
#include <qdesktopwidget.h>
#include <qpaintengine.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#ifdef Q_WS_QWS
#include <qscreen_qws.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=gui/image/qpixmap.h gui/image/qpixmap.cpp

class tst_QPixmap : public QObject
{
    Q_OBJECT

public:
    tst_QPixmap();
    virtual ~tst_QPixmap();


public slots:
    void init();
    void cleanup();
private slots:
    void convertFromImage_data();
    void convertFromImage();

    void testMetrics();

    void fill_data();
    void fill();

    void createMaskFromColor();

    void mask();
    void bitmapMask();

    void cacheKey();

#ifdef Q_WS_WIN
    void toWinHBITMAP_data();
    void toWinHBITMAP();
    void fromWinHBITMAP_data();
    void fromWinHBITMAP();
#endif
};

Q_DECLARE_METATYPE(QImage)
Q_DECLARE_METATYPE(QPixmap)
Q_DECLARE_METATYPE(QMatrix)
Q_DECLARE_METATYPE(QBitmap)

tst_QPixmap::tst_QPixmap()
{
}

tst_QPixmap::~tst_QPixmap()
{
}

void tst_QPixmap::init()
{
}

void tst_QPixmap::cleanup()
{
}

void tst_QPixmap::convertFromImage_data()
{
    QTest::addColumn<QImage>("img1");
    QTest::addColumn<QImage>("img2");

    const QString prefix = QLatin1String(SRCDIR) + "/convertFromImage";
    {
        QImage img1;
        QImage img2;
	QVERIFY(img1.load("convertFromImage/task31722_0/img1.png"));
	QVERIFY(img2.load("convertFromImage/task31722_0/img2.png"));
	QVERIFY(img1.load(prefix + "/task31722_0/img1.png"));
	QVERIFY(img2.load(prefix + "/task31722_0/img2.png"));
        QTest::newRow("Task 31722 0") << img1 << img2;
    }
    {
	QImage img1;
	QImage img2;
	QVERIFY(img1.load(prefix + "/task31722_1/img1.png"));
	QVERIFY(img2.load(prefix + "/task31722_1/img2.png"));
	QTest::newRow("Task 31722 1") << img1 << img2;
    }
}

void tst_QPixmap::convertFromImage()
{
    QFETCH(QImage, img1);
    QFETCH(QImage, img2);

    QPixmap pix = QPixmap::fromImage(img1);
    pix = QPixmap::fromImage(img2);

    QPixmap res = QPixmap::fromImage(img2);
    QVERIFY( pixmapsAreEqual(&pix, &res) );
}

void tst_QPixmap::fill_data()
{
    QTest::addColumn<uint>("pixel");
    QTest::addColumn<bool>("syscolor");
    QTest::addColumn<bool>("bitmap");
    for (int color = Qt::black; color < Qt::darkYellow; ++color)
        QTest::newRow(QString("syscolor_%1").arg(color).toLatin1())
            << uint(color) << true << false;

#ifndef Q_WS_QWS
    if (qApp->desktop()->paintEngine()->hasFeature(QPaintEngine::AlphaBlend)) {
        QTest::newRow("alpha_7f_red")   << 0x7fff0000u << false << false;
        QTest::newRow("alpha_3f_blue")  << 0x3f0000ffu << false << false;
        QTest::newRow("alpha_b7_green") << 0xbf00ff00u << false << false;
        QTest::newRow("alpha_7f_white") << 0x7fffffffu << false << false;
        QTest::newRow("alpha_3f_white") << 0x3fffffffu << false << false;
        QTest::newRow("alpha_b7_white") << 0xb7ffffffu << false << false;
        QTest::newRow("alpha_7f_black") << 0x7f000000u << false << false;
        QTest::newRow("alpha_3f_black") << 0x3f000000u << false << false;
        QTest::newRow("alpha_b7_black") << 0xbf000000u << false << false;
    }
#endif

    QTest::newRow("bitmap_color0") << uint(Qt::color0) << true << true;
    QTest::newRow("bitmap_color1") << uint(Qt::color1) << true << true;
}

void tst_QPixmap::fill()
{
    QFETCH(uint, pixel);
    QFETCH(bool, syscolor);
    QFETCH(bool, bitmap);

    QColor color;

    if (syscolor)
        color = QColor(Qt::GlobalColor(pixel));
    else
        color = QColor(qRed(pixel), qGreen(pixel), qBlue(pixel), qAlpha(pixel));

    QColor compareColor = color;
    if (bitmap && syscolor) {
        // special case color0 and color1 for bitmaps.
        if (pixel == Qt::color0)
            compareColor.setRgb(255, 255, 255);
        else
            compareColor.setRgb(0, 0, 0);
    }

    QPixmap pm;

    if (bitmap)
        pm = QBitmap(400, 400);
    else
        pm = QPixmap(400, 400);

#if defined(Q_WS_X11)
    if (!bitmap && !pm.x11PictureHandle())
        QSKIP("Requires XRender support", SkipSingle);
#endif

    pm.fill(color);
    if (syscolor && !bitmap && pm.depth() < 24) {
        QSKIP("Test does not work on displays without true color", SkipSingle);
        return;
    }

    QImage image = pm.toImage();
    if (bitmap && syscolor) {
        int pixelindex = (pixel == Qt::color0) ? 0 : 1;
        QVERIFY(image.pixelIndex(0,0) == pixelindex);
    }
    QImage::Format format = compareColor.alpha() != 255
                            ? QImage::Format_ARGB32
                            : QImage::Format_RGB32;
    image = image.convertToFormat(format);


    QImage shouldBe(400, 400, format);
    shouldBe.fill(compareColor.rgba());

    QCOMPARE(image, shouldBe);
}

void tst_QPixmap::mask()
{
    QPixmap pm(100, 100);
    QBitmap bm(100, 100);

    QVERIFY(!pm.isNull());
    QVERIFY(!bm.isNull());
    QVERIFY(pm.mask().isNull());
    QCOMPARE(bm.toImage().format(), QImage::Format_MonoLSB);

    pm.setMask(bm);
    QVERIFY(!pm.mask().isNull());

    bm = QBitmap();
    // Invalid format here, since isNull() == true
    QVERIFY(bm.toImage().isNull());
    QCOMPARE(bm.toImage().format(), QImage::Format_Invalid);
    pm.setMask(bm);
    QVERIFY(pm.mask().isNull());

    bm = QBitmap(100, 100);
    pm.setMask(bm);
    QVERIFY(!pm.mask().isNull());
}

void tst_QPixmap::bitmapMask()
{
    QImage image(3, 3, QImage::Format_Mono);
    image.setColor(0, Qt::color0);
    image.setColor(1, Qt::color1);
    image.fill(Qt::color0);
    image.setPixel(1, 1, Qt::color1);
    image.setPixel(0, 0, Qt::color1);

    QImage image_mask(3, 3, QImage::Format_Mono);
    image_mask.setColor(0, Qt::color0);
    image_mask.setColor(1, Qt::color1);
    image_mask.fill(Qt::color0);
    image_mask.setPixel(1, 1, Qt::color1);
    image_mask.setPixel(2, 0, Qt::color1);

    QBitmap pm = QBitmap::fromImage(image);
    QBitmap pm_mask = QBitmap::fromImage(image_mask);
    pm.setMask(pm_mask);

    image = pm.toImage();
    image.setColor(0, Qt::color0);
    image.setColor(1, Qt::color1);
    image_mask = pm_mask.toImage();
    image_mask.setColor(0, Qt::color0);
    image_mask.setColor(1, Qt::color1);

    QVERIFY(!image.pixel(0, 0));
    QVERIFY(!image.pixel(2, 0));
    QVERIFY(image.pixel(1, 1));
}

void tst_QPixmap::testMetrics()
{
    QPixmap pixmap(100, 100);

    QCOMPARE(pixmap.width(), 100);
    QCOMPARE(pixmap.height(), 100);
    QCOMPARE(pixmap.depth(), QPixmap::defaultDepth());

    QBitmap bitmap(100, 100);

    QCOMPARE(bitmap.width(), 100);
    QCOMPARE(bitmap.height(), 100);
    QCOMPARE(bitmap.depth(), 1);
}

void tst_QPixmap::createMaskFromColor()
{
    QImage image(3, 3, QImage::Format_Indexed8);
    image.setNumColors(10);
    image.setColor(0, 0xffffffff);
    image.setColor(1, 0xff000000);
    image.setColor(2, 0xffff0000);
    image.setColor(3, 0xff0000ff);
    image.fill(0);
    image.setPixel(1, 0, 1);
    image.setPixel(0, 1, 2);
    image.setPixel(1, 1, 3);

    QImage im_mask = image.createMaskFromColor(0xffff0000);
    QCOMPARE((uint) im_mask.pixel(0, 1), QColor(Qt::color0).rgba());
    QImage im_inv_mask = image.createMaskFromColor(0xffff0000, Qt::MaskOutColor);
    QCOMPARE((uint) im_mask.pixel(0, 1), QColor(Qt::color0).rgba());

    QPixmap pixmap = QPixmap::fromImage(image);
    QBitmap mask = pixmap.createMaskFromColor(Qt::red);
    QBitmap inv_mask = pixmap.createMaskFromColor(Qt::red, Qt::MaskOutColor);
    QCOMPARE((uint) mask.toImage().pixel(0, 1), QColor(Qt::color0).rgba());
    QCOMPARE((uint) inv_mask.toImage().pixel(0, 1), QColor(Qt::color1).rgba());
}


void tst_QPixmap::cacheKey()
{
    QPixmap pixmap1(1, 1);
    QPixmap pixmap2(1, 1);
    qint64 pixmap1_key = pixmap1.cacheKey();

    QVERIFY(pixmap1.cacheKey() != pixmap2.cacheKey());

    pixmap2 = pixmap1;
    QVERIFY(pixmap2.cacheKey() == pixmap1.cacheKey());

    pixmap2.detach();
    QVERIFY(pixmap2.cacheKey() != pixmap1.cacheKey());
    QVERIFY(pixmap1.cacheKey() == pixmap1_key);
}

#ifdef Q_WS_WIN
void tst_QPixmap::toWinHBITMAP_data()
{
    QTest::addColumn<int>("red");
    QTest::addColumn<int>("green");
    QTest::addColumn<int>("blue");

    QTest::newRow("red")   << 255 << 0 << 0;
    QTest::newRow("green") << 0 << 255 << 0;
    QTest::newRow("blue")  << 0 << 0 << 255;
}

void tst_QPixmap::toWinHBITMAP()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);

    QPixmap pm(100, 100);
    pm.fill(QColor(red, green, blue));

    HBITMAP bitmap = pm.toWinHBITMAP();

    QVERIFY(bitmap != 0);

    // Verify size
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));

    int res;
    QT_WA({
        res = GetObjectW(bitmap, sizeof(BITMAP), &bitmap_info);
    } , {
        res = GetObjectA(bitmap, sizeof(BITMAP), &bitmap_info);
    });
    QVERIFY(res);

    QCOMPARE(100, (int) bitmap_info.bmWidth);
    QCOMPARE(100, (int) bitmap_info.bmHeight);

    HDC display_dc = GetDC(0);
    HDC bitmap_dc = CreateCompatibleDC(display_dc);

    HBITMAP null_bitmap = (HBITMAP) SelectObject(bitmap_dc, bitmap);

    COLORREF pixel = GetPixel(bitmap_dc, 0, 0);
    QCOMPARE((int)GetRValue(pixel), red);
    QCOMPARE((int)GetGValue(pixel), green);
    QCOMPARE((int)GetBValue(pixel), blue);

    // Clean up
    SelectObject(bitmap_dc, null_bitmap);
    DeleteObject(bitmap);
    DeleteDC(bitmap_dc);
    ReleaseDC(0, display_dc);

}

void tst_QPixmap::fromWinHBITMAP_data()
{
    toWinHBITMAP_data();
}

void tst_QPixmap::fromWinHBITMAP()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);

    HDC display_dc = GetDC(0);
    HDC bitmap_dc = CreateCompatibleDC(display_dc);
    HBITMAP bitmap = CreateCompatibleBitmap(display_dc, 100, 100);
    HBITMAP null_bitmap = (HBITMAP) SelectObject(bitmap_dc, bitmap);

    SelectObject(bitmap_dc, GetStockObject(NULL_PEN));
    HGDIOBJ old_brush = SelectObject(bitmap_dc, CreateSolidBrush(RGB(red, green, blue)));
    Rectangle(bitmap_dc, 0, 0, 100, 100);

    QPixmap pixmap = QPixmap::fromWinHBITMAP(bitmap);
    QCOMPARE(pixmap.width(), 100);
    QCOMPARE(pixmap.height(), 100);

    QImage image = pixmap.toImage();
    QRgb pixel = image.pixel(0, 0);
    QCOMPARE(qRed(pixel), red);
    QCOMPARE(qGreen(pixel), green);
    QCOMPARE(qBlue(pixel), blue);

    DeleteObject(SelectObject(bitmap_dc, old_brush));
    DeleteObject(SelectObject(bitmap_dc, bitmap));
    DeleteDC(bitmap_dc);
    ReleaseDC(0, display_dc);
}

#endif

QTEST_MAIN(tst_QPixmap)
#include "tst_qpixmap.moc"
