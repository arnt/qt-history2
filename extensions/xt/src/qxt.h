/****************************************************************************
** $Id: //depot/qt/main/extensions/xt/src/qxt.h#1 $
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Created : 980107
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QXT_H
#define QXT_H

#include <qapp.h>
#include <qwidget.h>
#include <X11/Intrinsic.h>

class QXtApplication : public QApplication {
    void init();
public:
    QXtApplication(int& argc, char** argv,
	const char* appclass=0,
	XrmOptionDescRec *options=0, int num_options=0,
	const char** resources=0);
    QXtApplication(Display*);
    ~QXtApplication();

    bool x11EventFilter(XEvent*);
};

class QXtWidget : public QWidget {
    Widget xtw;
    void init(const char* name, WidgetClass widget_class,
		    Widget parent, ArgList args, Cardinal num_args,
		    bool managed);
    friend void qwidget_realize(
	Widget                widget,
	XtValueMask*          mask,
	XSetWindowAttributes* attributes
    );
public:
    QXtWidget(const char* name, Widget parent, bool managed=FALSE);
    QXtWidget(const char* name, WidgetClass widget_class,
	      QXtWidget *parent, ArgList args=0, Cardinal num_args=0,
	      bool managed=FALSE);
    ~QXtWidget();

    Widget xtWidget() const { return xtw; }

    void setGeometry( int x, int y, int w, int h );
    void setGeometry( const QRect & );

protected:
    bool x11Event( XEvent* );
    void leaveEvent(QEvent*);
};

#endif
