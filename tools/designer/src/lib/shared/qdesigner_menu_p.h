/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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

#ifndef QDESIGNER_MENU_H
#define QDESIGNER_MENU_H

#include "shared_global_p.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QHash>

class QTimer;
class QLineEdit;

class QDesignerFormWindowInterface;
class QDesignerActionProviderExtension;
class QDesignerMenu;
class QDesignerMenuBar;
class QPainter;
class QMimeData;

namespace qdesigner_internal {
    class CreateSubmenuCommand;
    class RemoveActionFromCommand;
    class InsertActionIntoCommand;
}

class QDESIGNER_SHARED_EXPORT QDesignerMenu: public QMenu
{
    Q_OBJECT
public:
    QDesignerMenu(QWidget *parent = 0);
    virtual ~QDesignerMenu();

    bool eventFilter(QObject *object, QEvent *event);

    QDesignerFormWindowInterface *formWindow() const;
    QDesignerActionProviderExtension *actionProvider();

    QDesignerMenu *parentMenu() const;
    QDesignerMenuBar *parentMenuBar() const;

    virtual void setVisible(bool visible);

    void adjustSpecialActions();

    bool interactive(bool i);
    void createRealMenuAction(QAction *action);
    void removeRealMenu(QAction *action);

    static void drawSelection(QPainter *p, const QRect &r);

    bool dragging() const;

    void closeMenuChain();

    void moveLeft();
    void moveRight();
    void moveUp(bool ctrl);
    void moveDown(bool ctrl);

private slots:
    void slotRemoveSelectedAction(QAction *action);
    void slotShowSubMenuNow();
    void slotDeactivateNow();
    void slotAdjustSizeNow();

protected:
    virtual void actionEvent(QActionEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    bool handleEvent(QWidget *widget, QEvent *event);
    bool handleMouseDoubleClickEvent(QWidget *widget, QMouseEvent *event);
    bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
    bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);
    bool handleKeyPressEvent(QWidget *widget, QKeyEvent *event);

    void startDrag(const QPoint &pos);

    void adjustIndicator(const QPoint &pos);
    int findAction(const QPoint &pos) const;
    int actionAtPosition(const QPoint &pos) const;

    QAction *currentAction() const;
    int realActionCount() const;
    QAction *actionMimeData(const QMimeData *mimeData) const;
    bool checkAction(QAction *action) const;

    void updateCurrentAction();
    void showSubMenu(QAction *action);

    enum LeaveEditMode {
        Default = 0,
        ForceAccept
    };

    void enterEditMode();
    void leaveEditMode(LeaveEditMode mode);
    void showLineEdit();

    QAction *createAction(const QString &text, bool separator = false);
    QDesignerMenu *findOrCreateSubMenu(QAction *action);

    QAction *safeActionAt(int index) const;
    QAction *safeMenuAction(QDesignerMenu *menu) const;
    bool swap(int a, int b);

    void hideSubMenu();
    void deleteAction();
    void deactivateMenu();

    bool canCreateSubMenu(QAction *action) const;
    QDesignerMenu *findRootMenu() const;
    QDesignerMenu *findActivatedMenu() const;

    QRect subMenuPixmapRect(QAction *action) const;
    bool hasSubMenuPixmap(QAction *action) const;

private:
    QPoint m_startPosition;
    int m_currentIndex;
    QAction *m_addItem;
    QAction *m_addSeparator;
    QHash<QAction*, QDesignerMenu*> m_subMenus;
    QTimer *m_showSubMenuTimer;
    QTimer *m_deactivateWindowTimer;
    QTimer *m_adjustSizeTimer;
    bool m_interactive;
    QLineEdit *m_editor;
    bool m_dragging;
    int m_lastSubMenuIndex;

    friend class qdesigner_internal::CreateSubmenuCommand;
    friend class qdesigner_internal::RemoveActionFromCommand;
    friend class qdesigner_internal::InsertActionIntoCommand;
};

#endif // QDESIGNER_MENU_H
