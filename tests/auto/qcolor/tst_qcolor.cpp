/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <limits.h>

#include <qcolor.h>
#include <qdebug.h>

//TESTED_CLASS=QColor
//TESTED_FILES=gui/painting/qcolor.h gui/painting/qcolor.cpp

class tst_QColor : public QObject
{
    Q_OBJECT

public:
    tst_QColor();

private slots:
    void getSetCheck();
    void isValid_data();
    void isValid();

    void name_data();
    void name();
    void setNamedColor();

    void colorNames();

    void spec();

    void globalColors_data();
    void globalColors();

    void alpha();
    void setAlpha();

    void red();
    void green();
    void blue();

    void setRed();
    void setGreen();
    void setBlue();

    void getRgb();
    void setRgb();

    void rgba();
    void setRgba();

    void rgb();

    void hue();
    void saturation();
    void value();

    void getHsv();
    void setHsv();

    void cyan();
    void magenta();
    void yellow();
    void black();

    void getCmyk();
    void setCmyk();

    void toRgb_data();
    void toRgb();

    void toHsv_data();
    void toHsv();

    void toCmyk_data();
    void toCmyk();

    void convertTo();

    void fromRgb();
    void fromHsv();
    void fromCmyk();

    void light();
    void dark();

    void assignmentOoperator();
    void equalityOperator();
};

// Testing get/set functions
void tst_QColor::getSetCheck()
{
    QColor obj1;
    // int QColor::alpha()
    // void QColor::setAlpha(int)
    obj1.setAlpha(0);
    QCOMPARE(obj1.alpha(), 0);
    obj1.setAlpha(-1);
    QCOMPARE(obj1.alpha(), 0); // range<0, 255>
    obj1.setAlpha(INT_MIN);
    QCOMPARE(obj1.alpha(), 0); // range<0, 255>
    obj1.setAlpha(255);
    QCOMPARE(obj1.alpha(), 255); // range<0, 255>
    obj1.setAlpha(INT_MAX);
    QCOMPARE(obj1.alpha(), 255); // range<0, 255>

    // qreal QColor::alphaF()
    // void QColor::setAlphaF(qreal)
    obj1.setAlphaF(0.0);
    QCOMPARE(obj1.alphaF(), 0.0); // range<0.0, 1.0>
    obj1.setAlphaF(-0.2);
    QCOMPARE(obj1.alphaF(), 0.0); // range<0.0, 1.0>
    obj1.setAlphaF(1.0);
    QCOMPARE(obj1.alphaF(), 1.0); // range<0.0, 1.0>
    obj1.setAlphaF(1.1);
    QCOMPARE(obj1.alphaF(), 1.0); // range<0.0, 1.0>

    // int QColor::red()
    // void QColor::setRed(int)
    obj1.setRed(0);
    QCOMPARE(obj1.red(), 0);
    obj1.setRed(-1);
    QCOMPARE(obj1.red(), 0); // range<0, 255>
    obj1.setRed(INT_MIN);
    QCOMPARE(obj1.red(), 0); // range<0, 255>
    obj1.setRed(255);
    QCOMPARE(obj1.red(), 255); // range<0, 255>
    obj1.setRed(INT_MAX);
    QCOMPARE(obj1.red(), 255); // range<0, 255>

    // int QColor::green()
    // void QColor::setGreen(int)
    obj1.setGreen(0);
    QCOMPARE(obj1.green(), 0);
    obj1.setGreen(-1);
    QCOMPARE(obj1.green(), 0); // range<0, 255>
    obj1.setGreen(INT_MIN);
    QCOMPARE(obj1.green(), 0); // range<0, 255>
    obj1.setGreen(255);
    QCOMPARE(obj1.green(), 255); // range<0, 255>
    obj1.setGreen(INT_MAX);
    QCOMPARE(obj1.green(), 255); // range<0, 255>

    // int QColor::blue()
    // void QColor::setBlue(int)
    obj1.setBlue(0);
    QCOMPARE(obj1.blue(), 0);
    obj1.setBlue(-1);
    QCOMPARE(obj1.blue(), 0); // range<0, 255>
    obj1.setBlue(INT_MIN);
    QCOMPARE(obj1.blue(), 0); // range<0, 255>
    obj1.setBlue(255);
    QCOMPARE(obj1.blue(), 255); // range<0, 255>
    obj1.setBlue(INT_MAX);
    QCOMPARE(obj1.blue(), 255); // range<0, 255>

    // qreal QColor::redF()
    // void QColor::setRedF(qreal)
    obj1.setRedF(0.0);
    QCOMPARE(obj1.redF(), 0.0);
    obj1.setRedF(-0.2);
    QCOMPARE(obj1.redF(), 0.0); // range<0.0, 1.0
    obj1.setRedF(1.1);
    QCOMPARE(obj1.redF(), 1.0); // range<0.0, 1.0

    // qreal QColor::greenF()
    // void QColor::setGreenF(qreal)
    obj1.setGreenF(0.0);
    QCOMPARE(obj1.greenF(), 0.0);
    obj1.setGreenF(-0.2);
    QCOMPARE(obj1.greenF(), 0.0); // range<0.0, 1.0
    obj1.setGreenF(1.1);
    QCOMPARE(obj1.greenF(), 1.0); // range<0.0, 1.0

    // qreal QColor::blueF()
    // void QColor::setBlueF(qreal)
    obj1.setBlueF(0.0);
    QCOMPARE(obj1.blueF(), 0.0);
    obj1.setBlueF(-0.2);
    QCOMPARE(obj1.blueF(), 0.0); // range<0.0, 1.0
    obj1.setBlueF(1.1);
    QCOMPARE(obj1.blueF(), 1.0); // range<0.0, 1.0

    // QRgb QColor::rgba()
    // void QColor::setRgba(QRgb)
    QRgb var9(qRgba(10, 20, 30, 40));
    obj1.setRgba(var9);
    QCOMPARE(obj1.rgba(), var9);
    obj1.setRgba(QRgb(0));
    QCOMPARE(obj1.rgba(), QRgb(0));

    // QRgb QColor::rgb()
    // void QColor::setRgb(QRgb)
    QRgb var10(qRgb(10, 20, 30));
    obj1.setRgb(var10);
    QCOMPARE(obj1.rgb(), var10);
    obj1.setRgb(QRgb(0));
    QCOMPARE(obj1.rgb(), qRgb(0, 0, 0));
}

Q_DECLARE_METATYPE(QColor)


tst_QColor::tst_QColor()

{ }

void tst_QColor::isValid_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("defaultConstructor") << QColor() << false;
    QTest::newRow("rgbConstructor-valid") << QColor(2,5,7) << true;
    QTest::newRow("rgbConstructor-invalid") << QColor(2,5,999) << false;
    QTest::newRow("nameQStringConstructor-valid") << QColor(QString("#ffffff")) << true;
    QTest::newRow("nameQStringConstructor-invalid") << QColor(QString("#ffffgg")) << false;
    QTest::newRow("nameQStringConstructor-empty") << QColor(QString("")) << false;
    QTest::newRow("nameQStringConstructor-named") << QColor(QString("red")) << true;
    QTest::newRow("nameCharConstructor-valid") << QColor("#ffffff") << true;
    QTest::newRow("nameCharConstructor-invalid") << QColor("#ffffgg") << false;
}

void tst_QColor::isValid()
{
    QFETCH(QColor, color);
    QFETCH(bool, isValid);
    QVERIFY(color.isValid() == isValid);
}

void tst_QColor::name_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<QString>("name");

    QTest::newRow("invalid") << QColor() << "#000000";
    QTest::newRow("global color black") << QColor(Qt::black) << "#000000";
    QTest::newRow("global color white") << QColor(Qt::white) << "#ffffff";
    QTest::newRow("global color darkGray") << QColor(Qt::darkGray) << "#808080";
    QTest::newRow("global color gray") << QColor(Qt::gray) << "#a0a0a4";
    QTest::newRow("global color lightGray") << QColor(Qt::lightGray) << "#c0c0c0";
    QTest::newRow("global color red") << QColor(Qt::red) << "#ff0000";
    QTest::newRow("global color green") << QColor(Qt::green) << "#00ff00";
    QTest::newRow("global color blue") << QColor(Qt::blue) << "#0000ff";
    QTest::newRow("global color cyan") << QColor(Qt::cyan) << "#00ffff";
    QTest::newRow("global color magenta") << QColor(Qt::magenta) << "#ff00ff";
    QTest::newRow("global color yellow") << QColor(Qt::yellow) << "#ffff00";
    QTest::newRow("global color darkRed") << QColor(Qt::darkRed) << "#800000";
    QTest::newRow("global color darkGreen") << QColor(Qt::darkGreen) << "#008000";
    QTest::newRow("global color darkBlue") << QColor(Qt::darkBlue) << "#000080";
    QTest::newRow("global color darkCyan") << QColor(Qt::darkCyan) << "#008080";
    QTest::newRow("global color darkMagenta") << QColor(Qt::darkMagenta) << "#800080";
    QTest::newRow("global color darkYellow") << QColor(Qt::darkYellow) << "#808000";
}

void tst_QColor::name()
{
    QFETCH(QColor, color);
    QFETCH(QString, name);
    QCOMPARE(color.name(), name);
}

void tst_QColor::globalColors_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<uint>("argb");

    QTest::newRow("invalid") << QColor() << 0xff000000;
    QTest::newRow("global color black") << QColor(Qt::black) << 0xff000000;
    QTest::newRow("global color white") << QColor(Qt::white) << 0xffffffff;
    QTest::newRow("global color darkGray") << QColor(Qt::darkGray) << 0xff808080;
    QTest::newRow("global color gray") << QColor(Qt::gray) << 0xffa0a0a4;
    QTest::newRow("global color lightGray") << QColor(Qt::lightGray) << 0xffc0c0c0;
    QTest::newRow("global color red") << QColor(Qt::red) << 0xffff0000;
    QTest::newRow("global color green") << QColor(Qt::green) << 0xff00ff00;
    QTest::newRow("global color blue") << QColor(Qt::blue) << 0xff0000ff;
    QTest::newRow("global color cyan") << QColor(Qt::cyan) << 0xff00ffff;
    QTest::newRow("global color magenta") << QColor(Qt::magenta) << 0xffff00ff;
    QTest::newRow("global color yellow") << QColor(Qt::yellow) << 0xffffff00;
    QTest::newRow("global color darkRed") << QColor(Qt::darkRed) << 0xff800000;
    QTest::newRow("global color darkGreen") << QColor(Qt::darkGreen) << 0xff008000;
    QTest::newRow("global color darkBlue") << QColor(Qt::darkBlue) << 0xff000080;
    QTest::newRow("global color darkCyan") << QColor(Qt::darkCyan) << 0xff008080;
    QTest::newRow("global color darkMagenta") << QColor(Qt::darkMagenta) << 0xff800080;
    QTest::newRow("global color darkYellow") << QColor(Qt::darkYellow) << 0xff808000;
    QTest::newRow("global color transparent") << QColor(Qt::transparent) << 0x00000000u;
}


void tst_QColor::globalColors()
{
    QFETCH(QColor, color);
    QFETCH(uint, argb);
    QCOMPARE(color.rgba(), argb);
}

/*
  CSS color names = SVG 1.0 color names + transparent (rgba(0,0,0,0))
*/

#ifdef rgb
#  undef rgb
#endif
#define rgb(r,g,b) (0xff000000 | r << 16 |  g << 8 | b)

static const struct RGBData {
    const char *name;
    uint  value;
} rgbTbl[] = {
    { "aliceblue", rgb(240, 248, 255) },
    { "antiquewhite", rgb(250, 235, 215) },
    { "aqua", rgb( 0, 255, 255) },
    { "aquamarine", rgb(127, 255, 212) },
    { "azure", rgb(240, 255, 255) },
    { "beige", rgb(245, 245, 220) },
    { "bisque", rgb(255, 228, 196) },
    { "black", rgb( 0, 0, 0) },
    { "blanchedalmond", rgb(255, 235, 205) },
    { "blue", rgb( 0, 0, 255) },
    { "blueviolet", rgb(138, 43, 226) },
    { "brown", rgb(165, 42, 42) },
    { "burlywood", rgb(222, 184, 135) },
    { "cadetblue", rgb( 95, 158, 160) },
    { "chartreuse", rgb(127, 255, 0) },
    { "chocolate", rgb(210, 105, 30) },
    { "coral", rgb(255, 127, 80) },
    { "cornflowerblue", rgb(100, 149, 237) },
    { "cornsilk", rgb(255, 248, 220) },
    { "crimson", rgb(220, 20, 60) },
    { "cyan", rgb( 0, 255, 255) },
    { "darkblue", rgb( 0, 0, 139) },
    { "darkcyan", rgb( 0, 139, 139) },
    { "darkgoldenrod", rgb(184, 134, 11) },
    { "darkgray", rgb(169, 169, 169) },
    { "darkgreen", rgb( 0, 100, 0) },
    { "darkgrey", rgb(169, 169, 169) },
    { "darkkhaki", rgb(189, 183, 107) },
    { "darkmagenta", rgb(139, 0, 139) },
    { "darkolivegreen", rgb( 85, 107, 47) },
    { "darkorange", rgb(255, 140, 0) },
    { "darkorchid", rgb(153, 50, 204) },
    { "darkred", rgb(139, 0, 0) },
    { "darksalmon", rgb(233, 150, 122) },
    { "darkseagreen", rgb(143, 188, 143) },
    { "darkslateblue", rgb( 72, 61, 139) },
    { "darkslategray", rgb( 47, 79, 79) },
    { "darkslategrey", rgb( 47, 79, 79) },
    { "darkturquoise", rgb( 0, 206, 209) },
    { "darkviolet", rgb(148, 0, 211) },
    { "deeppink", rgb(255, 20, 147) },
    { "deepskyblue", rgb( 0, 191, 255) },
    { "dimgray", rgb(105, 105, 105) },
    { "dimgrey", rgb(105, 105, 105) },
    { "dodgerblue", rgb( 30, 144, 255) },
    { "firebrick", rgb(178, 34, 34) },
    { "floralwhite", rgb(255, 250, 240) },
    { "forestgreen", rgb( 34, 139, 34) },
    { "fuchsia", rgb(255, 0, 255) },
    { "gainsboro", rgb(220, 220, 220) },
    { "ghostwhite", rgb(248, 248, 255) },
    { "gold", rgb(255, 215, 0) },
    { "goldenrod", rgb(218, 165, 32) },
    { "gray", rgb(128, 128, 128) },
    { "green", rgb( 0, 128, 0) },
    { "greenyellow", rgb(173, 255, 47) },
    { "grey", rgb(128, 128, 128) },
    { "honeydew", rgb(240, 255, 240) },
    { "hotpink", rgb(255, 105, 180) },
    { "indianred", rgb(205, 92, 92) },
    { "indigo", rgb( 75, 0, 130) },
    { "ivory", rgb(255, 255, 240) },
    { "khaki", rgb(240, 230, 140) },
    { "lavender", rgb(230, 230, 250) },
    { "lavenderblush", rgb(255, 240, 245) },
    { "lawngreen", rgb(124, 252, 0) },
    { "lemonchiffon", rgb(255, 250, 205) },
    { "lightblue", rgb(173, 216, 230) },
    { "lightcoral", rgb(240, 128, 128) },
    { "lightcyan", rgb(224, 255, 255) },
    { "lightgoldenrodyellow", rgb(250, 250, 210) },
    { "lightgray", rgb(211, 211, 211) },
    { "lightgreen", rgb(144, 238, 144) },
    { "lightgrey", rgb(211, 211, 211) },
    { "lightpink", rgb(255, 182, 193) },
    { "lightsalmon", rgb(255, 160, 122) },
    { "lightseagreen", rgb( 32, 178, 170) },
    { "lightskyblue", rgb(135, 206, 250) },
    { "lightslategray", rgb(119, 136, 153) },
    { "lightslategrey", rgb(119, 136, 153) },
    { "lightsteelblue", rgb(176, 196, 222) },
    { "lightyellow", rgb(255, 255, 224) },
    { "lime", rgb( 0, 255, 0) },
    { "limegreen", rgb( 50, 205, 50) },
    { "linen", rgb(250, 240, 230) },
    { "magenta", rgb(255, 0, 255) },
    { "maroon", rgb(128, 0, 0) },
    { "mediumaquamarine", rgb(102, 205, 170) },
    { "mediumblue", rgb( 0, 0, 205) },
    { "mediumorchid", rgb(186, 85, 211) },
    { "mediumpurple", rgb(147, 112, 219) },
    { "mediumseagreen", rgb( 60, 179, 113) },
    { "mediumslateblue", rgb(123, 104, 238) },
    { "mediumspringgreen", rgb( 0, 250, 154) },
    { "mediumturquoise", rgb( 72, 209, 204) },
    { "mediumvioletred", rgb(199, 21, 133) },
    { "midnightblue", rgb( 25, 25, 112) },
    { "mintcream", rgb(245, 255, 250) },
    { "mistyrose", rgb(255, 228, 225) },
    { "moccasin", rgb(255, 228, 181) },
    { "navajowhite", rgb(255, 222, 173) },
    { "navy", rgb( 0, 0, 128) },
    { "oldlace", rgb(253, 245, 230) },
    { "olive", rgb(128, 128, 0) },
    { "olivedrab", rgb(107, 142, 35) },
    { "orange", rgb(255, 165, 0) },
    { "orangered", rgb(255, 69, 0) },
    { "orchid", rgb(218, 112, 214) },
    { "palegoldenrod", rgb(238, 232, 170) },
    { "palegreen", rgb(152, 251, 152) },
    { "paleturquoise", rgb(175, 238, 238) },
    { "palevioletred", rgb(219, 112, 147) },
    { "papayawhip", rgb(255, 239, 213) },
    { "peachpuff", rgb(255, 218, 185) },
    { "peru", rgb(205, 133, 63) },
    { "pink", rgb(255, 192, 203) },
    { "plum", rgb(221, 160, 221) },
    { "powderblue", rgb(176, 224, 230) },
    { "purple", rgb(128, 0, 128) },
    { "red", rgb(255, 0, 0) },
    { "rosybrown", rgb(188, 143, 143) },
    { "royalblue", rgb( 65, 105, 225) },
    { "saddlebrown", rgb(139, 69, 19) },
    { "salmon", rgb(250, 128, 114) },
    { "sandybrown", rgb(244, 164, 96) },
    { "seagreen", rgb( 46, 139, 87) },
    { "seashell", rgb(255, 245, 238) },
    { "sienna", rgb(160, 82, 45) },
    { "silver", rgb(192, 192, 192) },
    { "skyblue", rgb(135, 206, 235) },
    { "slateblue", rgb(106, 90, 205) },
    { "slategray", rgb(112, 128, 144) },
    { "slategrey", rgb(112, 128, 144) },
    { "snow", rgb(255, 250, 250) },
    { "springgreen", rgb( 0, 255, 127) },
    { "steelblue", rgb( 70, 130, 180) },
    { "tan", rgb(210, 180, 140) },
    { "teal", rgb( 0, 128, 128) },
    { "thistle", rgb(216, 191, 216) },
    { "tomato", rgb(255, 99, 71) },
    { "transparent", 0 },
    { "turquoise", rgb( 64, 224, 208) },
    { "violet", rgb(238, 130, 238) },
    { "wheat", rgb(245, 222, 179) },
    { "white", rgb(255, 255, 255) },
    { "whitesmoke", rgb(245, 245, 245) },
    { "yellow", rgb(255, 255, 0) },
    { "yellowgreen", rgb(154, 205, 50) }
};

static const int rgbTblSize = sizeof(rgbTbl) / sizeof(RGBData);

#undef rgb

void tst_QColor::setNamedColor()
{
    for (int i = 0; i < rgbTblSize; ++i) {
        QColor color;
        color.setNamedColor(QLatin1String(rgbTbl[i].name));
        QColor expected(rgbTbl[i].value);
        QCOMPARE(color, expected);
    }
}

void tst_QColor::colorNames()
{
    DEPENDS_ON("setNamedColor()");

    QStringList all = QColor::colorNames();
    QCOMPARE(all.size(), rgbTblSize);
    for (int i = 0; i < all.size(); ++i)
        QCOMPARE(all.at(i), QString::fromLatin1(rgbTbl[i].name));
}

void tst_QColor::spec()
{
    QColor invalid;
    QCOMPARE(invalid.spec(), QColor::Invalid);

    QColor rgb = QColor::fromRgb(0, 0, 0);
    QCOMPARE(rgb.spec(), QColor::Rgb);

    QColor hsv = QColor::fromHsv(0, 0, 0);
    QCOMPARE(hsv.spec(), QColor::Hsv);

    QColor cmyk = QColor::fromCmyk(0, 0, 0, 0);
    QCOMPARE(cmyk.spec(), QColor::Cmyk);
}

void tst_QColor::alpha()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::red()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::green()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::blue()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::getRgb()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::setAlpha()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::setRed()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::setGreen()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::setBlue()
{ DEPENDS_ON(setRgb()); }


void tst_QColor::setRgb()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            QRgb rgb = qRgba(0, 0, 0, a);

            color.setRgb(0, 0, 0, a);
            QCOMPARE(color.alpha(), a);
            QCOMPARE(color.rgb(),  qRgb(0, 0, 0));

            color.setRgb(rgb);
            QCOMPARE(color.alpha(), 255);
            QCOMPARE(color.rgb(),   qRgb(0, 0, 0));

            int r, g, b, a2;
            color.setRgb(0, 0, 0, a);
            color.getRgb(&r, &g, &b, &a2);
            QCOMPARE(a2, a);

            QColor c(0, 0, 0);
            c.setAlpha(a);
            QCOMPARE(c.alpha(), a);
        }

        {
            // 0.0-1.0
            double a = A / double(USHRT_MAX);
            color.setRgbF(0.0, 0.0, 0.0, a);
            QCOMPARE(color.alphaF(), a);

            double r, g, b, a2;
            color.getRgbF(&r, &g, &b, &a2);
            QCOMPARE(a2, a);

            QColor c(0, 0, 0);
            c.setAlphaF(a);

            QCOMPARE(c.alphaF(), a);
        }
    }

    for (int R = 0; R <= USHRT_MAX; ++R) {
        {
            // 0-255
            int r = R >> 8;
            QRgb rgb = qRgb(r, 0, 0);

            color.setRgb(r, 0, 0);
            QCOMPARE(color.red(), r);
            QCOMPARE(color.rgb(), rgb);

            color.setRgb(rgb);
            QCOMPARE(color.red(), r);
            QCOMPARE(color.rgb(), rgb);

            int r2, g, b, a;
            color.getRgb(&r2, &g, &b, &a);
            QCOMPARE(r2, r);
        }

        {
            // 0.0-1.0
            double r = R / double(USHRT_MAX);
            color.setRgbF(r, 0.0, 0.0);
            QCOMPARE(color.redF(), r);

            double r2, g, b, a;
            color.getRgbF(&r2, &g, &b, &a);
            QCOMPARE(r2, r);
        }
    }

    for (int G = 0; G <= USHRT_MAX; ++G) {
        {
            // 0-255
            int g = G >> 8;
            QRgb rgb = qRgb(0, g, 0);

            color.setRgb(0, g, 0);
            QCOMPARE(color.green(), g);
            QCOMPARE(color.rgb(),   rgb);

            color.setRgb(rgb);
            QCOMPARE(color.green(), g);
            QCOMPARE(color.rgb(),   rgb);

            int r, g2, b, a;
            color.getRgb(&r, &g2, &b, &a);
            QCOMPARE(g2, g);
        }

        {
            // 0.0-1.0
            double g = G / double(USHRT_MAX);
            color.setRgbF(0.0, g, 0.0);
            QCOMPARE(color.greenF(), g);

            double r, g2, b, a;
            color.getRgbF(&r, &g2, &b, &a);
            QCOMPARE(g2, g);
        }
    }

    for (int B = 0; B <= USHRT_MAX; ++B) {
        {
            // 0-255
            int b = B >> 8;
            QRgb rgb = qRgb(0, 0, b);

            color.setRgb(0, 0, b);
            QCOMPARE(color.blue(),  b);
            QCOMPARE(color.rgb(),   rgb);

            color.setRgb(rgb);
            QCOMPARE(color.blue(),  b);
            QCOMPARE(color.rgb(),   rgb);

            int r, g, b2, a;
            color.getRgb(&r, &g, &b2, &a);
            QCOMPARE(b2, b);
        }

        {
            // 0.0-1.0
            double b = B / double(USHRT_MAX);
            color.setRgbF(0.0, 0.0, b);
            QCOMPARE(color.blueF(), b);

            double r, g, b2, a;
            color.getRgbF(&r, &g, &b2, &a);
            QCOMPARE(b2, b);
        }
    }
}

void tst_QColor::rgba()
{ DEPENDS_ON("setRgba()"); }

void tst_QColor::setRgba()
{
    for (int a = 0; a < 255; ++a) {
        const QRgb rgba1 = qRgba(a, a, a, a);
        QColor color;
        color.setRgba(rgba1);
        QCOMPARE(color.alpha(), a);
        const QRgb rgba2 = color.rgba();
        QCOMPARE(rgba2, rgba1);
        QCOMPARE(qAlpha(rgba2), a);
    }
}

void tst_QColor::rgb()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::hue()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::saturation()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::value()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::getHsv()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::setHsv()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            color.setHsv(0, 0, 0, a);
            QCOMPARE(color.alpha(), a);

            int h, s, v, a2;
            color.getHsv(&h, &s, &v, &a2);
            QCOMPARE(a2, a);
        }

        {
            // 0.0-1.0
            double a = A / double(USHRT_MAX);
            color.setHsvF(0.0, 0.0, 0.0, a); QCOMPARE(color.alphaF(), a);

            double h, s, v, a2;
            color.getHsvF(&h, &s, &v, &a2);
            QCOMPARE(a2, a);
        }
    }

    for (int H = 0; H < 36000; ++H) {
        {
            // 0-255
            int h = H / 100;

            color.setHsv(h, 0, 0, 0);
            QCOMPARE(color.hue(), h);

            int h2, s, v, a;
            color.getHsv(&h2, &s, &v, &a);
            QCOMPARE(h2, h);
        }

        {
            // 0.0-1.0
            qreal h = H / 36000.0;
            color.setHsvF(h, 0.0, 0.0, 0.0);
            QCOMPARE(color.hueF(), h);

            double h2, s, v, a;
            color.getHsvF(&h2, &s, &v, &a);
            QCOMPARE(h2, h);
        }
    }

    for (int S = 0; S <= USHRT_MAX; ++S) {
        {
            // 0-255
            int s = S >> 8;
            color.setHsv(0, s, 0, 0);
            QCOMPARE(color.saturation(), s);

            int h, s2, v, a;
            color.getHsv(&h, &s2, &v, &a);
            QCOMPARE(s2, s);
        }

        {
            // 0.0-1.0
            double s = S / double(USHRT_MAX);
            color.setHsvF(0.0, s, 0.0, 0.0);
            QCOMPARE(color.saturationF(), s);

            double h, s2, v, a;
            color.getHsvF(&h, &s2, &v, &a);
            QCOMPARE(s2, s);
        }
    }

    for (int V = 0; V <= USHRT_MAX; ++V) {
        {
            // 0-255
            int v = V >> 8;
            color.setHsv(0, 0, v, 0);
            QCOMPARE(color.value(),  v);

            int h, s, v2, a;
            color.getHsv(&h, &s, &v2, &a);
            QCOMPARE(v2, v);
        }

        {
            // 0.0-1.0
            double v = V / double(USHRT_MAX);
            color.setHsvF(0.0, 0.0, v, 0.0);
            QCOMPARE(color.valueF(), v);

            double h, s, v2, a;
            color.getHsvF(&h, &s, &v2, &a);
            QCOMPARE(v2, v);
        }
    }
}

void tst_QColor::cyan()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::magenta()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::yellow()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::black()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::getCmyk()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::setCmyk()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            color.setCmyk(0, 0, 0, 0, a);
            QCOMPARE(color.alpha(), a);

            int c, m, y, k, a2;
            color.getCmyk(&c, &m, &y, &k, &a2);
            QCOMPARE(a2, a);
        }

        {
            // 0.0-1.0
            double a = A / double(USHRT_MAX);
            color.setCmykF(0.0, 0.0, 0.0, 0.0, a);
            QCOMPARE(color.alphaF(), a);

            double c, m, y, k, a2;
            color.getCmykF(&c, &m, &y, &k, &a2);
            QCOMPARE(a2, a);
        }
    }

    for (int C = 0; C <= USHRT_MAX; ++C) {
        {
            // 0-255
            int c = C >> 8;
            color.setCmyk(c, 0, 0, 0, 0);
            QCOMPARE(color.cyan(), c);

            int c2, m, y, k, a;
            color.getCmyk(&c2, &m, &y, &k, &a);
            QCOMPARE(c2, c);
        }

        {
            // 0.0-1.0
            double c = C / double(USHRT_MAX);
            color.setCmykF(c, 0.0, 0.0, 0.0, 0.0);
            QCOMPARE(color.cyanF(), c);

            double c2, m, y, k, a;
            color.getCmykF(&c2, &m, &y, &k, &a);
            QCOMPARE(c2, c);
        }
    }

    for (int M = 0; M <= USHRT_MAX; ++M) {
        {
            // 0-255
            int m = M >> 8;
            color.setCmyk(0, m, 0, 0, 0);
            QCOMPARE(color.magenta(), m);

            int c, m2, y, k, a;
            color.getCmyk(&c, &m2, &y, &k, &a);
            QCOMPARE(m2, m);
        }

        {
            // 0.0-1.0
            double m = M / double(USHRT_MAX);
            color.setCmykF(0.0, m, 0.0, 0.0, 0.0);
            QCOMPARE(color.magentaF(), m);

            double c, m2, y, k, a;
            color.getCmykF(&c, &m2, &y, &k, &a);
            QCOMPARE(m2, m);
        }
    }

    for (int Y = 0; Y <= USHRT_MAX; ++Y) {
        {
            // 0-255
            int y = Y >> 8;
            color.setCmyk(0, 0, y, 0, 0);
            QCOMPARE(color.yellow(), y);

            int c, m, y2, k, a;
            color.getCmyk(&c, &m, &y2, &k, &a);
            QCOMPARE(y2, y);
        }

        {
            // 0.0-1.0
            double y = Y / double(USHRT_MAX);
            color.setCmykF(0.0, 0.0, y, 0.0, 0.0);
            QCOMPARE(color.yellowF(), y);

            double c, m, y2, k, a;
            color.getCmykF(&c, &m, &y2, &k, &a);
            QCOMPARE(y2, y);
        }
    }

    for (int K = 0; K <= USHRT_MAX; ++K) {
        {
            // 0-255
            int k = K >> 8;
            color.setCmyk(0, 0, 0, k, 0);
            QCOMPARE(color.black(), k);

            int c, m, y, k2, a;
            color.getCmyk(&c, &m, &y, &k2, &a);
            QCOMPARE(k2, k);
        }

        {
            // 0.0-1.0
            double k = K / double(USHRT_MAX);
            color.setCmykF(0.0, 0.0, 0.0, k, 0.0);
            QCOMPARE(color.blackF(), k);

            double c, m, y, k2, a;
            color.getCmykF(&c, &m, &y, &k2, &a);
            QCOMPARE(k2, k);
        }
    }
}

void tst_QColor::toRgb_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("hsvColor");
    QTest::addColumn<QColor>("cmykColor");

    QTest::newRow("black")
        << QColor::fromRgbF(0.0, 0.0, 0.0)
        << QColor::fromHsvF(-1.0, 0.0, 0.0)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 1.0);

    QTest::newRow("white")
        << QColor::fromRgbF(1.0, 1.0, 1.0)
        << QColor::fromHsvF(-1.0, 0.0, 1.0)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);

    QTest::newRow("red")
        << QColor::fromRgbF(1.0, 0.0, 0.0)
        << QColor::fromHsvF(0.0, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 1.0, 1.0, 0.0);

    QTest::newRow("green")
        << QColor::fromRgbF(0.0, 1.0, 0.0)
        << QColor::fromHsvF(0.33333, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 0.0, 1.0, 0.0);

    QTest::newRow("blue")
        << QColor::fromRgbF(0.0, 0.0, 1.0)
        << QColor::fromHsvF(0.66667, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 1.0, 0.0, 0.0);

    QTest::newRow("cyan")
        << QColor::fromRgbF(0.0, 1.0, 1.0)
        << QColor::fromHsvF(0.5, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 0.0, 0.0, 0.0);

    QTest::newRow("magenta")
        << QColor::fromRgbF(1.0, 0.0, 1.0)
        << QColor::fromHsvF(0.83333, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 1.0, 0.0, 0.0);

    QTest::newRow("yellow")
        << QColor::fromRgbF(1.0, 1.0, 0.0)
        << QColor::fromHsvF(0.16667, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 0.0, 1.0, 0.0);

    QTest::newRow("gray")
        << QColor::fromRgbF(0.6431375, 0.6431375, 0.6431375)
        << QColor::fromHsvF(-1.0, 0.0, 0.6431375)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 0.356863);

    // ### add colors using the 0-255 functions
}

void tst_QColor::toRgb()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toRgb().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, hsvColor);
    QFETCH(QColor, cmykColor);
    QCOMPARE(hsvColor.toRgb(), expectedColor);
    QCOMPARE(cmykColor.toRgb(), expectedColor);
}

void tst_QColor::toHsv_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("rgbColor");
    QTest::addColumn<QColor>("cmykColor");

    QTest::newRow("data0")
        << QColor::fromHsv(300, 255, 255)
        << QColor(255, 0, 255)
        << QColor::fromCmyk(0, 255, 0, 0);
}

void tst_QColor::toHsv()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toHsv().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, rgbColor);
    QFETCH(QColor, cmykColor);
    QCOMPARE(rgbColor.toHsv(), expectedColor);
    QCOMPARE(cmykColor.toHsv(), expectedColor);
}

void tst_QColor::toCmyk_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("rgbColor");
    QTest::addColumn<QColor>("hsvColor");

    QTest::newRow("data0")
        << QColor::fromCmykF(1.0, 0.0, 0.0, 0.0)
        << QColor(0, 255, 255)
        << QColor::fromHsv(180, 255, 255);

    QTest::newRow("data1")
        << QColor::fromCmyk(255, 255, 255, 255)
        << QColor::fromRgb(0, 0, 0)
        << QColor::fromRgb(0, 0, 0).toHsv();
}

void tst_QColor::toCmyk()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toCmyk().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, rgbColor);
    QFETCH(QColor, hsvColor);
    QCOMPARE(rgbColor.toHsv().toCmyk(), expectedColor);
    QCOMPARE(hsvColor.toCmyk(), expectedColor);
}

void tst_QColor::convertTo()
{
    QColor color(Qt::black);

    QColor rgb = color.convertTo(QColor::Rgb);
    QVERIFY(rgb.spec() == QColor::Rgb);

    QColor hsv = color.convertTo(QColor::Hsv);
    QVERIFY(hsv.spec() == QColor::Hsv);

    QColor cmyk = color.convertTo(QColor::Cmyk);
    QVERIFY(cmyk.spec() == QColor::Cmyk);

    QColor invalid = color.convertTo(QColor::Invalid);
    QVERIFY(invalid.spec() == QColor::Invalid);

    DEPENDS_ON(toRgb());
    DEPENDS_ON(toHsv());
    DEPENDS_ON(toCmyk());
}

void tst_QColor::fromRgb()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::fromHsv()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::fromCmyk()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::light()
{
    QColor gray(Qt::gray);
    QColor lighter = gray.light();
    QVERIFY(lighter.value() > gray.value());
}

void tst_QColor::dark()
{
    QColor gray(Qt::gray);
    QColor darker = gray.dark();
    QVERIFY(darker.value() < gray.value());
}

void tst_QColor::assignmentOoperator()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::equalityOperator()
{ DEPENDS_ON(convertTo()); }

QTEST_APPLESS_MAIN(tst_QColor)
#include "tst_qcolor.moc"
