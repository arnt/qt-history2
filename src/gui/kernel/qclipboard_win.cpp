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

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

#include "qapplication.h"
#include "qapplication_p.h"
#include "qeventloop.h"
#include "qwidget.h"
#include "qevent.h"
#include "qpixmap.h"
#include "qdatetime.h"
#include "qimage.h"
#include "qmime.h"
#include "qt_windows.h"


/*****************************************************************************
  Internal QClipboard functions for Win32.
 *****************************************************************************/

static HWND nextClipboardViewer = 0;
static bool inClipboardChain = false;
static QWidget *qt_cb_owner = 0;

static QWidget *clipboardOwner()
{
    if (!qt_cb_owner) {
        qt_cb_owner = new QWidget(0);
        qt_cb_owner->setObjectName("internal clipboard owner");
    }
    return qt_cb_owner;
}


typedef uint ClipboardFormat;

#define CFText            CF_TEXT
#define CFPixmap    CF_BITMAP
#define CFNothing   0


/*****************************************************************************
  QClipboard member functions for Win32.
 *****************************************************************************/

bool QClipboard::supportsSelection() const
{
    return false;
}


bool QClipboard::ownsSelection() const
{
    return false;
}


bool QClipboard::ownsClipboard() const
{
    return qt_cb_owner && GetClipboardOwner() == qt_cb_owner->winId();
}

void QClipboard::ownerDestroyed()
{
    if (inClipboardChain) {
        QWidget *owner = (QWidget *)sender();
        ChangeClipboardChain(owner->winId(), nextClipboardViewer);
    }
}


void QClipboard::connectNotify(const char *signal)
{
    if (qstrcmp(signal,SIGNAL(dataChanged())) == 0 && !inClipboardChain) {
        QWidget *owner = clipboardOwner();
        inClipboardChain = true;
        nextClipboardViewer = SetClipboardViewer(owner->winId());
#ifndef QT_NO_DEBUG
        if (!nextClipboardViewer)
            qErrnoWarning("QClipboard::connectNotify: Failed to set clipboard viewer");
#endif
        connect(owner, SIGNAL(destroyed()), SLOT(ownerDestroyed()));
    }
}

class QClipboardWatcher : public QMimeData {
public:
    QClipboardWatcher() 
        : QMimeData(0)
    {
    }

    bool hasFormat(const QString &mimetype) const;
    QStringList formats() const;
    QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const;
};


bool QClipboardWatcher::hasFormat(const QString &mime) const
{
    bool r = false;
    if (OpenClipboard(clipboardOwner()->winId())) {
        int cf = 0;
        while (!r && (cf = EnumClipboardFormats(cf))) {
            if (QWindowsMime::convertor(mime,cf))
                r = true;
        }
        if (!CloseClipboard())
            qErrnoWarning("QClipboardWatcher::provides: Failed to close clipboard");
    } else {
        qErrnoWarning("QClipboardWatcher::provides: Failed to open clipboard");
    }

    return r;
}

    // Not efficient to iterate over this (so we provide provides() above).

    // In order to be able to use UniCode characters, we have to give
    // it a higher priority than single-byte characters.  To do this, we
    // treat CF_TEXT as a special case and postpone its processing until
    // we're sure we do not have CF_UNICODETEXT
QStringList QClipboardWatcher::formats() const
{
    QStringList fmts;

    if (OpenClipboard(clipboardOwner()->winId())) {
        int cf = 0;
        while ((cf = EnumClipboardFormats(cf))) {
            QString mime = QWindowsMime::cfToMime(cf);
            if (!mime.isEmpty())
                fmts.append(mime);
        }
        CloseClipboard();
    }
    return fmts;
}

QVariant QClipboardWatcher::retrieveData(const QString &mime, QVariant::Type type) const
{
    if (!OpenClipboard(clipboardOwner()->winId())) {
        qErrnoWarning("QClipboardWatcher::retrieveData: Failed to open Clipboard");
        return QVariant();
    }

    QByteArray r;

    QList<QWindowsMime*> all = QWindowsMime::all();
    for (int i = 0; i<all.size(); ++i) {
        QWindowsMime *c = all[i];
        int cf = c->cfFor(mime);
        if (cf) {
            HANDLE h = GetClipboardData(cf);
            if (h) {
                const QByteArray cr = QByteArray::fromRawData((char *)GlobalLock(h), GlobalSize(h));
                r = c->convertToMime(cr,mime,cf);
                GlobalUnlock(h);
                break;
            } else {
                qErrnoWarning("QClipboardWatcher::encodedData: Failed to read clipboard data");
            }
        }
    }
    CloseClipboard();
    return r;
}



class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeData* s)
    {
        delete src;
        src = s;
    }
    QMimeData* source()
    {
        return src;
    }
    QMimeData* provider()
    {
        if (!prov)
            prov = new QClipboardWatcher();
        return prov;
    }

private:
    QMimeData *src;
    QMimeData *prov;
};

QClipboardData::QClipboardData()
{
    src = 0;
    prov = 0;
}

QClipboardData::~QClipboardData()
{
    delete src;
    delete prov;
}

static QClipboardData *internalCbData = 0;

static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if (internalCbData == 0) {
        internalCbData = new QClipboardData;
        qAddPostRoutine(cleanupClipboardData);
    }
    return internalCbData;
}


//#define QT_DEBUG_CB

static void setClipboardData(int cf, const QString &mime, QWindowsMime *c, QMimeData *s)
{
    QByteArray md = s->data(mime);
#if defined(QT_DEBUG_CB)
    qDebug("source is %d bytes of %s",md.size(),mime);
#endif
    md = c->convertFromMime(md, mime, cf);
    int len = md.size();
#if defined(QT_DEBUG_CB)
    qDebug("rendered %d bytes of CF %d by %s",len,cf,c->convertorName());
#endif
    HANDLE h = GlobalAlloc(GHND, len);
    char *d = (char *)GlobalLock(h);
    memcpy(d, md.data(), len);
    if (!SetClipboardData(cf, h))
        qErrnoWarning("setClipboardData: Failed to write data");
    GlobalUnlock(h);
}

static void renderFormat(int cf)
{
#if defined(QT_DEBUG_CB)
    qDebug("renderFormat(%d)",cf);
#endif
    if (!internalCbData)                        // Spurious Windows message
        return;
    QMimeData *s = internalCbData->source();
    if (!s)                                        // Spurious Windows message
        return;
    QString mime;
    QStringList fmts = s->formats();
    for (int i=0; i < fmts.size(); i++) {
        QString mime = fmts.at(i);
        QWindowsMime* c = QWindowsMime::convertor(mime, cf);
        if (c) {
            setClipboardData(cf, mime, c, s);
            return;
        }
    }
}

static bool ignore_empty_clipboard = false;

static void renderAllFormats()
{
#if defined(QT_DEBUG_CB)
    qDebug("renderAllFormats");
#endif
    if (!internalCbData)                        // Spurious Windows message
        return;
    QMimeData *s = internalCbData->source();
    if (!s)                                        // Spurious Windows message
        return;

    if (!qt_cb_owner)
        return;
    if (!OpenClipboard(qt_cb_owner->winId())) {
        qErrnoWarning("renderAllFormats: couldn't open clipboard");
        return;
    }

    ignore_empty_clipboard = true;
    EmptyClipboard();
    ignore_empty_clipboard = false;

    QList<QWindowsMime*> all = QWindowsMime::all();
    QStringList fmts = s->formats();
    for (int i = 0; i < fmts.size(); i++) {
        QString mime = fmts.at(i);
        for (int pos = 0; pos < all.size(); ++pos) {
            QWindowsMime* c = all[pos];
            if (c->cfFor(mime)) {
                for (int j = 0; j < c->countCf(); j++) {
                    int cf = c->cf(j);
                    if (c->canConvert(mime,cf)) {
                        setClipboardData(cf, mime, c, s);
                    }
                }
            }
        }
    }

    CloseClipboard();
}

QClipboard::~QClipboard()
{
    renderAllFormats();
    delete qt_cb_owner;
    qt_cb_owner = 0;
}

bool QClipboard::event(QEvent *e)
{
    if (e->type() != QEvent::Clipboard)
        return QObject::event(e);

    MSG *m = (MSG *)((QCustomEvent*)e)->data();
    if (!m) {
        renderAllFormats();
        return true;
    }

    bool propagate=false;
    switch (m->message) {

        case WM_CHANGECBCHAIN:
            if ((HWND)m->wParam == nextClipboardViewer)
                nextClipboardViewer = (HWND)m->lParam;
            else
                propagate = true;
            break;

        case WM_DRAWCLIPBOARD:
            propagate = true;
            emit dataChanged();
            break;

        case WM_DESTROYCLIPBOARD:
            if (!ignore_empty_clipboard)
                cleanupClipboardData();
            break;

        case WM_RENDERFORMAT:
            renderFormat(m->wParam);
            break;
        case WM_RENDERALLFORMATS:
            renderAllFormats();
            break;
    }
    if (propagate && nextClipboardViewer) {
        QT_WA({
            SendMessage(nextClipboardViewer, m->message,
                         m->wParam, m->lParam);
        } , {
            SendMessageA(nextClipboardViewer, m->message,
                         m->wParam, m->lParam);
        });
    }

    return true;
}


void QClipboard::clear(Mode mode)
{
    if (mode != Clipboard) return;

    if (OpenClipboard(clipboardOwner()->winId())) {
        EmptyClipboard();
        CloseClipboard();
    }
}


const QMimeData *QClipboard::mimeData(Mode mode) const
{
    if (mode != Clipboard) 
        return 0;

    QClipboardData *d = clipboardData();
    return d->provider();
}

extern bool qt_CF_HDROP_valid(const QString &mime, int cf, QMimeData * src);

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{
    if (mode != Clipboard) 
        return;

    if (!OpenClipboard(clipboardOwner()->winId())) {
        qErrnoWarning("QClipboard::setData: Failed to open clipboard");
        return;
    }

    QClipboardData *d = clipboardData();
    d->setSource(src);

    ignore_empty_clipboard = true;
    if (!EmptyClipboard())
        qErrnoWarning("QClipboard::setData: Failed to empty clipboard");

    ignore_empty_clipboard = false;
    // Register all the formats of src that we can render.
    QList<QWindowsMime*> all = QWindowsMime::all();
    QStringList fmts = src->formats();
    for (int i = 0; i < fmts.size(); i++) {
        QString mime = fmts.at(i);
        for (int pos = 0; pos < all.size(); ++pos) {
            QWindowsMime* c = all[pos];
            if (c->cfFor(mime)) {
                for (int j = 0; j < c->countCf(); j++) {
                    UINT cf = c->cf(j);
                    if (c->canConvert(mime,cf) /*&& qt_CF_HDROP_valid(mime, cf, src) ####### */) {
#ifndef Q_OS_TEMP
                        if (qApp && qApp->eventLoop()->loopLevel())
                            SetClipboardData(cf, 0); // 0 == ask me later
                        else // write now if we can't process data requests
#endif
                            setClipboardData(cf, mime, c, src);
                    }
                }
            }
        }
    }

    CloseClipboard();
}

#endif // QT_NO_CLIPBOARD
