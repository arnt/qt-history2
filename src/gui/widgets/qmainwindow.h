/****************************************************************************
**
** Definition of Q3MainWindow class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#ifndef QT_H
#include "qwidget.h"
#include "qtoolbar.h"
#include "qtextstream.h"
#endif // QT_H

#ifndef QT_NO_MAINWINDOW

class QMenuBar;
class QStatusBar;
class QToolTipGroup;
class Q3MainWindowPrivate;
class Q3MainWindowLayout;
class QPopupMenu;
template<class T> class QList;

class Q_COMPAT_EXPORT Q3MainWindow: public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3MainWindow)

    Q_PROPERTY(bool rightJustification READ rightJustification WRITE setRightJustification DESIGNABLE false)
    Q_PROPERTY(bool usesBigPixmaps READ usesBigPixmaps WRITE setUsesBigPixmaps)
    Q_PROPERTY(bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel)
    Q_PROPERTY(bool dockWindowsMovable READ dockWindowsMovable WRITE setDockWindowsMovable)
    Q_PROPERTY(bool opaqueMoving READ opaqueMoving WRITE setOpaqueMoving)

public:
    Q3MainWindow(QWidget* parent=0, const char* name=0, WFlags f = WType_TopLevel);
    ~Q3MainWindow();

#ifndef QT_NO_MENUBAR
    QMenuBar * menuBar() const;
#endif
    QStatusBar * statusBar() const;
#if 0
    QToolTipGroup * toolTipGroup() const;
#endif

    virtual void setCentralWidget(QWidget *);
    QWidget * centralWidget() const;

    virtual void setDockEnabled(Dock dock, bool enable);
    bool isDockEnabled(Dock dock) const;
    bool isDockEnabled(Q3DockArea *area) const;
    virtual void setDockEnabled(Q3DockWindow *tb, Dock dock, bool enable);
    bool isDockEnabled(Q3DockWindow *tb, Dock dock) const;
    bool isDockEnabled(Q3DockWindow *tb, Q3DockArea *area) const;

    virtual void addDockWindow(Q3DockWindow *, Dock = DockTop, bool newLine = false);
    virtual void addDockWindow(Q3DockWindow *, const QString &label,
                                Dock = DockTop, bool newLine = false);
    virtual void moveDockWindow(Q3DockWindow *, Dock = DockTop);
    virtual void moveDockWindow(Q3DockWindow *, Dock, bool nl, int index, int extraOffset = -1);
    virtual void removeDockWindow(Q3DockWindow *);

    void show();
    void hide();
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    bool rightJustification() const;
    bool usesBigPixmaps() const;
    bool usesTextLabel() const;
    bool dockWindowsMovable() const;
    bool opaqueMoving() const;

    bool eventFilter(QObject*, QEvent*);

    bool getLocation(Q3DockWindow *tb, Dock &dock, int &index, bool &nl, int &extraOffset) const;

    QList<Q3DockWindow *> dockWindows(Dock dock) const;
    QList<Q3DockWindow *> dockWindows() const;
    void lineUpDockWindows(bool keepNewLines = false);

    bool isDockMenuEnabled() const;

    // compatibility stuff
    bool hasDockWindow(Q3DockWindow *dw);
#ifndef QT_NO_TOOLBAR
    void addToolBar(Q3DockWindow *, Dock = DockTop, bool newLine = false);
    void addToolBar(Q3DockWindow *, const QString &label,
                     Dock = DockTop, bool newLine = false);
    void moveToolBar(Q3DockWindow *, Dock = DockTop);
    void moveToolBar(Q3DockWindow *, Dock, bool nl, int index, int extraOffset = -1);
    void removeToolBar(Q3DockWindow *);

    bool toolBarsMovable() const;
    QList<Q3ToolBar *> toolBars(Dock dock) const;
    void lineUpToolBars(bool keepNewLines = false);
#endif

    virtual Q3DockArea *dockingArea(const QPoint &p);
    Q3DockArea *leftDock() const;
    Q3DockArea *rightDock() const;
    Q3DockArea *topDock() const;
    Q3DockArea *bottomDock() const;

    virtual bool isCustomizable() const;

    bool appropriate(Q3DockWindow *dw) const;

    enum DockWindows { OnlyToolBars, NoToolBars, AllDockWindows };
    virtual QPopupMenu *createDockWindowMenu(DockWindows dockWindows = AllDockWindows) const;

public slots:
    virtual void setRightJustification(bool);
    virtual void setUsesBigPixmaps(bool);
    virtual void setUsesTextLabel(bool);
    virtual void setDockWindowsMovable(bool);
    virtual void setOpaqueMoving(bool);
    virtual void setDockMenuEnabled(bool);
    virtual void enterWhatsThis();
    virtual void setAppropriate(Q3DockWindow *dw, bool a);
    virtual void customize();

    // compatibility stuff
    void setToolBarsMovable(bool);

signals:
    void pixmapSizeChanged(bool);
    void usesTextLabelChanged(bool);
    void dockWindowPositionChanged(Q3DockWindow *);

#ifndef QT_NO_TOOLBAR
    // compatibility stuff
    void toolBarPositionChanged(Q3ToolBar *);
#endif

protected slots:
    virtual void setUpLayout();
    virtual bool showDockMenu(const QPoint &globalPos);

protected:
    void paintEvent(QPaintEvent *);
    void childEvent(QChildEvent *);
    bool event(QEvent *);

private slots:
    void slotPlaceChanged();
    void doLineUp() { lineUpDockWindows(true); }

private:
    void triggerLayout(bool deleteLayout = true);
    bool dockMainWindow(QObject *dock) const;

#ifndef QT_NO_MENUBAR
    virtual void setMenuBar(QMenuBar *);
#endif
    virtual void setStatusBar(QStatusBar *);
#if 0
    virtual void setToolTipGroup(QToolTipGroup *);
#endif

    friend class Q3DockWindow;
    friend class QMenuBarPrivate;
#ifdef QT_COMPAT
    friend class Q3MenuBar;
#endif
    friend class QHideDock;
    friend class Q3ToolBar;
    friend class Q3MainWindowLayout;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3MainWindow(const Q3MainWindow &);
    Q3MainWindow& operator=(const Q3MainWindow &);
#endif
};

#ifndef QT_NO_TOOLBAR
inline void Q3MainWindow::addToolBar(Q3DockWindow *w, ToolBarDock dock, bool newLine)
{
    addDockWindow(w, dock, newLine);
}

inline void Q3MainWindow::addToolBar(Q3DockWindow *w, const QString &label,
                              ToolBarDock dock, bool newLine)
{
    addDockWindow(w, label, dock, newLine);
}

inline void Q3MainWindow::moveToolBar(Q3DockWindow *w, ToolBarDock dock)
{
    moveDockWindow(w, dock);
}

inline void Q3MainWindow::moveToolBar(Q3DockWindow *w, ToolBarDock dock, bool nl, int index, int extraOffset)
{
    moveDockWindow(w, dock, nl, index, extraOffset);
}

inline void Q3MainWindow::removeToolBar(Q3DockWindow *w)
{
    removeDockWindow(w);
}

inline bool Q3MainWindow::toolBarsMovable() const
{
    return dockWindowsMovable();
}

inline void Q3MainWindow::lineUpToolBars(bool keepNewLines)
{
    lineUpDockWindows(keepNewLines);
}

inline void Q3MainWindow::setToolBarsMovable(bool b)
{
    setDockWindowsMovable(b);
}
#endif

#ifndef QT_NO_TEXTSTREAM
Q_COMPAT_EXPORT QTextStream &operator<<(QTextStream &, const Q3MainWindow &);
Q_COMPAT_EXPORT QTextStream &operator>>(QTextStream &, Q3MainWindow &);
#endif

#endif // QT_NO_MAINWINDOW

#endif // QMAINWINDOW_H
