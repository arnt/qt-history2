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

#include <qcolor.h>
#include <qshareddata.h>
#include <qobject.h>
#include <qfont.h>

class QString;

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

class Q_GUI_EXPORT QTextLength
{
    friend QDataStream &operator<<(QDataStream &, const QTextLength &);
    friend QDataStream &operator>>(QDataStream &, QTextLength &);
public:
    enum Type { VariableLength = 0, FixedLength, PercentageLength };

    inline QTextLength() : lengthType(VariableLength), fixedValueOrPercentage(0) {}

    inline Q_EXPLICIT QTextLength(Type type, int value)
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

private:
    Type lengthType;
    int fixedValueOrPercentage;
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
        FrameFormat = 5,

        UserFormat = 100
    };

    enum Property {
        ObjectIndex = 0x0,

        // paragraph and char
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

        TextColor = 0x2010,

        IsAnchor = 0x2020,
        AnchorHref = 0x2021,
        AnchorName = 0x2022,

        ObjectType = 0x2f00,

        // list properties
        ListStyle = 0x3000,
        ListIndent = 0x3001,

        // table and frame properties
        TableColumns = 0x4000,
        FrameBorder = 0x4080,
        FrameMargin = 0x4081,
        FramePadding = 0x4082,
        Width = 0x4100,
        Height = 0x4101,
        TableColumnWidthConstraints = 0x4200,
        TableCellSpacing = 0x4300,
        TableCellPadding = 0x4301,
        TableBackgroundColor = 0x4400,

        // table cell properties
        TableCellRowSpan = 0x4810,
        TableCellColumnSpan = 0x4811,
        TableCellBackgroundColor = 0x4820,

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
        FormatObject,
        Color,
        Length,
        LengthVector
    };

    enum ObjectTypes {
        NoObject,
        ImageObject,
        TableObject
    };

    QTextFormat();

    Q_EXPLICIT QTextFormat(int type);

    QTextFormat(const QTextFormat &rhs);
    QTextFormat &operator=(const QTextFormat &rhs);
    ~QTextFormat();

    void merge(const QTextFormat &other);

    inline bool isValid() const { return type() != InvalidFormat; }

    int type() const;

    int objectIndex() const;
    void setObjectIndex(int object);

    bool boolProperty(int propertyId, bool defaultValue = false) const;
    int intProperty(int propertyId, int defaultValue = 0) const;
    float floatProperty(int propertyId, float defaultValue = 0.0) const;
    QString stringProperty(int propertyId, const QString &defaultValue = QString::null) const;
    QColor colorProperty(int propertyId, const QColor &defaultValue = QColor()) const;
    QTextLength lengthProperty(int propertyId) const;
    QVector<QTextLength> lengthVectorProperty(int propertyId) const;

    void setProperty(int propertyId, bool value);
    void setProperty(int propertyId, int value);
    void setProperty(int propertyId, float value);
    void setProperty(int propertyId, const QString &value);
    void setProperty(int propertyId, const QColor &value);
    void setProperty(int propertyId, const QTextLength &length);
    void setProperty(int propertyId, const QVector<QTextLength> &lengths);

    //#### I would prefer to remove the next 5. Weird API and they
    //#### safe little. Instead the caller can simply have one
    //#### if-statement and call clearProperty().
    void setProperty(int propertyId, bool value, bool defaultValue);
    void setProperty(int propertyId, int value, int defaultValue);
    void setProperty(int propertyId, float value, float defaultValue);
    void setProperty(int propertyId, const QString &value, const QString &defaultValue);
    void setProperty(int propertyId, const QColor &value, const QColor &defaultValue);

    void clearProperty(int propertyId);

    bool hasProperty(int propertyId) const;

    QList<int> allPropertyIds() const;

    inline void setObjectType(int type)
    { setProperty(ObjectType, type, NoObject); }
    inline int objectType() const
    { return intProperty(ObjectType, NoObject); }

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

private:
    QSharedDataPointer<QTextFormatPrivate> d;
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
    { return floatProperty(FontPointSize); }

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

    inline void setTextColor(const QColor &color)
    { setProperty(TextColor, color); }
    inline QColor textColor() const
    { return colorProperty(TextColor); }

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
    { setProperty(TableCellRowSpan, tableCellRowSpan, 1); }
    inline int tableCellRowSpan() const
    { return intProperty(TableCellRowSpan, 1); }
    inline void setTableCellColumnSpan(int tableCellColumnSpan)
    { setProperty(TableCellColumnSpan, tableCellColumnSpan, 1); }
    inline int tableCellColumnSpan() const
    { return intProperty(TableCellColumnSpan, 1); }

    inline void setTableCellBackgroundColor(const QColor &color)
    { setProperty(TableCellBackgroundColor, color); }
    inline QColor tableCellBackgroundColor() const
    { return colorProperty(TableCellBackgroundColor); }

};

class Q_GUI_EXPORT QTextBlockFormat : public QTextFormat
{
public:
    enum Direction { LeftToRight, RightToLeft, AutoDirection };

    inline QTextBlockFormat() : QTextFormat(BlockFormat) {}

    bool isValid() const { return isBlockFormat(); }

    inline void setDirection(Direction dir)
    { setProperty(BlockDirection, dir, AutoDirection); }
    inline Direction direction() const
    { return static_cast<Direction>(intProperty(BlockDirection, AutoDirection)); }

    inline void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment), Qt::AlignAuto); }
    inline Qt::Alignment alignment() const
    { return QFlag(intProperty(BlockAlignment, Qt::AlignAuto)); }

    inline void setTopMargin(int margin)
    { setProperty(BlockTopMargin, margin, 0); }
    inline int topMargin() const
    { return intProperty(BlockTopMargin, 0); }

    inline void setBottomMargin(int margin)
    { setProperty(BlockBottomMargin, margin, 0); }
    inline int bottomMargin() const
    { return intProperty(BlockBottomMargin, 0); }

    inline void setLeftMargin(int margin)
    { setProperty(BlockLeftMargin, margin, 0); }
    inline int leftMargin() const
    { return intProperty(BlockLeftMargin, 0); }

    inline void setRightMargin(int margin)
    { setProperty(BlockRightMargin, margin, 0); }
    inline int rightMargin() const
    { return intProperty(BlockRightMargin, 0); }

    inline void setFirstLineMargin(int margin)
    { setProperty(BlockFirstLineMargin, margin, 0); }
    inline int firstLineMargin() const
    { return intProperty(BlockFirstLineMargin, 0); }

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

    inline void setStyle(int style)
    { setProperty(ListStyle, style, ListStyleUndefined); }
    inline int style() const
    { return intProperty(ListStyle, ListStyleUndefined); }

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
    { setProperty(CssFloat, f, InFlow); }
    inline Position position() const
    { return static_cast<Position>(intProperty(CssFloat, InFlow)); }

    inline void setBorder(int border)
    { setProperty(FrameBorder, border, 0); }
    inline int border() const
    { return intProperty(FrameBorder, 0); }

    inline void setMargin(int margin)
    { setProperty(FrameMargin, margin, 0); }
    inline int margin() const
    { return intProperty(FrameMargin, 0); }

    inline void setPadding(int padding)
    { setProperty(FramePadding, padding, 0); }
    inline int padding() const
    { return intProperty(FramePadding, 0); }

    inline void setWidth(int width)
    { setProperty(Width, QTextLength(QTextLength::FixedLength, width)); }
    inline void setWidth(const QTextLength &length)
    { setProperty(Width, length); }
    inline QTextLength width() const
    { return lengthProperty(Width); }

    inline void setHeight(int border)
    { setProperty(Height, border, -1); }
    inline int height() const
    { return intProperty(Height, -1); }


};

class Q_GUI_EXPORT QTextTableFormat : public QTextFrameFormat
{
public:
    inline QTextTableFormat() : QTextFrameFormat() { setObjectType(TableObject); }

    inline bool isValid() const { return isTableFormat(); }

    inline int columns() const
    { return intProperty(TableColumns, 1); }
    inline void setColumns(int columns)
    { setProperty(TableColumns, columns, 1); }

    inline void setColumnWidthConstraints(const QVector<QTextLength> &constraints)
    { setProperty(TableColumnWidthConstraints, constraints); }

    inline QVector<QTextLength> columnWidthConstraints() const
    { return lengthVectorProperty(TableColumnWidthConstraints); }

    inline void clearColumnWidthConstraints()
    { clearProperty(TableColumnWidthConstraints); }

    inline int cellSpacing() const
    { return intProperty(TableCellSpacing, 2); }
    inline void setCellSpacing(int spacing)
    { setProperty(TableCellSpacing, spacing, 2); }

    inline int cellPadding() const
    { return intProperty(TableCellPadding, 0); }
    inline void setCellPadding(int padding)
    { setProperty(TableCellPadding, padding, 0); }

    inline void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment), Qt::AlignAuto); }
    inline Qt::Alignment alignment() const
    { return QFlag(intProperty(BlockAlignment, Qt::AlignAuto)); }

    inline void setBackgroundColor(const QColor &color)
    { setProperty(TableBackgroundColor, color); }
    inline void clearBackgroundColor()
    { clearProperty(TableBackgroundColor); }
    inline QColor backgroundColor() const
    { return colorProperty(TableBackgroundColor); }
};

#endif
