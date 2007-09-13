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

#ifndef QMDIAREA_H
#define QMDIAREA_H

#include <QtGui/qabstractscrollarea.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_MDIAREA

class QMdiSubWindow;

class QMdiAreaPrivate;
class Q_GUI_EXPORT QMdiArea : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(WindowOrder activationOrder READ activationOrder WRITE setActivationOrder)
public:
    enum AreaOption {
        DontMaximizeSubWindowOnActivation = 0x1
    };
    Q_DECLARE_FLAGS(AreaOptions, AreaOption)

    enum WindowOrder {
        CreationOrder,
        StackingOrder,
        ActivationHistoryOrder
    };

    QMdiArea(QWidget *parent = 0);
    ~QMdiArea();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QMdiSubWindow *currentSubWindow() const;
    QMdiSubWindow *activeSubWindow() const;
    QList<QMdiSubWindow *> subWindowList(WindowOrder order = CreationOrder) const;

    QMdiSubWindow *addSubWindow(QWidget *widget, Qt::WindowFlags flags = 0);
    void removeSubWindow(QWidget *widget);

    QBrush background() const;
    void setBackground(const QBrush &background);

    WindowOrder activationOrder() const;
    void setActivationOrder(WindowOrder order);

    void setOption(AreaOption option, bool on = true);
    bool testOption(AreaOption opton) const;

Q_SIGNALS:
    void subWindowActivated(QMdiSubWindow *);

public Q_SLOTS:
    void setActiveSubWindow(QMdiSubWindow *window);
    void tileSubWindows();
    void cascadeSubWindows();
    void closeActiveSubWindow();
    void closeAllSubWindows();
    void activateNextSubWindow();
    void activatePreviousSubWindow();

protected Q_SLOTS:
    void setupViewport(QWidget *viewport);

protected:
    bool event(QEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *paintEvent);
    void childEvent(QChildEvent *childEvent);
    void resizeEvent(QResizeEvent *resizeEvent);
    void timerEvent(QTimerEvent *timerEvent);
    void showEvent(QShowEvent *showEvent);
    bool viewportEvent(QEvent *event);
    void scrollContentsBy(int dx, int dy);

private:
    Q_DISABLE_COPY(QMdiArea)
    Q_DECLARE_PRIVATE(QMdiArea)
    Q_PRIVATE_SLOT(d_func(), void _q_deactivateAllWindows())
    Q_PRIVATE_SLOT(d_func(), void _q_processWindowStateChanged(Qt::WindowStates, Qt::WindowStates))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMdiArea::AreaOptions)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_MDIAREA
#endif // QMDIAREA_H
