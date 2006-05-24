/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmimedata.h"

#include "private/qobject_p.h"
#include "qurl.h"
#include "qstringlist.h"
#include "qtextcodec.h"

struct QMimeDataStruct
{
    QString format;
    QVariant data;
};

class QMimeDataPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMimeData)
public:
    void setData(const QString &format, const QVariant &data);
    QVariant getData(const QString &format) const;

    QVariant retrieveTypedData(const QString &format, QVariant::Type type) const;

    QList<QMimeDataStruct> dataList;
};

void QMimeDataPrivate::setData(const QString &format, const QVariant &data)
{
    // remove it first if the format is already here.
    for (int i=0; i<dataList.size(); i++) {
        if (dataList.at(i).format == format) {
            dataList.removeAt(i);
            break;
        }
    }
    QMimeDataStruct mimeData;
    mimeData.format = format;
    mimeData.data = data;
    dataList += mimeData;
}


QVariant QMimeDataPrivate::getData(const QString &format) const
{
    QVariant data;
    for (int i=0; i<dataList.size(); i++) {
        if (dataList.at(i).format == format) {
            data = dataList.at(i).data;
            break;
        }
    }
    return data;
}

QVariant QMimeDataPrivate::retrieveTypedData(const QString &format, QVariant::Type type) const
{
    Q_Q(const QMimeData);

    QVariant data = q->retrieveData(format, type);
    if (data.type() == type || data.type() == QVariant::Invalid)
        return data;

    // provide more conversion possiblities than just what QVariant provides

    // URLs can be lists as well...
    if (type == QVariant::Url && data.type() == QVariant::List
        || type == QVariant::List && data.type() == QVariant::Url)
        return data;

    // images and pixmaps are interchangeable
    if (type == QVariant::Pixmap && data.type() == QVariant::Image
        || type == QVariant::Image && data.type() == QVariant::Pixmap)
        return data;

    if (data.type() == QVariant::ByteArray) {
        // see if we can convert to the requested type
        switch(type) {
#ifndef QT_NO_TEXTCODEC
        case QVariant::String: {
            const QByteArray ba = data.toByteArray();
            QTextCodec *codec = QTextCodec::codecForName("utf-8");
            if (format == QLatin1String("text/html"))
                codec = QTextCodec::codecForHtml(ba);
            return codec->toUnicode(ba);
        }
#endif // QT_NO_TEXTCODEC
        case QVariant::Color: {
            QVariant newData = data;
            newData.convert(QVariant::Color);
            return newData;
        }
        case QVariant::List: {
            if (format != QLatin1String("text/uri-list"))
                break;
            // fall through
        }
        case QVariant::Url: {
            QList<QVariant> list;
            QList<QByteArray> urls = data.toByteArray().split('\n');
            for (int i = 0; i < urls.size(); ++i) {
                QByteArray ba = urls.at(i).trimmed();
                list.append(QUrl::fromEncoded(ba));
            }
            return list;
        }
        default:
            break;
        }

    } else if (type == QVariant::ByteArray) {

        // try to convert to bytearray
        switch(data.type()) {
        case QVariant::ByteArray:
        case QVariant::Color:
            return data.toByteArray();
            break;
        case QVariant::String:
            return data.toString().toUtf8();
            break;
        case QVariant::Url:
            return data.toUrl().toEncoded();
            break;
        case QVariant::List: {
            // has to be list of URLs
            QByteArray result;
            QList<QVariant> list = data.toList();
            for (int i = 0; i < list.size(); ++i) {
                if (list.at(i).type() == QVariant::Url) {
                    result += list.at(i).toUrl().toEncoded();
                    result += "\r\n";
                }
            }
            if (!result.isEmpty())
                return result;
            break;
        }
        default:
            break;
        }
    }
    return data;
}

/*!
    \class QMimeData
    \brief The QMimeData class provides a container for data that records information
    about its MIME type.

    QMimeData is used to describe information that can be stored in the clipboard,
    and transferred via the drag and drop mechanism. QMimeData objects associate the
    data that they hold with the corresponding MIME types to ensure that information
    can be safely transferred between applications, and copied around within the
    same application.

    QMimeData objects are usually created on the heap and supplied to QDrag
    or QClipboard objects. This is to enable Qt to manage the memory that they
    use.

    The class provides a number of convenience functions to allow data in common
    formats to be stored and retrieved, and QMimeData objects can be queried to
    determine which kind of data they contain.

    Textual data types are stored with setText() and setHtml(); they can be retrieved
    with text() and html(). Visual data types are stored with setColorData() and
    setImageData(); they can be retrieved with colorData() and imageData().
    The contents of the QMimeData object can be cleared with the clear() function.

    Use the hasText() and hasHtml() functions to determine whether a given QMimeData
    object contains textual information; use hasColor() and hasImage()
    to determine whether it contains standard visual types.

    Custom data can be stored in a QMimeData object: Use the setData() function
    with a standard MIME description of the data, and a QByteArray containing the
    data itself. For example, although we could store an image using
    setImage(), we can take a Portable Network Graphics (PNG) image
    from a QByteArray and explicitly store it in a QMimeData object using the
    following code:

    \code
        QByteArray pngImage;
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("image/png", pngImage);
    \endcode

    Usually, it is easier to rely on QMimeData's support for QImage and QPixmap when
    handling images.

    \sa QClipboard, QDragEnterEvent, QDragMoveEvent, QDropEvent, QDrag,
        {Drag and Drop}
*/

/*!
    Constructs a new MIME data object.
*/
QMimeData::QMimeData()
    : QObject(*new QMimeDataPrivate, 0)
{
}

/*!
    Destroys the MIME data object.
*/
QMimeData::~QMimeData()
{
}

/*!
    Returns a list of URLs contained within the MIME data object.
*/
QList<QUrl> QMimeData::urls() const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(QLatin1String("text/uri-list"), QVariant::List);
    QList<QUrl> urls;
    if (data.type() == QVariant::Url)
        urls.append(data.toUrl());
    else if (data.type() == QVariant::List) {
        QList<QVariant> list = data.toList();
        for (int i = 0; i < list.size(); ++i) {
            if (list.at(i).type() == QVariant::Url)
                urls.append(list.at(i).toUrl());
        }
    }
    return urls;
}

/*!
    Sets the URLs stored in the MIME data object to those specified by \a urls.
*/
void QMimeData::setUrls(const QList<QUrl> &urls)
{
    Q_D(QMimeData);
    QList<QVariant> list;
    for (int i = 0; i < urls.size(); ++i)
        list.append(urls.at(i));

    d->setData(QLatin1String("text/uri-list"), list);
}

/*!
    Returns true if the object can return a list of urls otherwise returns false.
*/
bool QMimeData::hasUrls() const
{
    return hasFormat(QLatin1String("text/uri-list"));
}


/*!
    Returns a plain text representation of the data.
*/
QString QMimeData::text() const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(QLatin1String("text/plain"), QVariant::String);
    return data.toString();
}

/*!
    Sets \a text as the plain text used to represent the data.
*/
void QMimeData::setText(const QString &text)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("text/plain"), text);
}

/*!
    Returns true if the object can return text otherwise returns false.
*/
bool QMimeData::hasText() const
{
    return hasFormat(QLatin1String("text/plain"));
}

/*!
    Returns a string if the data stored in the object is HTML;
    otherwise returns an empty string.
*/
QString QMimeData::html() const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(QLatin1String("text/html"), QVariant::String);
    return data.toString();
}

/*!
    Sets the data in the object to the HTML in the \a html string.
*/
void QMimeData::setHtml(const QString &html)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("text/html"), html);
}

/*!
    Returns true if the object can return HTML otherwise returns false.
*/
bool QMimeData::hasHtml() const
{
    return hasFormat(QLatin1String("text/html"));
}

/*!
    Returns an image variant if the data stored in the object is in the correct
    form; otherwise returns an invalid variant.
*/
QVariant QMimeData::imageData() const
{
    Q_D(const QMimeData);
    return d->retrieveTypedData(QLatin1String("application/x-qt-image"), QVariant::Image);
}

/*!
    Sets the data in the object to the given \a image.
*/
void QMimeData::setImageData(const QVariant &image)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("application/x-qt-image"), image);
}

/*!
    Returns true if the object can return a image otherwise returns false.
*/
bool QMimeData::hasImage() const
{
    return hasFormat(QLatin1String("application/x-qt-image"));
}

/*!
    Returns a color if the data stored in the object represents a color;
    otherwise returns an invalid variant.
*/
QVariant QMimeData::colorData() const
{
    Q_D(const QMimeData);
    return d->retrieveTypedData(QLatin1String("application/x-color"), QVariant::Color);
}

/*!
    Sets the data in the object to the given \a color.
*/
void QMimeData::setColorData(const QVariant &color)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("application/x-color"), color);
}


/*!
    Returns true if the object can return a color otherwise returns false.
*/
bool QMimeData::hasColor() const
{
    return hasFormat(QLatin1String("application/x-color"));
}

/*!
    Returns the data stored in the object in the format described by the
    MIME type specified by \a mimetype.
*/
QByteArray QMimeData::data(const QString &mimetype) const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(mimetype, QVariant::ByteArray);
    return data.toByteArray();
}

/*!
    Sets the data associated with the MIME type given by \a mimetype to the
    specified \a data.
*/
void QMimeData::setData(const QString &mimetype, const QByteArray &data)
{
    Q_D(QMimeData);
    d->setData(mimetype, QVariant(data));
}

/*!
    Returns true if the object can return data for the MIME type specified by
    \a mimetype; otherwise returns false.
*/
bool QMimeData::hasFormat(const QString &mimetype) const
{
    return formats().contains(mimetype);
}

/*!
    Returns a list of formats supported by the object. This is a list of
    MIME types for which the object can return suitable data. The formats in
    the list are in a priority order.
*/
QStringList QMimeData::formats() const
{
    Q_D(const QMimeData);
    QStringList list;
    for (int i=0; i<d->dataList.size(); i++)
        list += d->dataList.at(i).format;
    return list;
}

/*!
    Returns a variant with the given \a type containing data for the MIME
    type specified by \a mimetype. If the object does not support the
    MIME type or variant type given, a null variant is returned instead.
*/
QVariant QMimeData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    Q_UNUSED(type);
    Q_D(const QMimeData);
    return d->getData(mimetype);
}

/*!
    Removes all the MIME type and data entries in the object.
*/
void QMimeData::clear()
{
    Q_D(QMimeData);
    d->dataList.clear();
}


