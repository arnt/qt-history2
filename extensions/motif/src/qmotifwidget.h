/****************************************************************************
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOTIFWIDGET_H
#define QMOTIFWIDGET_H

#include <qwidget.h>

#include <X11/Intrinsic.h>
#undef Bool
#undef Int

class QMotifWidgetPrivate;
class QKeyEvent;

class QMotifWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMotifWidget);

public:
    QMotifWidget( QWidget *, WidgetClass, ArgList = NULL, Cardinal = 0,
                  const char * = 0, WFlags = 0 );
    virtual ~QMotifWidget();

    Widget motifWidget() const;

    void show();
    void hide();

protected:
    bool event( QEvent * );
    bool eventFilter( QObject *object, QEvent *event );

private:
    void realize( Widget );

    friend void qmotif_widget_shell_realize( Widget, XtValueMask *,
                                             XSetWindowAttributes *);
    friend void qmotif_widget_shell_change_managed( Widget );
    static bool dispatchQEvent( QEvent*, QWidget*);
    friend class QMotifDialog;
};

#endif // QMOTIFWIDGET_H
