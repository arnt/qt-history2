#ifndef QTEXTFORMAT_H
#define QTEXTFORMAT_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>
#include <qvector.h>
#include <qstylesheet.h>

class QString;
class QByteArray;

class QTextFormatCollection;
class QTextFormatCollectionPrivate;
struct QTextFormatProperty;
class QTextFormatPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;

class QTextFormat
{
    friend class QTextFormatCollection;
public:
    enum FormatType { 
	BlockFormat = 1, 
	CharFormat = 2, 
	ListFormat = 3 ,

	UserFormat = 100,
    };

    enum Property {
	// paragraph
	BlockDirection = 0x1000,
	BlockAlignment = 0x1010,
	BlockListFormatIndex = 0x1020,
	BlockTopMargin = 0x1030,
	BlockBottomMargin = 0x1031,
	BlockLeftMargin = 0x1032,
	BlockRightMargin = 0x1033,
	BlockFirstLineMargin = 0x1034,
	BlockIndent = 0x1040,

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

	// list properties
	ListStyle = 0x3000,
	ListIndent = 0x3001,

	// --
	UserProperty = 0x10000,
    };

    enum PropertyType {
	Undefined,
	Boolean,
	Integer,
	Float,
	String,
	Binary,
	FormatReference
    };

    QTextFormat(int type, int inheritedType = -1);

    Q_EXPLICIT QTextFormat(const QTextFormatPrivate &priv);

    QTextFormat(const QTextFormat &rhs);
    QTextFormat &operator=(const QTextFormat &rhs);
    ~QTextFormat();

    void merge(const QTextFormat &other);

    int type() const;
    int inheritedType() const;

    bool inheritsFormatType(int otherType) const
    { return type() == otherType || inheritedType() == otherType; }

    bool isCharFormat() const { return inheritsFormatType(CharFormat); }
    bool isBlockFormat() const { return inheritsFormatType(BlockFormat); }
    bool isListFormat() const { return inheritsFormatType(ListFormat); }

    QTextBlockFormat toBlockFormat() const;
    QTextCharFormat toCharFormat() const;
    QTextListFormat toListFormat() const;

    bool booleanProperty(int propertyId, bool defaultValue = false) const;
    int intProperty(int propertyId, int defaultValue = 0) const;
    float floatProperty(int propertyId, float defaultValue = 0.0) const;
    QString stringProperty(int propertyId, const QString &defaultValue = QString::null) const;
    QByteArray binaryProperty(int propertyId, QByteArray defaultValue = QByteArray() ) const;

    void setProperty(int propertyId, bool value);
    void setProperty(int propertyId, int value);
    void setProperty(int propertyId, float value);
    void setProperty(int propertyId, const QString &value);
    void setProperty(int propertyId, const QByteArray &value);

    bool hasProperty(int propertyId) const;
    PropertyType propertyType(int propertyId) const;

private:
    QTextFormatPrivate *d;
};

class QTextCharFormat : public QTextFormat
{
public:
    QTextCharFormat() : QTextFormat(CharFormat) {}

    Q_EXPLICIT QTextCharFormat(const QTextFormatPrivate &priv) : QTextFormat(priv) {}

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
    { return booleanProperty(FontItalic); }

    void setFontUnderline(bool underline)
    { setProperty(FontUnderline, underline); }
    bool fontUnderline() const
    { return booleanProperty(FontUnderline); }

    void setFontOverline(bool overline)
    { setProperty(FontOverline, overline); }
    bool fontOverline() const
    { return booleanProperty(FontOverline); }

    void setFontStrikeOut(bool strikeOut)
    { setProperty(FontStrikeOut, strikeOut); }
    bool fontStrikeOut() const
    { return booleanProperty(FontStrikeOut); }

    void setFontFixedPitch(bool fixedPitch)
    { setProperty(FontFixedPitch, fixedPitch); }
    bool fontFixedPitch() const
    { return booleanProperty(FontFixedPitch); }

    void setColor(const QColor &color)
    { setProperty(Color, int(color.rgb())); }
    QColor color() const
    { return QColor(intProperty(Color)); }

    void setAnchor(bool anchor)
    { setProperty(IsAnchor, anchor); }
    bool isAnchor() const
    { return booleanProperty(IsAnchor); }

    void setAnchorHref(const QString &value)
    { setProperty(AnchorHref, value); }
    QString anchorHref() const
    { return stringProperty(AnchorHref); }

    void setAnchorName(const QString &name)
    { setProperty(AnchorName, name); }
    QString anchorName() const
    { return stringProperty(AnchorName); }

protected:
    QTextCharFormat(int type) : QTextFormat(type, CharFormat) {}
};

class QTextBlockFormat : public QTextCharFormat
{
public:
    enum Direction { LeftToRight, RightToLeft, AutoDirection };

    QTextBlockFormat() : QTextCharFormat(BlockFormat) {}

    Q_EXPLICIT QTextBlockFormat(const QTextFormatPrivate &priv) : QTextCharFormat(priv) {}

    void setDirection(Direction dir)
    { setProperty(BlockDirection, dir); }
    Direction direction() const
    { return static_cast<Direction>(intProperty(BlockDirection, AutoDirection)); }

    void setAlignment(Qt::Alignment alignment)
    { setProperty(BlockAlignment, int(alignment)); }
    Qt::Alignment alignment() const
    { return intProperty(BlockAlignment); }

    void setListFormatIndex(int idx)
    { setProperty(BlockListFormatIndex, idx); }
    int listFormatIndex() const
    { return intProperty(BlockListFormatIndex, -1); }

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

};

class QTextListFormat : public QTextFormat
{
public:
    QTextListFormat() : QTextFormat(ListFormat) {}

    Q_EXPLICIT QTextListFormat(const QTextFormatPrivate &priv) : QTextFormat(priv) {}

    void setStyle(QStyleSheetItem::ListStyle style)
    { setProperty(ListStyle, int(style)); }
    QStyleSheetItem::ListStyle style() const
    { return static_cast<QStyleSheetItem::ListStyle>(intProperty(ListStyle, QStyleSheetItem::ListStyleUndefined)); }

    void setIndent(int indent)
    { setProperty(ListIndent, indent); }
    int indent() const
    { return intProperty(ListIndent); }

};

class QTextFormatCollection : public QObject
{
    Q_OBJECT
public:
    QTextFormatCollection(QObject *parent = 0);
    ~QTextFormatCollection();

    int createReferenceIndex(const QTextFormat &newFormat);
    void updateReferenceIndex(int index, const QTextFormat &newFormat);

    int indexForFormat(const QTextFormat &f);

    QTextFormat format(int idx, int defaultFormatType = -1) const;

    QTextBlockFormat blockFormat(int index) const
    { return format(index, QTextFormat::BlockFormat).toBlockFormat(); }
    QTextCharFormat charFormat(int index) const
    { return format(index, QTextFormat::CharFormat).toCharFormat(); }
    QTextListFormat listFormat(int index) const
    { return format(index, QTextFormat::ListFormat).toListFormat(); }

    int numFormats() const;

private:
#if defined(Q_DISABLE_COPY)
    QTextFormatCollection(const QTextFormatCollection &);
    QTextFormatCollection &operator=(const QTextFormatCollection &);
#endif
    Q_DECL_PRIVATE(QTextFormatCollection);
};

#endif
