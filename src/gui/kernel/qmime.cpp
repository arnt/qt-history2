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

#include "qmime.h"
#include "qpixmap.h"
#include "qurl.h"
#include "qlist.h"
#include "qstring.h"
#include "qimageio.h"
#include "qbuffer.h"

#include "private/qobject_p.h"


/*!
    Destroys the MIME source.
*/
QMimeSource::~QMimeSource()
{
}

/*!
    Returns true if the object can provide the data in format \a
    mimeType; otherwise returns false.

    If you inherit from QMimeSource, for consistency reasons it is
    better to implement the more abstract canDecode() functions such
    as QTextDrag::canDecode() and QImageDrag::canDecode().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
        if (!qstricmp(mimeType,fmt))
            return true;
    }
    return false;
}

struct MimeData {
    QString format;
    QVariant data;
};

class QMimeDataPrivate : public QObjectPrivate
{
public:
    void setData(const QString &format, const QVariant &data);
    QVariant getData(const QString &format) const;
    QList<MimeData> dataList;
    //QMap<QString, QVariant> data;
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
    MimeData mimeData;
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

/*!
    \class QMimeData
    \brief The QMimeData class provides a container that organizes data by
    MIME type.

    \compat
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
    QVariant data = retrieveData("text/uri-list", QVariant::Url);
    QList<QUrl> urls;
    if (data.type() == QVariant::Url)
        urls.append(data.toUrl());
    else if (data.type() == QVariant::List) {
        QList<QCoreVariant> list = data.toList();
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
    QList<QCoreVariant> list;
    for (int i = 0; i < urls.size(); ++i)
        list.append(urls.at(i));

    d->setData("text/uri-list", list);
}

/*!
    Returns true if the object can return a list of urls otherwise returns false.
*/
bool QMimeData::hasUrls() const
{
    return hasFormat("text/uri-list");
}


/*!
    Returns a plain text representation of the data.
*/
QString QMimeData::text() const
{
    QVariant data = retrieveData("text/plain", QVariant::String);
    if (data.type() == QVariant::ByteArray)
        return QString::fromUtf8(data.toByteArray());
    else if (data.type() == QVariant::String)
        return data.toString();
    return QString();
}

/*!
    Sets \a text as the plain text used to represent the data.
*/
void QMimeData::setText(const QString &text)
{
    Q_D(QMimeData);
    d->setData("text/plain", text);
}

/*!
    Returns true if the object can return text otherwise returns false.
*/
bool QMimeData::hasText() const
{
    return hasFormat("text/plain");
}

/*!
    Returns a string if the data stored in the object is HTML;
    otherwise returns a null string.
*/
QString QMimeData::html() const
{
    QVariant data = retrieveData("text/html", QVariant::String);
    if (data.type() == QVariant::ByteArray)
        return QString::fromUtf8(data.toByteArray());
    else if (data.type() == QVariant::String)
        return data.toString();
    return QString();
}

/*!
    Sets the data in the object to the HTML in the \a html string.
*/
void QMimeData::setHtml(const QString &html)
{
    Q_D(QMimeData);
    d->setData("text/html", html);
}

/*!
    Returns true if the object can return HTML otherwise returns false.
*/
bool QMimeData::hasHtml() const
{
    return hasFormat("text/html");
}

/*!
    Returns a pixmap if the data stored in the object is in the correct
    form; otherwise returns a null pixmap.
*/
QPixmap QMimeData::pixmap() const
{
    // prefered format
    QVariant data = retrieveData("image/png", QVariant::Pixmap);
    if (data.type() == QVariant::Pixmap)
        return data.toPixmap();
    else if (data.type() == QVariant::Image)
        return data.toImage();
    // try any other image formats
    QStringList available = formats();
    for (int i=0; i<available.size(); i++) {
        if (available.at(i).startsWith("image/")) {
            data = retrieveData(available.at(i), QVariant::Pixmap);
            if (data.type() == QVariant::Pixmap)
                return data.toPixmap();
            else if (data.type() == QVariant::Image)
                return data.toImage();
        }
    }
    return QPixmap();
}

/*!
    Sets the data in the object to the given \a pixmap.
*/
void QMimeData::setPixmap(const QPixmap &pixmap)
{
    Q_D(QMimeData);
    QList<QByteArray> imageFormats = QImageIO::outputFormats();
    for (int i=0; i<imageFormats.size(); i++)
        d->setData("image/" + imageFormats.at(i).toLower(), pixmap);
}

/*!
    Returns true if the object can return a pixmap otherwise returns false.
*/
bool QMimeData::hasPixmap() const
{
    return hasImage();
}

/*!
    Returns a image if the data stored in the object is in the correct
    form; otherwise returns a null image.
*/
QImage QMimeData::image() const
{
    // prefered format
    QVariant data = retrieveData("image/png", QVariant::Image);
    if (data.type() == QVariant::Image)
        return data.toImage();
    else if (data.type() == QVariant::Pixmap)
        return data.toPixmap().toImage();
    // try any other image formats
    QStringList available = formats();
    for (int i=0; i<available.size(); i++) {
        if (available.at(i).startsWith("image/")) {
            data = retrieveData(available.at(i), QVariant::Image);
            if (data.type() == QVariant::Image)
                return data.toImage();
            else if (data.type() == QVariant::Pixmap)
                return data.toPixmap().toImage();
        }
    }
    return QImage();
}

/*!
    Sets the data in the object to the given \a image.
*/
void QMimeData::setImage(const QImage &image)
{
    Q_D(QMimeData);
    QList<QByteArray> imageFormats = QImageIO::outputFormats();
    for (int i=0; i<imageFormats.size(); i++)
        d->setData("image/" + imageFormats.at(i).toLower(), image);
}

/*!
    Returns true if the object can return a image otherwise returns false.
*/
bool QMimeData::hasImage() const
{
    QStringList available = formats();
    for (int i=0; i<available.size(); i++) {
        if (available.at(i).startsWith("image/"))
            return true;
    }
    return false;
}

/*!
    Returns a color if the data stored in the object represents a color;
    otherwise returns a null color.
*/
QColor QMimeData::color() const
{
    QVariant data = retrieveData("application/x-color", QVariant::Color);
    if (data.type() == QVariant::Color)
        return data.toColor();
    // ### try to decode
    return QColor();
}

/*!
    Sets the data in the object to the given \a color.
*/
void QMimeData::setColor(const QColor &color)
{
    Q_D(QMimeData);
    d->setData("application/x-color", color);
}


/*!
    Returns true if the object can return a color otherwise returns false.
*/
bool QMimeData::hasColor() const
{
    return hasFormat("application/x-color");
}

/*!
    Returns the data stored in the object in the format described by the
    MIME type specified by \a mimetype.
*/
QByteArray QMimeData::data(const QString &mimetype) const
{
    QVariant data = retrieveData(mimetype, QVariant::ByteArray);
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
    Q_D(const QMimeData);
    for (int i=0; i<d->dataList.size(); i++) {
        if (d->dataList.at(i).format == mimetype)
            return true;
    }
    return false;
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
    MIME type or variant type given, a null variant is returned instead. ###
*/
QVariant QMimeData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    Q_D(const QMimeData);
    QVariant data = d->getData(mimetype);
    if (data.type() == type || type == QVariant::Invalid)
        return data;

    // URLs can be lists as well...
    if (type == QVariant::Url && data.type() == QVariant::List)
        return data;

    // images and pixmaps are interchangeable
    if (type == QVariant::Pixmap && data.type() == QVariant::Image
        || type == QVariant::Image && data.type() == QVariant::Pixmap)
        return data;

    if (data.type() == QVariant::ByteArray) {
        QByteArray ba = data.toByteArray();
        // see if we can convert to the requested type
        switch(type) {
        case QVariant::String:
            return QString::fromUtf8(ba);
        case QVariant::Pixmap:
        case QVariant::Image:
            {
                QImage image;
                if (image.loadFromData(data.toByteArray()))
                    return image;
            }
            break;
        case QVariant::Color:
            return QColor(ba.data());
        case QVariant::Url: {
            QList<QCoreVariant> list;
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
    }

    // try to convert to bytearray
    QByteArray result;
    switch(data.type()) {
    case QVariant::ByteArray:
        result = data.toByteArray();
        break;
    case QVariant::String:
        result = data.toString().toUtf8();
        break;
    case QVariant::Pixmap:
    case QVariant::Image: {
        QImage image;
        if (QVariant::Pixmap)
            image = data.toPixmap().toImage();
        else
            image = data.toImage();
        QString format;
        if (mimetype.startsWith("image/"))
            format = mimetype.mid(6);
        else
            format = "png";
        QBuffer buf(&result);
        buf.open(QBuffer::WriteOnly);
        image.save(&buf, format.toUpper().latin1());
        break;
    }
    case QVariant::Color:
        result = data.toColor().name().toLatin1();
        break;
    case QVariant::Url:
        result = data.toUrl().toEncoded();
        break;
    case QVariant::List: {
        // has to be list of URLs
        QList<QCoreVariant> list = data.toList();
        for (int i = 0; i < list.size(); ++i) {
            if (list.at(i).type() == QVariant::Url) {
                result += list.at(i).toUrl().toEncoded();
                result += "\r\n";
            }
        }
        break;
    }
    default:
        break;
    }
    return result;
}

/*!
    Removes all the MIME type and data entries in the object.
*/
void QMimeData::clear()
{
    Q_D(QMimeData);
    d->dataList.clear();
}
