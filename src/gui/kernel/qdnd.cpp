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

#include "qplatformdefs.h"

#include "qdrag.h"
#include "qpixmap.h"
#include "qevent.h"
#include "qfile.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qbuffer.h"
#include "qimageio.h"
#include "qimage.h"
#include "qregexp.h"
#include "qdir.h"
#include "qdnd_p.h"
#include <ctype.h>

// These pixmaps approximate the images in the Windows User Interface Guidelines.

// XPM

static const char * const move_xpm[] = {
"11 20 3 1",
".        c None",
#if defined(Q_WS_WIN)
"a        c #000000",
"X        c #FFFFFF", // Windows cursor is traditionally white
#else
"a        c #FFFFFF",
"X        c #000000", // X11 cursor is traditionally black
#endif
"aa.........",
"aXa........",
"aXXa.......",
"aXXXa......",
"aXXXXa.....",
"aXXXXXa....",
"aXXXXXXa...",
"aXXXXXXXa..",
"aXXXXXXXXa.",
"aXXXXXXXXXa",
"aXXXXXXaaaa",
"aXXXaXXa...",
"aXXaaXXa...",
"aXa..aXXa..",
"aa...aXXa..",
"a.....aXXa.",
"......aXXa.",
".......aXXa",
".......aXXa",
"........aa."};

/* XPM */
static const char * const copy_xpm[] = {
"24 30 3 1",
".        c None",
"a        c #000000",
"X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXaaaaaXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

/* XPM */
static const char * const link_xpm[] = {
"24 30 3 1",
".        c None",
"a        c #000000",
"X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXaaaaXXa",
".............aXXXXaaaXXa",
".............aXXXaaaaXXa",
".............aXXaaaXaXXa",
".............aXXaaXXXXXa",
".............aXXaXXXXXXa",
".............aXXXaXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

#ifndef QT_NO_DRAGANDDROP

//#define QDND_DEBUG

#ifdef QDND_DEBUG
QString dragActionsToString(QDrag::DropActions actions)
{
    QString str;
    if (actions == QDrag::IgnoreAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "IgnoreAction";
    }
    if (actions & QDrag::LinkAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "LinkAction";
    }
    if (actions & QDrag::CopyAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "CopyAction";
    }
    if (actions & QDrag::MoveAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "MoveAction";
    }
    if ((actions & QDrag::TargetMoveAction) == QDrag::TargetMoveAction ) {
        if (!str.isEmpty())
            str += " | ";
        str += "TargetMoveAction";
    }
    return str;
}

QString KeyboardModifiersToString(Qt::KeyboardModifiers moderfies)
{
    QString str;
    if (moderfies & Qt::ControlButton) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::ControlButton;
    }
    if (moderfies & Qt::AltButton) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::AltButton;
    }
    if (moderfies & Qt::ShiftButton) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::ShiftButton;
    }
    return str;
}
#endif


// the universe's only drag manager
QDragManager *QDragManager::instance = 0;


QDragManager::QDragManager()
    : QObject(qApp)
{
    Q_ASSERT(!instance);
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap((const char **)move_xpm);
    pm_cursor[1] = QPixmap((const char **)copy_xpm);
    pm_cursor[2] = QPixmap((const char **)link_xpm);
    object = 0;
    beingCancelled = false;
    restoreCursor = false;
    willDrop = false;
    eventLoop = 0;
    dropData = new QDropData();
}


QDragManager::~QDragManager()
{
#ifndef QT_NO_CURSOR
    if (restoreCursor)
        QApplication::restoreOverrideCursor();
#endif
    instance = 0;
    delete [] pm_cursor;
    delete dropData;
}

QDragManager *QDragManager::self()
{
    if (!instance && qApp)
        instance = new QDragManager;
    return instance;
}

QDrag::DropAction QDragManager::defaultAction(QDrag::DropActions possibleActions) const
{
    QDrag::DropAction defaultAction = QDrag::CopyAction;

    //### on windows these are not updated as part of the drag ... need to put a hook some where for this
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
#ifdef QDND_DEBUG
    qDebug("QDragManager::defaultAction(QDrag::DropActions possibleActions)");
    qDebug("keyboard modifiers : %s", KeyboardModifiersToString(modifiers).latin1());
#endif

#ifdef Q_WS_MAC
    if (modifiers & Qt::ControlButton && modifiers & Qt::AltButton)
        defaultAction = QDrag::LinkAction;
    else if (modifiers & Qt::AltButton)
        defaultAction = QDrag::CopyAction;
    else
        defaultAction = QDrag::MoveAction;
#else
    if (modifiers & Qt::ControlButton && modifiers & Qt::ShiftButton)
        defaultAction = QDrag::LinkAction;
    else if (modifiers & Qt::ControlButton)
        defaultAction = QDrag::CopyAction;
    else if (modifiers & Qt::ShiftButton)
        defaultAction = QDrag::MoveAction;
    else if (modifiers & Qt::AltButton)
        defaultAction = QDrag::LinkAction;
#endif

    // if the object is set take the list of possibles from it
    if (object)
        possibleActions = object->d_func()->possible_actions;

#ifdef QDND_DEBUG
    qDebug("possible actions : %s", dragActionsToString(possibleActions).latin1());
#endif

    // Check if the action determined is allowed
    if (!(possibleActions & defaultAction))
        defaultAction = QDrag::CopyAction;

#ifdef QDND_DEBUG
    qDebug("default action : %s", dragActionsToString(defaultAction).latin1());
#endif

    return defaultAction;
}

#endif


QDropData::QDropData()
    : QMimeData()
{
}

QDropData::~QDropData()
{
}
