/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwsmanager_qws.h $
**
** Definition of QWSManager class. This manages QWS top-level windows.
**
** Created : 20000308
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_QWS_MANAGER
class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSButton;
class QTimer;

class QWSManager : public QObject
{
    Q_OBJECT

public:
    QWSManager(QWidget *);
    ~QWSManager();

    QRegion region();
    QWidget *widget() { return managed; }
    void maximize();

    static QWidget *grabbedMouse() { return active; }

    enum Region { None=0, All=1, Title=2, Top=3, Bottom=4, Left=5, Right=6,
		TopLeft=7, TopRight=8, BottomLeft=9, BottomRight=10,
		Close=11, Minimize=12, Maximize=13, Normalize=14,
		Menu=15, LastRegion=Menu };

protected slots:
    void menuActivated(int);
    void handleMove();

protected:
    virtual Region pointInRegion(const QPoint &);

    virtual bool event(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *) {}
    virtual void paintEvent(QPaintEvent *);

    void menu(const QPoint &);
    void close();
    void minimize();
    void toggleMaximize();

    Region activeRegion;
    QWidget *managed;
    QPopupMenu *popup;
    QRect   normalSize;
    QWSButton *menuBtn;
    QWSButton *closeBtn;
    QWSButton *minimizeBtn;
    QWSButton *maximizeBtn;

    int dx;
    int dy;
    int skipCount;
    QTimer *timer;

    static QWidget *active;
    static QPoint mousePos;
};

class QWSButton
{
public:
    QWSButton(QWSManager *m, QWSManager::Region t, bool tb = false);

    enum State { MouseOver = 0x01, Clicked = 0x02, On = 0x04 };
    int state() { return flags; }
    void setMouseOver(bool);
    void setClicked(bool);
    void setOn(bool);

protected:
    void paint();

private:
    int  flags;
    bool toggle;
    QWSManager::Region type;
    QWSManager *manager;
};

/*
 Implements decoration styles
*/
class QWSDecorator
{
public:
    QWSDecorator() {}
    virtual ~QWSDecorator() {}

    virtual QRegion region(const QWidget *, const QRect &rect, QWSManager::Region r=QWSManager::All) = 0;
#ifndef QT_NO_WIDGETS
    virtual QPopupMenu *menu(const QWidget *, const QPoint &);
#endif
    virtual void paint(QPainter *, const QWidget *) = 0;
    virtual void paintButton(QPainter *, const QWidget *, QWSManager::Region, int state) = 0;
};

class QWSDefaultDecorator : public QWSDecorator
{
public:
    QWSDefaultDecorator();
    virtual ~QWSDefaultDecorator();
    
    virtual QRegion region(const QWidget *, const QRect &rect, QWSManager::Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, QWSManager::Region, int state);

protected:
    virtual const QPixmap* pixmapFor(const QWidget*,QWSManager::Region,bool,int&,int&);
};

#endif //QT_NO_QWS_MANAGER
