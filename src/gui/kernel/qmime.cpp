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
    QString text;
    QString html;
    QPixmap pixmap;
    QList<QUrl> urls;

    QMap<QString, QByteArray> data;
};

QMimeData::QMimeData(QObject *parent)
    : QObject(*new QMimeDataPrivate, parent)
{
}

QMimeData::~QMimeData()
{
}

QList<QUrl> QMimeData::urls() const
{
    Q_D(const QMimeData);
    if (!d->urls.isEmpty())
        return d->urls;
    // ########
    return QList<QUrl>();
}

void QMimeData::setUrls(const QList<QUrl> &urls)
{
    Q_D(QMimeData);
    d->urls = urls;
}

QString QMimeData::text() const
{
    Q_D(const QMimeData);
    if (!d->text.isNull())
        return d->text;
    // ############
    return QString();
}

void QMimeData::setText(const QString &text)
{
    Q_D(QMimeData);
    d->text = text;
}

QString QMimeData::html() const
{
    Q_D(const QMimeData);
    if (!d->html.isNull())
        return d->html;
    // ############
    return QString();
}

void QMimeData::setHtml(const QString &html)
{
    Q_D(QMimeData);
    d->html = html;
}

QPixmap QMimeData::pixmap() const
{
    Q_D(const QMimeData);
    if (!d->pixmap.isNull())
        return d->pixmap;
    // ### try to decode
    return QPixmap();
}

void QMimeData::setPixmap(const QPixmap &pixmap)
{
    Q_D(QMimeData);
    d->pixmap = pixmap;
}

QByteArray QMimeData::data(const QString &mimetype) const
{
    Q_D(const QMimeData);
    if (d->data.contains(mimetype))
        return d->data.value(mimetype);

    // #### match with known mimetypes and convert if needed
    return QByteArray();
}

void QMimeData::setData(const QString &mimetype, const QByteArray &data)
{
    Q_D(QMimeData);
    d->data[mimetype] = data;
}

bool QMimeData::hasFormat(const QString &mimetype) const
{
    Q_D(const QMimeData);
    if (d->data.contains(mimetype))
        return true;

    return formats().contains(mimetype);
}

QStringList QMimeData::formats() const
{
    Q_D(const QMimeData);
    QStringList formats = d->data.keys();

    if (!d->text.isEmpty())
        formats += QLatin1String("text/plain");
    if (!d->html.isEmpty())
        formats += QLatin1String("text/html");
    if (!d->urls.isEmpty())
        formats += QLatin1String("text/uri-list");
    // ##### add keys for pixmaps
    return formats;
}

void QMimeData::clear()
{
    Q_D(QMimeData);
    d->text.clear();
    d->html.clear();
    d->pixmap = QPixmap();
    d->urls.clear();
    d->data.clear();
}
