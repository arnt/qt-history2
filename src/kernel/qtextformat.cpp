#include "qtextformat.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include "qobject_p.h"

enum { UndefinedIndex = -1 };

// ### use a placement new for bytearray/string
class QTextFormatProperty
{
public:
    QTextFormatProperty() : type(QTextFormat::Undefined) {}

    QTextFormatProperty(bool value) : type(QTextFormat::Boolean)
    { data.boolValue = value; }

    QTextFormatProperty(int value) : type(QTextFormat::Integer)
    { data.intValue = value; }

    QTextFormatProperty(float value) : type(QTextFormat::Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);
    QTextFormatProperty(const QByteArray &value);

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
    QByteArray binaryValue() const
    { return *static_cast<QByteArray *>(data.ptr); }

private:
    void assign( const QTextFormatProperty &rhs);
    void free();
};

#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &debug, const QTextFormatProperty &property)
{
    switch (property.type) {
	case QTextFormat::Undefined: debug << "[Undefined]"; break;
	case QTextFormat::Boolean: debug << "[" << "Boolean:" << property.data.boolValue << "]"; break;
	case QTextFormat::Integer: debug << "[" << "Integer:" << property.data.intValue << "]"; break;
	case QTextFormat::Float: debug << "[" << "Float:" << property.data.floatValue << "]"; break;
	case QTextFormat::String: debug << "[" << "String:" << property.stringValue() << "]"; break;
	case QTextFormat::Binary: debug << "[" << "Binary:" << property.binaryValue() << "]"; break;
	default: Q_ASSERT(false);
    }
    return debug;
}
#endif

typedef QMap<int, QTextFormatProperty> QTextFormatPropertyMap;

class QTextFormatPrivate
{
public:
    QTextFormatPrivate(int _type = -1, int _inheritedType = -1) : type(_type), inheritedType(_inheritedType) {}

    int type;
    int inheritedType;

    bool isValid() const { return type != -1; }

    void clearProperty(int property)
    { properties.remove(property); }
    void setProperty(int propertyId, const QTextFormatProperty &property)
    { properties[propertyId] = property; }

    const QTextFormatProperty property(int propertyId, QTextFormat::PropertyType propType) const;

    bool operator==(const QTextFormatPrivate &rhs) const;

    QTextFormatPropertyMap propertyMap() const { return properties; }

private:
    QTextFormatPropertyMap properties;
};

class QTextFormatCollectionPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QTextFormatCollection);
public:
    int indexToReference(int idx) const;
    int referenceToIndex(int ref) const;

    QVector<QTextFormatPrivate> formats;
    QVector<int> formatReferences;
};


QTextFormatProperty::QTextFormatProperty(const QString &value)
{
    type = QTextFormat::String;
    data.ptr = new QString(value);
}

QTextFormatProperty::QTextFormatProperty(const QByteArray &value)
{
    type = QTextFormat::Binary;
    data.ptr = new QByteArray(value);
}

QTextFormatProperty &QTextFormatProperty::operator=(const QTextFormatProperty &rhs)
{
    if ( this == &rhs )
	return *this;

    free();
    assign(rhs);

    return *this;
}

void QTextFormatProperty::assign(const QTextFormatProperty &rhs)
{
    type = rhs.type;
    data = rhs.data;
    switch (type) {
	case QTextFormat::String: data.ptr = new QString(*static_cast<QString *>(rhs.data.ptr)); break;
	case QTextFormat::Binary: data.ptr = new QByteArray(*static_cast<QByteArray *>(rhs.data.ptr)); break;
	default: break;
    }
}

void QTextFormatProperty::free()
{
    switch (type) {
	case QTextFormat::String: delete static_cast<QString *>(data.ptr); break;
	case QTextFormat::Binary: delete static_cast<QByteArray *>(data.ptr); break;
	default: break;
    }
}

bool QTextFormatProperty::operator==(const QTextFormatProperty &rhs) const
{
    if (type != rhs.type)
	return false;

    switch (type) {
	case QTextFormat::Undefined: return true;
	case QTextFormat::Boolean: return data.boolValue == rhs.data.boolValue;
	case QTextFormat::FormatReference:
	case QTextFormat::Integer: return data.intValue == rhs.data.intValue;
	case QTextFormat::Float: return data.floatValue == rhs.data.floatValue;
	case QTextFormat::String: return stringValue() == rhs.stringValue();
	case QTextFormat::Binary: return binaryValue() == rhs.binaryValue();
    }

    return true;
}

const QTextFormatProperty QTextFormatPrivate::property(int propertyId, QTextFormat::PropertyType propType) const
{
    const QTextFormatProperty prop = properties.value(propertyId);
    if (prop.isValid() && (prop.type == propType || propType == QTextFormat::Undefined))
	return prop;
    return QTextFormatProperty();
}

bool QTextFormatPrivate::operator==(const QTextFormatPrivate &rhs) const
{
    if (type != rhs.type || inheritedType != rhs.inheritedType)
	return false;

    if (properties.size() != rhs.properties.size())
	return false;

    QTextFormatPropertyMap::ConstIterator lhsIt = properties.begin();
    QTextFormatPropertyMap::ConstIterator rhsIt = rhs.properties.begin();
    for (; (lhsIt != properties.end()) && (rhsIt != rhs.properties.end()); ++lhsIt, ++rhsIt)
	if ((lhsIt.key() != rhsIt.key()) ||
	    (lhsIt.value() != rhsIt.value()))
	    return false;

    return true;
}

QTextFormat::QTextFormat()
    : d(new QTextFormatPrivate(-1, -1))
{
}

QTextFormat::QTextFormat(int type, int inheritedType)
    : d(new QTextFormatPrivate(type, inheritedType))
{
}

QTextFormat::QTextFormat(const QTextFormatPrivate &priv)
    : d(new QTextFormatPrivate(priv))
{
}

QTextFormat::QTextFormat(const QTextFormat &rhs)
    : d(new QTextFormatPrivate)
{
    (*this) = rhs;
}

QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
    if (&rhs == this)
	return *this;

    *d = *rhs.d;

    return *this;
}

QTextFormat::~QTextFormat()
{
    delete d;
}

void QTextFormat::merge(const QTextFormat &other)
{
    QList<int> allProps = other.d->propertyMap().keys();

    for (int i = 0; i < allProps.count(); ++i) {
	int propId = allProps.at(i);

	const QTextFormatProperty prop = other.d->property(propId, QTextFormat::Undefined);
	Q_ASSERT(prop.isValid());
	d->setProperty(propId, prop);
    }
}

int QTextFormat::type() const
{
    return d->type;
}

int QTextFormat::inheritedType() const
{
    return d->inheritedType;
}

QTextBlockFormat QTextFormat::toBlockFormat() const
{
    if (!isBlockFormat())
	return QTextBlockFormat();
    return QTextBlockFormat(*d);
}

QTextCharFormat QTextFormat::toCharFormat() const
{
    if (!isCharFormat())
	return QTextCharFormat();
    return QTextCharFormat(*d);
}

QTextListFormat QTextFormat::toListFormat() const
{
    if (!isListFormat())
	return QTextListFormat();
    return QTextListFormat(*d);
}

bool QTextFormat::booleanProperty(int propertyId, bool defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Boolean);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.boolValue;
}

int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Integer);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.intValue;
}

float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Float);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.floatValue;
}

QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::String);

    if (!prop.isValid())
	return defaultValue;

    return prop.stringValue();
}

QByteArray QTextFormat::binaryProperty(int propertyId, QByteArray defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::Binary);

    if (!prop.isValid())
	return defaultValue;

    return prop.binaryValue();
}

int QTextFormat::formatReferenceProperty(int propertyId, int defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormat::FormatReference);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.intValue;
}

void QTextFormat::setProperty(int propertyId, bool value)
{
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, int value)
{
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, float value)
{
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, const QString &value)
{
    d->setProperty(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, const QByteArray &value)
{
    d->setProperty(propertyId, value);
}

void QTextFormat::setFormatReferenceProperty(int propertyId, int value)
{
    QTextFormatProperty prop;
    prop.type = FormatReference;
    prop.data.intValue = value;
    d->setProperty(propertyId, prop);
}

bool QTextFormat::hasProperty(int propertyId) const
{
    return d->property(propertyId, QTextFormat::Undefined).isValid();
}

QTextFormat::PropertyType QTextFormat::propertyType(int propertyId) const
{
    QTextFormatProperty prop = d->property(propertyId, QTextFormat::Undefined);
    if (!prop.isValid())
	return QTextFormat::Undefined;
    return prop.type;
}

QList<int> QTextFormat::allPropertyIds() const
{
    return d->propertyMap().keys();
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

int QTextFormatCollectionPrivate::indexToReference(int idx) const
{
    if (idx >= -1)
	return -1;

    idx = -idx - 2;
    if (idx >= formatReferences.count())
	return -1;

    return idx;
}

int QTextFormatCollectionPrivate::referenceToIndex(int ref) const
{
    if (ref < 0 || ref >= formatReferences.count())
	return -1;

    return -(ref + 2);
}

#define d d_func()
#define q q_func()

QTextFormatCollection::QTextFormatCollection(QObject *parent)
    : QObject(*new QTextFormatCollectionPrivate, parent)
{
}

QTextFormatCollection::~QTextFormatCollection()
{
}

int QTextFormatCollection::indexForFormat(const QTextFormat &format)
{
    // certainly need speedup
    for (int i = 0; i < d->formats.size(); ++i)
	if (d->formats[i] ==
#undef d
	    *format.d)
#define d d_func()
	    return i;

    int idx = d->formats.size();
#undef d
    static_cast<QTextFormatCollectionPrivate *>(d_func())->formats.append(*format.d);
#define d d_func()
    return idx;
}

int QTextFormatCollection::createReferenceIndex(const QTextFormat &format)
{
    int formatIdx = indexForFormat(format);

    int ref = d->formatReferences.size();
    d->formatReferences.append(formatIdx);
    return d->referenceToIndex(ref);
}

void QTextFormatCollection::updateReferenceIndex(int index, const QTextFormat &newFormat)
{
    int ref = d->indexToReference(index);
    if (ref == -1)
	return;

    d->formatReferences[ref] = indexForFormat(newFormat);

    Q_ASSERT(d->referenceToIndex(ref) == index);
}

QTextFormat QTextFormatCollection::format(int idx, int defaultFormatType) const
{
    if (idx == -1 || idx > d->formats.count())
	return QTextFormat(defaultFormatType);

    if (idx < 0) {
	int ref = d->indexToReference(idx);
	if (ref == -1)
	    return QTextFormat(defaultFormatType);

	idx = d->formatReferences[ref];
	Q_ASSERT(idx >= 0 && idx < d->formats.count());
    }

    return QTextFormat(d->formats[idx]);
}

int QTextFormatCollection::numFormats() const
{
    return d->formats.count();
}

