#ifndef QTEXTFORMAT_H
#define QTEXTFORMAT_H

#ifndef QT_H
#include <qcolor.h>
#include <qsharedpointer.h>
#endif // QT_H

class QString;
class QFont;

class QTextFormatCollection;
class QTextFormatCollectionPrivate;
class QTextFormatProperty;
class QTextFormatPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextImageFormat;

class Q_GUI_EXPORT QTextFormat
{
    friend class QTextFormatCollection;
public:
    enum FormatType {
	InvalidFormat = -1,
	BlockFormat = 1,
	CharFormat = 2,
	ListFormat = 3,
	TableFormat = 4,
	ImageFormat = 5,

	UserFormat = 100
    };

    enum Property {
	// paragraph
	BlockDirection = 0x1000,
	BlockAlignment = 0x1010,
	BlockListFormatIndex = 0x1020,
	BlockTableFormatIndex = 0x1021,
	BlockTopMargin = 0x1030,
	BlockBottomMargin = 0x1031,
	BlockLeftMargin = 0x1032,
	BlockRightMargin = 0x1033,
	BlockFirstLineMargin = 0x1034,
	BlockIndent = 0x1040,
	BlockNonBreakableLines = 0x1050,

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
	BackgroundColor = 0x2011,

	IsAnchor = 0x2020,
	AnchorHref = 0x2021,
	AnchorName = 0x2022,

	NonDeletable = 0x2100,

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
	FormatReference
    };

    QTextFormat();

    Q_EXPLICIT QTextFormat(int type, int inheritedType = -1);

    QTextFormat(const QTextFormat &rhs);
    QTextFormat &operator=(const QTextFormat &rhs);
    ~QTextFormat();

    QTextFormat &operator+=(const QTextFormat &other);
    inline QTextFormat operator+(const QTextFormat &other) const
    { QTextFormat result(*this); result += other; return result; }

    bool isValid() const { return type() != -1; }

    int type() const { return _type; }
    int inheritedType() const { return _inheritedType; }

    bool inheritsFormatType(int otherType) const
    { return type() == otherType || inheritedType() == otherType; }

    bool isCharFormat() const { return inheritsFormatType(CharFormat); }
    bool isBlockFormat() const { return inheritsFormatType(BlockFormat); }
    bool isListFormat() const { return inheritsFormatType(ListFormat); }
    bool isTableFormat() const { return inheritsFormatType(TableFormat); }
    bool isImageFormat() const { return inheritsFormatType(ImageFormat); }

    QTextBlockFormat toBlockFormat() const;
    QTextCharFormat toCharFormat() const;
    QTextListFormat toListFormat() const;
    QTextTableFormat toTableFormat() const;
    QTextImageFormat toImageFormat() const;

    bool boolProperty(int propertyId, bool defaultValue = false) const;
    int intProperty(int propertyId, int defaultValue = 0) const;
    float floatProperty(int propertyId, float defaultValue = 0.0) const;
    QString stringProperty(int propertyId, const QString &defaultValue = QString::null) const;
    int formatReferenceProperty(int propertyId, int defaultValue = -1) const;

    void setProperty(int propertyId, bool value);
    void setProperty(int propertyId, int value);
    void setProperty(int propertyId, float value);
    void setProperty(int propertyId, const QString &value);
    void setFormatReferenceProperty(int propertyId, int value);

    bool hasProperty(int propertyId) const;
    PropertyType propertyType(int propertyId) const;

    QList<int> allPropertyIds() const;

    bool operator==(const QTextFormat &rhs) const;

private:
    QSharedPointer<QTextFormatPrivate> d;
    int _type;
    int _inheritedType;
};

class Q_GUI_EXPORT QTextCharFormat : public QTextFormat
{
public:
    QTextCharFormat() : QTextFormat(CharFormat) {}

    void setFont(const QFont &font);
    QFont font() const;

    void setFontFamily(const QString &family)
    { setProperty(FontFamily, family); }
    QString fontFamily() const
    { return stringProperty(FontFamily); }

    void setFontPointSize(float size)
    { setProperty(FontPointSize, size); }
    float fontPointSize() const
    { return floatProperty(FontPointSize); }

    void setFontWeight(int weight)
    { setProperty(FontWeight, weight); }
    int fontWeight() const
    { return intProperty(FontWeight); }

    void setFontItalic(bool italic)
    { setProperty(FontItalic, italic); }
    bool fontItalic() const
    { return boolProperty(FontItalic); }

    void setFontUnderline(bool underline)
    { setProperty(FontUnderline, underline); }
    bool fontUnderline() const
    { return boolProperty(FontUnderline); }

    void setFontOverline(bool overline)
    { setProperty(FontOverline, overline); }
    bool fontOverline() const
    { return boolProperty(FontOverline); }

    void setFontStrikeOut(bool strikeOut)
    { setProperty(FontStrikeOut, strikeOut); }
    bool fontStrikeOut() const
    { return boolProperty(FontStrikeOut); }

    void setFontFixedPitch(bool fixedPitch)
    { setProperty(FontFixedPitch, fixedPitch); }
    bool fontFixedPitch() const
    { return boolProperty(FontFixedPitch); }

    void setColor(const QColor &color)
    { setProperty(Color, int(color.rgb())); }
    QColor color() const
    { if (hasProperty(Color)) return QColor(intProperty(Color)); else return QColor(); }

    void setBackgroundColor(const QColor &color)
    { setProperty(BackgroundColor, int(color.rgb())); }
    QColor backgroundColor() const
    { if (hasProperty(BackgroundColor)) return QColor(intProperty(BackgroundColor)); else return QColor(); }

    void setAnchor(bool anchor)
    { setProperty(IsAnchor, anchor); }
    bool isAnchor() const
    { return boolProperty(IsAnchor); }

    void setAnchorHref(const QString &value)
    { setProperty(AnchorHref, value); }
    QString anchorHref() const
    { return stringProperty(AnchorHref); }

    void setAnchorName(const QString &name)
    { setProperty(AnchorName, name); }
    QString anchorName() const
    { return stringProperty(AnchorName); }

    void setNonDeletable(bool d)
    { setProperty(NonDeletable, d); }
    bool nonDeletable() const
    { return boolProperty(NonDeletable); }

protected:
    QTextCharFormat(int type) : QTextFormat(type, CharFormat) {}
};

class Q_GUI_EXPORT QTextBlockFormat : public QTextCharFormat
{
public:
    enum Direction { LeftToRight, RightToLeft, AutoDirection };

    QTextBlockFormat() : QTextCharFormat(BlockFormat) {}

    void setDirection(Direction dir)
    { setProperty(BlockDirection, dir); }
    Direction direction() const
    { return static_cast<Direction>(intProperty(BlockDirection, AutoDirection)); }

    void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment)); }
    Qt::Alignment alignment() const
    { return intProperty(BlockAlignment); }

    void setListFormatIndex(int idx)
    { setFormatReferenceProperty(BlockListFormatIndex, idx); }
    int listFormatIndex() const
    { return formatReferenceProperty(BlockListFormatIndex); }

    // ################# shouldn't we ensure you can only set one reference?
    // both a table and a list reference don't make sense
    void setTableFormatIndex(int idx)
    { setFormatReferenceProperty(BlockTableFormatIndex, idx); }
    int tableFormatIndex() const
    { return formatReferenceProperty(BlockTableFormatIndex); }

    void setTopMargin(int margin)
    { setProperty(BlockTopMargin, margin); }
    int topMargin() const
    { return intProperty(BlockTopMargin); }

    void setBottomMargin(int margin)
    { setProperty(BlockBottomMargin, margin); }
    int bottomMargin() const
    { return intProperty(BlockBottomMargin); }

    void setLeftMargin(int margin)
    { setProperty(BlockLeftMargin, margin); }
    int leftMargin() const
    { return intProperty(BlockLeftMargin); }

    void setRightMargin(int margin)
    { setProperty(BlockRightMargin, margin); }
    int rightMargin() const
    { return intProperty(BlockRightMargin); }

    void setFirstLineMargin(int margin)
    { setProperty(BlockFirstLineMargin, margin); }
    int firstLineMargin() const
    { return intProperty(BlockFirstLineMargin); }

    void setIndent(int indent)
    { setProperty(BlockIndent, indent); }
    int indent() const
    { return intProperty(BlockIndent); }

    void setTableCellEndOfRow(bool eor)
    { setProperty(TableCellEndOfRow, eor); }
    bool tableCellEndOfRow() const
    { return boolProperty(TableCellEndOfRow); }
    void setTableCellRowSpan(int tableCellRowSpan)
    { setProperty(TableCellRowSpan, tableCellRowSpan); }
    int tableCellRowSpan() const
    { return intProperty(TableCellRowSpan, 1); }
    void setTableCellColSpan(int tableCellColSpan)
    { setProperty(TableCellColSpan, tableCellColSpan); }
    int tableCellColSpan() const
    { return intProperty(TableCellColSpan, 1); }

    void setNonBreakableLines(bool b)
    { setProperty(BlockNonBreakableLines, b); }
    bool nonBreakableLines() const
    { return boolProperty(BlockNonBreakableLines); }
};

class Q_GUI_EXPORT QTextListFormat : public QTextFormat
{
public:
    QTextListFormat() : QTextFormat(ListFormat) {}

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

    void setStyle(int style)
    { setProperty(ListStyle, style); }
    int style() const
    { return intProperty(ListStyle, ListStyleUndefined); }

    void setIndent(int indent)
    { setProperty(ListIndent, indent); }
    int indent() const
    { return intProperty(ListIndent); }

};

class Q_GUI_EXPORT QTextTableFormat : public QTextFormat
{
public:
    QTextTableFormat() : QTextFormat(TableFormat) {}

    void setBorder(int border)
    { setProperty(TableBorder, border); }
    int border() const
    { return intProperty(TableBorder, 1); }
};

class Q_GUI_EXPORT QTextImageFormat : public QTextCharFormat
{
public:
    QTextImageFormat() : QTextCharFormat(ImageFormat) {}

    void setName(const QString &name)
    { setProperty(ImageName, name); }
    QString name() const
    { return stringProperty(ImageName); }

    void setWidth(int width)
    { setProperty(ImageWidth, width); }
    int width() const
    { return intProperty(ImageWidth); }

    void setHeight(int height)
    { setProperty(ImageHeight, height); }
    int height() const
    { return intProperty(ImageHeight); }
};

#endif
