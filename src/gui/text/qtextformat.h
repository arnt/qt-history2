#ifndef QTEXTFORMAT_H
#define QTEXTFORMAT_H

#ifndef QT_H
#include <qcolor.h>
#include <qshareddata.h>
#include <qobject.h>
#include <qfont.h>
#endif // QT_H

class QString;

class QTextFormatCollection;
class QTextFormatProperty;
class QTextFormatPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextFloatFormat;
class QTextImageFormat;
class QTextFormat;
class QTextBlockIterator;

class QTextGroupPrivate;

class QTextGroup : public QObject
{
    Q_DECLARE_PRIVATE(QTextGroup);
    Q_OBJECT
protected:
    QTextGroup(QObject *parent);
    ~QTextGroup();
    QTextGroup(QTextGroupPrivate &p, QObject *parent);
public:
    int commonFormatType() const;
    QTextFormat commonFormat() const;
    void setCommonFormat(const QTextFormat &format);

    QList<QTextBlockIterator> blockList() const;

protected:
    virtual void insertBlock(const QTextBlockIterator &block);
    virtual void removeBlock(const QTextBlockIterator &block);
    virtual void blockFormatChanged(const QTextBlockIterator &block);

private:
    friend class QTextFormatCollection;
    friend class QTextFormat;
    friend class QTextPieceTable;
};

class Q_GUI_EXPORT QTextFormat
{
    friend class QTextFormatCollection;
    friend QDataStream &operator<<(QDataStream &, const QTextFormat &);
    friend QDataStream &operator>>(QDataStream &, QTextFormat &);
public:
    enum FormatType {
        InvalidFormat = -1,
        BlockFormat = 1,
        CharFormat = 2,
        ListFormat = 3,
        TableFormat = 4,
        FloatFormat = 5,

        UserFormat = 100
    };

    enum Property {
        GroupIndex = 0x0,

        // paragrpah and char
        CssFloat = 0x0800,

        // paragraph
        BlockDirection = 0x1000,
        BlockAlignment = 0x1010,
        BlockTopMargin = 0x1030,
        BlockBottomMargin = 0x1031,
        BlockLeftMargin = 0x1032,
        BlockRightMargin = 0x1033,
        BlockFirstLineMargin = 0x1034,
        BlockIndent = 0x1040,
        BlockNonBreakableLines = 0x1050,
        BlockBackgroundColor = 0x1060,

        // character properties
        FontFamily = 0x2000,
        FontPointSize = 0x2001,
        FontSizeIncrement = 0x2002,
        FontWeight = 0x2003,
        FontItalic = 0x2004,
        FontUnderline = 0x2005,
        FontOverline = 0x2006,
        FontStrikeOut = 0x2007,
        FontFixedPitch = 0x2008,

        Color = 0x2010,

        IsAnchor = 0x2020,
        AnchorHref = 0x2021,
        AnchorName = 0x2022,

        NonDeletable = 0x2100,

        ObjectType = 0x2f00,

        // list properties
        ListStyle = 0x3000,
        ListIndent = 0x3001,

        // table properties
        TableBorder = 0x4000,

        // table cell properties
        TableCellEndOfRow = 0x4800,
        TableCellRowSpan = 0x4810,
        TableCellColSpan = 0x4811,

        // image properties
        ImageName = 0x5000,
        ImageWidth = 0x5010,
        ImageHeight = 0x5011,

        // --
        UserProperty = 0x10000
    };

    enum PropertyType {
        Undefined,
        Bool,
        Integer,
        Float,
        String,
        FormatGroup
    };

    enum ObjectTypes {
        NoObject,
        ImageObject
    };

    QTextFormat();

    Q_EXPLICIT QTextFormat(int type);

    QTextFormat(const QTextFormat &rhs);
    QTextFormat &operator=(const QTextFormat &rhs);
    ~QTextFormat();

    void merge(const QTextFormat &other);

    inline bool isValid() const { return type() != InvalidFormat; }

    int type() const;

    QTextGroup *group() const;
    void setGroup(QTextGroup *group);

    int groupIndex() const;
    void setGroupIndex(int group);

    bool boolProperty(int propertyId, bool defaultValue = false) const;
    int intProperty(int propertyId, int defaultValue = 0) const;
    float floatProperty(int propertyId, float defaultValue = 0.0) const;
    QString stringProperty(int propertyId, const QString &defaultValue = QString::null) const;

    void setProperty(int propertyId, bool value);
    void setProperty(int propertyId, int value);
    void setProperty(int propertyId, float value);
    void setProperty(int propertyId, const QString &value);

    bool hasProperty(int propertyId) const;
    PropertyType propertyType(int propertyId) const;

    QList<int> allPropertyIds() const;

    inline bool isCharFormat() const { return type() == CharFormat; }
    inline bool isBlockFormat() const { return type() == BlockFormat; }
    inline bool isListFormat() const { return type() == ListFormat; }
    inline bool isTableFormat() const { return type() == TableFormat; }
    inline bool isFloatFormat() const { return type() == FloatFormat; }
    inline bool isImageFormat() const { return type() == CharFormat && intProperty(ObjectType) == ImageObject; }

    QTextBlockFormat toBlockFormat() const;
    QTextCharFormat toCharFormat() const;
    QTextListFormat toListFormat() const;
    QTextTableFormat toTableFormat() const;
    QTextFloatFormat toFloatFormat() const;
    QTextImageFormat toImageFormat() const;

    bool operator==(const QTextFormat &rhs) const;
    inline bool operator!=(const QTextFormat &rhs) const { return !operator==(rhs); }

private:
    Q_EXPLICIT QTextFormat(QTextFormatCollection *c, QTextFormatPrivate *p);
    QSharedDataPointer<QTextFormatPrivate> d;
    QTextFormatCollection *collection;
};

QDataStream &operator<<(QDataStream &stream, const QTextFormat &format);
QDataStream &operator>>(QDataStream &stream, QTextFormat &format);

class Q_GUI_EXPORT QTextCharFormat : public QTextFormat
{
public:
    inline QTextCharFormat() : QTextFormat(CharFormat) {}

    bool isValid() const { return isCharFormat(); }
    void setFont(const QFont &font);
    QFont font() const;

    inline void setFontFamily(const QString &family)
    { setProperty(FontFamily, family); }
    inline QString fontFamily() const
    { return stringProperty(FontFamily); }

    inline void setFontPointSize(float size)
    { setProperty(FontPointSize, size); }
    inline float fontPointSize() const
    { return floatProperty(FontPointSize, 12); }

    inline void setFontWeight(int weight)
    { setProperty(FontWeight, weight); }
    inline int fontWeight() const
    { return intProperty(FontWeight, QFont::Normal); }

    inline void setFontItalic(bool italic)
    { setProperty(FontItalic, italic); }
    inline bool fontItalic() const
    { return boolProperty(FontItalic); }

    inline void setFontUnderline(bool underline)
    { setProperty(FontUnderline, underline); }
    inline bool fontUnderline() const
    { return boolProperty(FontUnderline); }

    inline void setFontOverline(bool overline)
    { setProperty(FontOverline, overline); }
    inline bool fontOverline() const
    { return boolProperty(FontOverline); }

    inline void setFontStrikeOut(bool strikeOut)
    { setProperty(FontStrikeOut, strikeOut); }
    inline bool fontStrikeOut() const
    { return boolProperty(FontStrikeOut); }

    inline void setFontFixedPitch(bool fixedPitch)
    { setProperty(FontFixedPitch, fixedPitch); }
    inline bool fontFixedPitch() const
    { return boolProperty(FontFixedPitch); }

    inline void setColor(const QColor &color)
    { setProperty(Color, int(color.rgb())); }
    inline QColor color() const
    { if (hasProperty(Color)) return QColor(intProperty(Color)); else return QColor(); }

    inline void setAnchor(bool anchor)
    { setProperty(IsAnchor, anchor); }
    inline bool isAnchor() const
    { return boolProperty(IsAnchor); }

    inline void setAnchorHref(const QString &value)
    { setProperty(AnchorHref, value); }
    inline QString anchorHref() const
    { return stringProperty(AnchorHref); }

    inline void setAnchorName(const QString &name)
    { setProperty(AnchorName, name); }
    inline QString anchorName() const
    { return stringProperty(AnchorName); }

    inline void setNonDeletable(bool d)
    { setProperty(NonDeletable, d); }
    inline bool nonDeletable() const
    { return boolProperty(NonDeletable); }

    inline void setObjectType(int type)
    { setProperty(ObjectType, type); }
    inline int objectType() const
    { return intProperty(ObjectType, NoObject); }
};

class Q_GUI_EXPORT QTextBlockFormat : public QTextFormat
{
public:
    enum Direction { LeftToRight, RightToLeft, AutoDirection };

    inline QTextBlockFormat() : QTextFormat(BlockFormat) {}

    bool isValid() const { return isBlockFormat(); }

    inline void setDirection(Direction dir)
    { setProperty(BlockDirection, dir); }
    inline Direction direction() const
    { return static_cast<Direction>(intProperty(BlockDirection, AutoDirection)); }

    inline void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment)); }
    inline Qt::Alignment alignment() const
    { return QFlag(intProperty(BlockAlignment)); }

    QTextListFormat listFormat() const;
    QTextTableFormat tableFormat() const;

    inline void setTopMargin(int margin)
    { setProperty(BlockTopMargin, margin); }
    inline int topMargin() const
    { return intProperty(BlockTopMargin); }

    inline void setBottomMargin(int margin)
    { setProperty(BlockBottomMargin, margin); }
    inline int bottomMargin() const
    { return intProperty(BlockBottomMargin); }

    inline void setLeftMargin(int margin)
    { setProperty(BlockLeftMargin, margin); }
    inline int leftMargin() const
    { return intProperty(BlockLeftMargin); }

    inline void setRightMargin(int margin)
    { setProperty(BlockRightMargin, margin); }
    inline int rightMargin() const
    { return intProperty(BlockRightMargin); }

    inline void setFirstLineMargin(int margin)
    { setProperty(BlockFirstLineMargin, margin); }
    inline int firstLineMargin() const
    { return intProperty(BlockFirstLineMargin); }

    inline void setIndent(int indent)
    { setProperty(BlockIndent, indent); }
    inline int indent() const
    { return intProperty(BlockIndent); }

    inline void setTableCellEndOfRow(bool eor)
    { setProperty(TableCellEndOfRow, eor); }
    inline bool tableCellEndOfRow() const
    { return boolProperty(TableCellEndOfRow); }
    inline void setTableCellRowSpan(int tableCellRowSpan)
    { setProperty(TableCellRowSpan, tableCellRowSpan); }
    inline int tableCellRowSpan() const
    { return intProperty(TableCellRowSpan, 1); }
    inline void setTableCellColSpan(int tableCellColSpan)
    { setProperty(TableCellColSpan, tableCellColSpan); }
    inline int tableCellColSpan() const
    { return intProperty(TableCellColSpan, 1); }

    inline void setNonBreakableLines(bool b)
    { setProperty(BlockNonBreakableLines, b); }
    inline bool nonBreakableLines() const
    { return boolProperty(BlockNonBreakableLines); }

    inline void setBackgroundColor(const QColor &color)
    { setProperty(BlockBackgroundColor, int(color.rgb())); }
    inline QColor backgroundColor() const
    { if (hasProperty(BlockBackgroundColor)) return QColor(intProperty(BlockBackgroundColor)); else return QColor(); }
};

class Q_GUI_EXPORT QTextListFormat : public QTextFormat
{
public:
    inline QTextListFormat() : QTextFormat(ListFormat) {}

    bool isValid() const { return isListFormat(); }

    enum Style {
        ListDisc = -1,
        ListCircle = -2,
        ListSquare = -3,
        ListDecimal = -4,
        ListLowerAlpha = -5,
        ListUpperAlpha = -6
#ifndef Q_QDOC
        , ListStyleUndefined = 0
#endif
    };

    inline void setStyle(int style)
    { setProperty(ListStyle, style); }
    inline int style() const
    { return intProperty(ListStyle, ListStyleUndefined); }

    inline void setIndent(int indent)
    { setProperty(ListIndent, indent); }
    inline int indent() const
    { return intProperty(ListIndent); }

};

class Q_GUI_EXPORT QTextTableFormat : public QTextFormat
{
public:
    inline QTextTableFormat() : QTextFormat(TableFormat) {}

    bool isValid() const { return isTableFormat(); }

    inline void setBorder(int border)
    { setProperty(TableBorder, border); }
    inline int border() const
    { return intProperty(TableBorder, 1); }
};

class Q_GUI_EXPORT QTextImageFormat : public QTextCharFormat
{
public:
    inline QTextImageFormat() : QTextCharFormat() { setObjectType(ImageObject); }

    bool isValid() const { return isImageFormat(); }

    inline void setName(const QString &name)
    { setProperty(ImageName, name); }
    inline QString name() const
    { return stringProperty(ImageName); }

    inline void setWidth(int width)
    { setProperty(ImageWidth, width); }
    inline int width() const
    { return intProperty(ImageWidth); }

    inline void setHeight(int height)
    { setProperty(ImageHeight, height); }
    inline int height() const
    { return intProperty(ImageHeight); }
};

class Q_GUI_EXPORT QTextFloatFormat : public QTextFormat
{
public:
    inline QTextFloatFormat() : QTextFormat(FloatFormat) {}

    bool isValid() const { return isFloatFormat(); }

    enum Position {
        None,
        Left,
        Right
    };

    inline void setPosition(Position f)
    { setProperty(CssFloat, (int)f); }
    inline Position position() const
    { return (Position)intProperty(CssFloat, None); }
};

#endif
