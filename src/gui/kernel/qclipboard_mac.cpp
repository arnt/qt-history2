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

#include "qclipboard.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qapplication_p.h"
#include <qdebug.h>
#include <private/qt_mac_p.h>
#include "qevent.h"
#include "qurl.h"
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
  QClipboard debug facilities
 *****************************************************************************/
#define DEBUG_PASTEBOARD

#ifndef QT_NO_CLIPBOARD

void qt_event_send_clipboard_changed(); //qapplication_mac.cpp

/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

static QMacPasteboard *qt_mac_pasteboards[2] = {0, 0};

static inline QMacPasteboard *qt_mac_pasteboard(QClipboard::Mode mode)
{
    // Assert on the mode, unsupported modes should be caught before
    // calling this function.
    Q_ASSERT(mode == QClipboard::Clipboard || mode == QClipboard::FindBuffer);

    if (mode == QClipboard::Clipboard)
        return qt_mac_pasteboards[0];
    else
        return qt_mac_pasteboards[1];
}

static void qt_mac_cleanupPasteboard() {
    delete qt_mac_pasteboards[0];
    delete qt_mac_pasteboards[1];
    qt_mac_pasteboards[0] = 0;
    qt_mac_pasteboards[1] = 0;
}

static bool qt_mac_updateScrap(QClipboard::Mode mode)
{
    if(!qt_mac_pasteboards[0]) {
        qt_mac_pasteboards[0] = new QMacPasteboard(kPasteboardClipboard, QMacPasteboardMime::MIME_CLIP);
        qt_mac_pasteboards[1] = new QMacPasteboard(kPasteboardFind, QMacPasteboardMime::MIME_CLIP);
        qAddPostRoutine(qt_mac_cleanupPasteboard);
        return true;
    }
    return qt_mac_pasteboard(mode)->sync();
}

void QClipboard::clear(Mode mode)
{
    if (supportsMode(mode) == false)
        return;
    qt_mac_updateScrap(mode);
    qt_mac_pasteboard(mode)->clear();
}

void QClipboard::ownerDestroyed()
{
}


void QClipboard::connectNotify(const char *signal)
{
    Q_UNUSED(signal);
}

bool QClipboard::event(QEvent *e)
{
    if(e->type() != QEvent::Clipboard)
        return QObject::event(e);

    if (qt_mac_updateScrap(QClipboard::Clipboard)) {
        qt_mac_pasteboard(QClipboard::Clipboard)->setMimeData(0);
        emitChanged(QClipboard::Clipboard);
    }

    if (qt_mac_updateScrap(QClipboard::FindBuffer)) {
        qt_mac_pasteboard(QClipboard::FindBuffer)->setMimeData(0);
        emitChanged(QClipboard::FindBuffer);
    }

    return QObject::event(e);
}

const QMimeData *QClipboard::mimeData(Mode mode) const
{
    if (supportsMode(mode) == false)
        return 0;
    qt_mac_updateScrap(mode);
    return qt_mac_pasteboard(mode)->mimeData();
}

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{
    if (supportsMode(mode) == false)
        return;
    qt_mac_updateScrap(mode);
    qt_mac_pasteboard(mode)->setMimeData(src);
    emitChanged(mode);
    qt_event_send_clipboard_changed();
}

bool QClipboard::supportsMode(Mode mode) const
{
    return (mode == Clipboard || mode == FindBuffer);
}

bool QClipboard::ownsMode(Mode mode) const
{
    Q_UNUSED(mode);
    return false;
}

#endif // QT_NO_CLIPBOARD

/*****************************************************************************
   QMacPasteboard code
*****************************************************************************/

QMacPasteboard::QMacPasteboard(PasteboardRef p, uchar mt)
{
    mac_mime_source = false;
    mime_type = mt ? mt : uchar(QMacPasteboardMime::MIME_ALL);
    paste = p;
    CFRetain(paste);
}

QMacPasteboard::QMacPasteboard(uchar mt)
{
    mac_mime_source = false;
    mime_type = mt ? mt : uchar(QMacPasteboardMime::MIME_ALL);
    paste = 0;
    OSStatus err = PasteboardCreate(0, &paste);
    if(err == noErr) {
        PasteboardSetPromiseKeeper(paste, promiseKeeper, this);
    } else {
        qDebug("PasteBoard: Error creating pasteboard: [%d]", (int)err);
    }
}

QMacPasteboard::QMacPasteboard(CFStringRef name, uchar mt)
{
    mac_mime_source = false;
    mime_type = mt ? mt : uchar(QMacPasteboardMime::MIME_ALL);
    paste = 0;
    OSStatus err = PasteboardCreate(name, &paste);
    if(err == noErr) {
        PasteboardSetPromiseKeeper(paste, promiseKeeper, this);
    } else {
        qDebug("PasteBoard: Error creating pasteboard: %s [%d]", QCFString::toQString(name).toLatin1().constData(), (int)err);
    }
}

QMacPasteboard::~QMacPasteboard()
{
    // Copy all promised data to the clipboard before exiting.
    for (int i = 0; i < promises.count(); ++i) {
        const Promise &promise = promises.at(i);
        QCFString flavor = QCFString(promise.convertor->flavorFor(promise.mime));
        promiseKeeper(paste, (PasteboardItemID)promise.itemId, flavor, this);
    }

    if(paste)
        CFRelease(paste);
}

PasteboardRef
QMacPasteboard::pasteBoard() const
{
    return paste;
}

OSStatus QMacPasteboard::promiseKeeper(PasteboardRef paste, PasteboardItemID id, CFStringRef flavor, void *data)
{
    QMacPasteboard *qpaste = (QMacPasteboard*) data;
    const long promise_id = (long)id;

    // Find the promise that promised to deliver
    // data for the given flavor:
    const QString flavorAsQString = QCFString::toQString(flavor);
    QMacPasteboard::Promise promise;
    for (int i = 0; i < qpaste->promises.size(); i++){
        QMacPasteboard::Promise tmp = qpaste->promises[i];
        if (tmp.itemId == promise_id && tmp.convertor->canConvert(tmp.mime, flavorAsQString)){
            promise = tmp;
            break;
        }
    }

    if (!promise.itemId) {
        // There was no promise that could deliver data for the
        // given id and flavor. This should not happend.
        qDebug("Pasteboard: %d: Request for %ld, %s, but no promise found!", __LINE__, promise_id, qPrintable(flavorAsQString));
        return cantGetFlavorErr;
    }

#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: Calling in promise for %s[%ld] [%s] (%s)", qPrintable(promise.mime), promise_id,
           qPrintable(flavorAsQString),
           qPrintable(promise.convertor->convertorName()));
#endif

    QList<QByteArray> md = promise.convertor->convertFromMime(promise.mime, promise.data, flavorAsQString);
    if (promise.mime != QLatin1String("text/uri-list")){
        for(int i = 0; i < md.size(); ++i) {
            const QByteArray &ba = md[i];
            QCFType<CFDataRef> data = CFDataCreate(0, (UInt8*)ba.constData(), ba.size());
            PasteboardPutItemFlavor(paste, id, flavor, data, kPasteboardFlavorNoFlags);
        }
    } else {
        // Mac expects different files to be different items
        // on the pasteboard. So we return only one URL.
        // The list index is synced with the promise_id.
        if (md.size() < promise_id)
            return cantGetFlavorErr;
        const QByteArray &ba = md[promise_id - 1];
        QCFType<CFDataRef> data = CFDataCreate(0, (UInt8*)ba.constData(), ba.size());
        PasteboardPutItemFlavor(paste, id, flavor, data, kPasteboardFlavorNoFlags);
    }
    return noErr;
}

bool
QMacPasteboard::hasOSType(int c_flavor) const
{
    if (!paste)
        return false;

    sync();

    ItemCount cnt = 0;
    if(PasteboardGetItemCount(paste, &cnt) || !cnt)
        return false;

#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: hasOSType [%c%c%c%c]", (c_flavor>>24)&0xFF, (c_flavor>>16)&0xFF,
           (c_flavor>>8)&0xFF, (c_flavor>>0)&0xFF);
#endif
    for(uint index = 1; index <= cnt; ++index) {

        PasteboardItemID id;
        if(PasteboardGetItemIdentifier(paste, index, &id) != noErr)
            return false;

        QCFType<CFArrayRef> types;
        if(PasteboardCopyItemFlavors(paste, id, &types ) != noErr)
            return false;

        const int type_count = CFArrayGetCount(types);
        for(int i = 0; i < type_count; ++i) {
            CFStringRef flavor = (CFStringRef)CFArrayGetValueAtIndex(types, i);
            const int os_flavor = UTGetOSTypeFromString(UTTypeCopyPreferredTagWithClass(flavor, kUTTagClassOSType));
            if(os_flavor == c_flavor) {
#ifdef DEBUG_PASTEBOARD
                qDebug("  - Found!");
#endif
                return true;
            }
        }
    }
#ifdef DEBUG_PASTEBOARD
    qDebug("  - NotFound!");
#endif
    return false;
}

bool
QMacPasteboard::hasFlavor(QString c_flavor) const
{
    if (!paste)
        return false;

    sync();

    ItemCount cnt = 0;
    if(PasteboardGetItemCount(paste, &cnt) || !cnt)
        return false;

#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: hasFlavor [%s]", qPrintable(c_flavor));
#endif
    for(uint index = 1; index <= cnt; ++index) {

        PasteboardItemID id;
        if(PasteboardGetItemIdentifier(paste, index, &id) != noErr)
            return false;

        PasteboardFlavorFlags flags;
        if(PasteboardGetItemFlavorFlags(paste, id, QCFString(c_flavor), &flags) == noErr) {
#ifdef DEBUG_PASTEBOARD
            qDebug("  - Found!");
#endif
            return true;
        }
    }
#ifdef DEBUG_PASTEBOARD
    qDebug("  - NotFound!");
#endif
    return false;
}

class QMacPasteboardMimeSource : public QMimeData {
    const QMacPasteboard *paste;
public:
    QMacPasteboardMimeSource(const QMacPasteboard *p) : QMimeData(), paste(p) { }
    ~QMacPasteboardMimeSource() { }
    virtual QStringList formats() const { return paste->formats(); }
    virtual QVariant retrieveData(const QString &format, QVariant::Type type) const { return paste->retrieveData(format, type); }
};

QMimeData
*QMacPasteboard::mimeData() const
{
    if(!mime) {
        mac_mime_source = true;
        mime = new QMacPasteboardMimeSource(this);

    }
    return mime;
}

class QMacMimeData : public QMimeData
{
public:
    QVariant variantData(const QString &mime) { return retrieveData(mime, QVariant::Invalid); }
private:
    QMacMimeData();
};

void QMacPasteboard::putMimetypeOntoPasteboard(int itemId, const QString &mimeType, QMimeData *mime_src)
{
    QList<QMacPasteboardMime*> availableConverters = QMacPasteboardMime::all(mime_type);
    for (QList<QMacPasteboardMime *>::Iterator it = availableConverters.begin(); it != availableConverters.end(); ++it) {
        QMacPasteboardMime *c = (*it);
        QString flavor(c->flavorFor(mimeType));
        if(!flavor.isEmpty()) {
            // We have a pasteboard mime that can convert
            // the given mimetype into a flavor
            promises.append(QMacPasteboard::Promise(itemId, c, mimeType,
                                                    static_cast<QMacMimeData*>(mime_src)->variantData(mimeType)));
            PasteboardPutItemFlavor(paste, (PasteboardItemID)itemId, QCFString(flavor), 0, kPasteboardFlavorNoFlags);
#ifdef DEBUG_PASTEBOARD
            qDebug(" -  adding %d %s [%s] <%s>",
                itemId, qPrintable(mimeType), qPrintable(flavor), qPrintable(c->convertorName()));
#endif
        }
    }
}

void
QMacPasteboard::setMimeData(QMimeData *mime_src)
{
    if (!paste)
        return;

    if (mime == mime_src || (!mime_src && mime && mac_mime_source))
        return;
    mac_mime_source = false;
    delete mime;
    mime = mime_src;

    if (mime != 0) {
        clear();
        QStringList formats = mime_src->formats();

        if (!mime_src->hasUrls()){
            // Put available flavors-for-mime onto
            // the pasteboard, but see them all as the
            // same item (with ID == 1)
            for (int i = 0; i < formats.size(); ++i)
                putMimetypeOntoPasteboard(1, formats.at(i), mime_src);
        } else {
            // Special case: we put one item for
            // each URL onto the pasteboard
            QList<QUrl> urls = mime_src->urls();
            for (int itemId = 1; itemId < urls.size() + 1; ++itemId) {
                for (int i = 0; i < formats.size(); ++i)
                    putMimetypeOntoPasteboard(itemId, formats.at(i), mime_src);
            }
        }
    }
}

QStringList
QMacPasteboard::formats() const
{
    if (!paste)
        return QStringList();

    sync();

    QStringList ret;
    ItemCount cnt = 0;
    if(PasteboardGetItemCount(paste, &cnt) || !cnt)
        return ret;

#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: Formats [%d]", (int)cnt);
#endif
    for(uint index = 1; index <= cnt; ++index) {

        PasteboardItemID id;
        if(PasteboardGetItemIdentifier(paste, index, &id) != noErr)
            continue;

        QCFType<CFArrayRef> types;
        if(PasteboardCopyItemFlavors(paste, id, &types ) != noErr)
            continue;

        const int type_count = CFArrayGetCount(types);
        for(int i = 0; i < type_count; ++i) {
            const QString flavor = QCFString::toQString((CFStringRef)CFArrayGetValueAtIndex(types, i));
#ifdef DEBUG_PASTEBOARD
            qDebug(" -%s", qPrintable(QString(flavor)));
#endif
            QString mimeType = QMacPasteboardMime::flavorToMime(mime_type, flavor);
            if(!mimeType.isEmpty() && !ret.contains(mimeType)) {
#ifdef DEBUG_PASTEBOARD
                qDebug("   -<%d> %s [%s]", ret.size(), qPrintable(mimeType), qPrintable(QString(flavor)));
#endif
                ret << mimeType;
            }
        }
    }
    return ret;
}

bool
QMacPasteboard::hasFormat(const QString &format) const
{
    if (!paste)
        return false;

    sync();

    ItemCount cnt = 0;
    if(PasteboardGetItemCount(paste, &cnt) || !cnt)
        return false;

#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: hasFormat [%s]", qPrintable(format));
#endif
    for(uint index = 1; index <= cnt; ++index) {

        PasteboardItemID id;
        if(PasteboardGetItemIdentifier(paste, index, &id) != noErr)
            continue;

        QCFType<CFArrayRef> types;
        if(PasteboardCopyItemFlavors(paste, id, &types ) != noErr)
            continue;

        const int type_count = CFArrayGetCount(types);
        for(int i = 0; i < type_count; ++i) {
            const QString flavor = QCFString::toQString((CFStringRef)CFArrayGetValueAtIndex(types, i));
#ifdef DEBUG_PASTEBOARD
            qDebug(" -%s [0x%x]", qPrintable(QString(flavor)), mime_type);
#endif
            QString mimeType = QMacPasteboardMime::flavorToMime(mime_type, flavor);
#ifdef DEBUG_PASTEBOARD
            if(!mimeType.isEmpty())
                qDebug("   - %s", qPrintable(mimeType));
#endif
            if(mimeType == format)
                return true;
        }
    }
    return false;
}

QVariant
QMacPasteboard::retrieveData(const QString &format, QVariant::Type) const
{
    if (!paste)
        return QVariant();

    sync();

    ItemCount cnt = 0;
    if(PasteboardGetItemCount(paste, &cnt) || !cnt)
        return QByteArray();

#ifdef DEBUG_PASTEBOARD
    qDebug("Pasteboard: retrieveData [%s]", qPrintable(format));
#endif
    const QList<QMacPasteboardMime *> mimes = QMacPasteboardMime::all(mime_type);
    for(int mime = 0; mime < mimes.size(); ++mime) {
        QMacPasteboardMime *c = mimes.at(mime);
        QString c_flavor = c->flavorFor(format);
        if(!c_flavor.isEmpty()) {
            // Handle text/plain a little differently. Try handling Unicode first.
            if((c_flavor == QLatin1String("com.apple.traditional-mac-plain-text") || c_flavor == QLatin1String("public.utf8-plain-text")) &&
               hasFlavor(QLatin1String("public.utf16-plain-text")))
                c_flavor = QLatin1String("public.utf16-plain-text");

            QVariant ret;
            QList<QByteArray> retList;
            for(uint index = 1; index <= cnt; ++index) {
                PasteboardItemID id;
                if(PasteboardGetItemIdentifier(paste, index, &id) != noErr)
                    continue;

                QCFType<CFArrayRef> types;
                if(PasteboardCopyItemFlavors(paste, id, &types ) != noErr)
                    continue;

                const int type_count = CFArrayGetCount(types);
                for(int i = 0; i < type_count; ++i) {
                    CFStringRef flavor = static_cast<CFStringRef>(CFArrayGetValueAtIndex(types, i));
                    if(c_flavor == QCFString::toQString(flavor)) {
                        QCFType<CFDataRef> macBuffer;
                        if(PasteboardCopyItemFlavorData(paste, id, flavor, &macBuffer) == noErr) {
                            QByteArray buffer((const char *)CFDataGetBytePtr(macBuffer), CFDataGetLength(macBuffer));
                            if(!buffer.isEmpty()) {
#ifdef DEBUG_PASTEBOARD
                                qDebug("  - %s [%s] (%s)", qPrintable(format), qPrintable(QCFString::toQString(flavor)), qPrintable(c->convertorName()));
#endif
                                buffer.detach(); //detach since we release the macBuffer
                                retList.append(buffer);
                                break; //skip to next element
                            }
                        }
                    } else {
#ifdef DEBUG_PASTEBOARD
                        qDebug("  - NoMatch %s [%s] (%s)", qPrintable(c_flavor), qPrintable(QCFString::toQString(flavor)), qPrintable(c->convertorName()));
#endif
                    }
                }
            }

            if (!retList.isEmpty()) {
                ret = c->convertToMime(format, retList, c_flavor);
                return ret;
            }
        }
    }
    return QVariant();
}

void
QMacPasteboard::clear()
{
#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: clear!");
#endif
    if (paste)
        PasteboardClear(paste);
    promises.clear();
}

bool
QMacPasteboard::sync() const
{
    if (!paste)
        return false;
    const bool ret = PasteboardSynchronize(paste) & kPasteboardModified;
#ifdef DEBUG_PASTEBOARD
    if(ret)
        qDebug("Pasteboard: Syncronize!");
#endif
    return ret;
}



