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

#ifndef QT_NO_MIME

#include "qimageio.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qalgorithms.h"
#include "qmap.h"
#include "qdnd_p.h"
#include <shlobj.h>
#include "qurl.h"
#include "qregexp.h"
#include "qtextdocument.h"

#ifndef QT_NO_IMAGEIO_BMP
extern bool qt_read_dib(QDataStream&, QImage&); // qimage.cpp
extern bool qt_write_dib(QDataStream&, QImage);   // qimage.cpp
#endif


//#define QMIME_DEBUG

static QList<QWindowsMime*> mimes;

/*!
\class QWindowsMime qmime.h
\brief The QWindowsMime class maps open-standard MIME to Window Clipboard formats.
\ingroup io
\ingroup draganddrop
\ingroup misc

  Qt's drag-and-drop and clipboard facilities use the MIME standard.
  On X11, this maps trivially to the Xdnd protocol, but on Windows
  although some applications use MIME types to describe clipboard
  formats, others use arbitrary non-standardized naming conventions,
  or unnamed built-in formats of Windows.
  
    By instantiating subclasses of QWindowsMime that provide conversions
    between Windows Clipboard and MIME formats, you can convert
    proprietary clipboard formats to MIME formats.
    
      Qt has predefined support for the following Windows Clipboard formats:
      \list
      \i CF_UNICODETEXT - converted to "text/plain;charset=ISO-10646-UCS-2"
      and supported by QTextDrag.
      \i CF_TEXT - converted to "text/plain;charset=system" or "text/plain"
      and supported by QTextDrag.
      \i CF_DIB - converted to "image/*", where * is
      a \link QImage::outputFormats() Qt image format\endlink,
      and supported by QImageDrag.
      \i CF_HDROP - converted to "text/uri-list",
      and supported by QUriDrag.
      \endlist
      
        An example use of this class would be to map the Windows Metafile
        clipboard format (CF_METAFILEPICT) to and from the MIME type "image/x-wmf".
        This conversion might simply be adding or removing a header, or even
        just passing on the data.  See the
        \link dnd.html Drag-and-Drop documentation\endlink for more information
        on choosing and definition MIME types.
        
          You can check if a MIME type is convertible using canConvert() and
          can perform conversions with convertToMime() and convertFromMime().
*/

/*!
Constructs a new conversion object, adding it to the globally accessed
list of available convertors.
*/
QWindowsMime::QWindowsMime()
{
    mimes.append(this);
}

/*!
Destroys a conversion object, removing it from the global
list of available convertors.
*/
QWindowsMime::~QWindowsMime()
{
    mimes.removeAll(this);
}


/*!
*/
int QWindowsMime::registerMimeType(const QString &mime)
{
#ifdef Q_OS_TEMP
    int f = RegisterClipboardFormat(mime.utf16());
#else
    int f = RegisterClipboardFormatA(mime.local8Bit());
#endif
    if (!f)
        qErrnoWarning("QWindowsMime::registerMimeType: Failed to register clipboard format");
    
    return f;
}


/*!
\fn int QWindowsMime::cf(int index)

  Returns the Windows Clipboard format supported by this convertor
  that is ordinarily at position \a index. This means that cf(0)
  returns the first Windows Clipboard format supported, and
  cf(countCf()-1) returns the last. If \a index is out of range the
  return value is undefined.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn bool QWindowsMime::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
    
  Returns true if the convertor can convert from the \a mimeData to
  the format specified in \a formatetc.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn bool QWindowsMime::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const

  Returns true if the convertor can convert to the \a mimeType from
  the available formats in \a pDataObject.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QString QWindowsMime::mimeForFormat(const FORMATETC &formatetc) const

  Returns the mime type that can be used for format specified in \a formatetc, or
  an empty string if this convertor does not support \a formatetc.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QVector<FORMATETC> QWindowsMime::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const

  Returns a QVector of FORMATETC structures reprasenting the diffent windows clipbaord
  formats that can be provided for the \a mimeType from the \a mimeData.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QVariant QWindowsMime::convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const

  Returns a Qvariant containing the converted data for \a mime from \a pDataObject.
  If possible the Qvarient should be of the \a prefferedType to avoid needless conversions.
  
  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn bool QWindowsMime::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const

  Convert the \a mimeData to the format specified in \a formatetc. 
  The converted data shold then be placed in STGMEDIUM sturcture \a pmedium.  
  
  Retrun true if the conversion was successful.
    
  All subclasses must reimplement this pure virtual function.
*/


QWindowsMime *QWindowsMime::converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData)
{
    for (int i=mimes.size()-1; i>=0; --i) {
        if (mimes.at(i)->canConvertFromMime(formatetc, mimeData))
            return mimes.at(i);
    }
    return 0;
}

QWindowsMime *QWindowsMime::converterToMime(const QString &mimeType, struct IDataObject *pDataObj)
{
    for (int i=mimes.size()-1; i>=0; --i) {
        if (mimes.at(i)->canConverToMime(mimeType, pDataObj))
            return mimes.at(i);
    }
    return 0;
}

QVector<FORMATETC> QWindowsMime::allFormatsForMime(const QMimeData *mimeData)
{
    QVector<FORMATETC> formatics;
    formatics.reserve(20);
    QStringList formats = mimeData->formats();
    for (int f=0; f<formats.size(); ++f) {
        for (int i=mimes.size()-1; i>=0; --i)
            formatics += mimes.at(i)->formatsForMime(formats.at(f), mimeData);
    }
    return formatics;
}

QStringList QWindowsMime::allMimesForFormats(struct IDataObject *pDataObj)
{
    QStringList formats;
    LPENUMFORMATETC FAR fmtenum;
    HRESULT hr = pDataObj->EnumFormatEtc(DATADIR_GET, &fmtenum);
    
    if (hr == NOERROR) {
        FORMATETC fmtetc;
        while (S_OK == fmtenum->Next(1, &fmtetc, 0)) {
#ifdef QMIME_DEBUG
            qDebug("QWindowsMime::allMimesForFormats()");
            char buf[256] = {0};
            GetClipboardFormatNameA(fmtetc.cfFormat, buf, 255);
            qDebug("CF = %d : %s", fmtetc.cfFormat, buf);
#endif
            for (int i=mimes.size()-1; i>=0; --i) {
                QString format = mimes.at(i)->mimeForFormat(fmtetc);
                if (!format.isEmpty() && !formats.contains(format)) {
                    formats += format;
                }
            }
            // as documented in MSDN to avoid possible memleak
            if (fmtetc.ptd)
                CoTaskMemFree(fmtetc.ptd);
        }
        fmtenum->Release();
    }
    
    return formats;
}


bool QWindowsMime::setData(const QByteArray &data, STGMEDIUM * pmedium) const
{
    HGLOBAL hData = GlobalAlloc(0, data.size());
    if (!hData) {
        return false;
    }
    void* out = GlobalLock(hData);
    memcpy(out, data.data(), data.size());
    GlobalUnlock(hData);
    pmedium->tymed = TYMED_HGLOBAL;
    pmedium->hGlobal = hData;
    return true;
}

QByteArray QWindowsMime::getData(int cf, struct IDataObject *pDataObj) const
{
    QByteArray data;
    FORMATETC formatetc;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.tymed = TYMED_HGLOBAL;
    formatetc.cfFormat = cf;
    formatetc.ptd = 0;
    formatetc.lindex = -1;
    STGMEDIUM s;
    if (pDataObj->GetData(&formatetc, &s) == S_OK) {
        DWORD * val = (DWORD*)GlobalLock(s.hGlobal);
        data = QByteArray::fromRawData((char*)GlobalLock(val), GlobalSize(val));
        data.detach();
        GlobalUnlock(s.hGlobal);
        ReleaseStgMedium(&s);
    }
    return data;
}

bool QWindowsMime::canGetData(int cf, struct IDataObject * pDataObj) const
{
    FORMATETC formatetc;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.tymed = TYMED_HGLOBAL;
    formatetc.cfFormat = cf;
    formatetc.ptd = 0;
    formatetc.lindex = -1;
    return pDataObj->QueryGetData(&formatetc) == S_OK;
}



/*
int QWindowsMimeText::cfFor(const QString &mime)
{
if (mime == QLatin1String("text/plain"))
return CF_TEXT;
QString m(mime);
int i = m.indexOf("charset=");
if (i >= 0) {
QString cs = m.mid(i+8);
i = cs.indexOf(";");
if (i>=0)
cs = cs.left(i);
if (cs == "system")
return CF_TEXT;
if (cs == "ISO-10646-UCS-2" || cs == "utf16")
return CF_UNICODETEXT;
}
return 0;
}
*/



class QWindowsMimeText : public QWindowsMime
{
public:
    QWindowsMimeText();
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, LPDATAOBJECT pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
private:
    QMap<int, QString> mimeMapping;
};


QWindowsMimeText::QWindowsMimeText()
{
    QT_WA({
        mimeMapping.insert(CF_UNICODETEXT, "text/plain;charset=ISO-10646-UCS-2");
    } , {
        mimeMapping.insert(CF_UNICODETEXT, "text/plain;charset=utf16");
    });
    mimeMapping.insert(CF_TEXT, "text/plain");
}

bool QWindowsMimeText::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    int cf = getCf(formatetc);
    return (cf == CF_TEXT || cf == CF_UNICODETEXT) && !mimeData->text().isEmpty();
}

/*
text/plain is defined as using CRLF, but so many programs don't,
and programmers just look for '\n' in strings.
Windows really needs CRLF, so we ensure it here.
*/
bool QWindowsMimeText::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
    if (canConvertFromMime(formatetc, mimeData)) {
        QByteArray data;
        int cf = getCf(formatetc);
        if (cf == CF_TEXT) {
            data = mimeData->text().toLocal8Bit();
            // Anticipate required space for CRLFs at 1/40
            int maxsize=data.size()+data.size()/40+3;
            QByteArray r(maxsize, '\0');
            char* o = r.data();
            const char* d = data.data();
            const int s = data.size();
            bool cr=false;
            int j=0;
            for (int i=0; i<s; i++) {
                char c = d[i];
                if (c=='\r')
                    cr=true;
                else {
                    if (c=='\n') {
                        if (!cr)
                            o[j++]='\r';
                    }
                    cr=false;
                }
                o[j++]=c;
                if (j+3 >= maxsize) {
                    maxsize += maxsize/4;
                    r.resize(maxsize);
                    o = r.data();
                }
            }
            o[j]=0;
            return setData(r, pmedium);
        } else if (cf == CF_UNICODETEXT) {
            QString str = mimeData->text();
            const QChar *u = str.unicode();
            QString res;
            const int s = str.length();
            int maxsize = s + s/40 + 3;
            res.resize(maxsize);
            int ri = 0;
            bool cr = false;
            for (int i=0; i < s; ++i) {
                if (*u == '\r')
                    cr = true;
                else {
                    if (*u == '\n' && !cr)
                        res[ri++] = QChar('\r');
                    cr = false;
                }
                res[ri++] = *u;
                if (ri+3 >= maxsize) {
                    maxsize += maxsize/4;
                    res.resize(maxsize);
                }
                ++u;
            }
            res.truncate(ri);
            const int byteLength = res.length()*2;
            QByteArray r(byteLength + 2, '\0');
            memcpy(r.data(), res.unicode(), byteLength);
            r[byteLength] = 0;
            r[byteLength+1] = 0;
            return setData(r, pmedium);
        }
        /* ///#### what is this ???
        if (data.size() < 2)
        return QByteArray();
        
          // Windows uses un-marked little-endian nul-terminated Unicode
          if ((uchar)data[0] == uchar(0xff) && (uchar)data[1] == uchar(0xfe))
          {
          // Right way - but skip header and add nul
          QByteArray r(data.size(), '\0');
          memcpy(r.data(),data.constData()+2,data.size()-2);
          r[(int)data.size()-2] = 0;
          r[(int)data.size()-1] = 0;
          return r;
          } else {
          // Wrong way - reorder.
          int s = data.size();
          if (s&1) {
          // Odd byte - drop last
          s--;
          }
          const char* i = data.data();
          if ((uchar)i[0] == uchar(0xfe) && (uchar)i[1] == uchar(0xff)) {
          i += 2;
          s -= 2;
          }
          QByteArray r(s+2, '\0');
          char* o = r.data();
          while (s) {
          o[0] = i[1];
          o[1] = i[0];
          i += 2;
          o += 2;
          s -= 2;
          }
          r[(int)r.size()-2] = 0;
          r[(int)r.size()-1] = 0;
          return r;
          }
        */
    }
    return false;
}

bool QWindowsMimeText::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    //### be smarter
    QList<int> possibles = mimeMapping.keys(mimeType);
    for (int i=0; i<possibles.size(); i++) {
        if (canGetData(possibles.at(i), pDataObj))
            return true;
    }
    return false;
}

QString QWindowsMimeText::mimeForFormat(const FORMATETC &formatetc) const
{
    return mimeMapping.value(getCf(formatetc));
}


QVector<FORMATETC> QWindowsMimeText::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatics;
    QList<int> cfs = mimeMapping.keys(mimeType);
    for (int i=0; i<cfs.size(); i++) {
        if (canConvertFromMime(setCf(cfs.at(i)), mimeData))
            formatics += setCf(cfs.at(i));
    }
    return formatics;
}

QVariant QWindowsMimeText::convertToMime(const QString &mime, QVariant::Type preferredType, LPDATAOBJECT pDataObj) const
{
    QVariant ret;
    
    if (canConverToMime(mime, pDataObj)) {
        if (mime == "text/plain") {
            QByteArray data = getData(CF_TEXT, pDataObj);
            const char* d = data.data();
            const int s = qstrlen(d);
            QByteArray r(data.size()+1, '\0');
            char* o = r.data();
            int j=0;
            for (int i=0; i<s; i++) {
                char c = d[i];
                if (c!='\r')
                    o[j++]=c;
            }
            o[j]=0;
            if (preferredType == QVariant::String)
                ret = QString::fromLocal8Bit(r);
            else
                ret = r;
        } else if (mime == "text/plain;charset=utf16" || mime == "text/plain;charset=ISO-10646-UCS-2") {
            QByteArray data = getData(CF_UNICODETEXT, pDataObj);
            // Windows uses un-marked little-endian nul-terminated Unicode
            int ms = data.size();
            int s;
            // Find NUL
            for (s=0; s<ms-1 && (data[s+0] || data[s+1]); s+=2);
            
            QByteArray r(s+2, '\0');
            r[0]=uchar(0xff); // BOM
            r[1]=uchar(0xfe);
            memcpy(r.data()+2,data.constData(),s);
            if (preferredType == QVariant::String)
                ret = QString::fromUtf16((const unsigned short *)r.data());
            else
                ret = r;
        }
    }
    
    return ret;
}

class QWindowsMimeURI : public QWindowsMime
{
public:
    QWindowsMimeURI();
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, LPDATAOBJECT pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
private:
    int CF_INETURL;
};

QWindowsMimeURI::QWindowsMimeURI()
{
    QT_WA({
        CF_INETURL = QWindowsMime::registerMimeType("UniformResourceLocatorW");
    } , {
        CF_INETURL = QWindowsMime::registerMimeType("UniformResourceLocator");
    });
}

bool QWindowsMimeURI::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    if (getCf(formatetc) == CF_HDROP) {
        QList<QUrl> urls = mimeData->urls();
        for (int i=0; i<urls.size(); i++) {
            if (!urls.at(i).toLocalFile().isEmpty())
                return true;
        }
    }
    return getCf(formatetc) == CF_INETURL && mimeData->hasFormat("text/uri-list");
}

bool QWindowsMimeURI::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
    if (canConvertFromMime(formatetc, mimeData)) {
        if (getCf(formatetc) == CF_HDROP) {
            QList<QUrl> urls = mimeData->urls();
            int size = sizeof(DROPFILES)+2;
            for (int i=0; i<urls.size(); i++) {
                QString fn = urls.at(i).toLocalFile();
                if (!fn.isEmpty()) {
                    QT_WA({
                        size += sizeof(TCHAR)*(fn.length()+1);
                    } , {
                        size += fn.toLocal8Bit().length()+1;
                    });
                }
            }
        
            QByteArray result(size, '\0');
            DROPFILES* d = (DROPFILES*)result.data();
            d->pFiles = sizeof(DROPFILES);
            GetCursorPos(&d->pt); // try
            d->fNC = true;
            char* files = ((char*)d) + d->pFiles;
        
            QT_WA({
                d->fWide = true;
                TCHAR* f = (TCHAR*)files;
                for (int i=0; i<urls.size(); i++) {
                    QString fn = urls.at(i).toLocalFile();
                    if (!fn.isEmpty()) {
                        int l = fn.length();
                        memcpy(f, fn.utf16(), l*sizeof(TCHAR));
                        f += l;
                        *f++ = 0;
                    }
                }
                *f = 0;
            } , {
                d->fWide = false;
                char* f = files;
                for (int i=0; i<urls.size(); i++) {
                    QByteArray c = urls.at(i).toLocalFile().toLocal8Bit();
                    if (!c.isEmpty()) {
                        int l = c.length();
                        memcpy(f, c.constData(), l);
                        f += l;
                        *f++ = 0;
                    }
                }
                *f = 0;
            });
            return setData(result, pmedium);
        } else if (getCf(formatetc) == CF_INETURL) {
            QList<QUrl> urls = mimeData->urls();
            QByteArray result;
            QT_WA({
                QString url = urls.at(0).toString(); 
                result = QByteArray((const char *)url.utf16(), url.length() * 2);
                result.append('\0');
                result.append('\0');
            } , {
                result = urls.at(0).toString().toLocal8Bit();
            });
            return setData(result, pmedium);
        }
    }
    
    return false;
}

bool QWindowsMimeURI::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    if (mimeType == "text/uri-list") {
        if (canGetData(CF_HDROP, pDataObj))
            return true;
        else if (canGetData(CF_INETURL, pDataObj))
            return true;
    }
    return false;
}

QString QWindowsMimeURI::mimeForFormat(const FORMATETC &formatetc) const
{
    QString format;
    if (getCf(formatetc) == CF_HDROP || getCf(formatetc) == CF_INETURL)
        format = "text/uri-list";
    return format;
}

QVector<FORMATETC> QWindowsMimeURI::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatics;
    if (canConvertFromMime(setCf(CF_HDROP), mimeData))
        formatics += setCf(CF_HDROP);
    if (canConvertFromMime(setCf(CF_INETURL), mimeData))
        formatics += setCf(CF_INETURL);
    return formatics;
}
    
QVariant QWindowsMimeURI::convertToMime(const QString &mime, QVariant::Type preferredType, LPDATAOBJECT pDataObj) const
{
    if (canGetData(CF_HDROP, pDataObj)) {
        QByteArray texturi;
        QList<QCoreVariant> urls;
        
        QByteArray data = getData(CF_HDROP, pDataObj);
        if (data.isEmpty())
            return QVariant();
        
        LPDROPFILES hdrop = (LPDROPFILES)data.data();
        if (hdrop->fWide) {
            const ushort* filesw = (const ushort*)(data.data() + hdrop->pFiles);
            int i=0;
            while (filesw[i]) {
                QString fileurl = QString::fromUtf16(filesw+i);
                urls += QUrl::fromLocalFile(fileurl);
                i += fileurl.length()+1;
            }
        } else {
            const char* files = (const char*)data.data() + hdrop->pFiles;
            int i=0;
            while (files[i]) {
                urls += QUrl::fromLocalFile(QString::fromLocal8Bit(files+i));
                i += strlen(files+i)+1;
            }
        }
        
        if (preferredType == QVariant::Url && urls.size() == 1)
            return urls.at(0);
        else if (!urls.isEmpty())
            return urls;
    } else if (canGetData(CF_INETURL, pDataObj)) {
        QByteArray data = getData(CF_INETURL, pDataObj);
        if (data.isEmpty())
            return QVariant();
        QT_WA({
            return QUrl(QString::fromUtf16((const unsigned short *)data.constData()));
        } , {
            return QUrl(QString::fromLocal8Bit(data.constData()));
        });
    }
    return QVariant();
}

class QWindowsMimeHtml : public QWindowsMime
{
public:
    QWindowsMimeHtml();
    
    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
    
    // for converting to Qt
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    
private:
    int CF_HTML;
};

QWindowsMimeHtml::QWindowsMimeHtml()
{
    CF_HTML = QWindowsMime::registerMimeType("HTML Format");
}

QVector<FORMATETC> QWindowsMimeHtml::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (mimeType == "text/html" && (!mimeData->html().isEmpty()))
        formatetcs += setCf(CF_HTML);
    return formatetcs;
}

QString QWindowsMimeHtml::mimeForFormat(const FORMATETC &formatetc) const
{
    if (getCf(formatetc) == CF_HTML)
        return "text/html";
    return QString();
}

bool QWindowsMimeHtml::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    return mimeType == "text/html" && canGetData(CF_HTML, pDataObj);
}


bool QWindowsMimeHtml::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    return getCf(formatetc) == CF_HTML && (!mimeData->html().isEmpty());
}

/*
The windows HTML clipboard format is as follows (xxxxxxxxxx is a 10 integer number giving the positions
in bytes). Charset used is mostly utf8, but can be different, ie. we have to look for the <meta> charset tag

  Version: 1.0
  StartHTML:xxxxxxxxxx
  EndHTML:xxxxxxxxxx
  StartFragment:xxxxxxxxxx
  EndFragment:xxxxxxxxxx
  ...html...
  
*/
QVariant QWindowsMimeHtml::convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const
{
    QVariant result;
    if (canConverToMime(mime, pDataObj)) {
        QString html = QString::fromUtf8(getData(CF_HTML, pDataObj));
#ifdef QMIME_DEBUG
        qDebug("QWindowsMimeHtml::convertToMime");
        qDebug("raw :");
        qDebug(html.latin1());
#endif
        //int ms = data.size();
        int start = html.indexOf("StartFragment:");
        int end = html.indexOf("EndFragment:");
        if(start != -1)
            start = html.mid(start+14, 10).toInt();
        if(end != -1)
            end = html.mid(end+12, 10).toInt();
        if (end > start && start > 0) {
            html = "<!--StartFragment-->" + html.mid(start, end - start);
            html += "<!--EndFragment-->";
            html.replace("\r", "");
            //result.replace("<o:p>", "");
            //result.replace("</o:p>", "");
            result = html;
        }
    }
    return result;
}

bool QWindowsMimeHtml::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    
    QByteArray data = mimeData->html().toUtf8();
    QByteArray result =
        "Version 1.0\r\n"                    // 0-12
        "StartHTML:0000000105\r\n"            // 13-35
        "EndHTML:0000000000\r\n"            // 36-55
        "StartFragment:0000000000\r\n"            // 58-86
        "EndFragment:0000000000\r\n\r\n";   // 87-105
    
    if (data.indexOf("<!--StartFragment-->") == -1)
        result += "<!--StartFragment-->";
    result += data;
    if (data.indexOf("<!--EndFragment-->") == -1)
        result += "<!--EndFragment-->";
    
    // set the correct number for EndHTML
    QByteArray pos = QString::number(result.size()).latin1();
    memcpy((char *)(result.data() + 53 - pos.length()), pos.constData(), pos.length());
    
    // set correct numbers for StartFragment and EndFragment
    pos = QString::number(result.indexOf("<!--StartFragment-->") + 20).latin1();
    memcpy((char *)(result.data() + 79 - pos.length()), pos.constData(), pos.length());
    pos = QString::number(result.indexOf("<!--EndFragment-->")).latin1();
    memcpy((char *)(result.data() + 103 - pos.length()), pos.constData(), pos.length());
    
    return setData(result, pmedium);
}



class QWindowsMimeImage : public QWindowsMime
{
public:
    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
    
    // for converting to Qt
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
};


QVector<FORMATETC> QWindowsMimeImage::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (!mimeData->pixmap().isNull() && mimeType.startsWith(QLatin1String("image/"))) {
        QList<QByteArray> ofmts = QImageIO::outputFormats();
        for (int i = 0; i < ofmts.count(); ++i) {
            if (qstricmp(ofmts.at(i),mimeType.latin1()+6)==0) {
                formatetcs += setCf(CF_DIB);
                break;
            }
        }
    }
    return formatetcs;
}

QString QWindowsMimeImage::mimeForFormat(const FORMATETC &formatetc) const
{
    if (getCf(formatetc) == CF_DIB)
        return "image/bmp";
    else
        return QString();
}

bool QWindowsMimeImage::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    bool haveDecoder = false;
    if (mimeType.startsWith(QLatin1String("image/"))) {
        QList<QByteArray> ifmts = QImageIO::inputFormats();
        for (int i = 0; i < ifmts.count(); ++i)
            if (qstricmp(ifmts.at(i),mimeType.latin1()+6)==0)
                haveDecoder = true;
    }
    
    return haveDecoder && canGetData(CF_DIB, pDataObj);
}

bool QWindowsMimeImage::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    return getCf(formatetc) == CF_DIB && !mimeData->pixmap().isNull();
}

bool QWindowsMimeImage::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    if (!canConvertFromMime(formatetc, mimeData))
        return false;
#ifndef QT_NO_IMAGEIO_BMP
    QImage img = mimeData->pixmap().toImage();
    if (img.isNull())
        return false;
    QByteArray ba;
    QBuffer iod(&ba);
    iod.open(QIODevice::WriteOnly);
    QDataStream s(&iod);
    s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
    if (qt_write_dib(s, img))
        return setData(ba, pmedium);
#endif
    return false;
}

QVariant QWindowsMimeImage::convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const
{
    QVariant result;
#ifndef QT_NO_IMAGEIO_BMP
    QImage img;
    QByteArray data = getData(CF_DIB, pDataObj);
    QBuffer iod(&data);
    iod.open(QIODevice::ReadOnly);
    QDataStream s(&iod);
    s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
    if (qt_read_dib(s, img)) { // ##### encaps "-14"
        result = QPixmap(img);
    }
#endif
    // Failed
    return result;
}


class QBuiltInMimes : public QWindowsMime
{
public:
    QBuiltInMimes();
    
    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
    
    // for converting to Qt
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    
private:
    QMap<int, QString> formats;
};

QBuiltInMimes::QBuiltInMimes()
: QWindowsMime()
{
    formats.insert(QWindowsMime::registerMimeType("text/uri-list"), "text/uri-list");
    formats.insert(QWindowsMime::registerMimeType("text/plain"), "text/plain");
    formats.insert(QWindowsMime::registerMimeType("text/html"), "text/html");
    formats.insert(QWindowsMime::registerMimeType("image/ppm"), "image/ppm");
    formats.insert(QWindowsMime::registerMimeType("application/x-color"), "application/x-color");
}

bool QBuiltInMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    // really check
    return formatetc.tymed & TYMED_HGLOBAL
        && formats.contains(formatetc.cfFormat)
        && mimeData->formats().contains(formats.value(formatetc.cfFormat));
}

bool QBuiltInMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    return canConvertFromMime(formatetc, mimeData)
        && setData(mimeData->data(formats.value(getCf(formatetc))), pmedium);
}

QVector<FORMATETC> QBuiltInMimes::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (!formats.keys(mimeType).isEmpty() && mimeData->formats().contains(mimeType))
        formatetcs += setCf(formats.key(mimeType));
    return formatetcs;
}

bool QBuiltInMimes::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    return (!formats.keys(mimeType).isEmpty())
        && canGetData(formats.key(mimeType), pDataObj);
}

QVariant QBuiltInMimes::convertToMime(const QString &mimeType, QVariant::Type preferredType, struct IDataObject *pDataObj) const
{
    QVariant val;
    if (canConverToMime(mimeType, pDataObj)) {
        QByteArray data = getData(formats.key(mimeType), pDataObj);
        if (!data.isEmpty()) {
#ifdef QMIME_DEBUG
            qDebug("QBuiltInMimes::convertToMime()");
#endif
            if (mimeType == "text/html" && preferredType == QVariant::String) {
                QTextCodec * codec = Qt::codecForHtml(data);
                // hack to get the right codec for mozillia
                if (codec) {
                    if (codec->name() == "ISO-8859-1" && data.size() > 1 && data.at(1) == 0)
                        val = QString::fromUtf16((const unsigned short*)data.data());
                    else
                        val = codec->toUnicode(data);
                } else {
                    val = data;
                }
            } else {
                val = data; // it should be enough to return the data and let QMimeData do the rest.
            }
        }
    }
    return val;
}

QString QBuiltInMimes::mimeForFormat(const FORMATETC &formatetc) const
{
    return formats.value(getCf(formatetc));
}


class QLastResortMimes : public QWindowsMime
{
public:
    
    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
    
    // for converting to Qt
    bool canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, QVariant::Type preferredType, struct IDataObject *pDataObj) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    
private:
    QMap<int, QString> formats;
};

bool QLastResortMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    // really check
    return formatetc.tymed & TYMED_HGLOBAL
        && formats.contains(formatetc.cfFormat)
        && mimeData->formats().contains(formats.value(formatetc.cfFormat));
}

bool QLastResortMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    return canConvertFromMime(formatetc, mimeData)
        && setData(mimeData->data(formats.value(getCf(formatetc))), pmedium);
}

QVector<FORMATETC> QLastResortMimes::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (!formats.keys(mimeType).isEmpty() && mimeData->formats().contains(mimeType)) {
        formatetcs += setCf(formats.key(mimeType));
    } else {
        // register any other available formats
        QStringList availableFormats = mimeData->formats();
        for (int i=0; i<availableFormats.size(); ++i) {
            int cf = QWindowsMime::registerMimeType(availableFormats.at(i));
            if (!formats.keys().contains(cf)) {
                QLastResortMimes *that = const_cast<QLastResortMimes *>(this);
                that->formats.insert(cf, availableFormats.at(i));
                formatetcs += setCf(cf);
            }
        }
    }
    return formatetcs;
}

bool QLastResortMimes::canConverToMime(const QString &mimeType, struct IDataObject *pDataObj) const
{
    if (formats.keys(mimeType).isEmpty()) {
        // if it is not in there then register it an see if we can get it
        int cf = QWindowsMime::registerMimeType(mimeType);
        return canGetData(cf, pDataObj);
    } else {
        return canGetData(formats.key(mimeType), pDataObj);
    }
    return false;
}

QVariant QLastResortMimes::convertToMime(const QString &mimeType, QVariant::Type preferredType, struct IDataObject *pDataObj) const
{
    QVariant val;
    if (canConverToMime(mimeType, pDataObj)) {
        QByteArray data;
        if (formats.keys(mimeType).isEmpty()) {
            int cf = QWindowsMime::registerMimeType(mimeType);
            data = getData(cf, pDataObj);
        } else {
            data = getData(formats.key(mimeType), pDataObj);
        }
        if (!data.isEmpty())
            val = data; // it should be enough to return the data and let QMimeData do the rest.
    }
    return val;
}

QString QLastResortMimes::mimeForFormat(const FORMATETC &formatetc) const
{
    return formats.value(getCf(formatetc));
}






static
void cleanup_mimes()
{
    while (mimes.size())
        delete mimes.first();
}

/*!
This is an internal function.
*/
void QWindowsMime::initialize()
{
    if (mimes.isEmpty()) {
        new QLastResortMimes;
        new QWindowsMimeText;
        new QWindowsMimeURI;
        new QWindowsMimeImage;
        new QWindowsMimeHtml;
        new QBuiltInMimes;
        qAddPostRoutine(cleanup_mimes);
    }
}

#endif // QT_NO_MIME
