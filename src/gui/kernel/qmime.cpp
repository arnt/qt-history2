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
#include "qmap.h"

#include "private/qobject_p.h"


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

class QMimeDataPrivate : public QObjectPrivate
{
public:
    QMap<QString, QVariant> data;
};

QMimeData::QMimeData()
    : QObject(*new QMimeDataPrivate, 0)
{
}

QMimeData::~QMimeData()
{
}

QList<QUrl> QMimeData::urls() const
{
    QVariant data = retrieveData("text/uri-list", QVariant::Invalid);
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

void QMimeData::setUrls(const QList<QUrl> &urls)
{
    Q_D(QMimeData);
    QList<QCoreVariant> list;
    for (int i = 0; i < urls.size(); ++i)
        list.append(urls.at(i));
    d->data["text/uri-list"] = list;
}

QString QMimeData::text() const
{
    QVariant data = retrieveData("text/plain", QVariant::String);
    if (data.type() == QVariant::ByteArray)
        return QString::fromUtf8(data.toByteArray());
    else if (data.type() == QVariant::String)
        return data.toString();
    return QString();
}

void QMimeData::setText(const QString &text)
{
    Q_D(QMimeData);
    d->data["text/plain"] = text;
}

QString QMimeData::html() const
{
    QVariant data = retrieveData("text/html", QVariant::String);
    if (data.type() == QVariant::ByteArray)
        return QString::fromUtf8(data.toByteArray());
    else if (data.type() == QVariant::String)
        return data.toString();
    return QString();
}

void QMimeData::setHtml(const QString &html)
{
    Q_D(QMimeData);
    d->data["text/html"] = html;
}

QPixmap QMimeData::pixmap() const
{
    QVariant data = retrieveData("image/ppm", QVariant::Pixmap);
    if (data.type() == QVariant::Pixmap)
        return data.toPixmap();
    // ### try to decode
    return QPixmap();
}

void QMimeData::setPixmap(const QPixmap &pixmap)
{
    Q_D(QMimeData);
    d->data["image/ppm"] = pixmap;
}

QByteArray QMimeData::data(const QString &mimetype) const
{
    QVariant data = retrieveData(mimetype, QVariant::ByteArray);
    return data.toByteArray();
}

void QMimeData::setData(const QString &mimetype, const QByteArray &data)
{
    Q_D(QMimeData);
    d->data[mimetype] = QVariant(data);
}

bool QMimeData::hasFormat(const QString &mimetype) const
{
    Q_D(const QMimeData);
    return d->data.contains(mimetype);
}

QStringList QMimeData::formats() const
{
    Q_D(const QMimeData);
    return d->data.keys();
}

QVariant QMimeData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    Q_D(const QMimeData);
    QVariant data = d->data.value(mimetype);
    if (data.type() == type || type == QVariant::Invalid)
        return data;

    // types don't match, convert to bytearray
    QByteArray result;
    switch(data.type()) {
    case QVariant::ByteArray:
        result = data.toByteArray();
        break;
    case QVariant::String:
        result = data.toString().toUtf8();
        break;
    case QVariant::Pixmap:
        // ######
        break;
    case QVariant::Color:
        // ######
    case QVariant::Url:
        // ######
    default:
        break;
    }
    return result;
}

void QMimeData::clear()
{
    Q_D(QMimeData);
    d->data.clear();
}
