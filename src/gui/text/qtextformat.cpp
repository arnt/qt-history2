#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include <private/qobject_p.h>
#include <qmap.h>
#include <qfont.h>


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
        case QTextFormat::FormatGroup:
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
    : d(new QTextFormatPrivate), collection(0)
{
    d->type = InvalidFormat;
    d->inheritedType = InvalidFormat;
}

QTextFormat::QTextFormat(int type, int inheritedType)
    : d(new QTextFormatPrivate), collection(0)
{
    d->type = type;
    d->inheritedType = inheritedType;
}

QTextFormat::QTextFormat(QTextFormatCollection *c, QTextFormatPrivate *p)
    : d(p), collection(c)
{
    ++collection->ref;
}


QTextFormat::QTextFormat(const QTextFormat &rhs)
{
    d = rhs.d;
    collection = rhs.collection;
    if (collection)
	++collection->ref;
}

QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
    d = rhs.d;
    QTextFormatCollection *x = rhs.collection;
    if (x)
	++x->ref;
    x = qAtomicSetPtr(&collection, x);
    if (x && !--x->ref)
	delete x;
    return *this;
}

QTextFormat::~QTextFormat()
{
    if (collection && !--collection->ref) {
	qDebug("deleting collection %p", collection);
	delete collection;
    }
}

QTextFormat &QTextFormat::operator+=(const QTextFormat &other)
{
    // don't use QMap's += operator, as it uses insertMulti!
    for (QTextFormatPrivate::PropertyMap::ConstIterator it = other.d->properties.begin();
	 it != other.d->properties.end(); ++it) {
	d->properties.insert(it.key(), it.value());
    }

    return *this;
}

int QTextFormat::type() const
{
    return d->type;
}

int QTextFormat::inheritedType() const
{
    return d->inheritedType;
}

bool QTextFormat::inheritsFormatType(int otherType) const
{
    return d->type == otherType || d->inheritedType == otherType;
}


QTextBlockFormat QTextFormat::toBlockFormat() const
{
    QTextBlockFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextCharFormat QTextFormat::toCharFormat() const
{
    QTextCharFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextListFormat QTextFormat::toListFormat() const
{
    QTextListFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextTableFormat QTextFormat::toTableFormat() const
{
    QTextTableFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

QTextImageFormat QTextFormat::toImageFormat() const
{
    QTextImageFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

bool QTextFormat::boolProperty(int propertyId, bool defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Bool)
	return defaultValue;
    return prop.data.boolValue;
}

int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Integer)
	return defaultValue;
    return prop.data.intValue;
}

float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Float)
	return defaultValue;
    return prop.data.floatValue;
}

QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::String)
	return defaultValue;
    return prop.stringValue();
}

void QTextFormat::setProperty(int propertyId, bool value)
{
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, int value)
{
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, float value)
{
    d->properties.insert(propertyId, value);
}

void QTextFormat::setProperty(int propertyId, const QString &value)
{
    d->properties.insert(propertyId, value);
}

QTextFormatGroup *QTextFormat::group() const
{
    const QTextFormatProperty prop = d->properties.value(GroupIndex);
    if (!collection || prop.type != QTextFormat::FormatGroup)
	return 0;
    return collection->group(prop.data.intValue);
}

void QTextFormat::setGroup(QTextFormatGroup *group)
{
    if (!group) {
	setGroupIndex(-1);
	return;
    }

    QTextFormatCollection *c = group->collection;
    Q_ASSERT(group->collection);
    ++c->ref;
    c = qAtomicSetPtr(&collection, c);
    if (c && !--c->ref)
	delete c;

    QTextFormatProperty prop;
    prop.type = FormatGroup;
    prop.data.intValue = collection->indexForGroup(group);
    d->properties.insert(GroupIndex, prop);
}

int QTextFormat::groupIndex() const
{
    const QTextFormatProperty prop = d->properties.value(GroupIndex);
    if (prop.type != QTextFormat::FormatGroup)
	return -1;
    return prop.data.intValue;
}

void QTextFormat::setGroupIndex(int group)
{
    QTextFormatProperty prop;
    prop.type = FormatGroup;
    prop.data.intValue = group;
    d->properties.insert(GroupIndex, prop);
}

bool QTextFormat::hasProperty(int propertyId) const
{
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
    if (!d)
	return !rhs.d;
    if (!rhs.d)
	return false;

    return *d == *rhs.d;
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

QTextListFormat QTextBlockFormat::listFormat() const
{
    QTextFormatGroup *g = group();
    return (g ? g->commonFormat() : QTextFormat()).toListFormat();
}

QTextTableFormat QTextBlockFormat::tableFormat() const
{
    QTextFormatGroup *g = group();
    return (g ? g->commonFormat() : QTextFormat()).toTableFormat();
}


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

QTextFormatCollection::QTextFormatCollection(const QTextFormatCollection &rhs)
{
    ref = 0;
    formats = rhs.formats;
    for (int i = 0; i < rhs.groups.size(); ++i) {
	QTextFormatGroup *g = rhs.groups.at(i);
	groups.append(new QTextFormatGroup(this, g->idx));
    }
}
QTextFormatCollection &QTextFormatCollection::operator=(const QTextFormatCollection &rhs)
{
    qDeleteAll(groups);
    groups.clear();

    formats = rhs.formats;
    for (int i = 0; i < rhs.groups.size(); ++i) {
	QTextFormatGroup *g = rhs.groups.at(i);
	groups.append(new QTextFormatGroup(this, g->idx));
    }
    return *this;
}

QTextFormatCollection::~QTextFormatCollection()
{
    qDeleteAll(groups);
}

int QTextFormatCollection::indexForFormat(const QTextFormat &format)
{
    Q_ASSERT(format.d);
    // certainly need speedup
    for (int i = 0; i < formats.size(); ++i) {
	Q_ASSERT(formats.at(i));
	if (formats.at(i) == format.d || (*formats.at(i)) == (*format.d))
	    return i;
    }

    int idx = formats.size();
    formats.append(format.d);
    return idx;
}

bool QTextFormatCollection::hasFormatCached(const QTextFormat &format) const
{
    for (int i = 0; i < formats.size(); ++i)
	if (formats.at(i) == format.d || (*formats.at(i)) == (*format.d))
	    return true;
    return false;
}

QTextFormatGroup *QTextFormatCollection::createGroup(const QTextFormat &format)
{
    int formatIdx = indexForFormat(format);

    QTextFormatGroup *g = new QTextFormatGroup(this, formatIdx);
    groups.append(g);
    return g;
}

QTextFormatGroup *QTextFormatCollection::group(int groupIndex) const
{
    if (groupIndex == -1)
	return 0;
    return groups.at(groupIndex);
}

int QTextFormatCollection::indexForGroup(QTextFormatGroup *group)
{
    Q_ASSERT(group->collection == this);
    for (int i = 0; i < groups.size(); ++i)
	if (groups.at(i) == group)
	    return i;
    Q_ASSERT(false);
    return -1;
}

QTextFormat QTextFormatCollection::format(int idx) const
{
    if (idx == -1 || idx > formats.count())
	return QTextFormat();

    Q_ASSERT(formats.at(idx));
    // ##### does this detach the formatprivate?
    return QTextFormat(const_cast<QTextFormatCollection *>(this), formats[idx]);
}


QTextFormat QTextFormatGroup::commonFormat() const
{
    return collection->format(idx);
}

void QTextFormatGroup::setCommonFormat(const QTextFormat &format)
{
    idx = collection->indexForFormat(format);
    // ####### undo/redo
}

