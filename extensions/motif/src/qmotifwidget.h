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

#ifndef QMOTIFWIDGET_H
#define QMOTIFWIDGET_H

#include <QtGui/qwidget.h>

#include <X11/Intrinsic.h>
#undef Bool
#undef Int

class QMotifWidgetPrivate;
class QKeyEvent;

class QMotifWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMotifWidget)

public:
    QMotifWidget(const char *name, WidgetClass widgetClass, QWidget *parent,
                 ArgList args = NULL, Cardinal argCount = 0,
                 Qt::WFlags flags = 0);
    ~QMotifWidget();

    Widget motifWidget() const;

protected:
    bool event(QEvent *);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    bool x11Event(XEvent *event);

private:
    void realize(Widget);

    friend void qmotif_widget_shell_destroy(Widget w);
    friend void qmotif_widget_shell_realize(Widget, XtValueMask *,
                                            XSetWindowAttributes *);
    friend void qmotif_widget_shell_change_managed(Widget);
    static bool dispatchQEvent(QEvent*, QWidget*);
    friend class QMotifDialog;
};

#endif // QMOTIFWIDGET_H
