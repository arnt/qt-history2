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
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
  QClipboard debug facilities
 *****************************************************************************/
//#define DEBUG_PASTEBOARD


#ifdef QT3_SUPPORT
//# define PASTEBOARD_USE_QT3SUPPORT
#endif

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
    // Copy all promised data to the clip board before exiting.
    for (int i = 0; i < promises.count(); ++i) {
        const Promise &promise = promises.at(i);
        QCFString flavor = QCFString(promise.convertor->flavorFor(promise.mime));
        promiseKeeper(paste, (void *)i, flavor, this);
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

    { //protect the marker!
        extern int qt_mac_mime_type; //qmime_mac.cpp
        extern CFStringRef qt_mac_mime_typeUTI; //qmime_mac.cpp
        if(promise_id == qt_mac_mime_type && CFStringCompare(flavor, qt_mac_mime_typeUTI, 0) == 0) {
            QCFType<CFDataRef> data = CFDataCreate(0, (UInt8*)QT_VERSION_STR, strlen(QT_VERSION_STR));
            PasteboardPutItemFlavor(paste, id, flavor, data, kPasteboardFlavorNoFlags);
            return noErr;
        }
    }

    if(promise_id < 0 || promise_id >= qpaste->promises.size()) {
        qDebug("Pasteboard: %d: Unexpected [%ld]!", __LINE__, promise_id); //shouldn't happen
        return cantGetFlavorErr;
    }

    QMacPasteboard::Promise promise = qpaste->promises[promise_id];
    const QString flavorAsQString = QCFString::toQString(flavor);
#ifdef DEBUG_PASTEBOARD
    qDebug("PasteBoard: Calling in promise for %s[%ld] [%s] (%s)", qPrintable(promise.mime), promise_id,
           qPrintable(flavorAsQString),
           qPrintable(promise.convertor->convertorName()));
#endif
    if(!promise.convertor->canConvert(promise.mime, flavorAsQString)) {
        qDebug("Pasteboard: %d: Unexpected [%ld]!", __LINE__, promise_id); //shouldn't happen
        return cantGetFlavorErr;
    }
    QList<QByteArray> md = promise.convertor->convertFromMime(promise.mime, promise.data, flavorAsQString);
    for(int i = 0; i < md.size(); ++i) {
        const QByteArray &ba = md[i];
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

void
QMacPasteboard::setMimeData(QMimeData *mime_src)
{
    if (!paste)
        return;

    if(mime == mime_src || (!mime_src && mime && mac_mime_source))
        return;
    mac_mime_source = false;
    delete mime;
    mime = 0;

    if((mime = mime_src)) {
        clear();
#ifdef DEBUG_PASTEBOARD
        qDebug("PasteBoard: setMimeData [%p]", mime_src);
#endif
        QList<QMacPasteboardMime*> all = QMacPasteboardMime::all(mime_type);
        QStringList formats = mime_src->formats();
#ifdef DEBUG_PASTEBOARD
        for(int i = 0; i < formats.size(); ++i)
            qDebug(" - FOUND FORMAT <%s>", qPrintable(formats.at(i)));
#endif

        for (int i = 0, count = 0; i < formats.size(); ++i) {
            QString mimeType = formats.at(i);
            for(QList<QMacPasteboardMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
                QMacPasteboardMime *c = (*it);
                QString flavor(c->flavorFor(mimeType));
                if(!flavor.isEmpty()) {
                    promises.append(QMacPasteboard::Promise(c, mimeType,
                                                            static_cast<QMacMimeData*>(mime_src)->variantData(mimeType)));
#ifdef DEBUG_PASTEBOARD
                    qDebug(" -  adding %s[%d] [%s] <%s>", qPrintable(mimeType), count, qPrintable(flavor),
                           qPrintable(c->convertorName()));
#endif
                    PasteboardPutItemFlavor(paste, (PasteboardItemID)count++, QCFString(flavor), 0, kPasteboardFlavorNoFlags);
                }
            }
        }

        { //write out the marker
            extern int qt_mac_mime_type; //qmime_mac.cpp
            extern CFStringRef qt_mac_mime_typeUTI; //qmime_mac.cpp
            PasteboardPutItemFlavor(paste, (PasteboardItemID)qt_mac_mime_type, qt_mac_mime_typeUTI, 0, kPasteboardFlavorNoFlags);
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

    uchar qmt = mime_type;
    {
        extern CFStringRef qt_mac_mime_typeUTI; //qmime_mac.cpp
        if(hasFlavor(QCFString::toQString(qt_mac_mime_typeUTI))) {
            qmt = QMacPasteboardMime::MIME_QT_CONVERTOR;
#ifdef PASTEBOARD_USE_QT3SUPPORT
        } else {
            extern int qt_mac_mime_type; //qmime_mac.cpp
            if(hasOSType(qt_mac_mime_type))
                qmt = QMacPasteboardMime::MIME_QT3_CONVERTOR;
#endif
        }
    }

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
            qDebug(" -%s [0x%x]", qPrintable(QString(flavor)), qmt);
#endif
            QString mimeType = QMacPasteboardMime::flavorToMime(qmt, flavor);
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

    uchar qmt = mime_type;
    {
        extern CFStringRef qt_mac_mime_typeUTI; //qmime_mac.cpp
        if(hasFlavor(QCFString::toQString(qt_mac_mime_typeUTI))) {
            qmt = QMacPasteboardMime::MIME_QT_CONVERTOR;
#ifdef PASTEBOARD_USE_QT3SUPPORT
        } else {
            extern int qt_mac_mime_type; //qmime_mac.cpp
            if(hasOSType(qt_mac_mime_type))
                qmt = QMacPasteboardMime::MIME_QT3_CONVERTOR;
#endif
        }
    }

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
            qDebug(" -%s [0x%x]", qPrintable(QString(flavor)), qmt);
#endif
            QString mimeType = QMacPasteboardMime::flavorToMime(qmt, flavor);
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

    uchar qmt = mime_type;
    {
        extern CFStringRef qt_mac_mime_typeUTI; //qmime_mac.cpp
        if(hasFlavor(QCFString::toQString(qt_mac_mime_typeUTI))) {
            qmt = QMacPasteboardMime::MIME_QT_CONVERTOR;
#ifdef PASTEBOARD_USE_QT3SUPPORT
        } else {
            extern int qt_mac_mime_type; //qmime_mac.cpp
            if(hasOSType(qt_mac_mime_type))
                qmt = QMacPasteboardMime::MIME_QT3_CONVERTOR;
#endif
        }
    }

#ifdef DEBUG_PASTEBOARD
    qDebug("Pasteboard: retrieveData [%s]", qPrintable(format));
#endif
    const QList<QMacPasteboardMime *> mimes = QMacPasteboardMime::all(qmt);
    for(int mime = 0; mime < mimes.size(); ++mime) {
        QMacPasteboardMime *c = mimes.at(mime);
        QString c_flavor = c->flavorFor(format);
        if(!c_flavor.isEmpty()) {
            // Handle text/plain a little differently. Try handling Unicode first.
            if(c_flavor == QLatin1String("public.utf8-plain-text")
               && hasFlavor(QLatin1String("public.utf16-plain-text")))
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

                                // Need a copy since we'll be releasing the macBuffer
                                buffer.detach();
                                retList.append(buffer);
                                // We've got the data for this flavor, skip to next element
                                break;
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



