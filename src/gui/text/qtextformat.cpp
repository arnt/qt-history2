/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextformat.h"
#include "qtextformat_p.h"

#include <qvariant.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qmap.h>
#include <qhash.h>

/*!
    \class QTextLength qtextformat.h
    \brief The QTextLength class encapsulates the different types of length
    used in a QTextDocument.

    \ingroup text

    When we specify a value for the length of an element in a text document,
    we often need to provide some other information so that the length is
    used in the way we expect. For example, when we specify a table width,
    the value can represent a fixed number of pixels, or it can be a percentage
    value. This information changes both the meaning of the value and the way
    it is used.

    Generally, this class is used to specify table widths. These can be
    specified either as a fixed amount of pixels, as a percentage of the
    containing frame's width, or by a variable width that allows it to take
    up just the space it requires.

    \sa QTextTable
*/

/*!
    \fn QTextLength::QTextLength()

    Constructs a new length object which represents a variable size.
*/

/*!
    \fn QTextLength::QTextLength(Type type, int value)

    Constructs a new length object of the given \a type and \a value.
*/

/*!
    \fn Type QTextLength::type() const

    Returns the type of length.

    \sa QTextLength::Type
*/

/*!
    \fn int QTextLength::value(int maximumLength) const

    Returns the effective length, constrained by the type of the length object
    and the specified \a maximumLength.

    \sa type()
*/

/*!
    \fn int QTextLength::rawValue() const

    Returns the constraint value that is specific for the type of the lenght.
    If the length is QTextLength::PercentageLengt then the raw value is in
    percent, in the range of 0 to 100. If the length is QTextLength::FixedLength
    then that fixed amount is returned. For variable lengths zero is returned.
*/

/*!
    \fn bool QTextLength::operator==(const QTextLength &other) const

    Returns true if this text length is the same as the \a other text
    length.
*/

/*!
    \fn bool QTextLength::operator!=(const QTextLength &other) const

    Returns true if this text length is different from the \a other text
    length.
*/

/*!
    \enum QTextLength::Type

    \value VariableLength
    \value FixedLength
    \value PercentageLength
*/

QDataStream &operator<<(QDataStream &stream, const QTextLength &length)
{
    return stream << qint32(length.lengthType) << length.fixedValueOrPercentage;
}

QDataStream &operator>>(QDataStream &stream, QTextLength &length)
{
    qint32 type;
    stream >> type >> length.fixedValueOrPercentage;
    length.lengthType = QTextLength::Type(type);
    return stream;
}

class QTextFormatPrivate : public QSharedData
{
public:
    QTextFormatPrivate() : hashDirty(true), hashValue(0) {}

    // keep qint* types here, so we can safely stream to a datastream
    typedef QMap<qint32, QVariant> PropertyMap;

    qint32 type;

    inline bool operator==(const QTextFormatPrivate &rhs) const {
        if (hash() != rhs.hash() || type != rhs.type)
            return false;

        return props == rhs.props;
    }

    inline void insertProperty(qint32 key, const QVariant &value)
    {
        hashDirty = true;
        props.insert(key, value);
    }

    inline void clearProperty(qint32 key)
    {
        hashDirty = true;
        props.remove(key);
    }

    const PropertyMap &properties() const { return props; }


private:
    PropertyMap props;

    inline uint hash() const
    {
        if (!hashDirty)
            return hashValue;
        return recalcHash();
    }

    uint recalcHash() const;

    mutable bool hashDirty;
    mutable uint hashValue;
    friend QDataStream &operator<<(QDataStream &, const QTextFormat &);
    friend QDataStream &operator>>(QDataStream &, QTextFormat &);
};

static uint variantHash(const QVariant &variant)
{
    switch (variant.type()) {
    case QVariant::Invalid: return 0;
    case QVariant::Bool: return variant.toBool();
    case QVariant::Int: return variant.toInt();
    case QVariant::Double: return static_cast<int>(variant.toDouble());
    case QVariant::String: return qHash(variant.toString());
    case QVariant::Color: return qHash(qVariant_to<QColor>(variant).rgb());
    default: break;
    }
    return qHash(variant.typeName());
}

uint QTextFormatPrivate::recalcHash() const
{
    hashValue = 0;
    for (PropertyMap::ConstIterator it = props.begin();
         it != props.end(); ++it)
        hashValue += (it.key() << 16) + variantHash(*it);

    hashDirty = false;
    return hashValue;
}


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QTextFormat &fmt)
{
    stream << fmt.d->type << fmt.d->props;
    return stream;
}

Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QTextFormat &fmt)
{
    stream >> fmt.d->type >> fmt.d->props;
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
    can format; e.g. a block of text, a list, a table, etc. A format
    also has various properties (some specific to particular format
    types), as described by the \c Property enum. Every property has a
    \c PropertyType.

    The format type is given by type(), and the format can be tested
    with isCharFormat(), isBlockFormat(), isListFormat(),
    isTableFormat(), isFrameFormat(), and isImageFormat(). If the
    type is determined, it can be retrieved with toCharFormat(),
    toBlockFormat(), toListFormat(), toTableFormat(), toFrameFormat(),
    and toImageFormat().

    A format's properties can be set with the setProperty() functions,
    and retrieved with boolProperty(), intProperty(), floatProperty(),
    and stringProperty() as appropriate. All the property IDs used in
    the format can be retrieved with allPropertyIds(). One format can
    be merged into another using merge().

    A format's object index can be set with setObjectIndex(), and
    retrieved with objectIndex(). These methods can be used to
    associate the format with a QTextObject. It is used to represent
    lists, frames, and tables inside the document.

    \sa {text.html}{Text Related Classes}
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

    \value TextColor
    \value TextBackgroundColor
    \value TextUnderlineColor
    \value TextVerticalAlignment

    \value IsAnchor
    \value AnchorHref
    \value AnchorName

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
    \value TableColumnWidthConstraints
    \value TableCellSpacing
    \value TableCellPadding
    \value TableBackgroundColor

    Table cell properties

    \value TableCellRowSpan
    \value TableCellColumnSpan
    \value TableCellBackgroundColor

    Image properties

    \value ImageName
    \value ImageWidth
    \value ImageHeight

    \omitvalue DocumentFragmentMark

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
    \fn QTextFormat::QTextFormat(const QTextFormat &other)

    Creates a new text format with the same attributes at the \a other
    text format.
*/
QTextFormat::QTextFormat(const QTextFormat &rhs)
{
    d = rhs.d;
}

/*!
    \fn QTextFormat &QTextFormat::operator=(const QTextFormat &other)

    Assigns the \a other text format to this text format, and returns a
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
    conflicts the \a other format takes precedence.
*/
void QTextFormat::merge(const QTextFormat &other)
{
    if (d->type != other.d->type)
        return;

    // don't use QMap's += operator, as it uses insertMulti!
    for (QTextFormatPrivate::PropertyMap::ConstIterator it = other.d->properties().begin();
         it != other.d->properties().end(); ++it) {
        d->insertProperty(it.key(), it.value());
    }
}

/*!
    Returns the type of this format.

    \sa ObjectTypes
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
    Returns the value of the property specified by \a propertyId. If the
    property isn't of \c QTextFormat::Bool type, false is returned instead.

    \sa setProperty() intProperty() floatProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() PropertyType
*/
bool QTextFormat::boolProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);
    if (prop.type() != QVariant::Bool)
        return false;
    return prop.toBool();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property is not of \c QTextFormat::Integer type, 0 is returned instead.

    \sa setProperty() boolProperty() floatProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() PropertyType
*/
int QTextFormat::intProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);
    if (prop.type() != QVariant::Int)
        return 0;
    return prop.toInt();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property isn't of \c QTextFormat::Float type, 0 is returned instead.

    \sa setProperty() boolProperty() intProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() PropertyType
*/
float QTextFormat::floatProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);
    if (prop.type() != QVariant::Double)
        return 0.;
    return prop.toDouble(); // ####
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::String type, a null string is
    returned instead.

    \sa setProperty() boolProperty() intProperty() floatProperty() colorProperty() lengthProperty() lengthVectorProperty() PropertyType
*/
QString QTextFormat::stringProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);
    if (prop.type() != QVariant::String)
        return QString();
    return prop.toString();
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of \c QTextFormat::Color type, an invalid color is
    returned instead.

    \sa setProperty() boolProperty() intProperty() floatProperty() stringProperty() lengthProperty() lengthVectorProperty() PropertyType
*/
QColor QTextFormat::colorProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);
    if (prop.type() != QVariant::Color)
        return QColor();
    return qVariant_to<QColor>(prop);
}

/*!
    Returns the value of the property given by \a propertyId.

    \sa setProperty() boolProperty() intProperty() floatProperty() stringProperty() colorProperty() lengthVectorProperty() PropertyType
*/
QTextLength QTextFormat::lengthProperty(int propertyId) const
{
    return qVariant_to<QTextLength>(d->properties().value(propertyId));
}

/*!
    Returns the value of the property given by \a propertyId. If the
    property isn't of \c QTextFormat::LengthVector type, an empty length
    vector is returned instead.

    \sa setProperty() boolProperty() intProperty() floatProperty() stringProperty() colorProperty() lengthProperty() PropertyType
*/
QVector<QTextLength> QTextFormat::lengthVectorProperty(int propertyId) const
{
    const QVariant prop = d->properties().value(propertyId);

    QVector<QTextLength> vector;
    if (prop.type() != QVariant::List)
        return vector;

    QList<QVariant> propertyList = prop.toList();
    for (int i=0; i<propertyList.size(); ++i) {
        QVariant var = propertyList.at(i);
        if (var.type() == QVariant::TextLength)
            vector.append(qVariant_to<QTextLength>(var));
    }

    return vector;
}

/*!
    Returns the property specified by the given \a propertyId.
*/
QVariant QTextFormat::property(int propertyId) const
{
    return d->properties().value(propertyId);
}

/*!
    Sets the property specified by the \a propertyId to the given \a value.
*/
void QTextFormat::setProperty(int propertyId, const QVariant &value)
{
    if (!value.isValid())
        clearProperty(propertyId);
    else
        d->insertProperty(propertyId, value);
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.

    \sa boolProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, bool value)
{
    d->insertProperty(propertyId, value);
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.
    \sa intProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, int value)
{
    d->insertProperty(propertyId, value);
}

/*!
    \overload

    Sets the value of the property given by \a propertyId to \a value.

    \sa floatProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, float value)
{
    d->insertProperty(propertyId, value);
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa stringProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, const QString &value)
{
    d->insertProperty(propertyId, value);
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa colorProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, const QColor &value)
{
    d->insertProperty(propertyId, qVariant(value));
}


/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa lengthProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, const QTextLength &value)
{
    d->insertProperty(propertyId, qVariant(value));
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa lengthVectorProperty() PropertyType
*/
void QTextFormat::setProperty(int propertyId, const QVector<QTextLength> &value)
{
    QVariantList list;
    for (int i=0; i<value.size(); ++i)
        list << qVariant(value.at(i));
    d->insertProperty(propertyId, list);
}

/*!
  Clears the value of the property given by \a propertyId
 */
void QTextFormat::clearProperty(int propertyId)
{
    d->clearProperty(propertyId);
}


/*!
    \fn void QTextFormat::setObjectType(int type)

    Sets the text format's object \a type. See \c{ObjectTypes}.
*/


/*!
    \fn int QTextFormat::objectType() const

    Returns the text format's object type. See \c{ObjectTypes}.
*/


/*!
    Returns the index of the format object, or -1 if the format object is invalid.

    \sa setObjectIndex()
*/
int QTextFormat::objectIndex() const
{
    const QVariant prop = d->properties().value(ObjectIndex);
    if (prop.type() != QVariant::Int) // ####
        return -1;
    return prop.toInt();
}

/*!
    \fn void QTextFormat::setObjectIndex(int index)

    Sets the format object's object \a index.

    \sa objectIndex()
*/
void QTextFormat::setObjectIndex(int o)
{
    if (o == -1) {
        d->clearProperty(ObjectIndex);
    } else {
        // ### type
        d->insertProperty(ObjectIndex, o);
    }
}

/*!
    Returns true if the text format has a property with the given \a
    propertyId; otherwise returns false.

    \sa propertyType() allPropertyIds() PropertyType
*/
bool QTextFormat::hasProperty(int propertyId) const
{
    return d->properties().contains(propertyId);
}

/*
    Returns the property type for the given \a propertyId.

    \sa hasProperty() allPropertyIds() PropertyType
*/

/*!
    Returns a map with all properties of this text format.
*/
QMap<int, QVariant> QTextFormat::properties() const
{
    return d->properties();
}


/*!
    \fn bool QTextFormat::operator!=(const QTextFormat &other) const

    Returns true if this text format is different from the \a other text
    format.
*/


/*!
    \fn bool QTextFormat::operator==(const QTextFormat &other) const

    Returns true if this text format is the same as the \a other text
    format.
*/
bool QTextFormat::operator==(const QTextFormat &rhs) const
{
    if (d == rhs.d)
        return true;

    return *d == *rhs.d;
}

/*!
    \class QTextCharFormat qtextformat.h
    \brief The QTextCharFormat class provides formatting information for
    characters in a QTextDocument.

    \ingroup text

    The character format of text in a document specifies the visual properties
    of the text, as well as information about its role in a hypertext document.

    The font used can be set by supplying a font to the setFont() function, and
    each aspect of its appearance can be adjusted to give the desired effect.
    setFontFamily() and setFontPointSize() define the font's family (e.g. Times)
    and printed size; setFontWeight() and setFontItalic() provide control over
    the style of the font. setFontUnderline(), setFontOverline(),
    setFontStrikeOut(), and setFontFixedPitch() provide additional effects for
    text.

    The color is set with setTextColor(). If the text is intended to be used
    as an anchor (for hyperlinks), this can be enabled with setAnchor(). The
    setAnchorHref() and setAnchorName() functions are used to specify the
    information about the hyperlink's destination and the anchor's name.

    If the text is written within a table, it can be made to span a number of
    rows and columns with the setTableCellRowSpan() and setTableCellColumnSpan()
    functions.

    \sa QTextFormat QTextBlockFormat QTextTableFormat QTextListFormat
*/

/*!
    \fn QTextCharFormat::QTextCharFormat()

    Constructs a new character format object.
*/


/*!
    \fn bool QTextCharFormat::isValid() const

    Returns true if this character format is valid; otherwise returns
    false.
*/


/*!
    \fn void QTextCharFormat::setFontFamily(const QString &family)

    Sets the text format's font \a family.

    \sa setFont()
*/


/*!
    \fn QString QTextCharFormat::fontFamily() const

    Returns the text format's font family.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontPointSize(float size)

    Sets the text format's font \a size.

    \sa setFont()
*/


/*!
    \fn float QTextCharFormat::fontPointSize() const

    Returns the font size used to display text in this format.

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

    If \a italic is true, sets the text format's font to be italic; otherwise
    the font will be non-italic.

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

    If \a underline is true, sets the text format's font to be underlined;
    otherwise it is displayed non-underlined.

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

    If \a overline is true, sets the text format's font to be overlined;
    otherwise the font is displayed non-overlined.

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

    If \a strikeOut is true, sets the text format's font with strike-out
    enabled (with a horizontal line through it); otherwise it is displayed
    without strikeout.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontStrikeOut() const

    Returns true if the text format's font is struck out (has a horizontal line
    drawn through it); otherwise returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setFontFixedPitch(bool fixedPitch)

    If \a fixedPitch is true, sets the text format's font to be fixed pitch;
    otherwise a non-fixed pitch font is used.

    \sa setFont()
*/


/*!
    \fn bool QTextCharFormat::fontFixedPitch() const

    Returns true if the text format's font is fixed pitch; otherwise
    returns false.

    \sa font()
*/


/*!
    \fn void QTextCharFormat::setTextColor(const QColor &color)

    Sets the text format's font color to \a color.

    \sa textColor()
*/


/*!
    \fn QColor QTextCharFormat::textColor() const

    Returns the text format's font color.

    \sa setTextColor()
*/


/*!
    \fn void QTextCharFormat::setAnchor(bool anchor)

    If \a anchor is true, text with this format represents an anchor, and is
    formatted in the appropriate way; otherwise the text is formatted normally.
    (Anchors are hyperlinks which are often shown underlined and in a different
    color from plain text.)

    The way the text is rendered is independent of whether or not the format
    has a valid anchor defined. Use setAnchorHref(), and optionally
    setAnchorName() to create a hypertext link.

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

    Sets the hypertext link for the text format to the given \a value.
    This is typically a URL like "http://www.trolltech.com/index.html".

    The anchor will be displayed with the \a value as its display text;
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

    Sets the text format's anchor \a name. For the anchor to work as a
    hyperlink, the destination must be set with setAnchorHref() and
    the anchor must be enabled with setAnchor().
*/


/*!
    \fn QString QTextCharFormat::anchorName() const

    Returns the anchor name associated with this text format, or an empty
    string if none has been set. If the anchor name is set, text with this
    format can be the destination of a hypertext link.
*/


/*!
    \fn void QTextCharFormat::setTableCellRowSpan(int tableCellRowSpan)

    If this character format is applied to characters in a table cell,
    the cell will span \a tableCellRowSpan rows.
*/


/*!
    \fn int QTextCharFormat::tableCellRowSpan() const

    If this character format is applied to characters in a table cell,
    this function returns the number of rows spanned by the text (this may
    be 1); otherwise it returns 1.
*/


/*!
    \fn void QTextCharFormat::setTableCellColumnSpan(int tableCellColumnSpan)

    If this character format is applied to characters in a table cell,
    the cell will span \a tableCellColumnSpan columns.
*/


/*!
    \fn int QTextCharFormat::tableCellColumnSpan() const

    If this character format is applied to characters in a table cell,
    this function returns the number of columns spanned by the text (this
    may be 1); otherwise it returns 1.
*/

/*!
    \fn void QTextCharFormat::setTableCellBackgroundColor(const QColor &color)

    Sets the background \a color of the selected table cells.

    \sa tableCellBackgroundColor()
*/

/*!
    \fn QColor QTextCharFormat::tableCellBackgroundColor() const

    Returns the color that the text format uses for the background of table
    cells.

    \sa tableCellBackgroundColor()
*/

/*!
    Sets the text format's \a font.
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
	font.setPointSizeF(fontPointSize());

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

    \value LeftToRight   The text flows from left to right.
    \value RightToLeft   The text flows from right to left.
    \value AutoDirection The configuration of the system determines the direction.
*/

/*!
    \fn QTextBlockFormat::QTextBlockFormat()

    Constructs a new QTextBlockFormat.
*/

/*!
    \fn QTextBlockFormat::isValid() const

    Returns true if this block format is valid; otherwise returns
    false.
*/

/*!
    \fn void QTextBlockFormat::setDirection(Direction direction)

    Sets the text's direction to the specified \a direction.

    \sa direction()
*/


/*!
    \fn Direction QTextBlockFormat::direction() const

    Returns the text's direction.

    \sa setDirection()
*/


/*!
    \fn void QTextBlockFormat::setAlignment(Qt::Alignment alignment)

    Sets the paragraph's \a alignment.

    \sa alignment()
*/


/*!
    \fn Qt::Alignment QTextBlockFormat::alignment() const

    Returns the paragraph's alignment.

    \sa setAlignment()
*/


/*!
    \fn void QTextBlockFormat::setTopMargin(int margin)

    Sets the paragraph's top \a margin.

    \sa topMargin() setBottomMargin() setLeftMargin() setRightMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::topMargin() const

    Returns the paragraph's top margin.

    \sa setTopMargin() bottomMargin()
*/


/*!
    \fn void QTextBlockFormat::setBottomMargin(int margin)

    Sets the paragraph's bottom \a margin.

    \sa bottomMargin() setTopMargin() setLeftMargin() setRightMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::bottomMargin() const

    Returns the paragraph's bottom margin.

    \sa setBottomMargin() topMargin()
*/


/*!
    \fn void QTextBlockFormat::setLeftMargin(int margin)

    Sets the paragraph's left \a margin. Indentation can be applied separately
    with setIndent().

    \sa leftMargin() setRightMargin() setTopMargin() setBottomMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::leftMargin() const

    Returns the paragraph's left margin.

    \sa setLeftMargin() rightMargin() indent()
*/


/*!
    \fn void QTextBlockFormat::setRightMargin(int margin)

    Sets the paragraph's right \a margin.

    \sa rightMargin() setLeftMargin() setTopMargin() setBottomMargin() setFirstLineMargin()
*/


/*!
    \fn int QTextBlockFormat::rightMargin() const

    Returns the paragraph's right margin.

    \sa setRightMargin() leftMargin()
*/


/*!
    \fn void QTextBlockFormat::setFirstLineMargin(int margin)

    Sets the \a margin for the first line in the block. This allows the first
    line of a paragraph to be indented differently to the other lines,
    enhancing the readability of the text.

    \sa firstLineMargin() setLeftMargin() setRightMargin() setTopMargin() setBottomMargin()
*/


/*!
    \fn int QTextBlockFormat::firstLineMargin() const

    Returns the paragraph's first line margin.

    \sa setFirstLineMargin()
*/


/*!
    \fn void QTextBlockFormat::setIndent(int indentation)

    Sets the paragraph's \a indentation. Margins are set independently of
    indentation with setLeftMargin() and setFirstLineMargin().

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

    Sets the paragraph's background \a color.

    \sa backgroundColor()
*/

/*!
    \fn void QTextBlockFormat::clearBackgroundColor()

    Clears the paragraph's background color. The default background color
    will be used for the paragraph.

    \sa backgroundColor() setBackgroundColor()
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

    A list is composed of one or more items, represented as text blocks.
    The list's format specifies the appearance of items in the list.
    In particular, it determines the indentation and the style of each item.

    The indentation of the items is an integer value that causes each item to
    be offset from the left margin by a certain amount. This value is read with
    indent() and set with setIndent().

    The style used to decorate each item is set with setStyle() and can be read
    with the style() function. The style controls the type of bullet points and
    numbering scheme used for items in the list. Note that lists that use the
    decimal numbering scheme begin counting at 1 rather than 0.

    \sa QTextList
*/

/*!
    \enum QTextListFormat::Style

    This enum describes the symbols used to decorate list items:

    \value ListDisc        a filled circle
    \value ListCircle      an empty circle
    \value ListSquare      a filled square
    \value ListDecimal     decimal values in ascending order
    \value ListLowerAlpha  lower case Latin characters in alphabetical order
    \value ListUpperAlpha  upper case Latin characters in alphabetical order
    \omitvalue ListStyleUndefined
*/

/*!
    \fn QTextListFormat::QTextListFormat()

    Constructs a new list format object.
*/

/*!
    \fn bool QTextListFormat::isValid() const

    Returns true if this list format is valid; otherwise
    returns false.
*/

/*!
    \fn void QTextListFormat::setStyle(Style style)

    Sets the list format's \a style. See \c{Style} for the available styles.

    \sa style()
*/

/*!
    \fn Style QTextListFormat::style() const

    Returns the list format's style. See \c{Style}.

    \sa setStyle()
*/


/*!
    \fn void QTextListFormat::setIndent(int indentation)

    Sets the list format's \a indentation.

    \sa indent()
*/


/*!
    \fn int QTextListFormat::indent() const

    Returns the list format's indentation.

    \sa setIndent()
*/


/*!
    \class QTextFrameFormat
    \brief The QTextFrameFormat class provides formatting information for
    frames in a QTextDocument.

    \ingroup text

    A text frame groups together one or more blocks of text, providing a layer
    of structure larger than the paragraph. The format of a frame specifies
    how it is rendered and positioned on the screen. It does not directly
    specify the behavior of the text formatting within, but provides
    constraints on the layout of its children.

    The frame format defines the width() and height() of the frame on the
    screen. Each frame can have a border() that surrounds its contents with
    a rectangular box. The border is surrounded by a margin() around the frame,
    and the contents of the frame are kept separate from the border by the
    frame's padding(). This scheme is similar to the box model used by Cascading
    Style Sheets for HTML pages.

    \img qtextframe-style.png

    The position() of a frame is set using setPosition() and determines how it
    is located relative to the surrounding text.

    The validity of a QTextFrameFormat object can be determined with the
    isValid() function.

    \sa QTextFrame QTextBlockFormat
*/

/*!
    \enum QTextFrameFormat::Position

    \value InFlow
    \value FloatLeft
    \value FloatRight

*/

/*!
    \fn QTextFrameFormat::QTextFrameFormat()

    Constructs a text frame format object with the default properties.
*/

/*!
    \fn QTextFrameFormat::isValid() const

    Returns true if the format description is valid; otherwise returns false.
*/

/*!
    \fn QTextFrameFormat::setPosition(Position policy)

    Sets the \a policy for positioning frames with this frame format.

*/

/*!
    \fn Position QTextFrameFormat::position() const

    Returns the positioning policy for frames with this frame format.
*/

/*!
    \fn QTextFrameFormat::setBorder(int width)

    Sets the \a width (in pixels) of the frame's border.
*/

/*!
    \fn int QTextFrameFormat::border() const

    Returns the width of the border in pixels.
*/

/*!
    \fn QTextFrameFormat::setMargin(int margin)

    Sets the frame's \a margin in pixels.
*/

/*!
    \fn int QTextFrameFormat::margin() const

    Returns the width of the frame's external margin in pixels.
*/

/*!
    \fn QTextFrameFormat::setPadding(int width)

    Sets the \a width of the frame's internal padding in pixels.
*/

/*!
    \fn int QTextFrameFormat::padding() const

    Returns the width of the frame's internal padding in pixels.
*/

/*!
    \fn QTextFrameFormat::setWidth(const QTextLength &width)

    Sets the frame's border rectangle's \a width.

    \sa QTextLength
*/

/*!
    \fn QTextFrameFormat::setWidth(int width)
    \overload

    Convenience method that sets the width of the frame's border
    rectangle's width to the specified fixed \a width.
*/

/*!
    \fn QTextLength QTextFrameFormat::width() const

    Returns the width of the frame's border rectangle.

    \sa QTextLength
*/

/*!
    \fn void QTextFrameFormat::setHeight(const QTextLength &height)

    Sets the frame's \a height.
*/

/*!
    \fn void QTextFrameFormat::setHeight(int height)
    \overload

    Sets the frame's \a height.
*/

/*!
    \fn int QTextFrameFormat::height() const

    Returns the height of the frame's border rectangle.
*/

/*!
    \class QTextTableFormat qtextformat.h
    \brief The QTextTableFormat class provides formatting information for
    tables in a QTextDocument.

    \ingroup text

    A table is a group of cells ordered into rows and columns. Each table
    contains at least one row and one column. Each cell contains a block.
    Tables in rich text documents are formatted using the properties
    defined in this class.

    Tables are horizontally justified within their parent frame according to the
    table's alignment. This can be read with the alignment() function and set
    with setAlignment().

    Cells within the table are separated by cell spacing. The number of pixels
    between cells is set with setCellSpacing() and read with cellSpacing().
    The contents of each cell is surrounded by cell padding. The number of pixels
    between each cell edge and its contents is set with setCellPadding() and read
    with cellPadding().

    \image qtexttableformat-cell.png

    The table's background color can be read with the backgroundColor() function,
    and can be specified with setBackgroundColor(). The background color of each
    cell can be set independently, and will control the color of the cell within
    the padded area.

    The table format also provides a way to constrain the widths of the columns
    in the table. Columns can be assigned a fixed width, a variable width, or
    a percentage of the available width (see QTextLength). The columns() function
    returns the number of columns with constraints, and the
    columnWidthConstraints() function returns the constraints defined for the
    table. These quantities can also be set by calling setColumnWidthConstraints()
    with a vector containing new constraints. The setColumns() function can be
    used to change the number of constraints in use. If no constraints are
    required, clearColumnWidthConstraints() can be used to remove them.

    \sa QTextTable QTextTableCell QTextLength
*/

/*!
    \fn QTextTableFormat::QTextTableFormat()

    Constructs a new table format object.
*/
QTextTableFormat::QTextTableFormat()
 : QTextFrameFormat()
{
    setObjectType(TableObject);
    setCellSpacing(2);
}


/*!
    \fn bool QTextTableFormat::isValid() const

    Returns true if this table format is valid; otherwise
    returns false.
*/


/*!
    \fn int QTextTableFormat::columns() const

    Returns the number of columns specified by the table format.

    \sa setColumns()
*/


/*!
    \fn void QTextTableFormat::setColumns(int columns)

    Sets the number of \a columns required by the table format.

    \sa columns()
*/

/*!
    \fn void QTextTableFormat::clearColumnWidthConstraints()

    Clears the column width constraints for the table.

    \sa columnWidthConstraints() setColumnWidthConstraints()
*/

/*!
    \fn void QTextTableFormat::setColumnWidthConstraints(const QVector<QTextLength> &constraints)

    Sets the column width \a constraints for the table.

    \sa columnWidthConstraints() clearColumnWidthConstraints()
*/

/*!
    \fn QVector<QTextLength> QTextTableFormat::columnWidthConstraints() const

    Returns a list of constraints used by this table format to control the
    appearance of columns in a table.

    \sa setColumnWidthConstraints()
*/

/*!
    \fn int QTextTableFormat::cellSpacing() const

    Returns the table's cell spacing. This describes the distance between
    adjacent cells.
*/

/*!
    \fn void QTextTableFormat::setCellSpacing(int spacing)

    Sets the cell \a spacing for the table. This determines the distance
    between adjacent cells.
*/

/*!
    \fn int QTextTableFormat::cellPadding() const

    Returns the table's cell padding. This describes the distance between
    the border of a cell and its contents.
*/

/*!
    \fn void QTextTableFormat::setCellPadding(int padding)

    Sets the cell \a padding for the table. This determines the distance
    between the border of a cell and its contents.
*/

/*!
    \fn void QTextTableFormat::setAlignment(Qt::Alignment alignment)

    Sets the table's \a alignment.

    \sa alignment()
*/

/*!
    \fn Qt::Alignment QTextTableFormat::alignment() const

    Returns the table's alignment.

    \sa setAlignment()
*/

/*!
    \fn void QTextTableFormat::setBackgroundColor(const QColor &color)

    Sets the table's background \a color.

    \sa backgroundColor()
*/

/*!
    \fn QColor QTextTableFormat::backgroundColor() const

    Returns the table's background color.

    \sa setBackgroundColor()
*/

/*!
    \fn void QTextTableFormat::clearBackgroundColor()

    Clears the table's background color. The default background color will
    be used for the table.

    \sa backgroundColor() setBackgroundColor()
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

    Creates a new image format object.
*/


/*!
    \fn bool QTextImageFormat::isValid() const

    Returns true if this image format is valid; otherwise returns false.
*/


/*!
    \fn void QTextImageFormat::setName(const QString &name)

    Sets the \a name of the image. The \a name is used to locate the image
    in the application's resources.

    \sa name()
*/


/*!
    \fn QString QTextImageFormat::name() const

    Returns the name of the image. The name refers to an entry in the
    application's resources file.

    \sa setName()
*/


/*!
    \fn void QTextImageFormat::setWidth(int width)

    Sets the \a width of the rectangle occupied by the image.

    \sa width() setHeight()
*/


/*!
    \fn int QTextImageFormat::width() const

    Returns the width of the rectangle occupied by the image.

    \sa height() setWidth()
*/


/*!
    \fn void QTextImageFormat::setHeight(int height)

    Sets the \a height of the rectangle occupied by the image.

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
    const int formatIndex = indexForFormat(f);
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
    const int objectIndex = objFormats.size();
    objFormats.append(indexForFormat(f));
    return objectIndex;
}

QTextFormat QTextFormatCollection::format(int idx) const
{
    if (idx < 0 || idx >= formats.count())
        return QTextFormat();

    return formats.at(idx);
}

