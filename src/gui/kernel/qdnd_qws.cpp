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
static QDropEvent::Action global_requested_action = QDropEvent::Copy;
static QDropEvent::Action global_accepted_action = QDropEvent::Copy;
static QDragPrivate *drag_object;

static Qt::ButtonState oldstate;

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
        bitBlt(this,0,0,&pixmap);
    }
};

QShapedPixmapWidget *qt_qws_dnd_deco = 0;

void QDragManager::updatePixmap()
{
    if (qt_qws_dnd_deco) {
        QPixmap pm;
        QPoint pm_hot(default_pm_hotx,default_pm_hoty);
        if (drag_object) {
            pm = drag_object->pixmap;
            if (!pm.isNull())
                pm_hot = drag_object->hotspot;
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

void myOverrideCursor(QCursor cursor, bool replace) {
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(cursor, replace);
#endif
}

void myRestoreOverrideCursor() {
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void QDragManager::updateCursor()
{
#ifndef QT_NO_CURSOR
    if (willDrop) {
        int cursorIndex = 0; // default is copy_cursor
        if (global_accepted_action == QDropEvent::Copy) {
            if (global_requested_action != QDropEvent::Move)
                cursorIndex = 1; // move_cursor
        } else if (global_accepted_action == QDropEvent::Link)
            cursorIndex = 2; // link_cursor
        if (qt_qws_dnd_deco)
            qt_qws_dnd_deco->show();
        myOverrideCursor(QCursor(pm_cursor[cursorIndex], 0, 0), true);
    } else {
        myOverrideCursor(QCursor(Qt::ForbiddenCursor), true);
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
            if (me->state() & (Qt::LeftButton | Qt::MidButton | Qt::RightButton)) {

                QWidget *cw = QApplication::widgetAt(me->globalPos());

                // Fix for when we move mouse on to the deco widget
                if (qt_qws_dnd_deco && cw == qt_qws_dnd_deco)
                    cw = object->target;

                if (object->target != cw) {
                    if (object->target) {
                        QDragLeaveEvent dle;
                        QApplication::sendEvent(object->target, &dle);
                        willDrop = false;
                        updateCursor();
                        restoreCursor = true;
                        object->target = 0;
                    }
                    if (cw && cw->acceptDrops()) {
                        object->target = cw;
                        QDragEnterEvent dee(me->pos(), QDragManager::self()->dropData);
                        QApplication::sendEvent(object->target, &dee);
                        willDrop = dee.isAccepted();
                        updateCursor();
                        restoreCursor = true;
                    }
                } else if (cw) {
                    QDragMoveEvent dme(me->pos(), QDragManager::self()->dropData);
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
                myRestoreOverrideCursor();
                restoreCursor = false;
            }
            if (object && object->target) {
                QMouseEvent *me = (QMouseEvent *)e;
                QDropEvent de(me->pos(), QDragManager::self()->dropData);
                QApplication::sendEvent(object->target, &de);
                object->target = 0;
            }
            return true; // Eat all mouse events
        }

        default:
             break;
    }

    return false;
}

QDrag::DropAction QDragManager::drag(QDragPrivate * o, QDrag::DropAction mode)
{
    if (object == o || !o || !o->source)
         return QDrag::NoAction;
    object = drag_object = o;
    qt_qws_dnd_deco = new QShapedPixmapWidget();
    oldstate = Qt::ButtonState(-1); // #### Should use state that caused the drag
    drag_mode = mode;
    global_accepted_action = QDropEvent::Copy; // #####
    willDrop = false;
    updateMode(0);
    updatePixmap();
    updateCursor();
    restoreCursor = true;
    object->target = 0;
    qApp->installEventFilter(this);
    return QDrag::DefaultAction;
}

void QDragManager::updateMode(Qt::KeyboardModifiers newstate)
{
    if (newstate == oldstate)
        return;
    const int both = Qt::ShiftButton|Qt::ControlButton;
    if ((newstate & both) == both) {
        global_requested_action = QDropEvent::Link;
    } else {
        bool local = drag_object != 0;
        if (drag_mode == QDrag::MoveAction)
            global_requested_action = QDropEvent::Move;
        else if (drag_mode == QDrag::CopyAction)
            global_requested_action = QDropEvent::Copy;
        else {                  //
            if (drag_mode == QDrag::DefaultAction && local) //
                global_requested_action = QDropEvent::Move;
            else
                global_requested_action = QDropEvent::Copy;
            if (newstate & Qt::ShiftButton)
                global_requested_action = QDropEvent::Move;
            else if (newstate & Qt::ControlButton)
                global_requested_action = QDropEvent::Copy;
        }
    }
    oldstate = newstate;
}

void QDragManager::cancel(bool deleteSource)
{
    if (object->target) {
        QDragLeaveEvent dle;
        QApplication::sendEvent(object->target, &dle);
    }

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        myRestoreOverrideCursor();
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
    if (!object->target)
        return;

    delete qt_qws_dnd_deco;
    qt_qws_dnd_deco = 0;

    QDropEvent de(QCursor::pos(), QDragManager::self()->dropData);
    QApplication::sendEvent(object->target, &de);

#ifndef QT_NO_CURSOR
    if (restoreCursor) {
        myRestoreOverrideCursor();
        restoreCursor = false;
    }
#endif
}

QVariant QDropData::retrieveData(const QString &mimetype, QVariant::Type type) const
{
    if (!drag_object)
        return QVariant();
    QByteArray data =  drag_object->data->data(mimetype);
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
        return drag_object->data->formats();
}


#endif // QT_NO_DRAGANDDROP

