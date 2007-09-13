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

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE


/*****************************************************************************
  Internal QClipboard functions for Qtopia Core
 *****************************************************************************/

static const int TextClipboard=424242;
static bool init = false;

static inline void qwsInitClipboard()
{
    //### this should go into QWSServer; it only needs to happen once.
    if( !init ) {
	QPaintDevice::qwsDisplay()->addProperty(0, TextClipboard);
	init = true;
    }
}

static QString qwsClipboardText()
{
    char * data;
    int len;
    qwsInitClipboard();
    if( !QPaintDevice::qwsDisplay()->getProperty(0, TextClipboard, data, len) ) {
//        qDebug("Property received: %d bytes", len);
    }

    QString s((const QChar*)data, len/sizeof(QChar));
 //       qDebug("Property received: '%s'", s.toAscii().constData());
    delete[] data;
    return s;
}


static void qwsSetClipboardText(const QString& s)
{
    qwsInitClipboard();
  //  qDebug("qwsSetClipboardText( %s )", s.toAscii().data());
    int len =  s.length()*sizeof(QChar);
    QByteArray ba((const char*)s.unicode(), len);
    QPaintDevice::qwsDisplay()->
        setProperty(0, TextClipboard, QWSPropertyManager::PropReplace, ba);

}

class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    void setSource(QMimeData* s)
    {
        if (s == src)
            return;
        delete src;
        src = s;
    }
    QMimeData* source()
        { return src; }
#if 0
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
#endif

    void clear();

private:
    QMimeData* src;

#if 0
    QPixmap transferred[2];
    int tindex;
#endif
};

QClipboardData::QClipboardData()
{
    src = 0;
#if 0
    tindex=0;
#endif
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

#if 0

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


bool QClipboard::event(QEvent *e)
{
    static bool recursionWatch = false;
    if (e->type() != QEvent::Clipboard || recursionWatch)
        return QObject::event(e);

    recursionWatch = true;
    QWSPropertyNotifyEvent *event = (QWSPropertyNotifyEvent *)(((QClipboardEvent *)e)->data());
    if (event && event->simpleData.state == QWSPropertyNotifyEvent::PropertyNewValue) {
	QClipboardData *d = clipboardData();
	QString t = qwsClipboardText();
	if( (d->source() == 0 && !t.isEmpty()) || (d->source() != 0 && d->source()->text() != t) ) {
	    if( !d->source() )
		d->setSource(new QMimeData);
	    d->source()->setText( t );
	    emitChanged(QClipboard::Clipboard);
	}
    }

    recursionWatch = false;
    return true;
}

const QMimeData* QClipboard::mimeData(Mode mode) const
{
    if (mode != Clipboard) return 0;

    QClipboardData *d = clipboardData();
    // Try and get data from QWSProperty if no mime data has been set on us.
    if( !d->source() ) {
	QString t = qwsClipboardText();
	if( !t.isEmpty() ) {
	    QMimeData* nd = new QMimeData;
	    nd->setText( t );
	    d->setSource( nd );
	}
    }
    return d->source();
}

void QClipboard::setMimeData(QMimeData* src, Mode mode)
{
    if (mode != Clipboard) return;

    QClipboardData *d = clipboardData();

    /* Propagate text data to other QWSClients */

    QString newText;
    if( src != 0 )
	newText = src->text();
    QString oldText;
    if( d->source() != 0 )
	oldText = d->source()->text();

    d->setSource(src);

    if( oldText != newText ) {
	if( d->source() == 0 ) {
	    qwsSetClipboardText( QString() );
	} else {
	    qwsSetClipboardText( d->source()->text() );
	}
    }

    emitChanged(QClipboard::Clipboard);
}

bool QClipboard::supportsMode(Mode mode) const
{
    return (mode == Clipboard);
}

bool QClipboard::ownsMode(Mode mode) const
{
    if (mode == Clipboard)
        qWarning("QClipboard::ownsClipboard: UNIMPLEMENTED!");
    return false;
}

void QClipboard::connectNotify( const char * )
{
}

void QClipboard::ownerDestroyed()
{
}

#endif // QT_NO_CLIPBOARD

QT_END_NAMESPACE
