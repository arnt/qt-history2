#ifndef QMOTIF_H
#define QMOTIF_H

#include <qnamespace.h>
#include <qwidgetintdict.h>

#include <X11/Intrinsic.h>

class QMotifPrivate;

class QMotif : public Qt
{
public:
    QMotif();
    virtual ~QMotif();

    XtAppContext applicationContext() const;
    void setApplicationContext( XtAppContext );

    void initialize( int *argc, char **argv, char *applicationClass,
		     XrmOptionDescRec *options, int numOptions );

    static bool redeliverEvent( XEvent * );

private:
    QMotifPrivate *d;

    static QWidgetIntDict *mapper();

    friend class QMotifEventLoop;
    friend class QMotifDialog;
    friend class QMotifWidget;
    friend Boolean qmotif_event_dispatcher( XEvent * );
};

#endif // QMOTIF_H
