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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_TOOLBAR_H
#define QDESIGNER_TOOLBAR_H

#include "shared_global_p.h"

#include <QtGui/QAction>
#include <QtGui/QToolButton>

class QToolBar;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT SentinelAction: public QAction
{
    Q_OBJECT
public:
    explicit SentinelAction(QWidget *widget);
    virtual ~SentinelAction();
};

// Dummy tool button used as 'insert before' parameter for  various commands
class QDESIGNER_SHARED_EXPORT Sentinel: public QToolButton
{
    Q_OBJECT
public:
    explicit Sentinel(QWidget *widget);
    virtual ~Sentinel();
};

// Special event filter for  tool bars in designer.
// Handles drag and drop to and from. Ensures that each
// child widget is  WA_TransparentForMouseEvents to enable  drag and drop.

class QDESIGNER_SHARED_EXPORT ToolBarEventFilter : public QObject {
    Q_OBJECT

public:
    static void install(QToolBar *tb);
    // Call after adding actions building the form to ensure the sentinel action is last
    static void adjustSpecialActions(QToolBar *tb);

    // Find action by position. Note that QToolBar::actionAt() will
    // not work as designer sets WA_TransparentForMouseEvents on its tool bar buttons
    // to be able to drag them. This function will return the dummy
    // sentinel action when applied to tool bars created by designer if the position matches.
    static QAction *actionAt(const QToolBar *tb, const QPoint &pos);
    static int actionIndexAt(const QToolBar *tb, const QPoint &pos);

    static bool withinHandleArea(const QToolBar *tb, const QPoint &pos);

    // Utility to create an action
    static QAction *createAction(QDesignerFormWindowInterface *fw, const QString &objectName, bool separator);

    virtual bool eventFilter (QObject *watched, QEvent *event);

private slots:
    void slotRemoveSelectedAction();
    void slotRemoveToolBar();
    void slotInsertSeparator();

private:
    explicit ToolBarEventFilter(QToolBar *tb);

    bool handleContextMenuEvent(QContextMenuEvent * event);
    bool handleDragEnterMoveEvent(QDragMoveEvent *event);
    bool handleDragLeaveEvent(QDragLeaveEvent *);
    bool handleDropEvent(QDropEvent *event);
    bool handleMousePressEvent(QMouseEvent *event);
    bool handleMouseReleaseEvent(QMouseEvent *event);
    bool handleMouseMoveEvent(QMouseEvent *event);

    QDesignerFormWindowInterface *formWindow() const;
    int findAction(const QPoint &pos) const;
    void adjustDragIndicator(const QPoint &pos);
    void hideDragIndicator();
    void startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers);
    void positionSentinel();
    bool withinHandleArea(const QPoint &pos) const;

    QToolBar *m_toolBar;
    QAction *m_sentinel;
    QPoint m_startPosition;
};
} // namespace qdesigner_internal

#endif // QDESIGNER_TOOLBAR_H
