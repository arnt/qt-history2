#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>

QTextFormatProperty::QTextFormatProperty(const QString &value)
{
    type = String;
    data.ptr = new QString(value);
}

QTextFormatProperty::QTextFormatProperty(const QByteArray &value)
{
    type = Binary;
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
	case String: data.ptr = new QString(*static_cast<QString *>(rhs.data.ptr)); break;
	case Binary: data.ptr = new QByteArray(*static_cast<QByteArray *>(rhs.data.ptr)); break;
	default: break;
    }
}

void QTextFormatProperty::free()
{
    switch (type) {
	case String: delete static_cast<QString *>(data.ptr); break;
	case Binary: delete static_cast<QByteArray *>(data.ptr); break;
	default: break;
    }
}

bool QTextFormatProperty::operator==(const QTextFormatProperty &rhs) const
{
    if (type != rhs.type)
	return false;

    switch (type) {
	case Undefined: return true;
	case Boolean: return data.boolValue == rhs.data.boolValue;
	case Integer: return data.intValue == rhs.data.intValue;
	case Float: return data.floatValue == rhs.data.floatValue;
	case String: return stringValue() == rhs.stringValue();
	case Binary: return binaryValue() == rhs.binaryValue();
    }

    return true;
}

const QTextFormatProperty QTextFormatPrivate::property(int propertyId, QTextFormatProperty::Type propType) const
{
    const QTextFormatProperty prop = properties.value(propertyId);
    if (prop.isValid() && (prop.type == propType || propType == QTextFormatProperty::Undefined))
	return prop;
    return QTextFormatProperty();
}

bool QTextFormatPrivate::operator==(const QTextFormatPrivate &rhs) const
{
    if (type != rhs.type)
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

	const QTextFormatProperty prop = other.d->property(propId, QTextFormatProperty::Undefined);
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
    const QTextFormatProperty prop = d->property(propertyId, QTextFormatProperty::Boolean);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.boolValue;
}

int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormatProperty::Integer);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.intValue;
}

float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormatProperty::Float);

    if (!prop.isValid())
	return defaultValue;

    return prop.data.floatValue;
}

QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormatProperty::String);

    if (!prop.isValid())
	return defaultValue;

    return prop.stringValue();
}

QByteArray QTextFormat::binaryProperty(int propertyId, QByteArray defaultValue) const
{
    const QTextFormatProperty prop = d->property(propertyId, QTextFormatProperty::Binary);

    if (!prop.isValid())
	return defaultValue;

    return prop.binaryValue();
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

bool QTextFormat::hasProperty(int propertyId) const
{
    return d->property(propertyId, QTextFormatProperty::Undefined).isValid();
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

