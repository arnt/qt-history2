/****************************************************************************
**
** Definition of QMenu and QMenuBar classes.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QMENU_H__
#define __QMENU_H__

#include <qwidget.h>
#include <qstring.h>
#include <qiconset.h>
#include <qaction.h>

class Q4MenuPrivate;
class Q4MenuBarPrivate;

class Q_GUI_EXPORT Q4Menu : public QWidget
{
private:
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4Menu);

    Q_PROPERTY( bool tearOffEnabled READ isTearOffEnabled WRITE setTearOffEnabled )

public:
    Q4Menu(QWidget *parent = 0);
    ~Q4Menu();

    void      addAction(QAction *action) { QWidget::addAction(action); }
    QAction *addAction(const QString &text);
    QAction *addAction(const QIconSet &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIconSet &icon, const QString &text, const QObject *receiver, const char* member);
    QAction *addMenu(const QString &text, Q4Menu *menu);
    QAction *addSeparator();

    QAction *insertMenu(QAction *before, const QString &text, Q4Menu *menu);
    QAction *insertSeparator(QAction *before);

    void setTearOffEnabled(bool);
    bool isTearOffEnabled() const;

    void setCheckable(bool);
    bool isCheckable() const;

    void popup(const QPoint &pos, QAction *at=0);
    QAction *exec(const QPoint &pos, QAction *at=0); 
    QSize sizeHint() const;
    static QAction *exec(QList<QAction*> actions, const QPoint &pos, QAction *at=0);

#ifdef Q_WS_MAC
    MenuRef macMenu();
#endif

signals:
    void activated(QAction *action);
    void highlighted(QAction *action);

protected:
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    void hideEvent(QHideEvent *);
    void paintEvent(QPaintEvent *);
    void actionEvent(QActionEvent *);
    void timerEvent(QTimerEvent *);
    bool event(QEvent *);

private slots:
    void internalSetSloppyAction();
    void internalDelayedPopup();

private:
    friend class Q4MenuBar;
    friend class Q4MenuBarPrivate;
    friend class QTornOffMenu;
#ifdef Q_WS_MAC
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    Q4Menu(const Q4Menu &);
    Q4Menu &operator=(const Q4Menu &);
#endif
};

class Q_GUI_EXPORT Q4MenuBar : public QWidget
{   
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4MenuBar);

public:
    Q4MenuBar(QWidget *parent = 0);
    ~Q4MenuBar();

    QAction *addMenu(const QString &title, Q4Menu *menu);
    QAction *insertMenu(QAction *before, const QString &title, Q4Menu *menu);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int heightForWidth(int) const;

#ifdef Q_WS_MAC
    MenuRef macMenu();
#endif

signals:
    void activated(QAction *action);
    void highlighted(QAction *action);

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void actionEvent(QActionEvent *);
    void focusOutEvent(QFocusEvent *);
    void focusInEvent(QFocusEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

private slots:
    void internalShortcutActivated(int);

private:
    friend class Q4Menu;
    friend class Q4MenuPrivate;
#ifdef Q_WS_MAC
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    Q4MenuBar(const Q4MenuBar &);
    Q4MenuBar &operator=(const Q4MenuBar &);
#endif
};

#endif /* __Q4MENU_H__ */
