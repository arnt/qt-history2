#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include "qobject_p.h"
#include <qmap.h>
#include <qfont.h>

enum { UndefinedIndex = -1 };

// ### use a placement new for bytearray/string
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

    bool isValid() const { return type != QTextFormat::Undefined; }

    bool operator==(const QTextFormatProperty &rhs) const;
    bool operator!=(const QTextFormatProperty &rhs) const
    { return !operator==(rhs); }

    QTextFormat::PropertyType type;
    union {
	bool boolValue;
	int intValue;
	float floatValue;
	void *ptr;
    } data;

    QString stringValue() const
    { return *static_cast<QString *>(data.ptr); }

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

typedef QMap<int, QTextFormatProperty> QTextFormatPropertyMap;

class QTextFormatPrivate : public QSharedObject
{
public:
    void clearProperty(int property)
    { properties.remove(property); }
    void setProperty(int propertyId, const QTextFormatProperty &property)
    { properties[propertyId] = property; }

    const QTextFormatProperty property(int propertyId, QTextFormat::PropertyType propType) const;

    bool operator==(const QTextFormatPrivate &rhs) const;

    QList<int> propertyIds() const { return properties.keys(); }

private:
    QTextFormatPropertyMap properties;
};

QTextFormatProperty::QTextFormatProperty(const QString &value)
{
    type = QTextFormat::String;
    data.ptr = new QString(value);
}

QTextFormatProperty &QTextFormatProperty::operator=(const QTextFormatProperty &rhs)
{
    if ( this == &rhs )
	return *this;

    free();

    type = rhs.type;

    if (type == QTextFormat::String)
	data.ptr = new QString(rhs.stringValue());
    else
	data = rhs.data;

    return *this;
}

void QTextFormatProperty::free()
{
    switch (type) {
	case QTextFormat::String: delete static_cast<QString *>(data.ptr); break;
	default: break;
    }
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

const QTextFormatProperty QTextFormatPrivate::property(int propertyId, QTextFormat::PropertyType propType) const
{
    const QTextFormatProperty prop = properties.value(propertyId);
    if (!prop.isValid() || (prop.type != propType && propType != QTextFormat::Undefined))
	return QTextFormatProperty();
    return prop;
}

bool QTextFormatPrivate::operator==(const QTextFormatPrivate &rhs) const
{
    if (properties.size() != rhs.properties.size())
	return false;

    QTextFormatPropertyMap::ConstIterator lhsIt = properties.begin();
    QTextFormatPropertyMap::ConstIterator rhsIt = rhs.properties.begin();
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

void QTextFormat::merge(const QTextFormat &other)
{
    if (!other.d)
	return;

    if (!d) {
	d = other.d;
	return;
    }

    Q_FOREACH(int propId, other.allPropertyIds()) {
	const QTextFormatProperty prop = other.d->property(propId, QTextFormat::Undefined);
	Q_ASSERT(prop.isValid());
	d->setProperty(propId, prop);
    }
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

    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Bool);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.boolValue;
}

int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Integer);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.intValue;
}

float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Float);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.floatValue;
}

QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::String);

    if (!prop.isValid())
	return defaultValue;

    return prop.stringValue();
}

int QTextFormat::formatReferenceProperty(int propertyId, int defaultValue) const
{
    if (!d)
	return defaultValue;

    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::FormatReference);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.intValue;
}

void QTextFormat::setProperty(int propertyId, bool value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, int value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, float value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, const QString &value)
{
    if (!d)
	d = new QTextFormatPrivate;
    d->setProperty(propertyId, value);
}

void QTextFormat::setFormatReferenceProperty(int propertyId, int value)
{
    if (!d)
	d = new QTextFormatPrivate;
    QTextFormatProperty prop;
    prop.type = FormatReference;
    prop.data.intValue = value;
    d->setProperty(propertyId, prop);
}

bool QTextFormat::hasProperty(int propertyId) const
{
    if (!d)
	return false;
    return d->property(propertyId, QTextFormat::Undefined).isValid();
}

QTextFormat::PropertyType QTextFormat::propertyType(int propertyId) const
{
    if (!d)
	return QTextFormat::Undefined;

    QTextFormatProperty prop = d->property(propertyId, QTextFormat::Undefined);
    if (!prop.isValid())
	return QTextFormat::Undefined;
    return prop.type;
}

QList<int> QTextFormat::allPropertyIds() const
{
    if (!d)
	return QList<int>();

    return d->propertyIds();
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

int QTextFormatCollection::indexForFormat(const QTextFormat &format) const
{
    // certainly need speedup
    for (int i = 0; i < formats.size(); ++i)
	if (formats[i] == format)
	    return i;

    int idx = formats.size();
    formats.append(format);
    return idx;
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

