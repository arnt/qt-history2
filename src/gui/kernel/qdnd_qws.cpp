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

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"
#include "qdatetime.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qevent.h"
#include "qpainter.h"
#include "qdnd_p.h"

static QPixmap *defaultPm = 0;
static const int default_pm_hotx = -2;
static const int default_pm_hoty = -16;
static const char* default_pm[] = {
"13 9 3 1",
".      c None",
"       c #000000",
"X      c #FFFFFF",
"X X X X X X X",
" X X X X X X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X X X X X X ",
"X X X X X X X",
};

// Shift/Ctrl handling, and final drop status
static QDrag::DropAction drag_mode;
static QDrag::DropAction global_requested_action = QDrag::CopyAction;
static QDrag::DropAction global_accepted_action = QDrag::CopyAction;
static QDrag *drag_object;

static Qt::KeyboardModifiers oldstate;

class QShapedPixmapWidget : public QWidget {
    QPixmap pixmap;
public:
    QShapedPixmapWidget() :
        QWidget(0,Qt::WStyle_Customize | Qt::WStyle_Tool | Qt::WStyle_NoBorder | Qt::WX11BypassWM)
    {
    }

    void setPixmap(QPixmap pm)
    {
        pixmap = pm;
        if (pixmap.mask()) {
            setMask(*pixmap.mask());
        } else {
            clearMask();
        }
        resize(pm.width(),pm.height());
    }

    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        p.drawPixmap(0,0,pixmap);
    }
};

QShapedPixmapWidget *qt_qws_dnd_deco = 0;

void QDragManager::updatePixmap()
{
    if (qt_qws_dnd_deco) {
        QPixmap pm;
        QPoint pm_hot(default_pm_hotx,default_pm_hoty);
        if (drag_object) {
            pm = drag_object->pixmap();
            if (!pm.isNull())
                pm_hot = drag_object->hotSpot();
        }
        if (pm.isNull()) {
            if (!defaultPm)
                defaultPm = new QPixmap(default_pm);
            pm = *defaultPm;
        }
        qt_qws_dnd_deco->setPixmap(pm);
        qt_qws_dnd_deco->move(QCursor::pos()-pm_hot);
        if (willDrop) {
            qt_qws_dnd_deco->show();
        } else {
            qt_qws_dnd_deco->hide();
        }
    }
}

void QDragManager::timerEvent(QTimerEvent *) { }

void QDragManager::move(const QPoint &) { }

void QDragManager::updateCursor()
{
#ifndef QT_NO_CURSOR
    if (willDrop) {
        int cursorIndex = 0; // default is copy_cursor
        if (global_accepted_action == QDrag::CopyAction) {
            if (global_requested_action != QDrag::MoveAction)
                cursorIndex = 1; // move_cursor
        } else if (global_accepted_action == QDrag::LinkAction)
            cursorIndex = 2; // link_cursor
        if (qt_qws_dnd_deco)
            qt_qws_dnd_deco->show();
        QApplication::changeOverrideCursor(QCursor(pm_cursor[cursorIndex], 0, 0));
    } else {
        QApplication::changeOverrideCursor(QCursor(Qt::ForbiddenCursor));
        if (qt_qws_dnd_deco)
            qt_qws_dnd_deco->hide();
    }
#endif
}


bool QDragManager::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;

    switch(e->type()) {

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            QKeyEvent *ke = ((QKeyEvent*)e);
            if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress) {
                cancel();
                qApp->removeEventFilter(this);
            } else {
                updateMode(ke->modifiers());
                updateCursor();
            }
            return true; // Eat all key events
        }

        case QEvent::MouseButtonPress:
        case QEvent::MouseMove:
        {
            if (!object)
                return true; //####
            QMouseEvent *me = (QMouseEvent *)e;
            if (me->buttons()) {

                QWidget *cw = QApplication::widgetAt(me->globalPos());

                // Fix for when we move mouse on to the deco widget
                if (qt_qws_dnd_deco && cw == qt_qws_dnd_deco)
                    cw = object->target();

                if (object->target() != cw) {
                    if (object->target()) {
                        QDragLeaveEvent dle;
                        QApplication::sendEvent(object->target(), &dle);
                        willDrop = false;
                        updateCursor();
                        restoreCursor = true;
                        object->d_func()->target = 0;
                    }
                    if (cw && cw->acceptDrops()) {
                        object->d_func()->target = cw;
                        QDragEnterEvent dee(cw->mapFromGlobal(me->globalPos()), QDrag::CopyAction/*####*/, QDragManager::self()->dropData);
                        QApplication::sendEvent(object->target(), &dee);
                        willDrop = dee.isAccepted();
                        updateCursor();
                        restoreCursor = true;
                    }
                } else if (cw) {
                    QDragMoveEvent dme(cw->mapFromGlobal(me->globalPos()), QDrag::CopyAction/*####*/, QDragManager::self()->dropData);
                    QApplication::sendEvent(cw, &dme);
                    updatePixmap();
                }
            }
            return true; // Eat all mouse events
        }

        case QEvent::MouseButtonRelease:
        {
            qApp->removeEventFilter(this);
            if (qt_qws_dnd_deco)
                delete qt_qws_dnd_deco;
            qt_qws_dnd_deco = 0;
            if (restoreCursor) {
                willDrop = false;
                QApplication::restoreOverrideCursor();
                restoreCursor = false;
            }
            if (object && object->target()) {
                QMouseEvent *me = (QMouseEvent *)e;
                QDropEvent de(me->pos(), QDrag::CopyAction /*####*/, QDragManager::self()->dropData);
                QApplication::sendEvent(object->target(), &de);
                object->d_func()->target = 0;
            }
            return true; // Eat all mouse events
        }

        default:
             break;
    }

    return false;
}

QDrag::DropAction QDragManager::drag(QDrag *o)
{
    if (object == o || !o || !o->source())
         return QDrag::IgnoreAction;
    object = drag_object = o;
    qt_qws_dnd_deco = new QShapedPixmapWidget();
    oldstate = Qt::NoModifier; // #### Should use state that caused the drag
//    drag_mode = mode;
    global_accepted_action = QDrag::CopyAction; // #####
    willDrop = false;
    updateMode(0);
    updatePixmap();
    updateCursor();
    restoreCursor = true;
    object->d_func()->target = 0;
    qApp->installEventFilter(this);
    return QDrag::CopyAction;
}

void QDragManager::updateMode(Qt::KeyboardModifiers newstate)
{
    if (newstate == oldstate)
        return;
    const Qt::KeyboardModifiers both = Qt::ShiftModifier|Qt::ControlModifier;
    if ((newstate & both) == both) {
        global_requested_action = QDrag::LinkAction;
    } else {
        bool local = drag_object != 0;
        if (drag_mode == QDrag::MoveAction)
            global_requested_action = QDrag::MoveAction;
        else if (drag_mode == QDrag::CopyAction)
            global_requested_action = QDrag::CopyAction;
        else {                  //
            if (drag_mode == QDrag::MoveAction && local) //
                global_requested_action = QDrag::MoveAction;
            else
                global_requested_action = QDrag::CopyAction;
            if (newstate & Qt::ShiftModifier)
                global_requested_action = QDrag::MoveAction;
            else if (newstate & Qt::ControlModifier)
                global_requested_action = QDrag::CopyAction;
        }
    }
    oldstate = newstate;
}

void QDragManager::cancel(bool deleteSource)
{
    if (object->target()) {
        QDragLeaveEvent dle;
        QApplication::sendEvent(object->target(), &dle);
    }

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif

    if (drag_object) {
        if (deleteSource)
            delete object;
        drag_object = object = 0;
    }

    delete qt_qws_dnd_deco;
    qt_qws_dnd_deco = 0;
}


void QDragManager::drop()
{
    if (!object->target())
        return;

    delete qt_qws_dnd_deco;
    qt_qws_dnd_deco = 0;

    QDropEvent de(QCursor::pos(), QDrag::CopyAction /*####*/, QDragManager::self()->dropData);
    QApplication::sendEvent(object->target(), &de);

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
}

QVariant QDropData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    if (!drag_object)
        return QVariant();
    QByteArray data =  drag_object->mimeData()->data(mimetype);
    if (type == QVariant::String)
        return QString::fromUtf8(data);
    return data;
}

bool QDropData::hasFormat(const QString &format) const
{
    return formats().contains(format);
}

QStringList QDropData::formats() const
{
    if (drag_object)
        return drag_object->mimeData()->formats();
    return QStringList();
}


#endif // QT_NO_DRAGANDDROP

