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

#ifndef QXTWIDGET_H
#define QXTWIDGET_H

#include <QtGui/qwidget.h>

#include <X11/Intrinsic.h>
#undef Bool
#undef Int


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
QXtWidget(const char* name, Widget parent, bool managed=false);
QXtWidget(const char* name, WidgetClass widget_class,
QWidget *parent=0, ArgList args=0, Cardinal num_args=0,
bool managed=false);
~QXtWidget();

Widget xtWidget() const { return xtw; }
bool isActiveWindow() const;
void setActiveWindow();

protected:
void moveEvent( QMoveEvent* );
void resizeEvent( QResizeEvent* );
bool x11Event( XEvent * );
};

#endif // QXTWIDGET_H
