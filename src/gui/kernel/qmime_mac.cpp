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

#include "qmime.h"

//#define USE_INTERNET_CONFIG

#ifndef USE_INTERNET_CONFIG
# include "qfile.h"
# include "qfileinfo.h"
# include "qtextstream.h"
# include "qdir.h"
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/fcntl.h>
#endif

#include "qdebug.h"
#include "qpixmap.h"
#include "qimagewriter.h"
#include "qimagereader.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qdatetime.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qurl.h"
#include "qmap.h"
#include <private/qt_mac_p.h>

extern CGImageRef qt_mac_createCGImageFromQImage(const QImage &img, const QImage **imagePtr = 0); // qpaintengine_mac.cpp

typedef QList<QMacPasteboardMime*> MimeList;
Q_GLOBAL_STATIC(MimeList, globalMimeList)

static void cleanup_mimes()
{
    MimeList *mimes = globalMimeList();
    while (!mimes->isEmpty())
        delete mimes->takeFirst();
}

/*****************************************************************************
  QDnD debug facilities
 *****************************************************************************/
//#define DEBUG_MIME_MAPS

//functions
extern QString qt_mac_from_pascal_string(const Str255);  //qglobal.cpp
extern void qt_mac_from_pascal_string(QString, Str255, TextEncoding encoding=0, int len=-1);  //qglobal.cpp

ScrapFlavorType qt_mac_mime_type = 'CUTE';
CFStringRef qt_mac_mime_typeUTI = CFSTR("com.pasteboard.trolltech.marker");

/*!
  \class QMacPasteboardMime
  \brief The QMacPasteboardMime class maps open-standard MIME to Mac flavors.
  \since 4.2
  \ingroup io
  \ingroup draganddrop
  \ingroup misc

  Qt's drag and drop support and clipboard facilities use the MIME
  standard. On X11, this maps trivially to the Xdnd protocol, but on
  Mac although some applications use MIME types to describe clipboard
  formats, others use arbitrary non-standardized naming conventions,
  or unnamed built-in Mac formats.

  By instantiating subclasses of QMacPasteboardMime that provide conversions
  between Mac flavors and MIME formats, you can convert proprietary
  clipboard formats to MIME formats.

  Qt has predefined support for the following Mac flavors:
  \list
    \i kScrapFlavorTypeUnicode - converted to "text/plain;charset=ISO-10646-UCS-2"
    \i kScrapFlavorTypeText - converted to "text/plain;charset=system" or "text/plain"
    \i kScrapFlavorTypePicture - converted to "application/x-qt-image"
    \i typeFileURL - converted to "text/uri-list"
  \endlist

  You can check if a MIME type is convertible using canConvert() and
  can perform conversions with convertToMime() and convertFromMime().
*/

/*! \enum QMacPasteboardMime::QMacPasteboardMimeType
    \internal
*/

/*!
  Constructs a new conversion object of type \a t, adding it to the
  globally accessed list of available convertors.
*/
QMacPasteboardMime::QMacPasteboardMime(char t) : type(t)
{
    globalMimeList()->append(this);
}

/*!
  Destroys a conversion object, removing it from the global
  list of available convertors.
*/
QMacPasteboardMime::~QMacPasteboardMime()
{
    if(!QApplication::closingDown())
        globalMimeList()->removeAll(this);
}

class QMacPasteboardMimeAny : public QMacPasteboardMime {
private:

public:
    QMacPasteboardMimeAny() : QMacPasteboardMime(MIME_QT_CONVERTOR|MIME_ALL) {
    }
    ~QMacPasteboardMimeAny() {
    }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeAny::convertorName()
{
    return QLatin1String("Any-Mime");
}

QString QMacPasteboardMimeAny::flavorFor(const QString &mime)
{
    QString ret = QLatin1String("com.trolltech.anymime.") + mime;
    return ret.replace(QLatin1String("/"), QLatin1String("--"));
}

QString QMacPasteboardMimeAny::mimeFor(QString flav)
{
    const QString any_prefix = QLatin1String("com.trolltech.anymime.");
    if(flav.size() > any_prefix.length() && flav.startsWith(any_prefix))
        return flav.mid(any_prefix.length()).replace(QLatin1String("--"), QLatin1String("/"));
    return QString();
}

bool QMacPasteboardMimeAny::canConvert(const QString &mime, QString flav)
{
    return mimeFor(flav) == mime;
}

QVariant QMacPasteboardMimeAny::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
    if(data.count() > 1)
        qWarning("QMacPasteboardMimeAny: Cannot handle multiple member data");
    QVariant ret;
    if (mime == QLatin1String("text/plain"))
        ret = QString::fromUtf8(data.first());
    else
        ret = data.first();
    return ret;
}

QList<QByteArray> QMacPasteboardMimeAny::convertFromMime(const QString &mime, QVariant data, QString)
{
    QList<QByteArray> ret;
    if (mime == QLatin1String("text/plain"))
        ret.append(data.toString().toUtf8());
    else
        ret.append(data.toByteArray());
    return ret;
}

class QMacPasteboardMimePlainText : public QMacPasteboardMime {
public:
    QMacPasteboardMimePlainText() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimePlainText::convertorName()
{
    return QLatin1String("PlainText");
}

QString QMacPasteboardMimePlainText::flavorFor(const QString &mime)
{
    if (mime == QLatin1String("text/plain"))
        return QLatin1String("com.apple.traditional-mac-plain-text");
    return QString();
}

QString QMacPasteboardMimePlainText::mimeFor(QString flav)
{
    if (flav == QLatin1String("com.apple.traditional-mac-plain-text"))
        return QLatin1String("text/plain");
    return QString();
}

bool QMacPasteboardMimePlainText::canConvert(const QString &mime, QString flav)
{
    return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimePlainText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
    if(data.count() > 1)
        qWarning("QMacPasteboardMimePlainText: Cannot handle multiple member data");
    const QByteArray &firstData = data.first();
    QVariant ret;
    if(flavor == QCFString(QLatin1String("com.apple.traditional-mac-plain-text"))) {
        QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                             reinterpret_cast<const UInt8 *>(firstData.constData()),
                                             firstData.size(), CFStringGetSystemEncoding(), false));
        ret = QString(str);
    } else {
        qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
    }
    return ret;
}

QList<QByteArray> QMacPasteboardMimePlainText::convertFromMime(const QString &, QVariant data, QString flavor)
{
    QList<QByteArray> ret;
    QString string = data.toString();
    if(flavor == QCFString(QLatin1String("com.apple.traditional-mac-plain-text")))
        ret.append(string.toLatin1());
    return ret;
}

class QMacPasteboardMimeUnicodeText : public QMacPasteboardMime {
public:
    QMacPasteboardMimeUnicodeText() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeUnicodeText::convertorName()
{
    return QLatin1String("UnicodeText");
}

QString QMacPasteboardMimeUnicodeText::flavorFor(const QString &mime)
{
    if (mime == QLatin1String("text/plain"))
        return QLatin1String("public.utf16-plain-text");
    int i = mime.indexOf(QLatin1String("charset="));
    if (i >= 0) {
        QString cs(mime.mid(i+8).toLower());
        i = cs.indexOf(QLatin1String(";"));
        if (i>=0)
            cs = cs.left(i);
        if (cs == QLatin1String("system"))
            return QLatin1String("public.utf8-plain-text");
        else if (cs == QLatin1String("iso-10646-ucs-2")
                 || cs == QLatin1String("utf16"))
            return QLatin1String("public.utf16-plain-text");
    }
    return QString();
}

QString QMacPasteboardMimeUnicodeText::mimeFor(QString flav)
{
    if (flav == QLatin1String("public.utf16-plain-text") || flav == QLatin1String("public.utf8-plain-text"))
        return QLatin1String("text/plain");
    return QString();
}

bool QMacPasteboardMimeUnicodeText::canConvert(const QString &mime, QString flav)
{
    return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimeUnicodeText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
    if(data.count() > 1)
        qWarning("QMacPasteboardMimeUnicodeText: Cannot handle multiple member data");
    const QByteArray &firstData = data.first();
    // I can only handle two types (system and unicode) so deal with them that way
    QVariant ret;
    if(flavor == QLatin1String("public.utf8-plain-text")) {
        QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                             reinterpret_cast<const UInt8 *>(firstData.constData()),
                                             firstData.size(), CFStringGetSystemEncoding(), false));
        ret = QString(str);
    } else if (flavor == QLatin1String("public.utf16-plain-text")) {
        ret = QString::fromUtf16(reinterpret_cast<const ushort *>(firstData.constData()),
                                 firstData.size() / sizeof(ushort));
    } else {
        qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
    }
    return ret;
}

QList<QByteArray> QMacPasteboardMimeUnicodeText::convertFromMime(const QString &, QVariant data, QString flavor)
{
    QList<QByteArray> ret;
    QString string = data.toString();
    if(flavor == QLatin1String("public.utf8-plain-text"))
        ret.append(string.toUtf8());
    else if (flavor == QLatin1String("public.utf16-plain-text"))
        ret.append(QByteArray((char*)string.utf16(), string.length()*2));
    return ret;
}

class QMacPasteboardMimeHTMLText : public QMacPasteboardMime {
public:
    QMacPasteboardMimeHTMLText() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeHTMLText::convertorName()
{
    return QLatin1String("HTML");
}

QString QMacPasteboardMimeHTMLText::flavorFor(const QString &mime)
{
    if (mime == QLatin1String("text/html"))
        return QLatin1String("public.html");
    return QString();
}

QString QMacPasteboardMimeHTMLText::mimeFor(QString flav)
{
    if (flav == QLatin1String("public.html"))
        return QLatin1String("text/html");
    return QString();
}

bool QMacPasteboardMimeHTMLText::canConvert(const QString &mime, QString flav)
{
    return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimeHTMLText::convertToMime(const QString &mimeType, QList<QByteArray> data, QString flavor)
{
    if (!canConvert(mimeType, flavor))
        return QVariant();
    if (data.count() > 1)
        qWarning("QMacPasteboardMimeHTMLText: Cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacPasteboardMimeHTMLText::convertFromMime(const QString &mime, QVariant data, QString flavor)
{
    QList<QByteArray> ret;
    if (!canConvert(mime, flavor))
        return ret;
    ret.append(data.toByteArray());
    return ret;
}


#ifdef Q_WS_MAC32
#include <QuickTime/QuickTime.h>
#include "qlibrary.h"
typedef ComponentResult (*PtrGraphicsImportSetDataHandle)(GraphicsImportComponent, Handle);
typedef ComponentResult (*PtrGraphicsImportCreateCGImage)(GraphicsImportComponent, CGImageRef*, UInt32);
typedef ComponentResult (*PtrGraphicsExportSetInputCGImage)(GraphicsExportComponent, CGImageRef);
typedef ComponentResult (*PtrGraphicsExportSetOutputHandle)(GraphicsExportComponent, Handle);
typedef ComponentResult (*PtrGraphicsExportDoExport)(GraphicsExportComponent, unsigned long *);

static PtrGraphicsImportSetDataHandle ptrGraphicsImportSetDataHandle = 0;
static PtrGraphicsImportCreateCGImage ptrGraphicsImportCreateCGImage = 0;
static PtrGraphicsExportSetInputCGImage ptrGraphicsExportSetInputCGImage = 0;
static PtrGraphicsExportSetOutputHandle ptrGraphicsExportSetOutputHandle = 0;
static PtrGraphicsExportDoExport ptrGraphicsExportDoExport = 0;

static bool resolveMimeQuickTimeSymbols()
{
    if (ptrGraphicsImportSetDataHandle == 0) {
        QLibrary library(QLatin1String("/System/Library/Frameworks/QuickTime.framework/QuickTime"));
        ptrGraphicsImportSetDataHandle = reinterpret_cast<PtrGraphicsImportSetDataHandle>(library.resolve("GraphicsImportSetDataHandle"));
        ptrGraphicsImportCreateCGImage = reinterpret_cast<PtrGraphicsImportCreateCGImage>(library.resolve("GraphicsImportCreateCGImage"));
        ptrGraphicsExportSetInputCGImage = reinterpret_cast<PtrGraphicsExportSetInputCGImage>(library.resolve("GraphicsExportSetInputCGImage"));
        ptrGraphicsExportSetOutputHandle = reinterpret_cast<PtrGraphicsExportSetOutputHandle>(library.resolve("GraphicsExportSetOutputHandle"));
        ptrGraphicsExportDoExport = reinterpret_cast<PtrGraphicsExportDoExport>(library.resolve("GraphicsExportDoExport"));
    }

    return ptrGraphicsImportSetDataHandle != 0
           && ptrGraphicsImportCreateCGImage != 0 && ptrGraphicsExportSetInputCGImage != 0
           && ptrGraphicsExportSetOutputHandle != 0 && ptrGraphicsExportDoExport != 0;
}

class QMacPasteboardMimePict : public QMacPasteboardMime {
public:
    QMacPasteboardMimePict() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimePict::convertorName()
{
    return QLatin1String("Pict");
}

QString QMacPasteboardMimePict::flavorFor(const QString &mime)
{
    if(mime.startsWith(QLatin1String("application/x-qt-image")))
        return QLatin1String("com.apple.pict");
    return QString();
}

QString QMacPasteboardMimePict::mimeFor(QString flav)
{
    if(flav == QLatin1String("com.apple.pict"))
        return QLatin1String("application/x-qt-image");
    return QString();
}

bool QMacPasteboardMimePict::canConvert(const QString &mime, QString flav)
{
    return flav == QLatin1String("com.apple.pict")
            && mime == QLatin1String("application/x-qt-image");
}


QVariant QMacPasteboardMimePict::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
    if(data.count() > 1)
        qWarning("QMacPasteboardMimePict: Cannot handle multiple member data");
    QVariant ret;
    if (!resolveMimeQuickTimeSymbols())
        return ret;

    if(!canConvert(mime, flav))
        return ret;
    const QByteArray &a = data.first();

    // This function expects the 512 header (just to skip it, so create the extra space for it).
    Handle pic = NewHandle(a.size() + 512);
    memcpy(*pic + 512, a.constData(), a.size());

    GraphicsImportComponent graphicsImporter;
    ComponentResult result = OpenADefaultComponent(GraphicsImporterComponentType,
                                                   kQTFileTypePicture, &graphicsImporter);
    QCFType<CGImageRef> cgImage;
    if (!result)
        result = ptrGraphicsImportSetDataHandle(graphicsImporter, pic);
    if (!result)
        result = ptrGraphicsImportCreateCGImage(graphicsImporter, &cgImage,
                                             kGraphicsImportCreateCGImageUsingCurrentSettings);
    if (!result)
        ret = QVariant(QPixmap::fromMacCGImageRef(cgImage).toImage());
    CloseComponent(graphicsImporter);
    DisposeHandle(pic);
    return ret;
}

QList<QByteArray> QMacPasteboardMimePict::convertFromMime(const QString &mime, QVariant variant,
                                                          QString flav)
{
    QList<QByteArray> ret;
    if (!resolveMimeQuickTimeSymbols())
        return ret;

    if (!canConvert(mime, flav))
        return ret;
    QCFType<CGImageRef> cgimage = qt_mac_createCGImageFromQImage(qvariant_cast<QImage>(variant));
    Handle pic = NewHandle(0);
    GraphicsExportComponent graphicsExporter;
    ComponentResult result = OpenADefaultComponent(GraphicsExporterComponentType,
                                                   kQTFileTypePicture, &graphicsExporter);
    if (!result) {
        unsigned long sizeWritten;
        result = ptrGraphicsExportSetInputCGImage(graphicsExporter, cgimage);
        if (!result)
            result = ptrGraphicsExportSetOutputHandle(graphicsExporter, pic);
        if (!result)
            result = ptrGraphicsExportDoExport(graphicsExporter, &sizeWritten);

        CloseComponent(graphicsExporter);
    }

    int size = GetHandleSize((Handle)pic);
    // Skip the Picture File header (512 bytes) and feed the raw data
    QByteArray ar(reinterpret_cast<char *>(*pic + 512), size - 512);
    ret.append(ar);
    DisposeHandle(pic);
    return ret;
}
#endif

class QMacPasteboardMimeTiff : public QMacPasteboardMime {
public:
    QMacPasteboardMimeTiff() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeTiff::convertorName()
{
    return QLatin1String("Tiff");
}

QString QMacPasteboardMimeTiff::flavorFor(const QString &mime)
{
    if(mime.startsWith(QLatin1String("application/x-qt-image")))
        return QLatin1String("public.tiff");
    return QString();
}

QString QMacPasteboardMimeTiff::mimeFor(QString flav)
{
    if(flav == QLatin1String("public.tiff"))
        return QLatin1String("application/x-qt-image");
    return QString();
}

bool QMacPasteboardMimeTiff::canConvert(const QString &mime, QString flav)
{
    return flav == QLatin1String("public.tiff") && mime == QLatin1String("application/x-qt-image");
}

QVariant QMacPasteboardMimeTiff::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
    if(data.count() > 1)
        qWarning("QMacPasteboardMimeTiff: Cannot handle multiple member data");
    QVariant ret;
    if (!canConvert(mime, flav))
        return ret;
    const QByteArray &a = data.first();
    QCFType<CGImageRef> image;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        QCFType<CFDataRef> data = CFDataCreateWithBytesNoCopy(0,
                                                    reinterpret_cast<const UInt8 *>(a.constData()),
                                                    a.size(), kCFAllocatorNull);
        QCFType<CGImageSourceRef> imageSource = CGImageSourceCreateWithData(data, 0);
        image = CGImageSourceCreateImageAtIndex(imageSource, 0, 0);
    } else
#endif
    {
#ifdef Q_WS_MAC32
        if (resolveMimeQuickTimeSymbols()) {
            Handle tiff = NewHandle(a.size());
            memcpy(*tiff, a.constData(), a.size());
            GraphicsImportComponent graphicsImporter;
            ComponentResult result = OpenADefaultComponent(GraphicsImporterComponentType,
                                                           kQTFileTypeTIFF, &graphicsImporter);
            if (!result)
                result = ptrGraphicsImportSetDataHandle(graphicsImporter, tiff);
            if (!result)
                result = ptrGraphicsImportCreateCGImage(graphicsImporter, &image,
                                                     kGraphicsImportCreateCGImageUsingCurrentSettings);
            CloseComponent(graphicsImporter);
            DisposeHandle(tiff);
        }
#endif
    }

    if (image != 0)
        ret = QVariant(QPixmap::fromMacCGImageRef(image).toImage());
    return ret;
}

QList<QByteArray> QMacPasteboardMimeTiff::convertFromMime(const QString &mime, QVariant variant, QString flav)
{
    QList<QByteArray> ret;
    if (!canConvert(mime, flav))
        return ret;

    QImage img = qvariant_cast<QImage>(variant);
    QCFType<CGImageRef> cgimage = qt_mac_createCGImageFromQImage(img);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        QCFType<CFMutableDataRef> data = CFDataCreateMutable(0, 0);
        QCFType<CGImageDestinationRef> imageDestination = CGImageDestinationCreateWithData(data, kUTTypeTIFF, 1, 0);
        if (imageDestination != 0) {
            CFTypeRef keys[2];
            QCFType<CFTypeRef> values[2];
            QCFType<CFDictionaryRef> options;
            keys[0] = kCGImagePropertyPixelWidth;
            keys[1] = kCGImagePropertyPixelHeight;
            int width = img.width();
            int height = img.height();
            values[0] = CFNumberCreate(0, kCFNumberIntType, &width);
            values[1] = CFNumberCreate(0, kCFNumberIntType, &height);
            options = CFDictionaryCreate(0, reinterpret_cast<const void **>(keys),
                                         reinterpret_cast<const void **>(values), 2,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
            CGImageDestinationAddImage(imageDestination, cgimage, options);
            CGImageDestinationFinalize(imageDestination);
        }
        QByteArray ar(CFDataGetLength(data), 0);
        CFDataGetBytes(data,
                CFRangeMake(0, ar.size()),
                reinterpret_cast<UInt8 *>(ar.data()));
        ret.append(ar);
    } else
#endif
    {
#ifdef Q_WS_MAC32
        Handle tiff = NewHandle(0);
        if (resolveMimeQuickTimeSymbols()) {
            GraphicsExportComponent graphicsExporter;
            ComponentResult result = OpenADefaultComponent(GraphicsExporterComponentType,
                                                           kQTFileTypeTIFF, &graphicsExporter);
            if (!result) {
                unsigned long sizeWritten;
                result = ptrGraphicsExportSetInputCGImage(graphicsExporter, cgimage);
                if (!result)
                    result = ptrGraphicsExportSetOutputHandle(graphicsExporter, tiff);
                if (!result)
                    result = ptrGraphicsExportDoExport(graphicsExporter, &sizeWritten);

                CloseComponent(graphicsExporter);
            }
        }
        int size = GetHandleSize((Handle)tiff);
        QByteArray ar(reinterpret_cast<char *>(*tiff), size);
        ret.append(ar);
        DisposeHandle(tiff);
#endif
    }
    return ret;
}


class QMacPasteboardMimeFileUri : public QMacPasteboardMime {
public:
    QMacPasteboardMimeFileUri() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeFileUri::convertorName()
{
    return QLatin1String("FileURL");
}

QString QMacPasteboardMimeFileUri::flavorFor(const QString &mime)
{
    if (mime == QLatin1String("text/uri-list"))
        return QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0));
    return QString();
}

QString QMacPasteboardMimeFileUri::mimeFor(QString flav)
{
    if (flav == QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0)))
        return QLatin1String("text/uri-list");
    return QString();
}

bool QMacPasteboardMimeFileUri::canConvert(const QString &mime, QString flav)
{
    return mime == QLatin1String("text/uri-list")
            && flav == QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0));
}

QVariant QMacPasteboardMimeFileUri::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
    if(!canConvert(mime, flav))
        return QVariant();
    QList<QVariant> ret;
    for(int i = 0; i < data.size(); ++i) {
        QUrl url = QUrl::fromEncoded(data.at(i));
        if (url.host().toLower() == QLatin1String("localhost"))
            url.setHost(QString());
        url.setPath(url.path().normalized(QString::NormalizationForm_C));
        ret.append(url);
    }
    return QVariant(ret);
}

QList<QByteArray> QMacPasteboardMimeFileUri::convertFromMime(const QString &mime, QVariant data, QString flav)
{
    QList<QByteArray> ret;
    if (!canConvert(mime, flav))
        return ret;
    QList<QVariant> urls = data.toList();
    for(int i = 0; i < urls.size(); ++i) {
        QUrl url = urls.at(i).toUrl();
        if (url.scheme().isEmpty())
            url.setScheme(QLatin1String("file"));
        if (url.scheme().toLower() == QLatin1String("file")) {
            if (url.host().isEmpty())
                url.setHost(QLatin1String("localhost"));
            url.setPath(url.path().normalized(QString::NormalizationForm_D));
        }
        ret.append(url.toEncoded());
    }
    return ret;
}

#ifdef QT3_SUPPORT
class QMacPasteboardMimeQt3Any : public QMacPasteboardMime {
private:
    int current_max;
    QFile library_file;
    QDateTime mime_registry_loaded;
    QMap<QString, int> mime_registry;
    int registerMimeType(const QString &mime);
    bool loadMimeRegistry();

public:
    QMacPasteboardMimeQt3Any() : QMacPasteboardMime(MIME_QT3_CONVERTOR) {
        current_max = 'QT00';
    }
    ~QMacPasteboardMimeQt3Any() {
    }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

static bool qt_mac_openMimeRegistry(bool global, QIODevice::OpenMode mode, QFile &file)
{
    QString dir = QLatin1String("/Library/Qt");
    if(!global)
        dir.prepend(QDir::homePath());
    file.setFileName(dir + QLatin1String("/.mime_types"));
    if(mode != QIODevice::ReadOnly) {
        if(!QFile::exists(dir)) {
            // Do it with a system call as I don't see much worth in
            // doing it with QDir since we have to chmod anyway.
            bool success = ::mkdir(dir.toLocal8Bit().constData(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
            if (success)
                success = ::chmod(dir.toLocal8Bit().constData(), S_IRUSR | S_IWUSR | S_IXUSR
                                      | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) == 0;
            if (!success)
                return false;
        }
        if (!file.exists()) {
            // Create the file and chmod it so that everyone can write to it.
            int fd = ::open(file.fileName().toLocal8Bit().constData(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            bool success = fd != -1;
            if (success)
                success = ::fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == 0;
            if (fd != -1)
                ::close(fd);
            if(!success)
                return false;
        }
    }
    return file.open(mode);
}
static void qt_mac_loadMimeRegistry(QFile &file, QMap<QString, int> &registry, int &max)
{
    file.reset();
    QTextStream stream(&file);
    while(!stream.atEnd()) {
	QString mime = stream.readLine();
	int mactype = stream.readLine().toInt();
	if(mactype > max)
	    max = mactype;
	registry.insert(mime, mactype);
    }
}

bool QMacPasteboardMimeQt3Any::loadMimeRegistry()
{
    if(!library_file.isOpen()) {
        if(!qt_mac_openMimeRegistry(true, QIODevice::ReadWrite, library_file)) {
            QFile global;
            if(qt_mac_openMimeRegistry(true, QIODevice::ReadOnly, global)) {
                ::qt_mac_loadMimeRegistry(global, mime_registry, current_max);
                global.close();
            }
            if(!qt_mac_openMimeRegistry(false, QIODevice::ReadWrite, library_file)) {
                qWarning("QMacPasteboardMimeAnyQt3Mime: Failure to open mime resources %s -- %s", library_file.fileName().toLatin1().constData(),
                         library_file.errorString().toLatin1().constData());
                return false;
            }
        }
    }

    QFileInfo fi(library_file);
    if(!mime_registry_loaded.isNull() && mime_registry_loaded == fi.lastModified())
        return true;
    mime_registry_loaded = fi.lastModified();
    ::qt_mac_loadMimeRegistry(library_file, mime_registry, current_max);
    return true;
}

int QMacPasteboardMimeQt3Any::registerMimeType(const QString &mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("QMacPasteboardMimeAnyQt3Mime: Internal error");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            if(!library_file.isOpen()) {
                if(!library_file.open(QIODevice::WriteOnly)) {
                    qWarning("QMacPasteboardMimeAnyQt3Mime: Failure to open %s -- %s", library_file.fileName().toLatin1().constData(),
                             library_file.errorString().toLatin1().constData());
                    return false;
                }
            }
            int ret = ++current_max;
            mime_registry_loaded = QFileInfo(library_file).lastModified();
            QTextStream stream(&library_file);
            stream << mime << endl;
            stream << ret << endl;
            mime_registry.insert(mime, ret);
            library_file.flush(); //flush and set mtime
            return ret;
        }
    }
    return mime_registry[mime];
}

QString QMacPasteboardMimeQt3Any::convertorName()
{
    return QLatin1String("Qt3-Any-Mime");
}

QString QMacPasteboardMimeQt3Any::flavorFor(const QString &mime)
{
    const int os_flav = registerMimeType(mime);
    QCFType<CFArrayRef> ids = UTTypeCreateAllIdentifiersForTag(0, kUTTagClassOSType,
                                                               QCFString(UTCreateStringForOSType(os_flav)));
    if(ids) {
        const int type_count = CFArrayGetCount(ids);
        if(type_count) {
            if(type_count > 1)
                qDebug("Can't happen!");
            return QCFString::toQString((CFStringRef)CFArrayGetValueAtIndex(ids, 0));
        }
    }
    return QString();
}

QString QMacPasteboardMimeQt3Any::mimeFor(QString flav)
{
    loadMimeRegistry();
    const int os_flav = UTGetOSTypeFromString(UTTypeCopyPreferredTagWithClass(QCFString(flav), kUTTagClassOSType));
    for(QMap<QString, int>::const_iterator it = mime_registry.constBegin();
        it != mime_registry.constEnd(); ++it) {
        if(it.value() == os_flav)
            return QString::fromLatin1(it.key().toLatin1());
    }
    return QString();
}

bool QMacPasteboardMimeQt3Any::canConvert(const QString &mime, QString flav)
{
    loadMimeRegistry();
    const int os_flav = UTGetOSTypeFromString(UTTypeCopyPreferredTagWithClass(QCFString(flav), kUTTagClassOSType));
    if(mime_registry.contains(mime) && mime_registry[mime] == os_flav)
        return true;
    return false;
}

QVariant QMacPasteboardMimeQt3Any::convertToMime(const QString &, QList<QByteArray>, QString)
{
    qWarning("QMacPasteboardMimeAnyQt3Mime: Cannot write anything!");
    return QVariant();
}

QList<QByteArray> QMacPasteboardMimeQt3Any::convertFromMime(const QString &mime, QVariant data, QString)
{
    QList<QByteArray> ret;
    if (mime == QLatin1String("text/plain")) {
        ret.append(data.toString().toUtf8());
    } else {
        ret.append(data.toByteArray());
    }
    return ret;
}
#endif

/*!
  \internal

  This is an internal function.
*/
void QMacPasteboardMime::initialize()
{
    if(globalMimeList()->isEmpty()) {
        qAddPostRoutine(cleanup_mimes);

        //standard types that we wrap
        new QMacPasteboardMimeTiff;
#ifdef Q_WS_MAC32
        new QMacPasteboardMimePict;
#endif
        new QMacPasteboardMimeUnicodeText;
        new QMacPasteboardMimePlainText;
        new QMacPasteboardMimeHTMLText;
        new QMacPasteboardMimeFileUri;

        //make sure our "non-standard" types are always last! --Sam
        new QMacPasteboardMimeAny;
#ifdef QT3_SUPPORT
        new QMacPasteboardMimeQt3Any;
#endif
    }
}

/*!
  Returns the most-recently created QMacPasteboardMime of type \a t that can convert
  between the \a mime and \a flav formats.  Returns 0 if no such convertor
  exists.
*/
QMacPasteboardMime*
QMacPasteboardMime::convertor(uchar t, const QString &mime, QString flav)
{
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
        qDebug("QMacPasteboardMime::convertor: seeing if %s (%d) can convert %s to %d[%c%c%c%c] [%d]",
               (*it)->convertorName().toLatin1().constData(),
               (*it)->type & t, mime.toLatin1().constData(),
               flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
               (*it)->canConvert(mime,flav));
        for(int i = 0; i < (*it)->countFlavors(); ++i) {
            int f = (*it)->flavor(i);
            qDebug("  %d) %d[%c%c%c%c] [%s]", i, f,
                   (f >> 24) & 0xFF, (f >> 16) & 0xFF, (f >> 8) & 0xFF, (f) & 0xFF,
                   (*it)->convertorName().toLatin1().constData());
        }
#endif
        if(((*it)->type & t) && (*it)->canConvert(mime, flav))
            return (*it);
    }
    return 0;
}
/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
QString QMacPasteboardMime::flavorToMime(uchar t, QString flav)
{
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
        qDebug("QMacMIme::flavorToMime: attempting %s (%d) for flavor %d[%c%c%c%c] [%s]",
               (*it)->convertorName().toLatin1().constData(),
               (*it)->type & t, flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
               (*it)->mimeFor(flav).toLatin1().constData());

#endif
        if((*it)->type & t) {
            QString mimeType = (*it)->mimeFor(flav);
            if(!mimeType.isNull())
                return mimeType;
        }
    }
    return QString();
}

/*!
  Returns a list of all currently defined QMacPasteboardMime objects of type \a t.
*/
QList<QMacPasteboardMime*> QMacPasteboardMime::all(uchar t)
{
    MimeList ret;
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
        if((*it)->type & t)
            ret.append((*it));
    }
    return ret;
}

/*!
  \fn QString QMacPasteboardMime::convertorName()

  Returns a name for the convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QMacPasteboardMime::canConvert(const QString &mime, QString flav)

  Returns true if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns false.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacPasteboardMime::mimeFor(QString flav)

  Returns the MIME UTI used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacPasteboardMime::flavorFor(const QString &mime)

  Returns the Mac UTI used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
    \fn QVariant QMacPasteboardMime::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)

    Returns \a data converted from Mac UTI \a flav to MIME type \a
    mime.

    Note that Mac flavors must all be self-terminating. The input \a
    data may contain trailing data.

    All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacPasteboardMime::convertFromMime(const QString &mime, QVariant data, QString flav)

  Returns \a data converted from MIME type \a mime
    to Mac UTI \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

