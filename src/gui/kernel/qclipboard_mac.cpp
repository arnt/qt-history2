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
#include "qbitmap.h"
#include "qdatetime.h"
#include "qapplication_p.h"
#include <private/qt_mac_p.h>
#include "qevent.h"
#include <stdlib.h>
#include <string.h>

void qt_event_send_clipboard_changed(); //qapplication_mac.cpp

static ScrapRef scrap = 0;
static QWidget * owner = 0;

bool hasScrapChanged()
{
    ScrapRef nref;
    GetCurrentScrap(&nref);
    if(nref != scrap) {
        scrap = nref;
        return true;
    }
    return false;
}

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if(owner)
        return;
    owner = new QWidget(0);
    owner->setObjectName("internal clipboard owner");
    qAddPostRoutine(cleanup);
}

class QMacMimeData : public QMimeData
{
public:
    QVariant variantData(const QString &mime) { return retrieveData(mime, QVariant::Invalid); }
private:
    QMacMimeData();
};

class QClipboardWatcher : public QMimeData {
public:
    QClipboardWatcher();
    QStringList formats() const;
    QVariant retrieveData(const QString &format, QVariant::Type type) const;
};

class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource(QMimeData *s)
    {
        QMimeData *s2 = src;
        src = s;
        delete s2;
    }

    QMimeData *source() const { return src; }
    void clear();

    // private:
    QMimeData *src;
};

QClipboardData::QClipboardData()
{
    src = 0;
}

QClipboardData::~QClipboardData()
{
    clear();
}

void QClipboardData::clear()
{
    QMimeData *s2 = src;
    src = 0;
    delete s2;
}


static QClipboardData *internalCbData = 0;
static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if(internalCbData == 0) {
        internalCbData = new QClipboardData;
        qAddPostRoutine(cleanupClipboardData);
    }
    return internalCbData;
}

QClipboardWatcher::QClipboardWatcher() : QMimeData()
{
    setupOwner();
}

QStringList QClipboardWatcher::formats() const
{
    QStringList sl;
    QString mime;
    UInt32 cnt = 0;
    if(GetScrapFlavorCount(scrap, &cnt) || !cnt)
        return sl;

    ScrapFlavorInfo *infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
    if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
        qWarning("Qt: internal: Failure to collect ScrapFlavorInfoList..");
    } else {
        QMacMime::QMacMimeType qmt = QMacMime::MIME_CLIP;
        {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetScrapFlavorSize(scrap, qt_mac_mime_type, &sz) == noErr)
                qmt = QMacMime::MIME_QT_CONVERTOR;
        }
        for(int i = 0; i < (int)cnt; i++) {
            QString m = QMacMime::flavorToMime(qmt, infos[i].flavorType);
            if (!m.isEmpty())
                sl << m;
        }
        free(infos);
    }
    return sl;
}

QVariant QClipboardWatcher::retrieveData(const QString &format, QVariant::Type) const
{
    Size flavorsize=0;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_CLIP;
    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    if(GetScrapFlavorSize(scrap, qt_mac_mime_type, &flavorsize) == noErr)
        qmt = QMacMime::MIME_QT_CONVERTOR;
    QList<QMacMime *> all = QMacMime::all(qmt);
    for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
        QMacMime *c = (*it);
        int flav = c->flavorFor(format);
        if(flav) {
            if(GetScrapFlavorSize(scrap, flav, &flavorsize) == noErr) {
                char *buffer = (char *)malloc(flavorsize);
                GetScrapFlavorData(scrap, flav, &flavorsize, buffer);
                QByteArray r = QByteArray::fromRawData(buffer, flavorsize);
                QVariant tr;
                {
                    QList<QByteArray> lst;
                    lst.append(r);
                    tr = c->convertToMime(format, lst, flav);
                }
                tr.detach();
                free(buffer);
                return tr;
            }
        }
    }
    return QByteArray();
}


/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

void QClipboard::clear(Mode mode)
{
    if(mode != Clipboard) return;
    ClearCurrentScrap();
}

void QClipboard::ownerDestroyed()
{
    owner = 0;
    clipboardData()->clear();
    emit dataChanged();
}

static int clipWatcherId = -1;

void QClipboard::connectNotify(const char *signal)
{
    if(qstrcmp(signal,SIGNAL(dataChanged())) == 0 && clipWatcherId == -1)
        clipWatcherId = startTimer(100);
}

bool QClipboard::event(QEvent *e)
{
    bool check_clip = false;
    if(e->type() == QEvent::Clipboard) {
        check_clip = true;
    } else if(clipWatcherId != -1  && e->type() == QEvent::Timer) {
        QTimerEvent *te = (QTimerEvent *)e;
        if(te->timerId() == clipWatcherId) {
            if(!receivers(SIGNAL(dataChanged()))) {
                killTimer(clipWatcherId);
                clipWatcherId = -1;
            } else {
                check_clip = true;
            }
        }
    }
    if(check_clip && hasScrapChanged()) {
        clipboardData()->clear();
        emit dataChanged();
    }
    return QObject::event(e);
}

const QMimeData *QClipboard::mimeData(Mode mode) const
{
    if(mode != Clipboard)
        return 0;

    QClipboardData *d = clipboardData();
    if(!d->source())
        d->setSource(new QClipboardWatcher());
    return d->source();
}

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{
    if(mode != Clipboard)
        return;

    QClipboardData *d = clipboardData();
    d->setSource(src);
    ClearCurrentScrap();
    hasScrapChanged();

    QList<QMacMime*> all = QMacMime::all(QMacMime::MIME_CLIP);
    QStringList formats = src->formats();
    for (int i = 0; i < formats.size(); ++i) {
        QString mime = formats.at(i);
        for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
            QMacMime *c = (*it);
            if(c->flavorFor(mime)) {
                for (int j = 0; j < c->countFlavors(); j++) {
                    uint flav = c->flavor(j);
                    if(c->canConvert(mime, flav)) {
                        QList<QByteArray> md = c->convertFromMime(mime,
                                                  static_cast<QMacMimeData*>(src)->variantData(mime), flav);
                        if(md.count() > 1)
                            qWarning("QClipBoard: cannot handle multiple byte array conversions..");
                        PutScrapFlavor(scrap, (ScrapFlavorType)flav, 0, md.first().size(), md.first().data());
                    }
                }
            }
        }
    }

    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    PutScrapFlavor(scrap, qt_mac_mime_type, 0, 0, 0);
    emit dataChanged();
    qt_event_send_clipboard_changed();
}

bool QClipboard::supportsSelection() const
{
    return false; //nei takk
}

bool QClipboard::ownsSelection() const
{
    return false;
}

bool QClipboard::ownsClipboard() const
{
    qWarning("Qt: QClipboard::ownsClipboard: UNIMPLEMENTED!");
    return false;
}

void QClipboard::loadScrap(bool)
{
    LoadScrap();
}

void QClipboard::saveScrap()
{
    UnloadScrap();
}


#endif // QT_NO_CLIPBOARD
