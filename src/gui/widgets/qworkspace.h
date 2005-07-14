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

#ifndef QWORKSPACE_H
#define QWORKSPACE_H

#include "QtGui/qwidget.h"

QT_MODULE(Gui)

#ifndef QT_NO_WORKSPACE

class QAction;
class QWorkspaceChild;
class QShowEvent;
class QWorkspacePrivate;

class Q_GUI_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)

public:
    explicit QWorkspace(QWidget* parent=0);
    ~QWorkspace();

    enum WindowOrder { CreationOrder, StackingOrder };

    QWidget* activeWindow() const;
    QWidgetList windowList(WindowOrder order = CreationOrder) const;

    QWidget * addWindow(QWidget *w, Qt::WFlags flags = 0);

    QSize sizeHint() const;

    bool scrollBarsEnabled() const;
    void setScrollBarsEnabled(bool enable);

#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QWorkspace(QWidget* parent, const char* name);
    QT3_SUPPORT void setPaletteBackgroundColor(const QColor &);
    QT3_SUPPORT void setPaletteBackgroundPixmap(const QPixmap &);
#endif

    void setBackground(const QBrush &background);
    QBrush background() const;

signals:
    void windowActivated(QWidget* w);

public slots:
    void setActiveWindow(QWidget *w);
    void cascade();
    void tile();
    void closeActiveWindow();
    void closeAllWindows();
    void activateNextWindow();
    void activatePreviousWindow();

protected:
    void paintEvent(QPaintEvent *e);
    void changeEvent(QEvent *);
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *, QEvent *);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *e);
#endif

private:
    Q_DECLARE_PRIVATE(QWorkspace)
    Q_DISABLE_COPY(QWorkspace)
    Q_PRIVATE_SLOT(d_func(), void normalizeActiveWindow())
    Q_PRIVATE_SLOT(d_func(), void minimizeActiveWindow())
    Q_PRIVATE_SLOT(d_func(), void showOperationMenu())
    Q_PRIVATE_SLOT(d_func(), void popupOperationMenu(const QPoint&))
    Q_PRIVATE_SLOT(d_func(), void operationMenuActivated(QAction *))
    Q_PRIVATE_SLOT(d_func(), void updateActions())
    Q_PRIVATE_SLOT(d_func(), void scrollBarChanged())

    friend class QWorkspaceChild;
};

#endif // QT_NO_WORKSPACE

#endif // QWORKSPACE_H
