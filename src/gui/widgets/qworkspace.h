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

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_WORKSPACE

class QAction;
class QWorkspaceChild;
class QShowEvent;
class QWorkspacePrivate;

class Q_GUI_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWorkspace)
    Q_PROPERTY(bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled)

public:
    QWorkspace(QWidget* parent=0);
#ifdef QT_COMPAT
    QWorkspace(QWidget* parent, const char* name);
#endif
    ~QWorkspace();

    enum WindowOrder { CreationOrder, StackingOrder };

    QWidget* activeWindow() const;
    QWidgetList windowList(WindowOrder order = CreationOrder) const;

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
    friend class QWorkspaceChild;
    Q_PRIVATE_SLOT(d, void normalizeActiveWindow());
    Q_PRIVATE_SLOT(d, void minimizeActiveWindow());
    Q_PRIVATE_SLOT(d, void showOperationMenu());
    Q_PRIVATE_SLOT(d, void popupOperationMenu(const QPoint&));
    Q_PRIVATE_SLOT(d, void operationMenuActivated(QAction *));
    Q_PRIVATE_SLOT(d, void updateActions());
    Q_PRIVATE_SLOT(d, void scrollBarChanged());

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWorkspace(const QWorkspace &);
    QWorkspace& operator=(const QWorkspace &);
#endif
};


#endif // QT_NO_WORKSPACE

#endif // QWORKSPACE_H
