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

#ifndef QMOTIF_H
#define QMOTIF_H

#include <qguieventloop.h>

#include <X11/Intrinsic.h>
#undef Bool
#undef Int

class QMotifPrivate;

class QMotif : public QGuiEventLoop
{
    Q_OBJECT

public:
    QMotif( const char *applicationClass, XtAppContext context = NULL, XrmOptionDescRec *options = 0, int numOptions = 0);
    ~QMotif();

    XtAppContext applicationContext() const;

    void registerSocketNotifier( QSocketNotifier * );
    void unregisterSocketNotifier( QSocketNotifier * );

    static void registerWidget( QWidget* );
    static void unregisterWidget( QWidget* );
    static bool redeliverEvent( XEvent *event );

    static Display *x11Display();
    static XEvent* lastEvent();

protected:
    bool processEvents( ProcessEventsFlags flags );

private:
    void appStartingUp();
    void appClosingDown();
    QMotifPrivate *d;
};

#endif // QMOTIF_H
