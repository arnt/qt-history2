#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qdatastream.h>
#include <qdebug.h>
#include <qmap.h>



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

    QTextFormatProperty &operator=(const QTextFormatProperty &rhs);
    inline QTextFormatProperty(const QTextFormatProperty &rhs) : type(QTextFormat::Undefined)
    { (*this) = rhs; }


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

Q_DECLARE_TYPEINFO(QTextFormatProperty, Q_PRIMITIVE_TYPE);

static QDataStream &operator<<(QDataStream &stream, const QTextFormatProperty &prop);
static QDataStream &operator>>(QDataStream &stream, QTextFormatProperty &prop);

class QTextFormatPrivate : public QSharedData
{
public:
    // keep Q_INT* types here, so we can safely stream to a datastream
    typedef QMap<Q_INT32, QTextFormatProperty> PropertyMap;

    PropertyMap properties;
    Q_INT32 type;

    inline bool operator==(const QTextFormatPrivate &rhs) const {
        if (type != rhs.type)
            return false;

        return properties == rhs.properties;
    }
};


#ifndef QT_NO_DEBUG
static QDebug &operator<<(QDebug &debug, const QTextFormatProperty &property)
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
    \c PropertyType.

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
    \value Qt::AnchorHref
    \value Qt::AnchorName

    \value ObjectType

    List properties

    \value ListStyle
    \value ListIndent

    Table and frame properties

    \value TableColumns
    \value FrameBorder
    \value FrameMargin
    \value FramePadding
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
    \value TableObject
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
    : d(new QTextFormatPrivate)
{
    d->type = InvalidFormat;
}

/*!
    Creates a new text format of the given \a type.

    \sa FormatType
*/
QTextFormat::QTextFormat(int type)
    : d(new QTextFormatPrivate)
{
    d->type = type;
}


/*!
    Creates a new text format with the same attributes at the \a rhs
    text format.
*/
QTextFormat::QTextFormat(const QTextFormat &rhs)
{
    d = rhs.d;
}

/*!
    Assigns the \a rhs text format to this text format and returns a
    reference to this text format.
*/
QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
    d = rhs.d;
    return *this;
}

/*!
    Destroys this text format.
*/
QTextFormat::~QTextFormat()
{
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
    \fn void QTextFormat::setObjectType(int type)

    Set's the text format's object type to \a type. See
    \c{ObjectTypes}.
*/


/*!
    \fn int QTextFormat::objectType() const

    Returns the text format's object type. See \c{ObjectTypes}.
*/


/*!
    Returns the index of the text format's format object, or -1 if
    there isn't a format object.

    \sa setObjectIndex()
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

    \sa objectIndex()
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
    if (d == rhs.d)
        return true;
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
    setColor(), and the anchor (for hyperlinks) with setAnchor(),
    setAnchorHref(), setAnchorName(). The characters can be marked as
    non-deletable with setNonDeletable().

    If the characters are in a table the cell and row spanning
    characteristics can be set with setTableCellRowSpan() and
    setTableCellColumnSpan().
*/

/*!
    \fn QTextCharFormat::QTextCharFormat()

    Constructs a new QTextCharFormat.
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

    A document is composed of a list of blocks. Each block can contain
    an item of some kind, for example, a paragraph of text, a table, a
    list, or an image. Every block has an associated QTextBlockFormat
    that specifies its characteristics.

    To cater for left-to-right and right-to-left languages you can set
    a block's direction with setDirection(). Paragraph alignment is
    set with setAlignment(). Margins are controlled by setTopMargin(),
    setBottomMargin(), setLeftMargin(), setRightMargin(), and
    setFirstLineMargin(). Overall indentation is set with setIndent().
    Line breaking is controlled with setNonBreakableLines(). The
    paragraph's background color is set with setBackgroundColor().

    A text block can also have a list format (if is part of a list);
    this is accessible using listFormat().
*/

/*!
    \enum QTextBlockFormat::Direction

    \value LeftToRight
    \value RightToLeft
    \value AutoDirection
*/

/*!
    \fn QTextBlockFormat::QTextBlockFormat()

    Constructs a new QTextBlockFormat.
*/

/*!
    \fn QTextBlockFormat::isValid() const

    \reimp
*/

/*!
    \fn void QTextBlockFormat::setDirection(Direction dir)

    Set's the text's direction to \a dir.

    \sa direction()
*/


/*!
    \fn Direction QTextBlockFormat::direction() const

    Returns the text's direction.

    \sa setDirection()
*/


/*!
    \fn void QTextBlockFormat::setAlignment(Qt::Alignment alignment)

    Sets the paragraph's alignment to \a alignment.

    \sa alignment()
*/


/*!
    \fn Qt::Alignment QTextBlockFormat::alignment() const

    Returns the paragraph's alignment.

    \sa setAlignment()
*/


/*!
    \fn void QTextBlockFormat::setTopMargin(int margin)

    Sets the paragraph's top margin to \a margin.

    \sa topMargin() setBottomMargin() setLeftMargin() setRightMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::topMargin() const

    Returns the paragraph's top margin.

    \sa setTopMargin() bottomMargin()
*/


/*!
    \fn void QTextBlockFormat::setBottomMargin(int margin)

    Sets the paragraph's bottom margin to \a margin.

    \sa bottomMargin() setTopMargin() setLeftMargin() setRightMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::bottomMargin() const

    Returns the paragraph's bottom margin.

    \sa setBottomMargin() topMargin()
*/


/*!
    \fn void QTextBlockFormat::setLeftMargin(int margin)

    Sets the paragraph's left margin to \a margin. Indentation can be
    applied separately with setIndent().

    \sa leftMargin() setRightMargin() setTopMargin() setBottomMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::leftMargin() const

    Returns the paragraph's left margin.

    \sa setLeftMargin() rightMargin() indent()
*/


/*!
    \fn void QTextBlockFormat::setRightMargin(int margin)

    Sets the paragraph's right margin to \a margin.

    \sa rightMargin() setLeftMargin() setTopMargin() setBottomMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::rightMargin() const

    Returns the paragraph's right margin.

    \sa setRightMargin() leftMargin()
*/


/*!
    \fn void QTextBlockFormat::setFirstLineMargin(int margin)

    Sets the paragraph's first line margin to \a margin.

    \sa firstLineMargin() setLeftMargin() setRightMargin() setTopMargin() setBottomMargin()
*/


/*!
    \fn int QTextBlockFormat::firstLineMargin() const

    Returns the paragraph's first line margin.

    \sa setFirstLineMargin()
*/


/*!
    \fn void QTextBlockFormat::setIndent(int indent)

    Sets the paragraph's indent to \a indent. Margins are set
    independently of indentation, for example with, setLeftMargin()
    and setFirstLineMargin().

    \sa indent()
*/


/*!
    \fn int QTextBlockFormat::indent() const

    Returns the paragraph's indent.

    \sa setIndent()
*/


/*!
    \fn void QTextBlockFormat::setNonBreakableLines(bool b)

    If \a b is true, the lines in the paragraph are treated as
    non-breakable; otherwise they are breakable.

    \sa nonBreakableLines()
*/


/*!
    \fn bool QTextBlockFormat::nonBreakableLines() const

    Returns true if the lines in the paragraph are non-breakable;
    otherwise returns false.

    \sa setNonBreakableLines()
*/


/*!
    \fn void QTextBlockFormat::setBackgroundColor(const QColor &color)

    Sets the paragraph's background color to \a color.

    \sa backgroundColor()
*/


/*!
    \fn QColor QTextBlockFormat::backgroundColor() const

    Returns the paragraph's background color.

    \sa setBackgroundColor()
*/



/*!
    \class QTextListFormat qtextformat.h
    \brief The QTextListFormat class provides formatting information for
    lists in a QTextDocument.

    \ingroup text

    A list is composed of one or more items; each item is a block. A
    list format is used to specify the characteristics of a list, for
    example, setStyle() controls the bulleting or numbering of the
    list items, and setIndent() controls the list's indentation.
*/

/*!
    \enum QTextListFormat::Style

    \value ListDisc
    \value ListCircle
    \value ListSquare
    \value ListDecimal
    \value ListLowerAlpha
    \value ListUpperAlpha
*/

/*!
    \fn QTextListFormat::QTextListFormat()

    Constructs a new QTextListFormat.
*/


/*!
    \fn bool QTextListFormat::isValid() const

    \reimp
*/


/*!
    \fn void QTextListFormat::setStyle(int style)

    Sets the list format's list style to \a style. See \c{Style}.

    \sa style()
*/


/*!
    \fn QTextListFormat::style() const

    Returns the list format's style. See \c{Style}.

    \sa setStyle()
*/


/*!
    \fn void QTextListFormat::setIndent(int indent)

    Sets the list format's indent to \a indent.

    \sa indent()
*/


/*!
    \fn int QTextListFormat::indent() const

    Returns the list format's indent.

    \sa setIndent()
*/


/*!
    \class QTextTableFormat qtextformat.h
    \brief The QTextTableFormat class provides formatting information for
    tables in a QTextDocument.

    \ingroup text

    A table comprises one or more rows of one or more columns of
    cells. Each cell contains a block.

    A QTextTableFormat specifies the characteristics of a table. The
    setColumns() function sets the number of columns; the number of
    rows is automatically derived based on the number of columns and
    the number of cells (blocks) contained in the table.
*/

/*!
    \fn QTextTableFormat::QTextTableFormat()

    Constructs a new QTextTableFormat.
*/


/*!
    \fn bool QTextTableFormat::isValid() const

    \reimp
*/


/*!
    \fn int QTextTableFormat::columns() const

    Returns the table format's number of columns.

    \sa setColumns()
*/


/*!
    \fn void QTextTableFormat::setColumns(int columns)

    Sets the table format's number of columns to \a columns.

    \sa columns()
*/


/*!
    \class QTextImageFormat qtextformat.h
    \brief The QTextImageFormat class provides formatting information for
    images in a QTextDocument.

    \ingroup text

    Inline images are represented by an object replacement character
    (0xFFFC in Unicode) which has an associated QTextImageFormat. The
    image format specifies a name with setName() that is used to
    locate the image. The size of the rectangle that the image will
    occupy is specified using setWidth() and setHeight().
*/

/*!
    \fn QTextImageFormat::QTextImageFormat()

    Creates a new QTextImageFormat.
*/


/*!
    \fn bool QTextImageFormat::isValid() const

    \reimp
*/


/*!
    \fn void QTextImageFormat::setName(const QString &name)

    Sets the name of the image to \a name. The \a name is used to
    locate the image.

    \sa name()
*/


/*!
    \fn QString QTextImageFormat::name() const

    Returns the name of the image.

    \sa setName()
*/


/*!
    \fn void QTextImageFormat::setWidth(int width)

    Sets the width of the rectangle occupied by the image to \a width.

    \sa width() setHeight()
*/


/*!
    \fn int QTextImageFormat::width() const

    Returns the width of the rectangle occupied by the image.

    \sa height() setWidth()
*/


/*!
    \fn void QTextImageFormat::setHeight(int height)

    Sets the height of the rectangle occupied by the image to \a
    height.

    \sa height() setWidth()
*/


/*!
    \fn int QTextImageFormat::height() const

    Returns the height of the rectangle occupied by the image.

    \sa width() setHeight()
*/


// ------------------------------------------------------


QTextFormatCollection::QTextFormatCollection(const QTextFormatCollection &rhs)
{
    formats = rhs.formats;
    objFormats = rhs.objFormats;
}

QTextFormatCollection &QTextFormatCollection::operator=(const QTextFormatCollection &rhs)
{
    formats = rhs.formats;
    objFormats = rhs.objFormats;
    return *this;
}

QTextFormatCollection::~QTextFormatCollection()
{
}

int QTextFormatCollection::indexForFormat(const QTextFormat &format)
{
    // ### certainly need speedup
    for (int i = 0; i < formats.size(); ++i) {
        if (formats.at(i) == format)
            return i;
    }

    int idx = formats.size();
    formats.append(format);
    return idx;
}

bool QTextFormatCollection::hasFormatCached(const QTextFormat &format) const
{
    for (int i = 0; i < formats.size(); ++i)
        if (formats.at(i) == format)
            return true;
    return false;
}

QTextFormat QTextFormatCollection::objectFormat(int objectIndex) const
{
    if (objectIndex == -1)
        return QTextFormat();
    return format(objFormats.at(objectIndex));
}

void QTextFormatCollection::setObjectFormat(int objectIndex, const QTextFormat &f)
{
    int formatIndex = indexForFormat(f);
    objFormats[objectIndex] = formatIndex;
}

int QTextFormatCollection::objectFormatIndex(int objectIndex) const
{
    if (objectIndex == -1)
        return -1;
    return objFormats.at(objectIndex);
}

void QTextFormatCollection::setObjectFormatIndex(int objectIndex, int formatIndex)
{
    objFormats[objectIndex] = formatIndex;
}

int QTextFormatCollection::createObjectIndex(const QTextFormat &f)
{
    int objectIndex = objFormats.size();
    objFormats.append(indexForFormat(f));
    return objectIndex;
}

QTextFormat QTextFormatCollection::format(int idx) const
{
    if (idx == -1 || idx > formats.count())
        return QTextFormat();

    return formats.at(idx);
}


QDataStream &operator<<(QDataStream &stream, const QTextFormatCollection &collection)
{
    return stream << collection.formats
                  << collection.objFormats;
}

QDataStream &operator>>(QDataStream &stream, QTextFormatCollection &collection)
{
    return stream >> collection.formats
                  >> collection.objFormats;
}

