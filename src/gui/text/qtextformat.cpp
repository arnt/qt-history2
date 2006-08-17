/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
    \class QTextLength
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
    \fn explicit QTextLength::QTextLength()

    Constructs a new length object which represents a variable size.
*/

/*!
    \fn QTextLength::QTextLength(Type type, qreal value)

    Constructs a new length object of the given \a type and \a value.
*/

/*!
    \fn Type QTextLength::type() const

    Returns the type of length.

    \sa QTextLength::Type
*/

/*!
    \fn qreal QTextLength::value(qreal maximumLength) const

    Returns the effective length, constrained by the type of the length object
    and the specified \a maximumLength.

    \sa type()
*/

/*!
    \fn qreal QTextLength::rawValue() const

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

/*!
   Returns the text length as a QVariant
*/
QTextLength::operator QVariant() const
{
    return QVariant(QVariant::TextLength, this);
}

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
    QTextFormatPrivate() : hashDirty(true), fontDirty(true), hashValue(0) {}

    struct Property
    {
        inline Property(qint32 k, const QVariant &v) : key(k), value(v) {}
        inline Property() {}

        qint32 key;
        QVariant value;

        inline bool operator==(const Property &other) const
        { return key == other.key && value == other.value; }
        inline bool operator!=(const Property &other) const
        { return key != other.key || value != other.value; }
    };

    inline uint hash() const
    {
        if (!hashDirty)
            return hashValue;
        return recalcHash();
    }

    inline bool operator==(const QTextFormatPrivate &rhs) const {
        if (hash() != rhs.hash())
            return false;

        return props == rhs.props;
    }

    inline void insertProperty(qint32 key, const QVariant &value)
    {
        hashDirty = true;
        fontDirty = true;
        for (int i = 0; i < props.count(); ++i)
            if (props.at(i).key == key) {
                props[i].value = value;
                return;
            }
        props.append(Property(key, value));
    }

    inline void clearProperty(qint32 key)
    {
        hashDirty = true;
        fontDirty = true;
        for (int i = 0; i < props.count(); ++i)
            if (props.at(i).key == key) {
                props.remove(i);
                return;
            }
    }

    inline int propertyIndex(qint32 key) const
    {
        for (int i = 0; i < props.count(); ++i)
            if (props.at(i).key == key)
                return i;
        return -1;
    }

    inline QVariant property(qint32 key) const
    {
        const int idx = propertyIndex(key);
        if (idx < 0)
            return QVariant();
        return props.at(idx).value;
    }

    inline bool hasProperty(qint32 key) const
    { return propertyIndex(key) != -1; }

    void resolveFont(const QFont &defaultFont);

    inline const QFont &font() const {
        if (fontDirty)
            recalcFont();
        return fnt;
    }

    QVector<Property> props;
private:

    uint recalcHash() const;
    void recalcFont() const;

    mutable bool hashDirty;
    mutable bool fontDirty;
    mutable uint hashValue;
    mutable QFont fnt;

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
    case QVariant::Color: return qHash(qvariant_cast<QColor>(variant).rgb());
    default: break;
    }
    return qHash(variant.typeName());
}

uint QTextFormatPrivate::recalcHash() const
{
    hashValue = 0;
    for (QVector<Property>::ConstIterator it = props.constBegin(); it != props.constEnd(); ++it)
        hashValue += (it->key << 16) + variantHash(it->value);

    hashDirty = false;

    return hashValue;
}

void QTextFormatPrivate::resolveFont(const QFont &defaultFont)
{
    recalcFont();
    const uint oldMask = fnt.resolve();
    fnt = fnt.resolve(defaultFont);

    if (hasProperty(QTextFormat::FontSizeAdjustment)) {
        const qreal scaleFactors[7] = {0.7, 0.8, 1.0, 1.2, 1.5, 2, 2.4};

        const int htmlFontSize = qBound(0, property(QTextFormat::FontSizeAdjustment).toInt() + 3 - 1, 6);


        if (defaultFont.pointSize() <= 0) {
            qreal pixelSize = scaleFactors[htmlFontSize] * defaultFont.pixelSize();
            fnt.setPixelSize(qRound(pixelSize));
        } else {
            qreal pointSize = scaleFactors[htmlFontSize] * defaultFont.pointSizeF();
            fnt.setPointSizeF(pointSize);
        }
    }

    fnt.resolve(oldMask);
}

void QTextFormatPrivate::recalcFont() const
{
    // update cached font as well
    QFont f;

    for (int i = 0; i < props.count(); ++i) {
        switch (props.at(i).key) {
            case QTextFormat::FontFamily:
                f.setFamily(props.at(i).value.toString());
                break;
            case QTextFormat::FontPointSize:
                f.setPointSizeF(props.at(i).value.toDouble());
                break;
            case  QTextFormat::FontPixelSize:
                f.setPixelSize(props.at(i).value.toInt());
                break;
            case QTextFormat::FontWeight:
                f.setWeight(props.at(i).value.toInt());
                break;
            case QTextFormat::FontItalic:
                f.setItalic(props.at(i).value.toBool());
                break;
            case QTextFormat::FontUnderline:
                f.setUnderline(props.at(i).value.toBool());
                break;
            case QTextFormat::TextUnderlineStyle:
                f.setUnderline(static_cast<QTextCharFormat::UnderlineStyle>(props.at(i).value.toInt()) == QTextCharFormat::SingleUnderline);
                break;
            case QTextFormat::FontOverline:
                f.setOverline(props.at(i).value.toBool());
                break;
            case QTextFormat::FontStrikeOut:
                f.setStrikeOut(props.at(i).value.toBool());
                break;
            case QTextFormat::FontFixedPitch:
                f.setFixedPitch(props.at(i).value.toBool());
                break;
            default:
                break;
            }
    }
    fnt = f;
    fontDirty = false;
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QTextFormat &fmt)
{
    stream << fmt.format_type << fmt.properties();
    return stream;
}

Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QTextFormat &fmt)
{
    QMap<qint32, QVariant> properties;
    stream >> fmt.format_type >> properties;
    for (QMap<qint32, QVariant>::ConstIterator it = properties.constBegin();
         it != properties.constEnd(); ++it)
        fmt.d->insertProperty(it.key(), it.value());
    return stream;
}

/*!
    \class QTextFormat
    \brief The QTextFormat class provides formatting information for a
    QTextDocument.

    \ingroup text
    \ingroup shared

    A QTextFormat is a generic class used for describing the format of
    parts of a QTextDocument. The derived classes QTextCharFormat,
    QTextBlockFormat, QTextListFormat, and QTextTableFormat are usually
    more useful, and describe the formatting that is applied to
    specific parts of the document.

    A format has a \c FormatType which specifies the kinds of thing it
    can format; e.g. a block of text, a list, a table, etc. A format
    also has various properties (some specific to particular format
    types), as described by the Property enum. Every property has a
    corresponding Property.

    The format type is given by type(), and the format can be tested
    with isCharFormat(), isBlockFormat(), isListFormat(),
    isTableFormat(), isFrameFormat(), and isImageFormat(). If the
    type is determined, it can be retrieved with toCharFormat(),
    toBlockFormat(), toListFormat(), toTableFormat(), toFrameFormat(),
    and toImageFormat().

    A format's properties can be set with the setProperty() functions,
    and retrieved with boolProperty(), intProperty(), doubleProperty(),
    and stringProperty() as appropriate. All the property IDs used in
    the format can be retrieved with allPropertyIds(). One format can
    be merged into another using merge().

    A format's object index can be set with setObjectIndex(), and
    retrieved with objectIndex(). These methods can be used to
    associate the format with a QTextObject. It is used to represent
    lists, frames, and tables inside the document.

    \sa {Text Related Classes}
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
    \value LayoutDirection  The layout direction of the text in the document
                            (Qt::LayoutDirection).

    \value OutlinePen
    \value ForegroundBrush
    \value BackgroundBrush

    Paragraph properties

    \value BlockAlignment
    \value BlockTopMargin
    \value BlockBottomMargin
    \value BlockLeftMargin
    \value BlockRightMargin
    \value TextIndent
    \value BlockIndent
    \value BlockNonBreakableLines
    \value BlockTrailingHorizontalRulerWidth

    Character properties

    \value FontFamily
    \value FontPointSize
    \omitvalue FontSizeAdjustment
    \value FontSizeIncrement
    \value FontWeight
    \value FontItalic
    \value FontUnderline
    \value FontOverline
    \value FontStrikeOut
    \value FontFixedPitch
    \value FontPixelSize

    \value TextUnderlineColor
    \value TextVerticalAlignment
    \value TextOutline
    \value TextUnderlineStyle

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
    \value FrameWidth
    \value FrameHeight
    \value TableColumnWidthConstraints
    \value TableCellSpacing
    \value TableCellPadding
    \value TableHeaderRowCount

    Table cell properties

    \value TableCellRowSpan
    \value TableCellColumnSpan

    Image properties

    \value ImageName
    \value ImageWidth
    \value ImageHeight

    Selection properties

    \value FullWidthSelection

    \value UserProperty
*/

/*!
    \enum QTextFormat::ObjectTypes

    \value NoObject
    \value ImageObject
    \value TableObject
    \value UserObject The first object that can be used for application-specific purposes.
*/

/*!
    \fn bool QTextFormat::isValid() const

    Returns true if the format is valid (i.e. is not
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
    : format_type(InvalidFormat)
{
}

/*!
    Creates a new text format of the given \a type.

    \sa FormatType
*/
QTextFormat::QTextFormat(int type)
    : format_type(type)
{
}


/*!
    \fn QTextFormat::QTextFormat(const QTextFormat &other)

    Creates a new text format with the same attributes at the \a other
    text format.
*/
QTextFormat::QTextFormat(const QTextFormat &rhs)
    : d(rhs.d), format_type(rhs.format_type)
{
}

/*!
    \fn QTextFormat &QTextFormat::operator=(const QTextFormat &other)

    Assigns the \a other text format to this text format, and returns a
    reference to this text format.
*/
QTextFormat &QTextFormat::operator=(const QTextFormat &rhs)
{
    d = rhs.d;
    format_type = rhs.format_type;
    return *this;
}

/*!
    Destroys this text format.
*/
QTextFormat::~QTextFormat()
{
}


/*!
   Returns the text format as a QVariant
*/
QTextFormat::operator QVariant() const
{
    return QVariant(QVariant::TextFormat, this);
}

/*!
    Merges the \a other format with this format; where there are
    conflicts the \a other format takes precedence.
*/
void QTextFormat::merge(const QTextFormat &other)
{
    if (format_type != other.format_type)
        return;

    if (!d) {
        d = other.d;
        return;
    }

    if (!other.d)
        return;

    QTextFormatPrivate *d = this->d;

    const QVector<QTextFormatPrivate::Property> &otherProps = other.d->props;
    d->props.reserve(d->props.size() + otherProps.size());
    for (int i = 0; i < otherProps.count(); ++i) {
        const QTextFormatPrivate::Property &p = otherProps.at(i);
        d->insertProperty(p.key, p.value);
    }
}

/*!
    Returns the type of this format.

    \sa ObjectTypes
*/
int QTextFormat::type() const
{
    return format_type;
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
    property isn't of QTextFormat::Bool type, false is returned instead.

    \sa setProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
bool QTextFormat::boolProperty(int propertyId) const
{
    if (!d)
        return false;
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Bool)
        return false;
    return prop.toBool();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property is not of QTextFormat::Integer type, 0 is returned instead.

    \sa setProperty() boolProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
int QTextFormat::intProperty(int propertyId) const
{
    if (!d)
        return 0;
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Int)
        return 0;
    return prop.toInt();
}

/*!
    Returns the value of the property specified by \a propertyId. If the
    property isn't of QVariant::Double type, 0 is returned instead.

    \sa setProperty() boolProperty() intProperty() stringProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
qreal QTextFormat::doubleProperty(int propertyId) const
{
    if (!d)
        return 0.;
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Double)
        return 0.;
    return prop.toDouble(); // ####
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::String type, an empty string is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() colorProperty() lengthProperty() lengthVectorProperty() Property
*/
QString QTextFormat::stringProperty(int propertyId) const
{
    if (!d)
        return QString();
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::String)
        return QString();
    return prop.toString();
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Color type, an invalid color is
    returned instead.

    \sa setProperty(), boolProperty(), intProperty(), doubleProperty(),
        stringProperty(), lengthProperty(), lengthVectorProperty(), Property
*/
QColor QTextFormat::colorProperty(int propertyId) const
{
    if (!d)
        return QColor();
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Color)
        return QColor();
    return qvariant_cast<QColor>(prop);
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Pen type, Qt::NoPen is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() lengthProperty() lengthVectorProperty() Property
*/
QPen QTextFormat::penProperty(int propertyId) const
{
    if (!d)
        return QPen(Qt::NoPen);
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Pen)
        return QPen(Qt::NoPen);
    return qvariant_cast<QPen>(prop);
}

/*!
    Returns the value of the property given by \a propertyId; if the
    property isn't of QVariant::Brush type, Qt::NoBrush is
    returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() lengthProperty() lengthVectorProperty() Property
*/
QBrush QTextFormat::brushProperty(int propertyId) const
{
    if (!d)
        return QBrush(Qt::NoBrush);
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Brush)
        return QBrush(Qt::NoBrush);
    return qvariant_cast<QBrush>(prop);
}

/*!
    Returns the value of the property given by \a propertyId.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthVectorProperty() Property
*/
QTextLength QTextFormat::lengthProperty(int propertyId) const
{
    if (!d)
        return QTextLength();
    return qvariant_cast<QTextLength>(d->property(propertyId));
}

/*!
    Returns the value of the property given by \a propertyId. If the
    property isn't of QTextFormat::LengthVector type, an empty length
    vector is returned instead.

    \sa setProperty() boolProperty() intProperty() doubleProperty() stringProperty() colorProperty() lengthProperty() Property
*/
QVector<QTextLength> QTextFormat::lengthVectorProperty(int propertyId) const
{
    QVector<QTextLength> vector;
    if (!d)
        return vector;
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::List)
        return vector;

    QList<QVariant> propertyList = prop.toList();
    for (int i=0; i<propertyList.size(); ++i) {
        QVariant var = propertyList.at(i);
        if (var.type() == QVariant::TextLength)
            vector.append(qvariant_cast<QTextLength>(var));
    }

    return vector;
}

/*!
    Returns the property specified by the given \a propertyId.
*/
QVariant QTextFormat::property(int propertyId) const
{
    return d ? d->property(propertyId) : QVariant();
}

/*!
    Sets the property specified by the \a propertyId to the given \a value.
*/
void QTextFormat::setProperty(int propertyId, const QVariant &value)
{
    if (!d)
        d = new QTextFormatPrivate;
    if (!value.isValid())
        clearProperty(propertyId);
    else
        d->insertProperty(propertyId, value);
}

/*!
    Sets the value of the property given by \a propertyId to \a value.

    \sa lengthVectorProperty() Property
*/
void QTextFormat::setProperty(int propertyId, const QVector<QTextLength> &value)
{
    if (!d)
        d = new QTextFormatPrivate;
    QVariantList list;
    for (int i=0; i<value.size(); ++i)
        list << value.at(i);
    d->insertProperty(propertyId, list);
}

/*!
  Clears the value of the property given by \a propertyId
 */
void QTextFormat::clearProperty(int propertyId)
{
    if (!d)
        return;
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
    if (!d)
        return -1;
    const QVariant prop = d->property(ObjectIndex);
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
        if (d)
            d->clearProperty(ObjectIndex);
    } else {
        if (!d)
            d = new QTextFormatPrivate;
        // ### type
        d->insertProperty(ObjectIndex, o);
    }
}

/*!
    Returns true if the text format has a property with the given \a
    propertyId; otherwise returns false.

    \sa properties() Property
*/
bool QTextFormat::hasProperty(int propertyId) const
{
    return d ? d->hasProperty(propertyId) : false;
}

/*
    Returns the property type for the given \a propertyId.

    \sa hasProperty() allPropertyIds() Property
*/

/*!
    Returns a map with all properties of this text format.
*/
QMap<int, QVariant> QTextFormat::properties() const
{
    QMap<int, QVariant> map;
    if (d) {
        for (int i = 0; i < d->props.count(); ++i)
            map.insert(d->props.at(i).key, d->props.at(i).value);
    }
    return map;
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
    if (format_type != rhs.format_type)
        return false;

    if (d == rhs.d)
        return true;

    if (d && d->props.isEmpty() && !rhs.d)
        return true;

    if (!d && rhs.d && rhs.d->props.isEmpty())
        return true;

    if (!d || !rhs.d)
        return false;

    return *d == *rhs.d;
}

/*!
    \class QTextCharFormat
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

    The color is set with setForeground(). If the text is intended to be used
    as an anchor (for hyperlinks), this can be enabled with setAnchor(). The
    setAnchorHref() and setAnchorName() functions are used to specify the
    information about the hyperlink's destination and the anchor's name.

    If the text is written within a table, it can be made to span a number of
    rows and columns with the setTableCellRowSpan() and setTableCellColumnSpan()
    functions.

    \sa QTextFormat QTextBlockFormat QTextTableFormat QTextListFormat
*/

/*!
    \enum QTextCharFormat::VerticalAlignment

    This enum describes the ways that adjacent characters can be vertically
    aligned.

    \value AlignNormal  Adjacent characters are positioned in the standard
                        way for text in the writing system in use.
    \value AlignSuperScript Characters are placed above the baseline for
                            normal text.
    \value AlignSubScript   Characters are placed below the baseline for
                            normal text.
*/

/*!
    \enum QTextCharFormat::UnderlineStyle

    This enum describes the different ways drawing underlined text.

    \value NoUnderline          Text is draw without any underlining decoration.
    \value SingleUnderline      A line is drawn using Qt::SolidLine.
    \value DashUnderline        Dashes are drawn using Qt::DashLine.
    \value DotLine              Dots are drawn using Qt::DotLine;
    \value DashDotLine          Dashs and dots are drawn using Qt::DashDotLine.
    \value DashDotDotLine       Underlines draw drawn using Qt::DashDotDotLine.
    \value WaveUnderline        The text is underlined using a wave shaped line.
    \value SpellCheckUnderline  The underline is drawn depending on the QStyle::SH_SpellCeckUnderlineStyle
                                style hint of the QApplication style. By default this is mapped to
                                WaveUnderline, on Mac OS X it is mapped to DashDotLine.

    \sa Qt::PenStyle
*/

/*!
    \fn QTextCharFormat::QTextCharFormat()

    Constructs a new character format object.
*/
QTextCharFormat::QTextCharFormat() : QTextFormat(CharFormat) {}


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
    \fn void QTextCharFormat::setFontPointSize(qreal size)

    Sets the text format's font \a size.

    \sa setFont()
*/


/*!
    \fn qreal QTextCharFormat::fontPointSize() const

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
bool QTextCharFormat::fontUnderline() const
{
    if (hasProperty(TextUnderlineStyle))
        return underlineStyle() == SingleUnderline;
    return boolProperty(FontUnderline);
}

/*!
    \fn UnderlineStyle QTextCharFormat::underlineStyle() const

    Returns the style of underlining the text.
*/

/*!
    \fn void QTextCharFormat::setUnderlineStyle(UnderlineStyle style)

    Sets the style of underlining the text.
*/
void QTextCharFormat::setUnderlineStyle(UnderlineStyle style)
{
    setProperty(TextUnderlineStyle, style);
    // for compatibility
    setProperty(FontUnderline, style == SingleUnderline);
}

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
    \fn QPen QTextCharFormat::textOutline() const

    Returns the pen used to draw the outlines of characters in this format.
*/


/*!
    \fn void QTextCharFormat::setTextOutline(const QPen &pen)

    Sets the pen used to draw the outlines of characters to the given \a pen.
*/


/*!
    \fn void QTextFormat::setForeground(const QBrush &brush)

    Sets the foreground brush to the specified \a brush. The foreground
    brush is mostly used to render text.

    \sa foreground() clearForeground() setBackground()
*/


/*!
    \fn QBrush QTextFormat::foreground() const

    Returns the brush used to render foreground details, such as text,
    frame outlines, and table borders.

    \sa setForeground() clearForeground() background()
*/

/*!
    \fn void QTextFormat::clearForeground()

    Clears the brush used to paint the document's foreground. The default
    brush will be used.

    \sa foreground() setForeground() clearBackground()
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
    \fn void QTextCharFormat::setUnderlineColor(const QColor &color)

    Sets the underline color used for the characters with this format to
    the \a color specified.

    \sa underlineColor()
*/

/*!
    \fn QColor QTextCharFormat::underlineColor() const

    Returns the color used to underline the characters with this format.

    \sa setUnderlineColor()
*/

/*!
    \fn void QTextCharFormat::setVerticalAlignment(VerticalAlignment alignment)

    Sets the vertical alignment used for the characters with this format to
    the \a alignment specified.

    \sa verticalAlignment()
*/

/*!
    \fn VerticalAlignment QTextCharFormat::verticalAlignment() const

    Returns the vertical alignment used for characters with this format.

    \sa setVerticalAlignment()
*/

/*!
    Sets the text format's \a font.
*/
void QTextCharFormat::setFont(const QFont &font)
{
    setFontFamily(font.family());

    const qreal pointSize = font.pointSizeF();
    if (pointSize > 0) {
        setFontPointSize(pointSize);
    } else {
        const int pixelSize = font.pixelSize();
        if (pixelSize > 0)
            setProperty(QTextFormat::FontPixelSize, pixelSize);
    }

    setFontWeight(font.weight());
    setFontItalic(font.italic());
    setUnderlineStyle(font.underline() ? SingleUnderline : NoUnderline);
    setFontOverline(font.overline());
    setFontStrikeOut(font.strikeOut());
    setFontFixedPitch(font.fixedPitch());
}

/*!
    Returns the font for this character format.
*/
QFont QTextCharFormat::font() const
{
    return d ? d->font() : QFont();
}

/*!
    \class QTextBlockFormat
    \brief The QTextBlockFormat class provides formatting information for
    blocks of text in a QTextDocument.

    \ingroup text

    A document is composed of a list of blocks, represented by QTextBlock
    objects. Each block can contain an item of some kind, such as a
    paragraph of text, a table, a list, or an image. Every block has an
    associated QTextBlockFormat that specifies its characteristics.

    To cater for left-to-right and right-to-left languages you can set
    a block's direction with setDirection(). Paragraph alignment is
    set with setAlignment(). Margins are controlled by setTopMargin(),
    setBottomMargin(), setLeftMargin(), setRightMargin(). Overall
    indentation is set with setIndent(), the indentation of the first
    line with setTextIndent().

    Line breaking can be enabled and disabled with setNonBreakableLines().

    The brush used to paint the paragraph's background
    is set with \l{QTextFormat::setBackground()}{setBackground()}, and other
    aspects of the text's appearance can be customized by using the
    \l{QTextFormat::setProperty()}{setProperty()} function with the
    \c OutlinePen, \c ForegroundBrush, and \c BackgroundBrush
    \l{QTextFormat::Property} values.

    If a text block is part of a list, it can also have a list format that
    is accessible with the listFormat() function.

    \sa QTextBlock, QTextCharFormat
*/

/*!
    \fn QTextBlockFormat::QTextBlockFormat()

    Constructs a new QTextBlockFormat.
*/
QTextBlockFormat::QTextBlockFormat() : QTextFormat(BlockFormat) {}

/*!
    \fn QTextBlockFormat::isValid() const

    Returns true if this block format is valid; otherwise returns
    false.
*/

/*!
    \fn void QTextFormat::setLayoutDirection(Qt::LayoutDirection direction)

    Sets the document's layout direction to the specified \a direction.

    \sa layoutDirection()
*/


/*!
    \fn Qt::LayoutDirection QTextFormat::layoutDirection() const

    Returns the document's layout direction.

    \sa setLayoutDirection()
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
    \fn void QTextBlockFormat::setTopMargin(qreal margin)

    Sets the paragraph's top \a margin.

    \sa topMargin() setBottomMargin() setLeftMargin() setRightMargin()
*/


/*!
    \fn qreal QTextBlockFormat::topMargin() const

    Returns the paragraph's top margin.

    \sa setTopMargin() bottomMargin()
*/


/*!
    \fn void QTextBlockFormat::setBottomMargin(qreal margin)

    Sets the paragraph's bottom \a margin.

    \sa bottomMargin() setTopMargin() setLeftMargin() setRightMargin()
*/


/*!
    \fn qreal QTextBlockFormat::bottomMargin() const

    Returns the paragraph's bottom margin.

    \sa setBottomMargin() topMargin()
*/


/*!
    \fn void QTextBlockFormat::setLeftMargin(qreal margin)

    Sets the paragraph's left \a margin. Indentation can be applied separately
    with setIndent().

    \sa leftMargin() setRightMargin() setTopMargin() setBottomMargin()
*/


/*!
    \fn qreal QTextBlockFormat::leftMargin() const

    Returns the paragraph's left margin.

    \sa setLeftMargin() rightMargin() indent()
*/


/*!
    \fn void QTextBlockFormat::setRightMargin(qreal margin)

    Sets the paragraph's right \a margin.

    \sa rightMargin() setLeftMargin() setTopMargin() setBottomMargin()
*/


/*!
    \fn qreal QTextBlockFormat::rightMargin() const

    Returns the paragraph's right margin.

    \sa setRightMargin() leftMargin()
*/


/*!
    \fn void QTextBlockFormat::setTextIndent(qreal indent)

    Sets the \a indent for the first line in the block. This allows the first
    line of a paragraph to be indented differently to the other lines,
    enhancing the readability of the text.

    \sa textIndent() setLeftMargin() setRightMargin() setTopMargin() setBottomMargin()
*/


/*!
    \fn qreal QTextBlockFormat::textIndent() const

    Returns the paragraph's text indent.

    \sa setTextIndent()
*/


/*!
    \fn void QTextBlockFormat::setIndent(int indentation)

    Sets the paragraph's \a indentation. Margins are set independently of
    indentation with setLeftMargin() and setTextIdent().

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
    \class QTextListFormat
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
QTextListFormat::QTextListFormat()
    : QTextFormat(ListFormat)
{
    setIndent(1);
}

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
QTextFrameFormat::QTextFrameFormat() : QTextFormat(FrameFormat) {}

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
    \fn QTextFrameFormat::setBorder(qreal width)

    Sets the \a width (in pixels) of the frame's border.
*/

/*!
    \fn qreal QTextFrameFormat::border() const

    Returns the width of the border in pixels.
*/

/*!
    \fn QTextFrameFormat::setMargin(qreal margin)

    Sets the frame's \a margin in pixels.
*/

/*!
    \fn qreal QTextFrameFormat::margin() const

    Returns the width of the frame's external margin in pixels.
*/

/*!
    \fn QTextFrameFormat::setPadding(qreal width)

    Sets the \a width of the frame's internal padding in pixels.
*/

/*!
    \fn qreal QTextFrameFormat::padding() const

    Returns the width of the frame's internal padding in pixels.
*/

/*!
    \fn QTextFrameFormat::setWidth(const QTextLength &width)

    Sets the frame's border rectangle's \a width.

    \sa QTextLength
*/

/*!
    \fn QTextFrameFormat::setWidth(qreal width)
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
    \fn void QTextFrameFormat::setHeight(qreal height)
    \overload

    Sets the frame's \a height.
*/

/*!
    \fn qreal QTextFrameFormat::height() const

    Returns the height of the frame's border rectangle.
*/

/*!
    \class QTextTableFormat
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

    The table's background color can be read with the background() function,
    and can be specified with setBackground(). The background color of each
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
    setBorder(1);
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
    \fn qreal QTextTableFormat::cellSpacing() const

    Returns the table's cell spacing. This describes the distance between
    adjacent cells.
*/

/*!
    \fn void QTextTableFormat::setCellSpacing(qreal spacing)

    Sets the cell \a spacing for the table. This determines the distance
    between adjacent cells.
*/

/*!
    \fn qreal QTextTableFormat::cellPadding() const

    Returns the table's cell padding. This describes the distance between
    the border of a cell and its contents.
*/

/*!
    \fn void QTextTableFormat::setCellPadding(qreal padding)

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
    \fn void QTextTableFormat::setHeaderRowCount(int count)
    \since 4.2

    Declares the first \a count rows of the table as table header.
    The table header rows get repeated when a table is broken
    across a page boundary.
*/

/*!
    \fn int QTextTableFormat::headerRowCount() const
    \since 4.2

    Returns the number of rows in the table that define the header.

    \sa setHeaderRowCount()
*/

/*!
    \fn void QTextFormat::setBackground(const QBrush &brush)

    Sets the brush use to paint the document's background to the
    \a brush specified.

    \sa background() clearBackground() setForeground()
*/

/*!
    \fn QColor QTextFormat::background() const

    Returns the brush used to paint the document's background.

    \sa setBackground() clearBackground() foreground()
*/

/*!
    \fn void QTextFormat::clearBackground()

    Clears the brush used to paint the document's background. The default
    brush will be used.

    \sa background() setBackground() clearForeground()
*/


/*!
    \class QTextImageFormat
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
QTextImageFormat::QTextImageFormat() : QTextCharFormat() { setObjectType(ImageObject); }


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
    \fn void QTextImageFormat::setWidth(qreal width)

    Sets the \a width of the rectangle occupied by the image.

    \sa width() setHeight()
*/


/*!
    \fn qreal QTextImageFormat::width() const

    Returns the width of the rectangle occupied by the image.

    \sa height() setWidth()
*/


/*!
    \fn void QTextImageFormat::setHeight(qreal height)

    Sets the \a height of the rectangle occupied by the image.

    \sa height() setWidth()
*/


/*!
    \fn qreal QTextImageFormat::height() const

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
    uint hash = format.d ? format.d->hash() : 0;
    if (hashes.contains(hash)) {
        for (int i = 0; i < formats.size(); ++i) {
            if (formats.at(i) == format)
                return i;
        }
    }
    int idx = formats.size();
    formats.append(format);

    QTextFormat &f = formats.last();
    if (!f.d)
        f.d = new QTextFormatPrivate;
    f.d->resolveFont(defaultFnt);

    hashes.insert(hash);
    return idx;
}

bool QTextFormatCollection::hasFormatCached(const QTextFormat &format) const
{
    uint hash = format.d ? format.d->hash() : 0;
    if (hashes.contains(hash)) {
        for (int i = 0; i < formats.size(); ++i)
            if (formats.at(i) == format)
                return true;
    }
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

void QTextFormatCollection::setDefaultFont(const QFont &f)
{
    defaultFnt = f;
    for (int i = 0; i < formats.count(); ++i)
        if (formats[i].d)
            formats[i].d->resolveFont(defaultFnt);
}

