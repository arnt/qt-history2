#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include "qobject_p.h"
#include <qmap.h>
#include <qfont.h>

class QTextFormatProperty
{
public:
    QTextFormatProperty() : type(QTextFormat::Undefined) {}

    QTextFormatProperty(bool value) : type(QTextFormat::Bool)
    { data.boolValue = value; }

    QTextFormatProperty(int value) : type(QTextFormat::Integer)
    { data.intValue = value; }

    QTextFormatProperty(float value) : type(QTextFormat::Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);

    QTextFormatProperty(const QTextFormatProperty &rhs) : type(QTextFormat::Undefined)
    { (*this) = rhs; }

    QTextFormatProperty &operator=(const QTextFormatProperty &rhs);

    ~QTextFormatProperty()
    { free(); }

    bool operator==(const QTextFormatProperty &rhs) const;
    bool operator!=(const QTextFormatProperty &rhs) const
    { return !operator==(rhs); }

    QTextFormat::PropertyType type;
    union {
	bool boolValue;
	int intValue;
	float floatValue;
	mutable void *ptr;
    } data;

    QString stringValue() const
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

    bool operator==(const QTextFormatPrivate &rhs) const;

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

bool QTextFormatPrivate::operator==(const QTextFormatPrivate &rhs) const
{
    if (properties.size() != rhs.properties.size())
	return false;

    PropertyMap::ConstIterator lhsIt = properties.begin();
    PropertyMap::ConstIterator rhsIt = rhs.properties.begin();
    for (; lhsIt != properties.end(); ++lhsIt, ++rhsIt)
	if ((lhsIt.key() != rhsIt.key()) ||
	    (lhsIt.value() != rhsIt.value()))
	    return false;

    return true;
}

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
    else if (other.d)
	d->properties += other.d->properties;

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

    return *d == *rhs.d;
}

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

