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

#ifndef QWORKSPACE_H
#define QWORKSPACE_H

#include <QAbstractScrollArea>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QMdiSubWindow;

class QWorkspacePrivate;
class Q_GUI_EXPORT QWorkspace : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
public:
    enum WindowOrder {
        CreationOrder,
        StackingOrder
    };

    explicit QWorkspace(QWidget *parent = 0);
    ~QWorkspace();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QWidget *activeWindow() const;
    QMdiSubWindow *activeSubWindow() const;

    QWidgetList windowList(WindowOrder order = CreationOrder) const;
    QList<QMdiSubWindow *> subWindowList(WindowOrder order = CreationOrder) const;

    QWidget *addWindow(QWidget *childWidget, Qt::WindowFlags flags = 0);

    QMdiSubWindow *addSubWindow(QWidget *widget, Qt::WindowFlags flags = 0);
    void removeSubWindow(QWidget *widget);

    void addChildWindow(QMdiSubWindow *mdiChild);
    void removeChildWindow(QMdiSubWindow *mdiChild);

    bool scrollBarsEnabled() const;
    void setScrollBarsEnabled(bool enable);

    QBrush background() const;
    void setBackground(const QBrush &background);

#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QWorkspace(QWidget* parent, const char* name);
    QT3_SUPPORT void setPaletteBackgroundColor(const QColor &);
    QT3_SUPPORT void setPaletteBackgroundPixmap(const QPixmap &);
#endif

Q_SIGNALS:
    void windowActivated(QWidget *);
    void subWindowActivated(QMdiSubWindow *);

public Q_SLOTS:
    void setActiveWindow(QWidget *window);
    void setActiveSubWindow(QMdiSubWindow *window);
    void tile();
    void cascade();
    void arrangeIcons();
    void closeActiveWindow();
    void closeAllWindows();
    void activateNextWindow();
    void activatePreviousWindow();

protected Q_SLOTS:
    void setupViewport(QWidget *viewport);

protected:
    bool event(QEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *paintEvent);
    void childEvent(QChildEvent *childEvent);
    void resizeEvent(QResizeEvent *resizeEvent);
    bool viewportEvent(QEvent *event);
    void scrollContentsBy(int dx, int dy);

private:
    Q_DECLARE_PRIVATE(QWorkspace)
    Q_PRIVATE_SLOT(d_func(), void _q_deactivateAllWindows())
    Q_PRIVATE_SLOT(d_func(), void _q_processWindowStateChanged(Qt::WindowStates, Qt::WindowStates))
};

QT_END_HEADER

#endif // QWORKSPACE_H
