#include "qwsinputcontext_p.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"
#include "qevent.h"
#include "qtextformat.h"

#ifndef QT_NO_QWS_IM

static QWidget* activeWidget = 0;


QWSInputContext::QWSInputContext(QObject *parent)
    :QInputContext(parent)
{
}

void QWSInputContext::reset()
{
    QPaintDevice::qwsDisplay()->resetIM();
}


void QWSInputContext::setFocusWidget( QWidget *w )
{
  QWidget *oldFocus = focusWidget();
    if (oldFocus == w)
        return;

    //########### if (language() != QLatin1String("ja"))
        reset();

    // unsetfocus ???

    QInputContext::setFocusWidget(w);

    if (!w)
        return;

    //setfocus ???

    update();
}


void QWSInputContext::widgetDestroyed(QWidget *w)
{
    if (w == ::activeWidget)
        ::activeWidget = 0;
    QInputContext::widgetDestroyed(w);
}

void QWSInputContext::update()
{
    QWidget *w = focusWidget();
    if (!w)
        return;

    QPoint p = w->inputMethodQuery(Qt::ImMicroFocus).toRect().bottomLeft();

    QWidget *tlw = w->topLevelWidget();
    int winid = tlw->winId();
    QPoint gp = w->mapToGlobal( p );

    QRect r = QRect( w->mapToGlobal( QPoint(0,0) ),
                     w->size() );

    r.setBottom( tlw->geometry().bottom() );

    //qDebug( "QWidget::setMicroFocusHint %d %d %d %d", r.x(),
    //	r.y(),  r.width(), r.height() );

    QPaintDevice::qwsDisplay()->setIMInfo( winid, gp.x(), gp.y(), r);

}

void QWSInputContext::mouseHandler( int x, QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
        QPaintDevice::qwsDisplay()->sendIMMouseEvent( x, event->type() == QEvent::MouseButtonPress );
}

QWidget *QWSInputContext::activeWidget()
{
    return ::activeWidget;
}

bool QWSInputContext::translateIMEvent(QWidget *w, const QWSIMEvent *e)
{
    QString txt(e->text, e->simpleData.textLen);
    QInputContext *qic = w->inputContext();
    Q_ASSERT(qic);

    if (e->simpleData.type == QWSServer::InputMethodCompose) {
        const int cpos = qMax(0, qMin(e->simpleData.cpos, int(txt.length())));
        const int selLen = qMin(e->simpleData.selLen, int(txt.length())-cpos);

        QList<QInputMethodEvent::Attribute> attrs;
        if (cpos > 0)
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, cpos,
                                                  qic->standardFormat(QInputContext::PreeditFormat));
        if (selLen)
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, cpos, selLen,
                                                  qic->standardFormat(QInputContext::SelectionFormat));
        if (cpos + selLen < txt.length())
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                                                  cpos + selLen, txt.length() - cpos - selLen,
                                                  qic->standardFormat(QInputContext::PreeditFormat));

        QInputMethodEvent ime(txt, attrs);
        qt_sendSpontaneousEvent(w, &ime);
        if (txt.length() > 0)
            ::activeWidget = w;
        else
            ::activeWidget = 0;
    } else if (e->simpleData.type == QWSServer::InputMethodEnd) {
        QInputMethodEvent ime;
        ime.setCommitString(txt);
        qt_sendSpontaneousEvent(::activeWidget ? ::activeWidget : w, &ime);
        ::activeWidget = 0;
    }
}





#endif // QT_NO_QWS_IM
