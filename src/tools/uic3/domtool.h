/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DOMTOOL_H
#define DOMTOOL_H

#include <QVariant>

class QDomElement;
class QDomDocument;
class QDomNode;
class QDomNodeList;

struct Common
{
    int kind;

    enum {
        Kind_Unknown = 0,
        Kind_Color,
        Kind_Point,
        Kind_Size,
        Kind_Rect,
        Kind_Font,
        Kind_SizePolicy,
        Kind_Cursor
    };

    inline void init()
    { kind = Kind_Unknown; }
};
Q_DECLARE_METATYPE(Common)

struct Color
{
    Common common;
    int red, green, blue;

    inline void init(int r, int g, int b)
    {
        common.kind = Common::Kind_Color;
        red = r;
        green = g;
        blue = b;
    }

    inline bool operator == (const Color &other) const
    { return red == other.red && green == other.green && blue == other.blue; }
};
Q_DECLARE_METATYPE(Color)

struct Point
{
    Common common;
    int x, y;

    inline void init(int x, int y)
    {
        common.kind = Common::Kind_Point;
        this->x = x;
        this->y = y;
    }
};
Q_DECLARE_METATYPE(Point)

struct Size
{
    Common common;
    int width, height;

    inline bool isNull() const
    { return this->width == 0 && this->height == 0; }

    inline void init(int width, int height)
    {
        common.kind = Common::Kind_Size;
        this->width = width;
        this->height = height;
    }
};
Q_DECLARE_METATYPE(Size)

struct Rect
{
    Common common;
    int x, y;
    int width, height;

    inline void init(int x, int y, int width, int height)
    {
        common.kind = Common::Kind_Rect;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }
};
Q_DECLARE_METATYPE(Rect)

struct Font
{
    Common common;
    char *family;
    int pointsize;
    bool bold;
    bool italic;
    bool underline;
    bool strikeout;

    inline void init()
    {
        common.kind = Common::Kind_Font;
        family = 0;
        pointsize = 0;
        bold = false;
        italic = false;
        underline = false;
        strikeout = false;
    }
};
Q_DECLARE_METATYPE(Font)

struct SizePolicy
{
    Common common;
    int hsizetype;
    int vsizetype;
    int horstretch;
    int verstretch;

    inline void init()
    {
        common.kind = Common::Kind_SizePolicy;
        hsizetype = 0;
        vsizetype = 0;
        horstretch = 0;
        verstretch = 0;
    }
};
Q_DECLARE_METATYPE(SizePolicy)

struct Cursor
{
    Common common;
    int shape;

    inline void init(int shape)
    {
        common.kind = Common::Kind_Cursor;
        this->shape = shape;
    }
};
Q_DECLARE_METATYPE(Cursor)

union Variant
{
    Common common;
    Color color;
    Size size;
    Point point;
    Rect rect;
    Font font;
    SizePolicy sizePolicy;
    Cursor cursor;

    inline Variant()
    { common.kind = Common::Kind_Unknown; }

    inline ~Variant()
    {
        if (common.kind == Common::Kind_Font) {
            delete[] font.family;
            font.family = 0;
        }
    }

    inline int kind() const
    { return common.kind; }

    inline Variant &createColor(int r, int g, int b)
    { color.init(r, g, b); return *this; }

    inline Variant &createPoint(int x, int y)
    { point.init(x, y); return *this; }

    inline Variant &createSize(int width, int height)
    { size.init(width, height); return *this; }

    inline Variant &createRect(int x, int y, int w, int h)
    { rect.init(x, y, w, h); return *this; }

    inline Variant &createFont()
    { font.init(); return *this; }

    inline Variant &createSizePolicy()
    { sizePolicy.init(); return *this; }

    inline Variant &createCursor(int shape)
    { cursor.init(shape); return *this; }
};
Q_DECLARE_METATYPE(Variant)

class DomTool
{
public:
    static QVariant readProperty(const QDomElement& e, const QString& name, const QVariant& defValue);
    static QVariant readProperty(const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment);
    static bool hasProperty(const QDomElement& e, const QString& name);
    static QStringList propertiesOfType(const QDomElement& e, const QString& type);
    static QVariant elementToVariant(const QDomElement& e, const QVariant& defValue);
    static QVariant elementToVariant(const QDomElement& e, const QVariant& defValue, QString &comment);
    static QVariant readAttribute(const QDomElement& e, const QString& name, const QVariant& defValue);
    static QVariant readAttribute(const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment);
    static bool hasAttribute(const QDomElement& e, const QString& name);
    static Color readColor(const QDomElement &e);
    static void fixDocument(QDomDocument&);
    static void fixAttributes(QDomNodeList&, double);
    static void fixAttribute(QDomNode&, double);
};

inline Variant asVariant(const QVariant &v)
{
    Variant var;
    var = qVariantValue<Variant>(v);
    return var;
}

#endif // DOMTOOL_H
