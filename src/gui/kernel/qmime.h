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
#include "qvariant.h"

#if defined(Q_WS_WIN)
    #include <objidl.h>
#endif

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
    QMimeData();
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

    QByteArray data(const QString &mimetype) const;
    void setData(const QString &mimetype, const QByteArray &data);

    virtual bool hasFormat(const QString &mimetype) const;
    virtual QStringList formats() const;

    void clear();
protected:
    virtual QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const;
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


    // intrenal for converting to Qt
    static QWindowsMime *converterToMime(const QString &mimeType, struct IDataObject *pDataObj);
    static QStringList allMimesForFormats(struct IDataObject *pDataObj);
    
    // intrenal for converting from Qt
    static QWindowsMime *converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData);
    static QVector<FORMATETC> allFormatsForMime(const QMimeData *mimeData);
    

    // for converting from Qt
    virtual bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const = 0;
    virtual bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const = 0;
    virtual QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const = 0;
    

    // for converting to Qt
    virtual bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const = 0;
    virtual QVariant convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const = 0;
    virtual QString mimeForFormat(const FORMATETC &formatetc) const = 0;
    
protected:
    // helpers for using global memory
    inline int getCf(const FORMATETC &formatetc) const { return formatetc.cfFormat; } 
    inline FORMATETC setCf(int cf) const 
    { 
      FORMATETC formatic;
      formatic.cfFormat = cf;
      formatic.dwAspect = DVASPECT_CONTENT;
      formatic.lindex = -1;
      formatic.ptd =  NULL;
      formatic.tymed = TYMED_HGLOBAL;
      return formatic; 
    }
    bool setData(const QByteArray &data, STGMEDIUM * pmedium) const;
    QByteArray getData(int cf, struct IDataObject * pDataObj) const;
    bool canGetData(int cf, struct IDataObject * pDataObj) const;

public:


    static void initialize();
    static int registerMimeType(const QString &mime);


    //old

    static QList<QWindowsMime*> all();
    static QWindowsMime* convertor(const QString &mime, int cf);
    
    static QString cfToMime(int) { return QString();}

    
    virtual QString convertorName() { return QString(); };
    virtual int countCf() { return 0; };
    virtual int cf(int index) { return 0; };
    virtual bool canConvert(const QString &mime, int cf) { return false; };
    
    virtual int cfFor(const QString &mime) { return 0; };
    virtual QByteArray convertToMime(const QByteArray &data, const QString &mime, int cf) { return QByteArray(); };
    virtual QByteArray convertFromMime(const QByteArray &data, const QString &mime, int cf) { return QByteArray();};
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
    static QMacMime *convertor(QMacMimeType, const QString &mime, int flav);
    static QString flavorToMime(QMacMimeType, int flav);

    virtual QString convertorName()=0;
    virtual int countFlavors()=0;
    virtual int flavor(int index)=0;
    virtual bool canConvert(const QString &mime, int flav)=0;
    virtual QString mimeFor(int flav)=0;
    virtual int flavorFor(const QString &mime)=0;
    virtual QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav)=0;
    virtual QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav)=0;
};

#endif // Q_WS_MAC

#endif // QMIME_H
