/****************************************************************************
**
** Definition of mime classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMIME_H
#define QMIME_H

#ifndef QT_H
#include "qwindowdefs.h"
#endif // QT_H

#ifndef QT_NO_MIME

class QImageDrag;
class QTextDrag;
template<class Key, class T> class QMap;
template<class T> class QList;

class Q_GUI_EXPORT QMimeSource
{
    friend class QClipboardData;

public:
    QMimeSource();
    virtual ~QMimeSource();
    virtual const char* format(int n = 0) const = 0;
    virtual bool provides(const char*) const;
    virtual QByteArray encodedData(const char*) const = 0;
    int serialNumber() const;

private:
    int ser_no;
    enum { NoCache, Text, Graphics } cacheType;
    union
    {
        struct
        {
            QString *str;
            QString *subtype;
        } txt;
        struct
        {
            QImage *img;
            QPixmap *pix;
        } gfx;
    } cache;
    void clearCache();

    // friends for caching
    friend class QImageDrag;
    friend class QTextDrag;

};

inline int QMimeSource::serialNumber() const
{ return ser_no; }

class QStringList;
class QMimeSourceFactoryData;

class Q_GUI_EXPORT QMimeSourceFactory {
public:
    QMimeSourceFactory();
    virtual ~QMimeSourceFactory();

    static QMimeSourceFactory* defaultFactory();
    static void setDefaultFactory(QMimeSourceFactory*);
    static QMimeSourceFactory* takeDefaultFactory();
    static void addFactory(QMimeSourceFactory *f);
    static void removeFactory(QMimeSourceFactory *f);

    virtual const QMimeSource* data(const QString& abs_name) const;
    virtual QString makeAbsolute(const QString& abs_or_rel_name, const QString& context) const;
    const QMimeSource* data(const QString& abs_or_rel_name, const QString& context) const;

    virtual void setText(const QString& abs_name, const QString& text);
    virtual void setImage(const QString& abs_name, const QImage& im);
    virtual void setPixmap(const QString& abs_name, const QPixmap& pm);
    virtual void setData(const QString& abs_name, QMimeSource* data);
    virtual void setFilePath(const QStringList&);
    virtual QStringList filePath() const;
    void addFilePath(const QString&);
    virtual void setExtensionType(const QString& ext, const char* mimetype);

private:
    QMimeSource *dataInternal(const QString& abs_name, const QMap<QString, QString> &extensions) const;
    QMimeSourceFactoryData* d;
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
    static QWindowsMime* convertor(const char* mime, int cf);
    static const char* cfToMime(int cf);

    static int registerMimeType(const char *mime);

    virtual const char* convertorName()=0;
    virtual int countCf()=0;
    virtual int cf(int index)=0;
    virtual bool canConvert(const char* mime, int cf)=0;
    virtual const char* mimeFor(int cf)=0;
    virtual int cfFor(const char*)=0;
    virtual QByteArray convertToMime(QByteArray data, const char* mime, int cf)=0;
    virtual QByteArray convertFromMime(QByteArray data, const char* mime, int cf)=0;
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

#endif // QT_NO_MIME

#endif // QMIME_H
