/****************************************************************************
**
** Definition of the QWorkspace class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
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

class QWorkspaceChild;
class QShowEvent;
class QWorkspacePrivate;

class Q_GUI_EXPORT QWorkspace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled)

public:
    QWorkspace(QWidget* parent=0, const char* name=0);

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
    void activatePrevWindow();

protected:
    void changeEvent(QEvent *);
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *, QEvent *);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *e);
#endif

private slots:
    void normalizeActiveWindow();
    void minimizeActiveWindow();
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);
    void operationMenuActivated(int);
    void operationMenuAboutToShow();
    void toolMenuAboutToShow();
    void activatePreviousWindow(); // ### remove in Qt 4.0
    void scrollBarChanged();

private:
    void init();
    void insertIcon(QWidget* w);
    void removeIcon(QWidget* w);
    void place(QWidget*);

    QWorkspaceChild* findChild(QWidget* w);
    void showMaximizeControls();
    void hideMaximizeControls();
    void activateWindow(QWidget* w, bool change_focus = true);
    void showWindow(QWidget* w);
    void maximizeWindow(QWidget* w);
    void minimizeWindow(QWidget* w);
    void normalizeWindow(QWidget* w);

    QRect updateWorkspace();

    QWorkspacePrivate* d;
    friend class QWorkspacePrivate;
    friend class QWorkspaceChild;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWorkspace(const QWorkspace &);
    QWorkspace& operator=(const QWorkspace &);
#endif
};


#endif // QT_NO_WORKSPACE

#endif // QWORKSPACE_H
