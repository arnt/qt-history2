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

#ifndef QMIME_H
#define QMIME_H

#include "qobject.h"

class QUrl;
class QString;
class QByteArray;
class QColor;
class QPixmap;
class QByteArray;

class QMimeDataPrivate;
class Q_GUI_EXPORT QMimeData : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMimeData)
public:
    QMimeData(QObject *parent);
    ~QMimeData();

    QList<QUrl> urls() const;
    void setUrls(const QList<QUrl> &urls);

    QString text() const;
    void setText(const QString &text);

    QString html() const;
    void setHtml(const QString &html);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    QColor color() const;
    void setColor(const QColor &color);

    virtual QByteArray data(const QString &mimetype) const;
    void setData(const QString &mimetype, const QByteArray &data);

    virtual bool hasFormat(const QString &mimetype) const;
    virtual QStringList formats() const;

    void clear();
};

class Q_GUI_EXPORT QMimeSource
{
public:
    virtual ~QMimeSource();
    virtual const char* format(int n = 0) const = 0;
    virtual bool provides(const char*) const;
    virtual QByteArray encodedData(const char*) const = 0;
};


#if defined(Q_WS_WIN)

/*
  Encapsulation of conversion between MIME and Windows CLIPFORMAT.
  Not need on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QWindowsMime {
public:
    QWindowsMime();
    virtual ~QWindowsMime();

    static void initialize();

    static QList<QWindowsMime*> all();
    static QWindowsMime* convertor(const QString &mime, int cf);
    static QString cfToMime(int cf);

    static int registerMimeType(const QString &mime);

    virtual QString convertorName()=0;
    virtual int countCf()=0;
    virtual int cf(int index)=0;
    virtual bool canConvert(const QString &mime, int cf)=0;
    virtual QString mimeFor(int cf)=0;
    virtual int cfFor(const QString &mime)=0;
    virtual QByteArray convertToMime(const QByteArray &data, const QString &mime, int cf)=0;
    virtual QByteArray convertFromMime(const QByteArray &data, const QString &mime, int cf)=0;
};

#endif
#if defined(Q_WS_MAC)

/*
  Encapsulation of conversion between MIME and Mac flavor.
  Not need on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QMacMime {
    char type;
public:
    enum QMacMimeType { MIME_DND=0x01, MIME_CLIP=0x02, MIME_QT_CONVERTOR=0x04, MIME_ALL=MIME_DND|MIME_CLIP };
    QMacMime(char);
    virtual ~QMacMime();

    static void initialize();

    static QList<QMacMime*> all(QMacMimeType);
    static QMacMime* convertor(QMacMimeType, const char* mime, int flav);
    static const char* flavorToMime(QMacMimeType, int flav);

    virtual const char* convertorName()=0;
    virtual int countFlavors()=0;
    virtual int flavor(int index)=0;
    virtual bool canConvert(const char* mime, int flav)=0;
    virtual const char* mimeFor(int flav)=0;
    virtual int flavorFor(const char*)=0;
    virtual QByteArray convertToMime(QList<QByteArray> data, const char* mime, int flav)=0;
    virtual QList<QByteArray> convertFromMime(QByteArray data, const char* mime, int flav)=0;
};

#endif // Q_WS_MAC

#endif // QMIME_H
