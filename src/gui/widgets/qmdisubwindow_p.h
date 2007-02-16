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

#ifndef QMDISUBWINDOW_P_H
#define QMDISUBWINDOW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmdisubwindow.h"

#include <QStyle>
#include <QStyleOptionTitleBar>
#include <QMenuBar>
#include <QSizeGrip>
#include <QPointer>
#include <QDebug>
#include <private/qwidget_p.h>

class QVBoxLayout;
class QMouseEvent;

template<typename T>
class ControlElement : public T
{
public:
    ControlElement(QMdiSubWindow *child) : T(0)
    {
        Q_ASSERT(child);
        mdiChild = child;
    }

    void *qt_metacast(const char *classname)
    {
        if (classname && strcmp(classname, "ControlElement") == 0)
            return this;
        return 0;
    }

    QPointer<QMdiSubWindow> mdiChild;
};

class ControlContainer : public QObject
{
public:
    ControlContainer(QMdiSubWindow *mdiChild);
    ~ControlContainer();

    void showButtonsInMenuBar(QMenuBar *menuBar);
    void removeButtonsFromMenuBar();
    QMenuBar *menuBar() const { return m_menuBar; }
    QWidget *controllerWidget() const { return m_controllerWidget; }
    QWidget *systemMenuLabel() const { return m_menuLabel; }

private:
    QPointer<QWidget> previousLeft;
    QPointer<QWidget> previousRight;
    QPointer<QMenuBar> m_menuBar;
    QPointer<QWidget> m_controllerWidget;
    QPointer<QWidget> m_menuLabel;
    QPointer<QMdiSubWindow> mdiChild;
};

class QMdiSubWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMdiSubWindow)
public:
    // Enums and typedefs.
    enum Operation {
        None,
        Move,
        TopResize,
        BottomResize,
        LeftResize,
        RightResize,
        TopLeftResize,
        TopRightResize,
        BottomLeftResize,
        BottomRightResize
    };

    enum ChangeFlag {
        HMove = 0x01,
        VMove = 0x02,
        HResize = 0x04,
        VResize = 0x08,
        HResizeReverse = 0x10,
        VResizeReverse = 0x20
    };

    enum WindowStateAction {
        RestoreAction,
        MoveAction,
        ResizeAction,
        MinimizeAction,
        MaximizeAction,
        StayOnTopAction,
        CloseAction,
        /* Add new states _above_ this line! */
        NumWindowStateActions
    };

    struct OperationInfo {
        uint changeFlags;
        Qt::CursorShape cursorShape;
        QRegion region;
        bool hover;
        OperationInfo(uint changeFlags, Qt::CursorShape cursorShape, bool hover = true)
            : changeFlags(changeFlags),
              cursorShape(cursorShape),
              hover(hover)
        {}
    };

    typedef QMap<Operation, OperationInfo> OperationInfoMap;

    QMdiSubWindowPrivate();

    // Variables.
    QPointer<QWidget> baseWidget;
    QPointer<QWidget> restoreFocusWidget;
    QPointer<ControlContainer> controlContainer;
#ifndef QT_NO_SIZEGRIP
    QPointer<QSizeGrip> sizeGrip;
#endif
    QRubberBand *rubberBand;
    QPoint mousePressPosition;
    QRect oldGeometry;
    QSize internalMinimumSize;
    QSize restoreSize;
    bool resizeEnabled;
    bool moveEnabled;
    bool isInInteractiveMode;
    bool isInRubberBandMode;
    bool isShadeMode;
    bool ignoreWindowTitleChange;
    bool isShadeRequestFromMinimizeMode;
    int keyboardSingleStep;
    int keyboardPageStep;
    Operation currentOperation;
    QStyle::SubControl hoveredSubControl;
    QStyle::SubControl activeSubControl;
    Qt::FocusReason focusInReason;
    OperationInfoMap operationMap;
    QPointer<QMenu> systemMenu;
    QPointer<QAction> actions[NumWindowStateActions];
    QMdiSubWindow::SubWindowOptions options;
    QString lastChildWindowTitle;

    // Slots.
    void _q_updateStaysOnTopHint();
    void _q_enterInteractiveMode();
    void _q_processFocusChanged(QWidget *old, QWidget *now);

    // Functions.
    void leaveInteractiveMode();
    void removeBaseWidget();
    void initOperationMap();
    void createSystemMenu();
    void updateCursor();
    void updateDirtyRegions();
    void updateGeometryConstraints();
    void updateMask();
    void setNewGeometry(const QPoint &pos);
    void setMinimizeMode();
    void setNormalMode();
    void setMaximizeMode();
    void setActive(bool activate);
    void processClickedSubControl();
    QRegion getRegion(Operation operation) const;
    Operation getOperation(const QPoint &pos) const;
    QStyleOptionTitleBar titleBarOptions() const;
    void ensureWindowState(Qt::WindowState state);
    int titleBarHeight(const QStyleOptionTitleBar &options) const;
    void sizeParameters(int *margin, int *minWidth) const;
    bool drawTitleBarWhenMaximized() const;
    void showButtonsInMenuBar(QMenuBar *menuBar);
    void removeButtonsFromMenuBar();
    void updateWindowTitle(bool requestFromChild);
    void enterRubberBandMode();
    void leaveRubberBandMode();
    QPalette desktopPalette() const;
    void updateActions();
    void setFocusWidget();
    void restoreFocus();
    void setWindowFlags(Qt::WindowFlags windowFlags);
    void setEnabled(WindowStateAction, bool enable = true);
    void setVisible(WindowStateAction, bool visible = true);
    void addToSystemMenu(WindowStateAction, const QString &text, const char *slot);
#ifndef QT_NO_SIZEGRIP
    void setSizeGrip(QSizeGrip *sizeGrip);
    void setSizeGripVisible(bool visible = true) const;
#endif

    inline int titleBarHeight() const
    {
        Q_Q(const QMdiSubWindow);
        if (!q->parent() || q->windowFlags() & Qt::FramelessWindowHint
            || (q->isMaximized() && !drawTitleBarWhenMaximized())) {
            return 0;
        }
        QStyleOptionTitleBar options = titleBarOptions();
        int height = options.rect.height();
        if (hasBorder(options))
            height += q->isMinimized() ? 8 : 4;
        return height;
    }

    inline QStyle::SubControl getSubControl(const QPoint &pos) const
    {
        Q_Q(const QMdiSubWindow);
        QStyleOptionTitleBar titleBarOptions = this->titleBarOptions();
        return q->style()->hitTestComplexControl(QStyle::CC_TitleBar, &titleBarOptions, pos, q);
    }

    inline void setNewGeometry(QRect *geometry)
    {
        Q_Q(QMdiSubWindow);
        Q_ASSERT(q->parent());
        geometry->setSize(geometry->size().expandedTo(internalMinimumSize));
        if (isInRubberBandMode)
            rubberBand->setGeometry(*geometry);
        else
            q->setGeometry(*geometry);
    }

    inline bool hasBorder(const QStyleOptionTitleBar &options) const
    {
        Q_Q(const QMdiSubWindow);
        return !q->style()->styleHint(QStyle::SH_TitleBar_NoBorder, &options, q);
    }

    inline bool autoRaise() const
    {
        Q_Q(const QMdiSubWindow);
        return q->style()->styleHint(QStyle::SH_TitleBar_AutoRaise, 0, q);
    }

    inline bool isResizeOperation() const
    {
        return currentOperation != None && currentOperation != Move;
    }

    inline bool isMoveOperation() const
    {
        return currentOperation == Move;
    }
};

#endif // QMDISUBWINDOW_P_H
