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

#include "qimagereader.h"
#include "qimagewriter.h"
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
#include "qvariant.h"
#include "qregexp.h"
#include "qtextdocument.h"
#include "qdir.h"

#ifndef QT_NO_IMAGEIO_BMP
extern bool qt_read_dib(QDataStream&, QImage&); // qimage.cpp
extern bool qt_write_dib(QDataStream&, QImage);   // qimage.cpp
#endif


//#define QMIME_DEBUG


// helpers for using global memory

static int getCf(const FORMATETC &formatetc)
{
    return formatetc.cfFormat;
}

static FORMATETC setCf(int cf)
{
    FORMATETC formatetc;
    formatetc.cfFormat = cf;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.lindex = -1;
    formatetc.ptd = NULL;
    formatetc.tymed = TYMED_HGLOBAL;
    return formatetc;
}

static bool setData(const QByteArray &data, STGMEDIUM *pmedium)
{
    HGLOBAL hData = GlobalAlloc(0, data.size());
    if (!hData)
        return false;

    void *out = GlobalLock(hData);
    memcpy(out, data.data(), data.size());
    GlobalUnlock(hData);
    pmedium->tymed = TYMED_HGLOBAL;
    pmedium->hGlobal = hData;
    return true;
}

static QByteArray getData(int cf, IDataObject *pDataObj)
{
    QByteArray data;
    FORMATETC formatetc = setCf(cf);
    STGMEDIUM s;
    if (pDataObj->GetData(&formatetc, &s) == S_OK) {
        DWORD * val = (DWORD*)GlobalLock(s.hGlobal);
        data = QByteArray::fromRawData((char*)val, GlobalSize(val));
        data.detach();
        GlobalUnlock(s.hGlobal);
        ReleaseStgMedium(&s);
    }
    return data;
}

static bool canGetData(int cf, IDataObject * pDataObj)
{
    FORMATETC formatetc = setCf(cf);
    return pDataObj->QueryGetData(&formatetc) == S_OK;
}

class QWindowsMimeList
{
public:
    QWindowsMimeList();
    ~QWindowsMimeList();
    void addWindowsMime(QWindowsMime * mime);
    void removeWindowsMime(QWindowsMime * mime);
    QList<QWindowsMime*> windowsMimes();

private:
    void init();
    bool initialized;
    QList<QWindowsMime*> mimes;
};

Q_GLOBAL_STATIC(QWindowsMimeList, mimeList);


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
  \i CF_UNICODETEXT - converted to "text/plain"
  \i CF_TEXT - converted to "text/plain"
  \i CF_DIB - converted to "image/...", where ... is
     a \link QImage::outputFormats() Qt image format\endlink
  \i CF_HDROP - converted to "text/uri-list"
  \i CF_INETURL - converted to "text/uri-list"
  \i CF_HTML - converted to "text/html"
  \endlist

  An example use of this class would be to map the Windows Metafile
  clipboard format (CF_METAFILEPICT) to and from the MIME type "image/x-wmf".
  This conversion might simply be adding or removing a header, or even
  just passing on the data.  See the
  \link dnd.html Drag-and-Drop documentation\endlink for more information
  on choosing and definition MIME types.

  You can check if a MIME type is convertible using canConvertFromMime() and
  can perform conversions with convertToMime() and convertFromMime().
*/

/*!
Constructs a new conversion object, adding it to the globally accessed
list of available converters.
*/
QWindowsMime::QWindowsMime()
{
    ::mimeList()->addWindowsMime(this);
}

/*!
Destroys a conversion object, removing it from the global
list of available converters.
*/
QWindowsMime::~QWindowsMime()
{
    ::mimeList()->removeWindowsMime(this);
}


/*!
*/
int QWindowsMime::registerMimeType(const QString &mime)
{
#ifdef Q_OS_TEMP
    int f = RegisterClipboardFormat(mime.utf16());
#else
    int f = RegisterClipboardFormatA(mime.toLocal8Bit());
#endif
    if (!f)
        qErrnoWarning("QWindowsMime::registerMimeType: Failed to register clipboard format");

    return f;
}


/*!
\fn bool QWindowsMime::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const

  Returns true if the converter can convert from the \a mimeData to
  the format specified in \a formatetc.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QWindowsMime::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const

  Returns true if the converter can convert to the \a mimeType from
  the available formats in \a pDataObject.

  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QString QWindowsMime::mimeForFormat(const FORMATETC &formatetc) const

  Returns the mime type that will be created form the format specified
  in \a formatetc, or an empty string if this converter does not support
  \a formatetc.

  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QVector<FORMATETC> QWindowsMime::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const

  Returns a QVector of FORMATETC structures representing the different windows clipboard
  formats that can be provided for the \a mimeType from the \a mimeData.

  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn QVariant QWindowsMime::convertToMime(const QString &mime, QVariant::Type preferredType, IDataObject *pDataObj) const

  Returns a QVariant containing the converted data for \a mime from \a pDataObject.
  If possible the QVariant should be of the \a preferredType to avoid needless conversions.

  All subclasses must reimplement this pure virtual function.
*/

/*!
\fn bool QWindowsMime::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const

  Convert the \a mimeData to the format specified in \a formatetc.
  The converted data should then be placed in \a pmedium structure.

  Return true if the conversion was successful.

  All subclasses must reimplement this pure virtual function.
*/


QWindowsMime *QWindowsMime::converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData)
{
    QList<QWindowsMime*> mimes = ::mimeList()->windowsMimes();
    for (int i=mimes.size()-1; i>=0; --i) {
        if (mimes.at(i)->canConvertFromMime(formatetc, mimeData))
            return mimes.at(i);
    }
    return 0;
}

QWindowsMime *QWindowsMime::converterToMime(const QString &mimeType, IDataObject *pDataObj)
{
    QList<QWindowsMime*> mimes = ::mimeList()->windowsMimes();
    for (int i=mimes.size()-1; i>=0; --i) {
        if (mimes.at(i)->canConvertToMime(mimeType, pDataObj))
            return mimes.at(i);
    }
    return 0;
}

QVector<FORMATETC> QWindowsMime::allFormatsForMime(const QMimeData *mimeData)
{
    QList<QWindowsMime*> mimes = ::mimeList()->windowsMimes();
    QVector<FORMATETC> formatics;
    formatics.reserve(20);
    QStringList formats = mimeData->formats();
    for (int f=0; f<formats.size(); ++f) {
        for (int i=mimes.size()-1; i>=0; --i)
            formatics += mimes.at(i)->formatsForMime(formats.at(f), mimeData);
    }
    return formatics;
}

QStringList QWindowsMime::allMimesForFormats(IDataObject *pDataObj)
{
    QList<QWindowsMime*> mimes = ::mimeList()->windowsMimes();
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


class QWindowsMimeText : public QWindowsMime
{
public:
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
};

bool QWindowsMimeText::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    int cf = getCf(formatetc);
    return (cf == CF_UNICODETEXT || cf == CF_TEXT) && mimeData->hasText();
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
    }
    return false;
}

bool QWindowsMimeText::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
    return mimeType.startsWith("text/plain")
           && (canGetData(CF_UNICODETEXT, pDataObj)
           || canGetData(CF_TEXT, pDataObj));
}

QString QWindowsMimeText::mimeForFormat(const FORMATETC &formatetc) const
{
    int cf = getCf(formatetc);
    if (cf == CF_UNICODETEXT || cf == CF_TEXT)
        return "text/plain";
    return QString();
}


QVector<FORMATETC> QWindowsMimeText::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatics;
    if (mimeType.startsWith("text/plain") && mimeData->hasText()) {
        formatics += setCf(CF_UNICODETEXT);
        formatics += setCf(CF_TEXT);
    }
    return formatics;
}

QVariant QWindowsMimeText::convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const
{
    QVariant ret;

    if (canConvertToMime(mime, pDataObj)) {
        QString str;
        QByteArray data = getData(CF_UNICODETEXT, pDataObj);
        if (!data.isEmpty()) {
            str = QString::fromUtf16((const unsigned short *)data.data());
        } else {
            data = getData(CF_TEXT, pDataObj);
            if (!data.isEmpty()) {
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
                str = QString::fromLocal8Bit(r);
            }
        }
        if (preferredType == QVariant::String)
            ret = str;
        else
            ret = str.toUtf8();
    }

    return ret;
}

class QWindowsMimeURI : public QWindowsMime
{
public:
    QWindowsMimeURI();
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const;
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
            QStringList fileNames;
            int size = sizeof(DROPFILES)+2;
            for (int i=0; i<urls.size(); i++) {
                QString fn = QDir::convertSeparators(urls.at(i).toLocalFile());
                if (!fn.isEmpty()) {
                    QT_WA({
                        size += sizeof(TCHAR)*(fn.length()+1);
                    } , {
                        size += fn.toLocal8Bit().length()+1;
                    });
                    fileNames.append(fn);
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
                for (int i=0; i<fileNames.size(); i++) {
                    int l = fileNames.at(i).length();
                    memcpy(f, fileNames.at(i).utf16(), l*sizeof(TCHAR));
                    f += l;
                    *f++ = 0;
                }
                *f = 0;
            } , {
                d->fWide = false;
                char* f = files;
                for (int i=0; i<fileNames.size(); i++) {
                    QByteArray c = fileNames.at(i).toLocal8Bit();
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

bool QWindowsMimeURI::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
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
    if (mimeType == "text/uri-list") {
        if (canConvertFromMime(setCf(CF_HDROP), mimeData))
            formatics += setCf(CF_HDROP);
        if (canConvertFromMime(setCf(CF_INETURL), mimeData))
            formatics += setCf(CF_INETURL);
    }
    return formatics;
}

QVariant QWindowsMimeURI::convertToMime(const QString &mimeType, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const
{
    if (mimeType == "text/uri-list") {
        if (canGetData(CF_HDROP, pDataObj)) {
            QByteArray texturi;
            QList<QVariant> urls;

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
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const;
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

bool QWindowsMimeHtml::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
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
QVariant QWindowsMimeHtml::convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const
{
    Q_UNUSED(preferredType);
    QVariant result;
    if (canConvertToMime(mime, pDataObj)) {
        QString html = QString::fromUtf8(getData(CF_HTML, pDataObj));
#ifdef QMIME_DEBUG
        qDebug("QWindowsMimeHtml::convertToMime");
        qDebug("raw :");
        qDebug(html.toLatin1());
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
    if (canConvertFromMime(formatetc, mimeData)) {
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
        QByteArray pos = QString::number(result.size()).toLatin1();
        memcpy((char *)(result.data() + 53 - pos.length()), pos.constData(), pos.length());

        // set correct numbers for StartFragment and EndFragment
        pos = QString::number(result.indexOf("<!--StartFragment-->") + 20).toLatin1();
        memcpy((char *)(result.data() + 79 - pos.length()), pos.constData(), pos.length());
        pos = QString::number(result.indexOf("<!--EndFragment-->")).toLatin1();
        memcpy((char *)(result.data() + 103 - pos.length()), pos.constData(), pos.length());

        return setData(result, pmedium);
    }
    return false;
}


#ifndef QT_NO_IMAGEIO_BMP
class QWindowsMimeImage : public QWindowsMime
{
public:
    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;
    
    // for converting to Qt
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;
};


QVector<FORMATETC> QWindowsMimeImage::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (mimeData->hasImage() && mimeType == "application/x-qt-image")
        formatetcs += setCf(CF_DIB);
    return formatetcs;
}

QString QWindowsMimeImage::mimeForFormat(const FORMATETC &formatetc) const
{
    if (getCf(formatetc) == CF_DIB)
        return "application/x-qt-image";
    return QString();
}

bool QWindowsMimeImage::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
    return mimeType == "application/x-qt-image" && canGetData(CF_DIB, pDataObj);
}

bool QWindowsMimeImage::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    return getCf(formatetc) == CF_DIB && mimeData->hasImage();
}

bool QWindowsMimeImage::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    if (!canConvertFromMime(formatetc, mimeData))
        return false;
    QImage img = qvariant_cast<QImage>(mimeData->imageData());
    if (img.isNull())
        return false;
    QByteArray ba;
    QDataStream s(&ba, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
    if (qt_write_dib(s, img))
        return setData(ba, pmedium);
    return false;
}

QVariant QWindowsMimeImage::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
    Q_UNUSED(preferredType);
    QVariant result;
    if (canConvertToMime(mimeType, pDataObj)) {
        QImage img;
        QByteArray data = getData(CF_DIB, pDataObj);
        QDataStream s(&data, QIODevice::ReadOnly);
        s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
        if (qt_read_dib(s, img)) { // ##### encaps "-14"
            result = img;
        }
    }
    // Failed
    return result;
}
#endif

class QBuiltInMimes : public QWindowsMime
{
public:
    QBuiltInMimes();

    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;

    // for converting to Qt
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const;
    QString mimeForFormat(const FORMATETC &formatetc) const;

private:
    QMap<int, QString> outFormats;
    QMap<int, QString> inFormats;
};

QBuiltInMimes::QBuiltInMimes()
: QWindowsMime()
{
    outFormats.insert(QWindowsMime::registerMimeType("text/uri-list"), "text/uri-list");
    inFormats.insert(QWindowsMime::registerMimeType("text/uri-list"), "text/uri-list");
    outFormats.insert(QWindowsMime::registerMimeType("text/plain"), "text/plain");
    inFormats.insert(QWindowsMime::registerMimeType("text/plain"), "text/plain");
    outFormats.insert(QWindowsMime::registerMimeType("text/html"), "text/html");
    inFormats.insert(QWindowsMime::registerMimeType("text/html"), "text/html");
    outFormats.insert(QWindowsMime::registerMimeType("application/x-color"), "application/x-color");
    inFormats.insert(QWindowsMime::registerMimeType("application/x-color"), "application/x-color");
    outFormats.insert(QWindowsMime::registerMimeType("application/x-qt-image"), "application/x-qt-image");
    inFormats.insert(QWindowsMime::registerMimeType("application/x-qt-image"), "application/x-qt-image");
}

bool QBuiltInMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    // really check
    return formatetc.tymed & TYMED_HGLOBAL
        && outFormats.contains(formatetc.cfFormat)
        && mimeData->formats().contains(outFormats.value(formatetc.cfFormat));
}

bool QBuiltInMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const
{
    if (canConvertFromMime(formatetc, mimeData)) {
        QByteArray data;
        if (outFormats.value(getCf(formatetc)) == "text/html") {
            // text/html is in wide chars on windows (compatible with mozillia)
            QString html = mimeData->html();
            // same code as in the text converter up above
            const QChar *u = html.unicode();
            QString res;
            const int s = html.length();
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
            data = r;
        } else if (outFormats.value(getCf(formatetc)) == "application/x-qt-image") {
            QImage image = qvariant_cast<QImage>(mimeData->imageData());
            QDataStream ds(&data, QIODevice::WriteOnly);
            ds << image;
        } else {
            data = mimeData->data(outFormats.value(getCf(formatetc)));
        }
        return setData(data, pmedium);
    }
    return false;
}

QVector<FORMATETC> QBuiltInMimes::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    QVector<FORMATETC> formatetcs;
    if (!outFormats.keys(mimeType).isEmpty() && mimeData->formats().contains(mimeType))
        formatetcs += setCf(outFormats.key(mimeType));
    return formatetcs;
}

bool QBuiltInMimes::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
    return (!inFormats.keys(mimeType).isEmpty())
        && canGetData(inFormats.key(mimeType), pDataObj);
}

QVariant QBuiltInMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
    QVariant val;
    if (canConvertToMime(mimeType, pDataObj)) {
        QByteArray data = getData(inFormats.key(mimeType), pDataObj);
        if (!data.isEmpty()) {
#ifdef QMIME_DEBUG
            qDebug("QBuiltInMimes::convertToMime()");
#endif
            if (mimeType == "text/html" && preferredType == QVariant::String) {
                // text/html is in wide chars on windows (compatible with mozillia)
                val = QString::fromUtf16((const unsigned short *)data.data());
            } else if (mimeType == "application/x-qt-image") {
                QDataStream ds(&data, QIODevice::ReadOnly);
                QImage image;
                ds >> image;
                val = image;
            } else {
                val = data; // it should be enough to return the data and let QMimeData do the rest.
            }
        }
    }
    return val;
}

QString QBuiltInMimes::mimeForFormat(const FORMATETC &formatetc) const
{
    return inFormats.value(getCf(formatetc));
}


class QLastResortMimes : public QWindowsMime
{
public:

    // for converting from Qt
    bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
    bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const;
    QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const;

    // for converting to Qt
    bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const;
    QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const;
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

bool QLastResortMimes::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
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

QVariant QLastResortMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
    Q_UNUSED(preferredType);
    QVariant val;
    if (canConvertToMime(mimeType, pDataObj)) {
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

QWindowsMimeList::QWindowsMimeList()
    : initialized(false)
{
}

QWindowsMimeList::~QWindowsMimeList()
{
    while (mimes.size())
        delete mimes.first();
}


void QWindowsMimeList::init()
{
    if (!initialized) {
        initialized = true;
        new QLastResortMimes;
        new QWindowsMimeText;
        new QWindowsMimeURI;
#ifndef QT_NO_IMAGEIO_BMP
        new QWindowsMimeImage;
#endif
        new QWindowsMimeHtml;
        new QBuiltInMimes;
    }
}

void QWindowsMimeList::addWindowsMime(QWindowsMime * mime)
{
    init();
    mimes.append(mime);
}

void QWindowsMimeList::removeWindowsMime(QWindowsMime * mime)
{
    init();
    mimes.removeAll(mime);
}

QList<QWindowsMime*> QWindowsMimeList::windowsMimes()
{
    init();
    return mimes;
}

#endif // QT_NO_MIME
