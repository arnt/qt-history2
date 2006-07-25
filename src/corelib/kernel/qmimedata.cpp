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

    QMimeData is used to describe information that can be stored in
    the \l{QClipboard}{clipboard}, and transferred via the \l{drag
    and drop} mechanism. QMimeData objects associate the data that
    they hold with the corresponding MIME types to ensure that
    information can be safely transferred between applications, and
    copied around within the same application.

    QMimeData objects are usually created using \c new and supplied
    to QDrag or QClipboard objects. This is to enable Qt to manage
    the memory that they use.

    A single QMimeData object can store the same data using several
    different formats at the same time. The formats() function
    returns a list of the available formats in order of preference.
    The data() function returns the raw data associated with a MIME
    type, and setData() allows you to set the data for a MIME type.

    For the most common MIME types, QMimeData provides convenience
    functions to access the data:

    \table
    \header \o Tester       \o Getter       \o Setter           \o MIME Types
    \row    \o hasText()    \o text()       \o setText()        \o \c text/plain
    \row    \o hasHtml()    \o html()       \o setHtml()        \o \c text/html
    \row    \o hasUrls()    \o urls()       \o setUrls()        \o \c text/uri-list
    \row    \o hasImage()   \o imageData()  \o setImageData()   \o \c image/*
    \row    \o hasColor()   \o colorData()  \o setColorData()   \o \c application/x-color
    \endtable

    For example, if your write a widget that accepts URL drags, you
    would end up writing code like this:

    \code
        void MyWidget::dragEnterEvent(QDragEnterEvent *event)
        {
            if (event->mimeData()->hasUrl())
                event->acceptProposedEvent();
        }

        void MyWidget::dropEvent(QDropEvent *event)
        {
            if (event->mimeData()->hasUrl()) {
                QUrl url = event->mimeData()->url();
                ...
            }
        }
    \endcode

    There are three appraches for storing custom data in a QMimeData
    object:

    \list 1
    \o  Custom data can be stored directly in a QMimeData object as a
        QByteArray using setData(). For example:

        \code
            QByteArray csvData = ...;

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("text/csv", csvData);
        \endcode

    \o  We can subclass QMimeData and reimplement hasFormat(),
        formats(), and retrieveData().

    \o  If the drag and drop operation occurs withing a single
        application, we can subclass QMimeData and add extra data in
        it, and use a qobject_cast() in the receiver's drop event
        handler. For example:

        \code
        void MyWidget::dropEvent(QDropEvent *event)
        {
            const MyMimeData *myData =
                    qobject_cast<const MyMimeData *>(event->mimeData());
            if (myData) {
                // access myData's data directly (not through QMimeData's API)
            }
        }
        \endcode
    \endlist

    \sa QClipboard, QDragEnterEvent, QDragMoveEvent, QDropEvent, QDrag,
        {Drag and Drop}
*/

/*!
    Constructs a new MIME data object with no data in it.
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

    URLs correspond to the MIME type \c text/uri-list.

    \sa hasUrls(), data()
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

    URLs correspond to the MIME type \c text/uri-list.

    \sa hasUrls(), setData()
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
    Returns true if the object can return a list of urls; otherwise
    returns false.

    URLs correspond to the MIME type \c text/uri-list.

    \sa setUrls(), urls(), hasFormat()
*/
bool QMimeData::hasUrls() const
{
    return hasFormat(QLatin1String("text/uri-list"));
}


/*!
    Returns a plain text (MIME type \c text/plain) representation of
    the data.

    \sa hasText(), html(), data()
*/
QString QMimeData::text() const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(QLatin1String("text/plain"), QVariant::String);
    return data.toString();
}

/*!
    Sets \a text as the plain text (MIME type \c text/plain) used to
    represent the data.

    \sa hasText(), setHtml(), setData()
*/
void QMimeData::setText(const QString &text)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("text/plain"), text);
}

/*!
    Returns true if the object can return plain text (MIME type \c
    text/plain); otherwise returns false.

    \sa setText(), text(), hasHtml(), hasFormat()
*/
bool QMimeData::hasText() const
{
    return hasFormat(QLatin1String("text/plain"));
}

/*!
    Returns a string if the data stored in the object is HTML (MIME
    type \c text/html); otherwise returns an empty string.

    \sa hasHtml(), setData()
*/
QString QMimeData::html() const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(QLatin1String("text/html"), QVariant::String);
    return data.toString();
}

/*!
    Sets \a html as the HTML (MIME type \c text/html) used to
    represent the data.

    \sa hasHtml(), setText(), setData()
*/
void QMimeData::setHtml(const QString &html)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("text/html"), html);
}

/*!
    Returns true if the object can return HTML (MIME type \c
    text/html); otherwise returns false.

    \sa setHtml(), html(), hasFormat()
*/
bool QMimeData::hasHtml() const
{
    return hasFormat(QLatin1String("text/html"));
}

/*!
    Returns a QVariant storing a QImage if the object can return an
    image; otherwise returns a null variant.

    A QVariant is used because QMimeData belongs to the \l QtCore
    library, whereas QImage belongs to \l QtGui. To convert the
    QVariant to a QImage, simply use qvariant_cast(). For example:

    \code
        if (event->mimeData()->hasImage()) {
            QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
            ...
        }
    \endcode

    \sa hasImage()
*/
QVariant QMimeData::imageData() const
{
    Q_D(const QMimeData);
    return d->retrieveTypedData(QLatin1String("application/x-qt-image"), QVariant::Image);
}

/*!
    Sets the data in the object to the given \a image.

    A QVariant is used because QMimeData belongs to the \l QtCore
    library, whereas QImage belongs to \l QtGui. The conversion
    from QImage to QVariant is implicit. For example:

    \code
        mimeData->setImageData(QImage("beautifulfjord.png"));
    \endcode

    \sa hasImage(), setData()
*/
void QMimeData::setImageData(const QVariant &image)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("application/x-qt-image"), image);
}

/*!
    Returns true if the object can return an image; otherwise returns
    false.

    \sa setImageData(), imageData(), hasFormat()
*/
bool QMimeData::hasImage() const
{
    return hasFormat(QLatin1String("application/x-qt-image"));
}

/*!
    Returns a color if the data stored in the object represents a
    color (MIME type \c application/x-color); otherwise returns a
    null variant.

    A QVariant is used because QMimeData belongs to the \l QtCore
    library, whereas QColor belongs to \l QtGui. To convert the
    QVariant to a QImage, simply use qvariant_cast(). For example:

    \code
        if (event->mimeData()->hasColor()) {
            QColor color = qvariant_cast<QColor>(event->mimeData()->colorData());
            ...
        }
    \endcode

    \sa hasColor(), setColorData(), data()
*/
QVariant QMimeData::colorData() const
{
    Q_D(const QMimeData);
    return d->retrieveTypedData(QLatin1String("application/x-color"), QVariant::Color);
}

/*!
    Sets the color data in the object to the given \a color.

    Colors correspond to the MIME type \c application/x-color.

    \sa hasColor(), setData()
*/
void QMimeData::setColorData(const QVariant &color)
{
    Q_D(QMimeData);
    d->setData(QLatin1String("application/x-color"), color);
}


/*!
    Returns true if the object can return a color (MIME type \c
    application/x-color); otherwise returns false.

    \sa setColorData(), colorData(), hasFormat()
*/
bool QMimeData::hasColor() const
{
    return hasFormat(QLatin1String("application/x-color"));
}

/*!
    Returns the data stored in the object in the format described by
    the MIME type specified by \a mimeType.
*/
QByteArray QMimeData::data(const QString &mimeType) const
{
    Q_D(const QMimeData);
    QVariant data = d->retrieveTypedData(mimeType, QVariant::ByteArray);
    return data.toByteArray();
}

/*!
    Sets the data associated with the MIME type given by \a mimeType
    to the specified \a data.

    For the most common types of data, you can call the higher-level
    functions setText(), setHtml(), setUrls(), setImageData(), and
    setColorData() instead.

    \sa hasFormat()
*/
void QMimeData::setData(const QString &mimeType, const QByteArray &data)
{
    Q_D(QMimeData);
    d->setData(mimeType, QVariant(data));
}

/*!
    Returns true if the object can return data for the MIME type
    specified by \a mimeType; otherwise returns false.

    For the most common types of data, you can call the higher-level
    functions hasText(), hasHtml(), hasUrls(), hasImage(), and
    hasColor() instead.

    \sa formats(), setData(), data()
*/
bool QMimeData::hasFormat(const QString &mimeType) const
{
    return formats().contains(mimeType);
}

/*!
    Returns a list of formats supported by the object. This is a list
    of MIME types for which the object can return suitable data. The
    formats in the list are in a priority order.

    For the most common types of data, you can call the higher-level
    functions hasText(), hasHtml(), hasUrls(), hasImage(), and
    hasColor() instead.

    \sa hasFormat(), setData(), data()
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
    Returns a variant with the given \a type containing data for the
    MIME type specified by \a mimeType. If the object does not
    support the MIME type or variant type given, a null variant is
    returned instead.

    This function is called by the general data() getter and by the
    convenience getters (text(), html(), urls(), imageData(), and
    colorData()). You can reimplement it if you want to store your
    data using a custom data structure (instead of a QByteArray,
    which is what setData() provides). You would then also need
    to reimplement hasFormat() and formats().

    \sa data()
*/
QVariant QMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
    Q_UNUSED(type);
    Q_D(const QMimeData);
    return d->getData(mimeType);
}

/*!
    Removes all the MIME type and data entries in the object.
*/
void QMimeData::clear()
{
    Q_D(QMimeData);
    d->dataList.clear();
}
