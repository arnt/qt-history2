#include "qtextformat.h"
#include "qtextformat_p.h"
#include "qtextpiecetable_p.h"
#include <qtextblockiterator.h>
#include <qtextlist.h>
#include <qtexttable.h>
#include <qdatastream.h>

#include <qstring.h>
#include <qfontmetrics.h>
#include <qdebug.h>
#include <private/qobject_p.h>
#include <qmap.h>
#include <qfont.h>
#include <qvector.h>


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
    if (this == &rhs)
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
        case QTextFormat::FormatObject:
        case QTextFormat::Integer: return data.intValue == rhs.data.intValue;
        case QTextFormat::Float: return data.floatValue == rhs.data.floatValue;
        case QTextFormat::String: return stringValue() == rhs.stringValue();
    }

    return true;
}

QDataStream &operator<<(QDataStream &stream, const QTextFormatProperty &prop)
{
    stream <<(Q_INT32(prop.type));

    switch (prop.type) {
        case QTextFormat::Undefined: break;
        case QTextFormat::Bool: stream << Q_INT8(prop.data.boolValue); break;
        case QTextFormat::FormatObject:
        case QTextFormat::Integer: stream << Q_INT32(prop.data.intValue); break;
        case QTextFormat::Float: stream << prop.data.floatValue; break;
        case QTextFormat::String: stream << prop.stringValue(); break;
        default: Q_ASSERT(false); break;
    }

    return stream;
}

QDataStream &operator>>(QDataStream &stream, QTextFormatProperty &prop)
{
    Q_INT32 t;
    stream >> t;
    prop.type = static_cast<QTextFormat::PropertyType>(t);

    switch (prop.type) {
        case QTextFormat::Undefined: break;
        case QTextFormat::Bool: {
            Q_INT8 b;
            stream >> b;
            prop.data.boolValue = b;
            break;
        }
        case QTextFormat::FormatObject:
        case QTextFormat::Integer: {
            Q_INT32 i;
            stream >> i;
            prop.data.intValue = i;
            break;
        }
        case QTextFormat::Float: stream >> prop.data.floatValue; break;
        case QTextFormat::String: {
            QString s;
            stream >> s;
            prop.type = QTextFormat::Undefined;
            prop = QTextFormatProperty(s);
            break;
        }
        default: Q_ASSERT(false); break;
    }

    return stream;
}

/*!
    \class QTextFormat qtextformat.h
    \brief The QTextFormat class provides formatting information for a
    QTextDocument.

    \ingroup text

    A QTextFormat is a generic class used for describing the format of
    parts of a QTextDocument. The derived classes QTextCharFormat,
    QTextBlockFormat, QTextListFormat, and QTextTableFormat are usually
    more useful, and describe the formatting that is applied to
    specific parts of the document.

    A format has a \c FormatType which specifies the kinds of thing it
    can format, e.g. a block of text, a list, a table, etc. A format
    also has various properties (some specific to particular format
    types), as described by the \c Property enum. Every property has a
    \c PropertyType. A property can also have an associated object()
    (currently the only object type that is supported is \c
    ImageObject).

    The format type is given by type(), tested with
    isCharFormat(), isBlockFormat(), isListFormat(),
    isTableFormat(), isFrameFormat(), and isImageFormat(), and
    retrieved with toCharFormat(), toBlockFormat(), toListFormat(),
    toTableFormat(), toFrameFormat(), and toImageFormat().

    A format's properties can be set with the setProperty() functions,
    and retrieved with boolProperty(), intProperty(), floatProperty(),
    and stringProperty() as appropriate. All the property IDs used in
    the format can be retrieved with allPropertyIds(). One format can
    be merged into another using merge().

    A format's object can be set with setObject(), and the object's
    index set with setObjectIndex().
*/

/*!
    \enum QTextFormat::FormatType

    \value InvalidFormat
    \value BlockFormat
    \value CharFormat
    \value ListFormat
    \value TableFormat
    \value FrameFormat

    \value UserFormat
*/

/*!
    \enum QTextFormat::PropertyType

    \value Undefined
    \value Bool
    \value Integer
    \value Float
    \value String
    \value FormatObject
*/

/*!
    \enum QTextFormat::Property

    \value ObjectIndex

    Paragraph and character properties

    \value CssFloat

    Paragraph properties

    \value BlockDirection
    \value BlockAlignment
    \value BlockTopMargin
    \value BlockBottomMargin
    \value BlockLeftMargin
    \value BlockRightMargin
    \value BlockFirstLineMargin
    \value BlockIndent
    \value BlockNonBreakableLines
    \value BlockBackgroundColor

    Character properties

    \value FontFamily
    \value FontPointSize
    \value FontSizeIncrement
    \value FontWeight
    \value FontItalic
    \value FontUnderline
    \value FontOverline
    \value FontStrikeOut
    \value FontFixedPitch

    \value Color

    \value IsAnchor
    \value AnchorHref
    \value AnchorName

    \value NonDeletable

    \value ObjectType

    List properties

    \value ListStyle
    \value ListIndent

    Table properties

    \value TableColumns
    \value TableBorder
    \value Width
    \value Height

    Table cell properties

    \value TableCellRowSpan
    \value TableCellColumnSpan

    Image properties

    \value ImageName
    \value ImageWidth
    \value ImageHeight

    \value UserProperty
*/

/*!
    \enum QTextFormat::ObjectTypes

    \value NoObject
    \value ImageObject
*/

/*!
    \fn bool QTextFormat::isValid() const

    Returns true if the format is valid (i.e. is not \c
    InvalidFormat); otherwise returns false.
*/

/*!
    \fn bool QTextFormat::isCharFormat() const

    Returns true if this text format is a \c CharFormat; otherwise
    returns false.
*/


/*!
    \fn bool QTextFormat::isBlockFormat() const

    Returns true if this text format is a \c BlockFormat; otherwise
    returns false.
*/


/*!
    \fn bool QTextFormat::isListFormat() const

    Returns true if this text format is a \c ListFormat; otherwise
    returns false.
*/


/*!
    \fn bool QTextFormat::isTableFormat() const

    Returns true if this text format is a \c TableFormat; otherwise
    returns false.
*/


/*!
    \fn bool QTextFormat::isFrameFormat() const

    Returns true if this text format is a \c FrameFormat; otherwise
    returns false.
*/


/*!
    \fn bool QTextFormat::isImageFormat() const

    Returns true if this text format is an image format; otherwise
    returns false.
*/


/*!
    Creates a new text format with an \c InvalidFormat.

    \sa FormatType
*/
QTextFormat::QTextFormat()
    : d(new QTextFormatPrivate), collection(0)
{
    d->type = InvalidFormat;
}

/*!
    Creates a new text format of the given \a type.

    \sa FormatType
*/
QTextFormat::QTextFormat(int type)
    : d(new QTextFormatPrivate), collection(0)
{
    d->type = type;
}

QTextFormat::QTextFormat(QTextFormatCollection *c, QTextFormatPrivate *p)
    : d(p), collection(c)
{
    ++collection->ref;
}


/*!
    Creates a new text format with the same attributes at the \a rhs
    text format.
*/
QTextFormat::QTextFormat(const QTextFormat &rhs)
{
    d = rhs.d;
    collection = rhs.collection;
    if (collection)
        ++collection->ref;
}

/*!
    Assigns the \a rhs text format to this text format and returns a
    reference to this text format.
*/
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

/*!
    Destroys this text format.
*/
QTextFormat::~QTextFormat()
{
    if (collection && !--collection->ref) {
        qDebug("deleting collection %p", collection);
        delete collection;
    }
}

/*!
    Merges the \a other format with this format; where there are
    conflicts \a other takes precedence.
*/
void QTextFormat::merge(const QTextFormat &other)
{
    if (d->type != other.d->type)
        return;

    // don't use QMap's += operator, as it uses insertMulti!
    for (QTextFormatPrivate::PropertyMap::ConstIterator it = other.d->properties.begin();
         it != other.d->properties.end(); ++it) {
        d->properties.insert(it.key(), it.value());
    }
}

/*!
    Returns the type of this format.
*/
int QTextFormat::type() const
{
    return d->type;
}

/*!
    Returns this format as a block format.
*/
QTextBlockFormat QTextFormat::toBlockFormat() const
{
    QTextBlockFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns this format as a character format.
*/
QTextCharFormat QTextFormat::toCharFormat() const
{
    QTextCharFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns this format as a list format.
*/
QTextListFormat QTextFormat::toListFormat() const
{
    QTextListFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns this format as a table format.
*/
QTextTableFormat QTextFormat::toTableFormat() const
{
    QTextTableFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns this format as a frame format.
*/
QTextFrameFormat QTextFormat::toFrameFormat() const
{
    QTextFrameFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns this format as an image format.
*/
QTextImageFormat QTextFormat::toImageFormat() const
{
    QTextImageFormat f;
    f.QTextFormat::operator=(*this);
    return f;
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::Bool type the \a defaultValue is
    returned instead.

    \sa setProperty() intProperty() floatProperty() stringProperty() PropertyType
*/
bool QTextFormat::boolProperty(int propertyId, bool defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Bool)
        return defaultValue;
    return prop.data.boolValue;
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::Integer type the \a defaultValue is
    returned instead.

    \sa setProperty() boolProperty() floatProperty() stringProperty() PropertyType
*/
int QTextFormat::intProperty(int propertyId, int defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Integer)
        return defaultValue;
    return prop.data.intValue;
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::Float type the \a defaultValue is
    returned instead.

    \sa setProperty() boolProperty() intProperty() stringProperty() PropertyType
*/
float QTextFormat::floatProperty(int propertyId, float defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::Float)
        return defaultValue;
    return prop.data.floatValue;
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::String type the \a defaultValue is
    returned instead.

    \sa setProperty() boolProperty() intProperty() floatProperty() PropertyType
*/
QString QTextFormat::stringProperty(int propertyId, const QString &defaultValue) const
{
    const QTextFormatProperty prop = d->properties.value(propertyId);
    if (prop.type != QTextFormat::String)
        return defaultValue;
    return prop.stringValue();
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.

    \sa boolProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, bool value)
{
    d->properties.insert(propertyId, value);
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.

    \sa intProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, int value)
{
    d->properties.insert(propertyId, value);
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.

    \sa floatProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, float value)
{
    d->properties.insert(propertyId, value);
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa stringProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, const QString &value)
{
    d->properties.insert(propertyId, value);
}

/*!
    Return's the text format's format object or 0 if there isn't one.

    \sa setObject() ObjectTypes
*/
QTextFormatObject *QTextFormat::object() const
{
    const QTextFormatProperty prop = d->properties.value(ObjectIndex);
    if (!collection || prop.type != QTextFormat::FormatObject)
        return 0;
    return collection->object(prop.data.intValue);
}

/*!
    Sets the text format's format object to \a object.

    \sa object() ObjectTypes
*/
void QTextFormat::setObject(QTextFormatObject *object)
{
    if (!object) {
        setObjectIndex(-1);
        return;
    }

    QTextFormatCollection *c = object->d_func()->collection;
    Q_ASSERT(c);
    ++c->ref;
    c = qAtomicSetPtr(&collection, c);
    if (c && !--c->ref)
        delete c;

    QTextFormatProperty prop;
    prop.type = FormatObject;
    prop.data.intValue = collection->indexForObject(object);
    d->properties.insert(ObjectIndex, prop);
}

/*!
    Returns the index of the text format's format object, or -1 if
    there isn't a format object.

    \sa setObjectIndex() setObject()
*/
int QTextFormat::objectIndex() const
{
    const QTextFormatProperty prop = d->properties.value(ObjectIndex);
    if (prop.type != QTextFormat::FormatObject)
        return -1;
    return prop.data.intValue;
}

/*!
    Set's the format object's object index to \a o.

    \sa objectIndex() setObject()
*/
void QTextFormat::setObjectIndex(int o)
{
    QTextFormatProperty prop;
    prop.type = FormatObject;
    prop.data.intValue = o;
    d->properties.insert(ObjectIndex, prop);
}

/*!
    Returns true if the text format has a property with the given \a
    propertyId; otherwise returns false.

    \sa propertyType() allPropertyIds() PropertyType
*/
bool QTextFormat::hasProperty(int propertyId) const
{
    return d->properties.contains(propertyId);
}

/*!
    Returns the property type for the given \a propertyId.

    \sa hasProperty() allPropertyIds() PropertyType
*/
QTextFormat::PropertyType QTextFormat::propertyType(int propertyId) const
{
    if (!d)
        return QTextFormat::Undefined;

    return d->properties.value(propertyId).type;
}

/*!
    Returns a list of all the property IDs for this text format.

    \sa hasProperty() propertyType() PropertyType
*/
QList<int> QTextFormat::allPropertyIds() const
{
    if (!d)
        return QList<int>();

    return d->properties.keys();
}


/*!
    \fn bool QTextFormat::operator!=(const QTextFormat &rhs) const

    Returns true if this text format is different from the \a rhs text
    format.
*/


/*!
    Returns true if this text format is the same as the \a rhs text
    format.
*/
bool QTextFormat::operator==(const QTextFormat &rhs) const
{
    if (!d)
        return !rhs.d;
    if (!rhs.d)
        return false;

    return *d == *rhs.d;
}

QDataStream &operator<<(QDataStream &stream, const QTextFormat &format)
{
    return stream << format.d->type << format.d->properties;
}

QDataStream &operator>>(QDataStream &stream, QTextFormat &format)
{
    return stream >> format.d->type >> format.d->properties;
}

/*!
    \class QTextCharFormat qtextformat.h
    \brief The QTextCharFormat class provides formatting information for
    characters in a QTextDocument.

    \ingroup text

    The format's font can be set with setFont(), or piecemeal using
    setFontFamily(), setFontPointSize(), setFontWeight() (for bold),
    setFontItalic(), setFontUnderline(), setFontOverline(),
    setFontStrikeOut(), and setFontFixedPitch(). The color is set with
    setColor(), and the anchor (for hyperlinks) with setAnchor() and
    setAnchorHref(). The characters can be marked as non-deletable
    with setNonDeletable().

    If the characters are in a table the cell and row spanning
    characteristics can be set with setTableCellRowSpan() and
    setTableCellColumnSpan().
*/

/*!
    \fn bool QTextCharFormat::isValid() const

    Returns true if this character format is valid; otherwise returns
    false.
*/


/*!
    \fn void QTextCharFormat::setFontFamily(const QString &family)

    Sets the text format's font family to \a family.

    \sa setFont()
*/


/*!
    \fn QString QTextCharFormat::fontFamily() const

    Returns the text format's font family.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontPointSize(float size)

    Sets the text format's font size to \a size.

    \sa setFont()
*/


/*!
    \fn float QTextCharFormat::fontPointSize() const

    Returns the text format's font size.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontWeight(int weight)

    Sets the text format's font weight to \a weight.

    \sa setFont()
*/


/*!
    \fn int QTextCharFormat::fontWeight() const

    Returns the text format's font weight.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontItalic(bool italic)

    Sets the text format's font to be italic if \a italic is true;
    otherwise to non-italic.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontItalic() const

    Returns true if the text format's font is italic; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontUnderline(bool underline)

    Sets the text format's font to be underlined if \a underline is
    true; otherwise to non-underlined.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontUnderline() const

    Returns true if the text format's font is underlined; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontOverline(bool overline)

    Sets the text format's font to be overlined if \a overline is
    true; otherwise to non-overlined.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontOverline() const

    Returns true if the text format's font is overlined; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontStrikeOut(bool strikeOut)

    Sets the text format's font to be struck out if \a strikeOut is
    true; otherwise to non-struck out.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontStrikeOut() const

    Returns true if the text format's font is struck out; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontFixedPitch(bool fixedPitch)

    Sets the text format's font to be fixed pitch if \a fixedPitch is
    true; otherwise to non-fixed pitch.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontFixedPitch() const

    Returns true if the text format's font is fixed pitch; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setColor(const QColor &color)

    Sets the text format's font color to \a color.

    \sa color()
*/


/*!
    \fn QColor QTextCharFormat::color() const

    Returns the text format's font color.

    \sa setColor()
*/


/*!
    \fn void QTextCharFormat::setAnchor(bool anchor)

    If \a anchor is true, sets the text format signify an anchor;
    otherwise removes anchor formatting. (Anchors are hyperlinks,
    often shown underlined and in a different color from plain text.)

    This is independent of whether or not the format has an anchor.
    Use setAnchorHref(), and optionally setAnchorName() to create a
    hypertext link.

    \sa isAnchor()
*/


/*!
    \fn bool QTextCharFormat::isAnchor() const

    Returns true if the text is formatted as an anchor; otherwise
    returns false.

    \sa setAnchor() setAnchorHref() setAnchorName()
*/


/*!
    \fn void QTextCharFormat::setAnchorHref(const QString &value)

    Sets the anchor's hypertext link to be \a value. (This is
    typically a URL like http://www.trolltech.com/index.html.)

    The anchor will be displayed using \a value as its display text;
    if you want to display different text call setAnchorName().

    To format the text as a hypertext link use setAnchor().
*/


/*!
    \fn QString QTextCharFormat::anchorHref() const

    Returns the text format's hypertext link, or an empty string if
    none has been set.
*/


/*!
    \fn void QTextCharFormat::setAnchorName(const QString &name)

    Sets the text format's hypertext link's name to \a name. For the
    link to work a hyperlink must be set with setAnchorHref(); use
    setAnchor() to apply a hyperlink format.
*/


/*!
    \fn QString QTextCharFormat::anchorName() const

    Returns the text format's hypertext link's name, or an empty
    string if none has been set.
*/


/*!
    \fn void QTextCharFormat::setNonDeletable(bool d)

    If \a d is true, the characters formatted by this character format
    cannot be deleted by the user; otherwise they can be deleted
    normally.
*/


/*!
    \fn bool QTextCharFormat::nonDeletable() const

    Returns true if characters in this character format cannot be
    deleted; otherwise returns false.
*/


/*!
    \fn void QTextCharFormat::setObjectType(int type)

    \reimp
*/


/*!
    \fn int QTextCharFormat::objectType() const

    \reimp
*/


/*!
    \fn void QTextCharFormat::setTableCellRowSpan(int tableCellRowSpan)

    If this character format is applied to characters in a table cell,
    the cell will span \a tableCellRowSpan rows.
*/


/*!
    \fn int QTextCharFormat::tableCellRowSpan() const

    If this character format is applied to characters in a table cell,
    returns the number of rows spanned which may be 1. Otherwise
    returns 1.
*/


/*!
    \fn void QTextCharFormat::setTableCellColumnSpan(int tableCellColumnSpan)

    If this character format is applied to characters in a table cell,
    the cell will span \a tableCellColumnSpan columns.
*/


/*!
    \fn int QTextCharFormat::tableCellColumnSpan() const

    If this character format is applied to characters in a table cell,
    returns the number of columns spanned which may be 1. Otherwise
    returns 1.
*/


/*!
    Sets the character format's font to \a font.
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

/*!
    Returns the font for this character format.
*/
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
    \brief The QTextBlockFormat class provides formatting information for
    blocks of text in a QTextDocument.

    \ingroup text

    A document is composed out a list of blocks, that each contain one
    paragraph of text. Each block has an associated block format
    describing block specific properties as alignment, margins,
    indentation and possibly references to list and table formats.
*/

/*!
    Returns the list format for this character format.
*/
QTextListFormat QTextBlockFormat::listFormat() const
{
    QTextFormatObject *obj = object();
    return (obj ? obj->format() : QTextFormat()).toListFormat();
}

/*!
    Returns the table format for this character format.
*/
QTextTableFormat QTextCharFormat::tableFormat() const
{
    QTextFormatObject *obj = object();
    return (obj ? obj->format() : QTextFormat()).toTableFormat();
}


/*!
    \class QTextListFormat qtextformat.h
    \brief The QTextListFormat class provides formatting information for
    lists in a QTextDocument.

    \ingroup text

    Several blocks in a document can together form a list. The list
    format is a generic format describing the properties common for
    the whole list, as list style and indentation.
*/

/*!
    \class QTextTableFormat qtextformat.h
    \brief The QTextTableFormat class provides formatting information for
    tables in a QTextDocument.

    \ingroup text

    Several blocks in a document can together form a table. The table
    format is a generic format describing the properties common for
    the whole list, as border style and width.
*/

/*!
    \class QTextImageFormat qtextformat.h
    \brief The QTextImageFormat class provides formatting information for
    images in a QTextDocument.

    \ingroup text

    Inline images are represented by an object replacement character
    (U+fffc in Unicode) with a special image format. The image format
    contains a name property to locate the image and width and height
    for the images dimension.
*/


// ------------------------------------------------------

QTextFormatObject *QTextFormatCollection::createObject(int index)
{
    QTextFormat f = format(index);

    QTextFormatObject *obj;
    if (f.isListFormat())
        obj = new QTextList(pieceTable);
    else if (f.isTableFormat())
        obj = new QTextTable(pieceTable);
    else if (f.isFrameFormat())
        obj = new QTextFrame(pieceTable);
    else
        obj = new QTextFormatObject(pieceTable);
    obj->d_func()->collection = this;
    obj->d_func()->index = index;

    return obj;
}


QTextFormatCollection::QTextFormatCollection(const QTextFormatCollection &rhs)
{
    // ref == 1 just during construction, to avoid the formats begin created
    // in the loop below ref and then derefing this collection (and then
    // deleting it)
    ref = 1;
    pieceTable = 0;
    formats = rhs.formats;
    for (int i = 0; i < rhs.objs.size(); ++i) {
        QTextFormatObject *o = rhs.objs.at(i);
        objs.append(createObject(o->d_func()->index));
    }
    ref = 0;
}
QTextFormatCollection &QTextFormatCollection::operator=(const QTextFormatCollection &rhs)
{
    if (this == &rhs)
        return *this;

    for (int i = 0; i < objs.size(); ++i)
        delete objs[i];
    objs.clear();

    formats = rhs.formats;
    for (int i = 0; i < rhs.objs.size(); ++i) {
        QTextFormatObject *o = rhs.objs.at(i);
        objs.append(createObject(o->d_func()->index));
    }
    return *this;
}

QTextFormatCollection::~QTextFormatCollection()
{
    for (int i = 0; i < objs.size(); ++i)
        delete objs[i];
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

QTextFormatObject *QTextFormatCollection::createObject(const QTextFormat &format)
{
    int formatIdx = indexForFormat(format);

    QTextFormatObject *o = createObject(formatIdx);
    objs.append(o);
    return o;
}

QTextFormatObject *QTextFormatCollection::object(int objectIndex) const
{
    if (objectIndex == -1)
        return 0;
    return objs.at(objectIndex);
}

int QTextFormatCollection::indexForObject(QTextFormatObject *o)
{
    Q_ASSERT(o->d_func()->collection == this);
    for (int i = 0; i < objs.size(); ++i)
        if (objs.at(i) == o)
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


#define d d_func()
#define q q_func()


QTextFormatObject::QTextFormatObject(QObject *parent)
    : QObject(*new QTextFormatObjectPrivate, parent)
{
}

QTextFormatObject::QTextFormatObject(QTextFormatObjectPrivate &p, QObject *parent)
    :QObject(p, parent)
{
}

QTextFormatObject::~QTextFormatObject()
{
}


QTextFormat QTextFormatObject::format() const
{
    return d->collection->format(d->index);
}

void QTextFormatObject::setFormat(const QTextFormat &format)
{
    int idx = d->collection->indexForFormat(format);
    QTextPieceTable *pt = d->collection->pieceTable;
    if (pt)
        pt->changeObjectFormat(this, idx);
    else
        d->index = idx;
}


QTextBlockGroup::QTextBlockGroup(QObject *parent)
    : QTextFormatObject(*new QTextBlockGroupPrivate, parent)
{
}

QTextBlockGroup::QTextBlockGroup(QTextBlockGroupPrivate &p, QObject *parent)
    : QTextFormatObject(p, parent)
{
}

QTextBlockGroup::~QTextBlockGroup()
{
}

void QTextBlockGroup::insertBlock(const QTextBlockIterator &block)
{
    QTextBlockGroupPrivate::BlockList::Iterator it = qLowerBound(d->blocks.begin(), d->blocks.end(), block);
    d->blocks.insert(it, block);
}

void QTextBlockGroup::removeBlock(const QTextBlockIterator &block)
{
    d->blocks.removeAll(block);
}

void QTextBlockGroup::blockFormatChanged(const QTextBlockIterator &)
{
}

QList<QTextBlockIterator> QTextBlockGroup::blockList() const
{
    return d->blocks;
}



QTextFrameLayoutData::~QTextFrameLayoutData()
{
}



QTextFrame::QTextFrame(QObject *parent)
    : QTextFormatObject(*new QTextFramePrivate, parent)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}

QTextFrame::~QTextFrame()
{
    delete d->layoutData;
}

QTextFrame::QTextFrame(QTextFramePrivate &p, QObject *parent)
    : QTextFormatObject(p, parent)
{
    d->fragment_start = 0;
    d->fragment_end = 0;
    d->parentFrame = 0;
    d->layoutData = 0;
}


QList<QTextFrame *> QTextFrame::children()
{
    return d->childFrames;
}

QTextFrame *QTextFrame::parent()
{
    return d->parentFrame;
}


/*!
  The first cursor position inside the frame
*/
QTextCursor QTextFrame::start()
{
    return QTextCursor(d->pieceTable(), startPosition());
}

/*!
  The last cursor position inside the frame
*/
QTextCursor QTextFrame::end()
{
    return QTextCursor(d->pieceTable(), endPosition());
}

int QTextFrame::startPosition()
{
    if (!d->fragment_start)
        return 0;
    return d->pieceTable()->fragmentMap().position(d->fragment_start) + 1;
}

int QTextFrame::endPosition()
{
    if (!d->fragment_end)
        return d->pieceTable()->length();
    return d->pieceTable()->fragmentMap().position(d->fragment_end);
}

QTextFrameLayoutData *QTextFrame::layoutData() const
{
    return d->layoutData;
}

void QTextFrame::setLayoutData(QTextFrameLayoutData *data)
{
    delete d->layoutData;
    d->layoutData = data;
}



void QTextFramePrivate::fragmentAdded(const QChar &type, uint fragment)
{
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(!fragment_start);
        fragment_start = fragment;
    } else if (type == QTextEndOfFrame) {
        Q_ASSERT(!fragment_end);
        fragment_end = fragment;
    } else if (type == QChar::ObjectReplacementCharacter) {
        Q_ASSERT(!fragment_start);
        Q_ASSERT(!fragment_end);
        fragment_start = fragment;
        fragment_end = fragment;
    } else {
        Q_ASSERT(false);
    }
}

void QTextFramePrivate::fragmentRemoved(const QChar &type, uint fragment)
{
    if (type == QTextBeginningOfFrame) {
        Q_ASSERT(fragment_start == fragment);
        fragment_start = 0;
    } else if (type == QTextEndOfFrame) {
        Q_ASSERT(fragment_end == fragment);
        fragment_end = 0;
    } else if (type == QChar::ObjectReplacementCharacter) {
        Q_ASSERT(fragment_start == fragment);
        Q_ASSERT(fragment_end == fragment);
        fragment_start = 0;
        fragment_end = 0;
    } else {
        Q_ASSERT(false);
    }
    remove_me();
}


void QTextFramePrivate::remove_me()
{
    if (!parentFrame)
        return;

    int index = parentFrame->d->childFrames.indexOf(q);

    // iterator over all children and move them to the parent
    for (int i = 0; i < childFrames.size(); ++i) {
        QTextFrame *c = childFrames.at(i);
        parentFrame->d->childFrames.insert(index, c);
        c->d->parentFrame = parentFrame;
        ++index;
    }
    Q_ASSERT(parentFrame->d->childFrames.at(index) == q);
    parentFrame->d->childFrames.removeAt(index);

    childFrames.clear();
    parentFrame = 0;
}
