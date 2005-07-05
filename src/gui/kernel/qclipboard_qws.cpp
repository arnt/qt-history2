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
#include "qbuffer.h"
#include "qwidget.h"
#include "qevent.h"

#include <qwsdisplay_qws.h>
#include <qwsproperty_qws.h>
#include <qwsevent_qws.h>


/*****************************************************************************
  Internal QClipboard functions for Qt/Embedded
 *****************************************************************************/

static const int TextClipboard=424242;
static bool init=false;


static inline void qwsInitClipboard()
{
    //### this should go into QWSServer; it only needs to happen once.
    if (!init) {
        QPaintDevice::qwsDisplay()->addProperty(0, TextClipboard);
        init = true;
    }
}

#ifdef QT_NO_CLIPBOARD
static QString qwsClipboardText()
{
    char * data;
    int len;
    qwsInitClipboard();
    QPaintDevice::qwsDisplay()->getProperty(0, TextClipboard, data, len);
    //    qDebug("Property received: %d bytes", len);

    QString s((const QChar*)data, len/2);
    //    qDebug("Property received: '%s'", s.latin1());
    delete[] data;
    return s;
}


static void qwsSetClipboardText(const QString& s)
{
    qwsInitClipboard();
    QByteArray ba;
    int len =  s.length()*sizeof(QChar);
    ba.duplicate((const char*)s.unicode(), len);
    QPaintDevice::qwsDisplay()->
        setProperty(0, TextClipboard, QWSPropertyManager::PropReplace, ba);

}
#endif


static QWidget * owner = 0;

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if (owner)
        return;
    owner = new QWidget(0);
    owner->setObjectName("internal clibpoard owner");
    qAddPostRoutine(cleanup);
}


class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeData* s)
        { delete src; src = s; }
    QMimeData* source()
        { return src; }
    void addTransferredPixmap(QPixmap pm)
        { /* TODO: queue them */
            transferred[tindex] = pm;
            tindex=(tindex+1)%2;
        }
    void clearTransfers()
        {
            transferred[0] = QPixmap();
            transferred[1] = QPixmap();
        }

    void clear();

private:
    QMimeData* src;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    tindex=0;
}

QClipboardData::~QClipboardData()
{
    delete src;
}

void QClipboardData::clear()
{
    delete src;
    src = 0;
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


/*****************************************************************************
  QClipboard member functions for FB.
 *****************************************************************************/

#ifdef QT_NO_CLIPBOARD

QString QClipboard::text() const
{
    return qwsClipboardText();
}

void QClipboard::setText(const QString &text)
{
    qwsSetClipboardText(text);
}

QString QClipboard::text(QString& subtype) const
{
    QString r;
    if (subtype == "plain")
        r = text();
    return r;
}

#endif

void QClipboard::clear(Mode mode)
{
    setText(QString(), mode);
}


void QClipboard::ownerDestroyed()
{
}


void QClipboard::connectNotify(const char *)
{
}


bool QClipboard::event(QEvent *e)
{
    if (e->type() != QEvent::Clipboard)
        return QObject::event(e);

    QWSPropertyNotifyEvent *event = (QWSPropertyNotifyEvent *)(((QClipboardEvent *)e)->data());
    if (event && event->simpleData.state == QWSPropertyNotifyEvent::PropertyNewValue) {
        emit dataChanged();
    }

    return true;
}

const QMimeData* QClipboard::mimeData(Mode mode) const
{
    if (mode != Clipboard) return 0;

    QClipboardData *d = clipboardData();
    return d->source();
}

void QClipboard::setMimeData(QMimeData* src, Mode mode)
{
    if (mode != Clipboard) return;

    QClipboardData *d = clipboardData();
    setupOwner();

    d->setSource(src);
    emit dataChanged();
}

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
    qWarning("QClipboard::ownsClipboard: UNIMPLEMENTED!");
    return false;
}


#endif // QT_NO_CLIPBOARD
