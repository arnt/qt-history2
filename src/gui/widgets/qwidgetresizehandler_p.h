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

#ifndef QWIDGETRESIZEHANDLER_P_H
#define QWIDGETRESIZEHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qobject.h"
#include "qpoint.h"
#endif // QT_H
#ifndef QT_NO_RESIZEHANDLER
class QMouseEvent;
class QKeyEvent;

class Q_GUI_EXPORT QWidgetResizeHandler : public QObject
{
    Q_OBJECT

public:
    enum Action {
        Move        = 0x01,
        Resize        = 0x02,
        Any        = Move|Resize
    };

    QWidgetResizeHandler(QWidget *parent, QWidget *cw = 0);
    void setActive(bool b) { setActive(Any, b); }
    void setActive(Action ac, bool b);
    bool isActive() const { return isActive(Any); }
    bool isActive(Action ac) const;
    void setMovingEnabled(bool b) { moving = b; }
    bool isMovingEnabled() const { return moving; }

    bool isButtonDown() const { return buttonDown; }

    void setExtraHeight(int h) { extrahei = h; }
    void setSizeProtection(bool b) { sizeprotect = b; }

    void doResize();
    void doMove();

signals:
    void activate();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    enum MousePosition {
        Nowhere,
        TopLeft, BottomRight, BottomLeft, TopRight,
        Top, Bottom, Left, Right,
        Center
    };

    QWidget *widget;
    QWidget *childWidget;
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    MousePosition mode;
    int extrahei;
    int range;
    uint buttonDown            :1;
    uint moveResizeMode            :1;
    uint activeForResize    :1;
    uint sizeprotect            :1;
    uint moving                    :1;
    uint activeForMove            :1;

    void setMouseCursor(MousePosition m);
    bool isMove() const {
        return moveResizeMode && mode == Center;
    }
    bool isResize() const {
        return moveResizeMode && !isMove();
    }

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWidgetResizeHandler(const QWidgetResizeHandler &);
    QWidgetResizeHandler& operator=(const QWidgetResizeHandler &);
#endif

};

#endif //QT_NO_RESIZEHANDLER
#endif
