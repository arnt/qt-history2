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

#ifndef Q3WORKSPACE_H
#define Q3WORKSPACE_H

#include "qwidget.h"

#ifndef QT_NO_WORKSPACE

class QAction;
class QWorkspaceChild;
class QShowEvent;
class Q3WorkspacePrivate;

class Q_COMPAT_EXPORT Q3Workspace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled)

public:
    Q3Workspace(QWidget* parent=0);
    Q3Workspace(QWidget* parent, const char* name);
    ~Q3Workspace();

    enum WindowOrder { CreationOrder, StackingOrder };

    QWidget* activeWindow() const;
    QWidgetList windowList(WindowOrder order = CreationOrder) const;

    QWidget * addWindow(QWidget *w, Qt::WFlags flags = 0);

    QSize sizeHint() const;

    bool scrollBarsEnabled() const;
    void setScrollBarsEnabled(bool enable);

    void setPaletteBackgroundColor(const QColor &);
    void setPaletteBackgroundPixmap(const QPixmap &);

signals:
    void windowActivated(QWidget* w);

public slots:
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
    Q_DECLARE_PRIVATE(Q3Workspace)
    Q_DISABLE_COPY(Q3Workspace)
    Q_PRIVATE_SLOT(d, void normalizeActiveWindow())
    Q_PRIVATE_SLOT(d, void minimizeActiveWindow())
    Q_PRIVATE_SLOT(d, void showOperationMenu())
    Q_PRIVATE_SLOT(d, void popupOperationMenu(const QPoint&))
    Q_PRIVATE_SLOT(d, void operationMenuActivated(QAction *))
    Q_PRIVATE_SLOT(d, void updateActions())
    Q_PRIVATE_SLOT(d, void scrollBarChanged())

    friend class Q3WorkspaceChild;
};

#endif // QT_NO_WORKSPACE

#endif // Q3WORKSPACE_H
