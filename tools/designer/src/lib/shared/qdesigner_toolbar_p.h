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
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

class QTimer;
class QDesignerFormWindowInterface;
class QDesignerActionProviderExtension;

class QT_SHARED_EXPORT SentinelAction: public QAction
{
    Q_OBJECT
public:
    SentinelAction(QWidget *widget);
    virtual ~SentinelAction();
};

class QT_SHARED_EXPORT Sentinel: public QToolButton
{
    Q_OBJECT
public:
    Sentinel(QWidget *widget);
    virtual ~Sentinel();
};

class QT_SHARED_EXPORT QDesignerToolBar: public QToolBar
{
    Q_OBJECT
public:
    QDesignerToolBar(QWidget *parent = 0);
    virtual ~QDesignerToolBar();

    bool eventFilter(QObject *object, QEvent *event);

    QDesignerFormWindowInterface *formWindow() const;
    QDesignerActionProviderExtension *actionProvider();

private slots:
    void slotRemoveSelectedAction(QAction *action);
    void slotCheckSentinel();
    void slotNewToolBar();

protected:
    virtual void actionEvent(QActionEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    void startDrag(const QPoint &pos);
    bool handleEvent(QWidget *widget, QEvent *event);
    bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
    bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);

    void adjustIndicator(const QPoint &pos);
    int findAction(const QPoint &pos) const;
    bool isPassiveWidget(QWidget *widget) const;

    bool blockSentinelChecker(bool b);

private:
    QTimer *m_sentinelChecker;
    QAction *m_sentinel;
    bool m_blockSentinelChecker;
    QPoint m_startPosition;
};

#endif // QDESIGNER_TOOLBAR_H
