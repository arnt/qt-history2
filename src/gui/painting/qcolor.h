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

#ifndef QCOLOR_H
#define QCOLOR_H

#  include "qstringlist.h"
#  include "qnamespace.h"
#  include "qrgb.h"

class QColor;
class QColormap;

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QColor &);
#endif
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QColor &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QColor &);
#endif

class Q_GUI_EXPORT QColor
{
public:
    enum Spec { Invalid, Rgb, Hsv, Cmyk };

    enum ColorFormat { RgbFormat, RgbaFormat };

    QColor();
    QColor(Qt::GlobalColor color);
    QColor(int r, int g, int b, int a = 255);
    QColor(uint color, ColorFormat format = RgbFormat);
    QColor(const QString& name);
    QColor(const char *name);
    QColor(const QColor &color);

    bool isValid() const;

    QString name() const;
    void setNamedColor(const QString& name);

    static QStringList colorNames();

    inline Spec spec() const
    { return cspec; }

    int alpha() const;

    float alphaF() const;

    int red() const;
    int green() const;
    int blue() const;

    float redF() const;
    float greenF() const;
    float blueF() const;

    void getRgb(int *r, int *g, int *b, int *a = 0) const;
    void setRgb(int r, int g, int b, int a = 255);

    void getRgb(float *r, float *g, float *b, float *a = 0) const;
    void setRgb(float r, float g, float b, float a = 1.0);

    QRgb rgb() const;
    void setRgb(QRgb rgb);

    int hue() const; // 0 <= hue < 360
    int saturation() const;
    int value() const;

    float hueF() const; // 0.0 <= hueF < 360.0
    float saturationF() const;
    float valueF() const;

    void getHsv(int *h, int *s, int *v, int *a = 0) const;
    void setHsv(int h, int s, int v, int a = 255);

    void getHsv(float *h, float *s, float *v, float *a = 0) const;
    void setHsv(float h, float s, float v, float a = 1.0);

    int cyan() const;
    int magenta() const;
    int yellow() const;
    int black() const;

    float cyanF() const;
    float magentaF() const;
    float yellowF() const;
    float blackF() const;

    void getCmyk(int *c, int *m, int *y, int *k, int *a = 0);
    void setCmyk(int c, int m, int y, int k, int a = 255);

    void getCmyk(float *c, float *m, float *y, float *k, float *a = 0);
    void setCmyk(float c, float m, float y, float k, float a = 1.0);

    QColor toRgb() const;
    QColor toHsv() const;
    QColor toCmyk() const;

    QColor convertTo(Spec colorSpec) const;

    static QColor fromRgb(int r, int g, int b, int a = 255);
    static QColor fromRgb(float r, float g, float b, float a = 1.0);

    static QColor fromHsv(int h, int s, int v, int a = 255);
    static QColor fromHsv(float h, float s, float v, float a = 1.0);

    static QColor fromCmyk(int c, int m, int y, int k, int a = 255);
    static QColor fromCmyk(float c, float m, float y, float k, float a = 1.0);

    QColor light(int f = 150) const;
    QColor dark(int f = 200) const;

    QColor &operator=(const QColor &);
    QColor &operator=(Qt::GlobalColor color);

    bool operator==(const QColor &c) const;
    bool operator!=(const QColor &c) const;

#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QColor(int x, int y, int z, Spec colorSpec)
    { if (colorSpec == Hsv) setHsv(x, y, z); else setRgb(x, y, z); }

    inline QT_COMPAT void rgb(int *r, int *g, int *b) const
    { getRgb(r, g, b); }
    inline QT_COMPAT void hsv(int *h, int *s, int *v) const
    { getHsv(h, s, v); }

    inline QT_COMPAT void setRgba(int r, int g, int b, int a)
    { setRgb(r, g, b, a); }
    inline QT_COMPAT void getRgba(int *r, int *g, int *b, int *a) const
    { getRgb(r, g, b, a); }

    QT_COMPAT uint pixel(int screen = -1) const;
#endif

private:
#ifndef QT_COMPAT
    // do not allow a spec to be used as an alpha value
    QColor(int, int, int, Spec);
#endif

    void invalidate();

    Spec cspec;
    union {
        struct {
            ushort alpha;
            ushort red;
            ushort green;
            ushort blue;
            ushort pad;
        } argb;
        struct {
            ushort alpha;
            ushort hue;
            ushort saturation;
            ushort value;
            ushort pad;
        } ahsv;
        struct {
            ushort alpha;
            ushort cyan;
            ushort magenta;
            ushort yellow;
            ushort black;
        } acmyk;
    };

    friend class QColormap;
#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QColor &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QColor &);
#endif
};

inline QColor::QColor()
{ invalidate(); }

inline QColor::QColor(int r, int g, int b, int a)
{ setRgb(r, g, b, a); }

inline QColor::QColor(const char *name)
{ setNamedColor(QLatin1String(name)); }

inline QColor::QColor(const QString& name)
{ setNamedColor(name); }

inline QColor::QColor(const QColor &color)
    : cspec(color.cspec)
{ argb = color.argb; }

inline bool QColor::isValid() const
{ return cspec != Invalid; }

#endif // QCOLOR_H
