/****************************************************************************
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QXT_H
#define QXT_H

#include <qapplication.h>
#include <qwidget.h>

#include <X11/Intrinsic.h>

#ifdef Bool
#undef Bool
#endif // Bool

class QXtApplication : public QApplication {
    Q_OBJECT
    void init();

public:
    QXtApplication(int& argc, char** argv,
	const char* appclass=0,
	XrmOptionDescRec *options=0, int num_options=0,
	const char** resources=0);
    QXtApplication(Display *, HANDLE = 0, HANDLE = 0);
    QXtApplication(Display *, int, char **, HANDLE = 0, HANDLE = 0);
    ~QXtApplication();
};

class QXtWidget : public QWidget {
    Q_OBJECT
    Widget xtw;
    Widget xtparent;
    bool   need_reroot;
    void init(const char* name, WidgetClass widget_class,
		    Widget parent, QWidget* qparent,
		    ArgList args, Cardinal num_args,
		    bool managed);
    friend void qwidget_realize( Widget widget, XtValueMask* mask,
				 XSetWindowAttributes* attributes );

public:
    QXtWidget(const char* name, Widget parent, bool managed=FALSE);
    QXtWidget(const char* name, WidgetClass widget_class,
	      QWidget *parent=0, ArgList args=0, Cardinal num_args=0,
	      bool managed=FALSE);
    ~QXtWidget();

    Widget xtWidget() const { return xtw; }
    bool isActiveWindow() const;
    void setActiveWindow();

protected:
    void moveEvent( QMoveEvent* );
    void resizeEvent( QResizeEvent* );
    bool x11Event( XEvent * );
};

#endif
