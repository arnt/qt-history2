#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include <private/qobject_p.h>
#include <qmap.h>
#include <qfont.h>

class QTextFormatProperty
{
public:
    inline QTextFormatProperty() : type(QTextFormat::Undefined) {}

    inline QTextFormatProperty(bool value) : type(QTextFormat::Bool)
    { data.boolValue = value; }

    inline QTextFormatProperty(int value) : type(QTextFormat::Integer)
    { data.intValue = value; }

    inline QTextFormatProperty(float value) : type(QTextFormat::Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);

    inline QTextFormatProperty(const QTextFormatProperty &rhs) : type(QTextFormat::Undefined)
    { (*this) = rhs; }

    inline QTextFormatProperty &operator=(const QTextFormatProperty &rhs);

    inline ~QTextFormatProperty()
    { free(); }

    bool operator==(const QTextFormatProperty &rhs) const;

    QTextFormat::PropertyType type;
    union {
	bool boolValue;
	int intValue;
	float floatValue;
	mutable void *ptr;
    } data;

    inline QString stringValue() const
    { return *reinterpret_cast<QString *>(&data.ptr); }

private:
    void free();
};

#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &debug, const QTextFormatProperty &property)
{
    switch (property.type) {
	case QTextFormat::Undefined: debug << "[Undefined]"; break;
	case QTextFormat::Bool: debug << "[" << "Bool:" << property.data.boolValue << "]"; break;
	case QTextFormat::Integer: debug << "[" << "Integer:" << property.data.intValue << "]"; break;
	case QTextFormat::Float: debug << "[" << "Float:" << property.data.floatValue << "]"; break;
	case QTextFormat::String: debug << "[" << "String:" << property.stringValue() << "]"; break;
	default: Q_ASSERT(false);
    }
    return debug;
}
#endif

class QTextFormatPrivate : public QSharedObject
{
public:
    typedef QMap<int, QTextFormatProperty> PropertyMap;

    PropertyMap properties;
};

QTextFormatProperty::QTextFormatProperty(const QString &value)
{
    type = QTextFormat::String;
    new (&data.ptr) QString(value);
}

QTextFormatProperty &QTextFormatProperty::operator=(const QTextFormatProperty &rhs)
{
    if ( this == &rhs )
	return *this;

    free();

    type = rhs.type;

    if (type == QTextFormat::String)
	new (&data.ptr) QString(rhs.stringValue());
    else
	data = rhs.data;

    return *this;
}

void QTextFormatProperty::free()
{
    if (type == QTextFormat::String)
	reinterpret_cast<QString *>(&data.ptr)->~QString();
}

bool QTextFormatProperty::operator==(const QTextFormatProperty &rhs) const
{
    if (type != rhs.type)
	return false;

    switch (type) {
	case QTextFormat::Undefined: return true;
	case QTextFormat::Bool: return data.boolValue == rhs.data.boolValue;
	case QTextFormat::FormatReference:
	case QTextFormat::Integer: return data.intValue == rhs.data.intValue;
	case QTextFormat::Float: return data.floatValue == rhs.data.floatValue;
	case QTextFormat::String: return stringValue() == rhs.stringValue();
    }

    return true;
}


/*!
    \class QTextFormat qtextformat.h
    \brief The format of a part of a QTextDocument

    \ingroup text

    A QTextFormat is a generic class used for describing the format of
    parts of a QTextDocument.  The derived classes QTextCharFormat,
    QTextBlockFormat, QTextListFormat and QTextTableFormat are usually
    more useful, and describe the formatting that gets applied to
    specific parts of the document.
*/

QTextFormat::QTextFormat()
    : _type(InvalidFormat), _inheritedType(InvalidFormat)
{
}

QTextFormat::QTextFormat(int type, int inheritedType)
    : _type(type), _inheritedType(inheritedType)
{
}

QTextFormat::QTextFormat(const QTextFormat &rhs)
{
    (*this) = rhs;
}

QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
    d = rhs.d;
    _type = rhs._type;
    _inheritedType = rhs._inheritedType;
    return *this;
}

QTextFormat::~QTextFormat()
{
}

QTextFormat &QTextFormat::operator+=(const QTextFormat &other)
{
    if (!d)
	d = other.d;
    else if (other.d) {
	// don't use QMap's += operator, as it uses insertMulti!
	for (QTextFormatPrivate::PropertyMap::ConstIterator it = other.d->properties.begin();
	     it != other.d->properties.end(); ++it) {
	    d->properties.insert(it.key(), it.value());
	}
    }

    return *this;
}

QTextBlockFormat QTextFormat::toBlockFormat() const
{
    QTextBlockFormat f;
    if (!isBlockFormat())
	return f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextCharFormat QTextFormat::toCharFormat() const
{
    QTextCharFormat f;
    if (!isCharFormat())
	return f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextListFormat QTextFormat::toListFormat() const
{
    QTextListFormat f;
    if (!isListFormat())
	return f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextTableFormat QTextFormat::toTableFormat() const
{
    QTextTableFormat f;
    if (!isTableFormat())
	return f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextImageFormat QTextFormat::toImageFormat() const
{
    QTextImageFormat f;
    if (!isImageFormat())
	return f;
    f.QTextFormat::operator=(*this);
    return f;
}

bool QTextFormat::boolProperty(int propertyId, bool defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Bool)
	return defaultValue;
    return prop.data.boolValue;
}

int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Integer)
	return defaultValue;
    return prop.data.intValue;
}

float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Float)
	return defaultValue;
    return prop.data.floatValue;
}

QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::String)
	return defaultValue;
    return prop.stringValue();
}

int QTextFormat::formatReferenceProperty(int propertyId, int defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::FormatReference)
	return defaultValue;
    return prop.data.intValue;
}

void QTextFormat::setProperty(int propertyId, bool value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, int value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, float value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, const QString &value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->properties.insert(propertyId, value);
}

void QTextFormat::setFormatReferenceProperty(int propertyId, int value)
{
    if (!d)
	d = new QTextFormatPrivate;
    QTextFormatProperty prop;
    prop.type = FormatReference;
    prop.data.intValue = value;
    d->properties.insert(propertyId, prop);
}

bool QTextFormat::hasProperty(int propertyId) const
{
    if (!d)
	return false;
    return d->properties.contains(propertyId);
}

QTextFormat::PropertyType QTextFormat::propertyType(int propertyId) const
{
    if (!d)
	return QTextFormat::Undefined;

    return d->properties.value(propertyId).type;
}

QList<int> QTextFormat::allPropertyIds() const
{
    if (!d)
	return QList<int>();

    return d->properties.keys();
}

bool QTextFormat::operator==(const QTextFormat &rhs) const
{
    if (_type != rhs._type || _inheritedType != rhs._inheritedType)
	return false;

    if (!d)
	return !rhs.d;

    if (!rhs.d)
	return false;

    return d->properties == rhs.d->properties;
}


/*!
    \class QTextCharFormat qtextformat.h
    \brief The format of a character in a QTextDocument

    \ingroup text

    A QTextCharFormat describes the format to be applied to characters
    in a QTextDocument. It contains properties as font, color and
    anchors (if the character belongs to a hyperlink.
*/


void QTextCharFormat::setFont(const QFont &font)
{
    setFontFamily(font.family());
    setFontWeight(font.weight());
    setFontItalic(font.italic());
    setFontUnderline(font.underline());
    setFontOverline(font.overline());
    setFontStrikeOut(font.strikeOut());
    setFontFixedPitch(font.fixedPitch());
}

QFont QTextCharFormat::font() const
{
    QFont font;

    if (hasProperty(FontFamily))
	font.setFamily(fontFamily());

    if (hasProperty(FontPointSize))
	font.setPointSizeFloat(fontPointSize());

    if (hasProperty(FontWeight))
	font.setWeight(fontWeight());

    if (hasProperty(FontItalic))
	font.setItalic(fontItalic());

    if (hasProperty(FontUnderline))
	font.setUnderline(fontUnderline());

    if (hasProperty(FontOverline))
	font.setOverline(fontOverline());

    if (hasProperty(FontStrikeOut))
	font.setStrikeOut(fontStrikeOut());

    if (hasProperty(FontFixedPitch))
	font.setFixedPitch(fontFixedPitch());

    return font;
}

/*!
    \class QTextBlockFormat qtextformat.h
    \brief The format of a block of text in a QTextDocument

    \ingroup text

    A document is composed out a list of blocks, that each contain one
    paragraph of text. Each block has an associated block format
    describing block specific properties as alignment, margins,
    indentation and possibly references to list and table formats.
*/

/*!
    \class QTextListFormat qtextformat.h
    \brief The format of a list in a QTextDocument

    \ingroup text

    Several blocks in a document can together form a list. The list
    format is a generic format describing the properties common for
    the whole list, as list style and indentation.
*/

/*!
    \class QTextTableFormat qtextformat.h
    \brief The format of a table in a QTextDocument

    \ingroup text

    Several blocks in a document can together form a table. The table
    format is a generic format describing the properties common for
    the whole list, as border style and width.
*/

/*!
    \class QTextImageFormat qtextformat.h
    \brief The format of an image in a QTextDocument

    \ingroup text

    Inline images are represented by an object replacement character
    (U+fffc in Unicode) with a special image format. The image format
    contains a name property to locate the image and width and height
    for the images dimension.
*/


// ------------------------------------------------------

int QTextFormatCollection::indexToReference(int idx) const
{
    if (idx >= -1)
	return -1;

    idx = -(idx + 2);
    if (idx >= formatReferences.count())
	return -1;

    return idx;
}

int QTextFormatCollection::referenceToIndex(int ref) const
{
    if (ref < 0 || ref >= formatReferences.count())
	return -1;

    return -(ref + 2);
}

int QTextFormatCollection::indexForFormat(const QTextFormat &format)
{
    // certainly need speedup
    for (int i = 0; i < formats.size(); ++i)
	if (formats[i] == format)
	    return i;

    int idx = formats.size();
    formats.append(format);
    return idx;
}

bool QTextFormatCollection::hasFormatCached(const QTextFormat &format) const
{
    Q_FOREACH(const QTextFormat &f, formats)
	if (f == format)
	    return true;
    return false;
}

int QTextFormatCollection::createReferenceIndex(const QTextFormat &format)
{
    int formatIdx = indexForFormat(format);

    int ref = formatReferences.size();
    formatReferences.append(formatIdx);
    return referenceToIndex(ref);
}

QTextFormat QTextFormatCollection::updateReferenceIndex(int index, const QTextFormat &newFormat)
{
    int ref = indexToReference(index);
    if (ref == -1)
	return QTextFormat();

    int oldIdx = formatReferences[ref];
    formatReferences[ref] = indexForFormat(newFormat);

    Q_ASSERT(referenceToIndex(ref) == index);

    return format(oldIdx);
}

QTextFormat QTextFormatCollection::format(int idx, int defaultFormatType) const
{
    if (idx == -1 || idx > formats.count())
	return QTextFormat(defaultFormatType);

    if (idx < 0) {
	int ref = indexToReference(idx);
	if (ref == -1)
	    return QTextFormat(defaultFormatType);

	idx = formatReferences[ref];
	Q_ASSERT(idx >= 0 && idx < formats.count());
    }

    return QTextFormat(formats[idx]);
}

