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

#ifndef QTEXTFORMAT_H
#define QTEXTFORMAT_H

#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qvector.h>

class QString;
class QVariant;
class QFont;

class QTextFormatCollection;
class QTextFormatPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextFrameFormat;
class QTextImageFormat;
class QTextFormat;
class QTextObject;
class QTextCursor;
class QTextDocument;
class QTextLength;

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextLength &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextLength &);

class Q_GUI_EXPORT QTextLength
{
public:
    enum Type { VariableLength = 0, FixedLength, PercentageLength };

    inline QTextLength() : lengthType(VariableLength), fixedValueOrPercentage(0) {}

    inline explicit QTextLength(Type type, int value)
        : lengthType(type), fixedValueOrPercentage(value) {}

    inline Type type() const { return lengthType; }
    inline int value(int maximumLength) const
    {
        switch (lengthType) {
            case FixedLength: return fixedValueOrPercentage;
            case VariableLength: return maximumLength;
            case PercentageLength: return fixedValueOrPercentage * maximumLength / 100;
        }
        return -1;
    }

    inline int rawValue() const { return fixedValueOrPercentage; }

    inline bool operator==(const QTextLength &other) const
    { return lengthType == other.lengthType && fixedValueOrPercentage == other.fixedValueOrPercentage; }
    inline bool operator!=(const QTextLength &other) const
    { return lengthType != other.lengthType || fixedValueOrPercentage != other.fixedValueOrPercentage; }
    operator QVariant() const;

private:
    Type lengthType;
    int fixedValueOrPercentage;
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextLength &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextLength &);
};

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextFormat &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextFormat &);

class Q_GUI_EXPORT QTextFormat
{
public:
    enum FormatType {
        InvalidFormat = -1,
        BlockFormat = 1,
        CharFormat = 2,
        ListFormat = 3,
        TableFormat = 4,
        FrameFormat = 5,

        UserFormat = 100
    };

    enum Property {
        ObjectIndex = 0x0,

        // paragraph and char
        CssFloat = 0x0800,
        LayoutDirection = 0x0801,

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

        TextUnderlineColor = 0x2010,
        TextBackgroundColor = 0x2011,

        TextColor = 0x2020,
        TextVerticalAlignment = 0x2021,

        IsAnchor = 0x2030,
        AnchorHref = 0x2031,
        AnchorName = 0x2032,

        ObjectType = 0x2f00,

        // list properties
        ListStyle = 0x3000,
        ListIndent = 0x3001,

        // table and frame properties
        FrameBorder = 0x4000,
        FrameMargin = 0x4001,
        FramePadding = 0x4002,
        FrameWidth = 0x4003,
        FrameHeight = 0x4004,
        TableColumns = 0x4100,
        TableColumnWidthConstraints = 0x4101,
        TableCellSpacing = 0x4102,
        TableCellPadding = 0x4103,
        TableBackgroundColor = 0x4104,

        // table cell properties
        TableCellRowSpan = 0x4810,
        TableCellColumnSpan = 0x4811,
        TableCellBackgroundColor = 0x4820,

        // image properties
        ImageName = 0x5000,
        ImageWidth = 0x5010,
        ImageHeight = 0x5011,

        // internal properties
        DocumentFragmentMark = 0x6000,

        // --
        UserProperty = 0x100000
    };

    enum ObjectTypes {
        NoObject,
        ImageObject,
        TableObject
    };

    QTextFormat();

    explicit QTextFormat(int type);

    QTextFormat(const QTextFormat &rhs);
    QTextFormat &operator=(const QTextFormat &rhs);
    ~QTextFormat();

    void merge(const QTextFormat &other);

    inline bool isValid() const { return type() != InvalidFormat; }

    int type() const;

    int objectIndex() const;
    void setObjectIndex(int object);

    QVariant property(int propertyId) const;
    void setProperty(int propertyId, const QVariant &value);
    void clearProperty(int propertyId);
    bool hasProperty(int propertyId) const;

    bool boolProperty(int propertyId) const;
    int intProperty(int propertyId) const;
    float floatProperty(int propertyId) const;
    QString stringProperty(int propertyId) const;
    QColor colorProperty(int propertyId) const;
    QTextLength lengthProperty(int propertyId) const;
    QVector<QTextLength> lengthVectorProperty(int propertyId) const;

    void setProperty(int propertyId, bool value);
    void setProperty(int propertyId, int value);
    void setProperty(int propertyId, float value);
    void setProperty(int propertyId, const QString &value);
    void setProperty(int propertyId, const QColor &value);
    void setProperty(int propertyId, const QTextLength &length);
    void setProperty(int propertyId, const QVector<QTextLength> &lengths);


    QMap<int, QVariant> properties() const;

    inline void setObjectType(int type)
    { setProperty(ObjectType, type); }
    inline int objectType() const
    { return intProperty(ObjectType); }

    inline bool isCharFormat() const { return type() == CharFormat; }
    inline bool isBlockFormat() const { return type() == BlockFormat; }
    inline bool isListFormat() const { return type() == ListFormat; }
    inline bool isFrameFormat() const { return type() == FrameFormat; }
    inline bool isImageFormat() const { return type() == CharFormat && objectType() == ImageObject; }
    inline bool isTableFormat() const { return type() == FrameFormat && objectType() == TableObject; }

    QTextBlockFormat toBlockFormat() const;
    QTextCharFormat toCharFormat() const;
    QTextListFormat toListFormat() const;
    QTextTableFormat toTableFormat() const;
    QTextFrameFormat toFrameFormat() const;
    QTextImageFormat toImageFormat() const;

    bool operator==(const QTextFormat &rhs) const;
    inline bool operator!=(const QTextFormat &rhs) const { return !operator==(rhs); }
    operator QVariant() const;

    inline void setLayoutDirection(Qt::LayoutDirection direction)
        { setProperty(QTextFormat::LayoutDirection, direction); }
    inline Qt::LayoutDirection layoutDirection() const
        { return (Qt::LayoutDirection)intProperty(QTextFormat::LayoutDirection); }

private:
    QSharedDataPointer<QTextFormatPrivate> d;
    friend class QTextFormatCollection;
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextFormat &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextFormat &);
};

class Q_GUI_EXPORT QTextCharFormat : public QTextFormat
{
public:
    enum VerticalAlignment { AlignNormal = 0, AlignSuperScript, AlignSubScript };

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
    { return floatProperty(FontPointSize); }

    inline void setFontWeight(int weight)
    { if (weight == QFont::Normal) weight = 0; setProperty(FontWeight, weight); }
    inline int fontWeight() const
    { int weight = intProperty(FontWeight); if (weight == 0) weight = QFont::Normal; return weight; }
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

    inline void setUnderlineColor(const QColor &color)
    { setProperty(TextUnderlineColor, color); }
    inline QColor underlineColor() const
    { return colorProperty(TextUnderlineColor); }

    inline void setBackgroundColor(const QColor &color)
    { setProperty(TextBackgroundColor, color); }
    inline void clearBackgroundColor()
    { clearProperty(TextBackgroundColor); }
    inline QColor backgroundColor() const
    { return colorProperty(TextBackgroundColor); }

    inline void setFontFixedPitch(bool fixedPitch)
    { setProperty(FontFixedPitch, fixedPitch); }
    inline bool fontFixedPitch() const
    { return boolProperty(FontFixedPitch); }

    inline void setTextColor(const QColor &color)
    { setProperty(TextColor, color); }
    inline QColor textColor() const
    { return colorProperty(TextColor); }

    inline void setVerticalAlignment(VerticalAlignment alignment)
    { setProperty(TextVerticalAlignment, alignment); }
    inline VerticalAlignment verticalAlignment() const
    { return static_cast<VerticalAlignment>(intProperty(TextVerticalAlignment)); }

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

    inline void setTableCellRowSpan(int tableCellRowSpan)
    { if (tableCellRowSpan == 1) tableCellRowSpan = 0; setProperty(TableCellRowSpan, tableCellRowSpan); }
    inline int tableCellRowSpan() const
    { int s = intProperty(TableCellRowSpan); if (s == 0) s = 1; return s; }
    inline void setTableCellColumnSpan(int tableCellColumnSpan)
    { if (tableCellColumnSpan == 1) tableCellColumnSpan = 0; setProperty(TableCellColumnSpan, tableCellColumnSpan); }
    inline int tableCellColumnSpan() const
    { int s = intProperty(TableCellColumnSpan); if (s == 0) s = 1; return s; }

    inline void setTableCellBackgroundColor(const QColor &color)
    { setProperty(TableCellBackgroundColor, color); }
    inline QColor tableCellBackgroundColor() const
    { return colorProperty(TableCellBackgroundColor); }
};

class Q_GUI_EXPORT QTextBlockFormat : public QTextFormat
{
public:
    enum Direction { AutoDirection, LeftToRight, RightToLeft };

    inline QTextBlockFormat() : QTextFormat(BlockFormat) {}

    bool isValid() const { return isBlockFormat(); }

    inline void setDirection(Direction dir)
    { setProperty(BlockDirection, dir); }
    inline Direction direction() const
    { return static_cast<Direction>(intProperty(BlockDirection)); }

    inline void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment)); }
    inline Qt::Alignment alignment() const
    { return QFlag(intProperty(BlockAlignment)); }

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

    inline void setNonBreakableLines(bool b)
    { setProperty(BlockNonBreakableLines, b); }
    inline bool nonBreakableLines() const
    { return boolProperty(BlockNonBreakableLines); }

    inline void setBackgroundColor(const QColor &color)
    { setProperty(BlockBackgroundColor, color); }
    inline void clearBackgroundColor()
    { clearProperty(BlockBackgroundColor); }
    inline QColor backgroundColor() const
    { return colorProperty(BlockBackgroundColor); }
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
        ListUpperAlpha = -6,
        ListStyleUndefined = 0
    };

    inline void setStyle(Style style)
    { setProperty(ListStyle, style); }
    inline Style style() const
    { return static_cast<Style>(intProperty(ListStyle)); }

    inline void setIndent(int indent)
    { setProperty(ListIndent, indent); }
    inline int indent() const
    { return intProperty(ListIndent); }

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

class Q_GUI_EXPORT QTextFrameFormat : public QTextFormat
{
public:
    inline QTextFrameFormat() : QTextFormat(FrameFormat) {}

    bool isValid() const { return isFrameFormat(); }

    enum Position {
        InFlow,
        FloatLeft,
        FloatRight
        // ######
//        Absolute
    };

    inline void setPosition(Position f)
    { setProperty(CssFloat, f); }
    inline Position position() const
    { return static_cast<Position>(intProperty(CssFloat)); }

    inline void setBorder(int border)
    { setProperty(FrameBorder, border); }
    inline int border() const
    { return intProperty(FrameBorder); }

    inline void setMargin(int margin)
    { setProperty(FrameMargin, margin); }
    inline int margin() const
    { return intProperty(FrameMargin); }

    inline void setPadding(int padding)
    { setProperty(FramePadding, padding); }
    inline int padding() const
    { return intProperty(FramePadding); }

    inline void setWidth(int width)
    { setProperty(FrameWidth, QTextLength(QTextLength::FixedLength, width)); }
    inline void setWidth(const QTextLength &length)
    { setProperty(FrameWidth, length); }
    inline QTextLength width() const
    { return lengthProperty(FrameWidth); }

    inline void setHeight(int height)
    { setProperty(FrameHeight, QTextLength(QTextLength::FixedLength, height)); }
    inline void setHeight(const QTextLength &height)
    { setProperty(FrameHeight, height); }
    inline QTextLength height() const
    { return lengthProperty(FrameHeight); }


};

class Q_GUI_EXPORT QTextTableFormat : public QTextFrameFormat
{
public:
    QTextTableFormat();

    inline bool isValid() const { return isTableFormat(); }

    inline int columns() const
    { int cols = intProperty(TableColumns); if (cols == 0) cols = 1; return cols; }
    inline void setColumns(int columns)
    { if (columns == 1) columns = 0; setProperty(TableColumns, columns); }

    inline void setColumnWidthConstraints(const QVector<QTextLength> &constraints)
    { setProperty(TableColumnWidthConstraints, constraints); }

    inline QVector<QTextLength> columnWidthConstraints() const
    { return lengthVectorProperty(TableColumnWidthConstraints); }

    inline void clearColumnWidthConstraints()
    { clearProperty(TableColumnWidthConstraints); }

    inline int cellSpacing() const
    { return intProperty(TableCellSpacing); }
    inline void setCellSpacing(int spacing)
    { setProperty(TableCellSpacing, spacing); }

    inline int cellPadding() const
    { return intProperty(TableCellPadding); }
    inline void setCellPadding(int padding)
    { setProperty(TableCellPadding, padding); }

    inline void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment)); }
    inline Qt::Alignment alignment() const
    { return QFlag(intProperty(BlockAlignment)); }

    inline void setBackgroundColor(const QColor &color)
    { setProperty(TableBackgroundColor, color); }
    inline void clearBackgroundColor()
    { clearProperty(TableBackgroundColor); }
    inline QColor backgroundColor() const
    { return colorProperty(TableBackgroundColor); }
};

#endif // QTEXTFORMAT_H
