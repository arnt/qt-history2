#ifndef QMOTIF_H
#define QMOTIF_H

#include <qnamespace.h>
#include <qwidgetintdict.h>

#include <X11/Intrinsic.h>

// QMotif provides the core of the Qt<->Motif integration...

class QMotifPrivate;

class QMotif : public Qt
{
public:
    // Creates QMotif, which allows Qt and Xt/Motif integration.
    // The Xt toolkit is initialized by this constructor by calling
    // XtToolkitInitialize().
    QMotif();

    // Destroys QMotif.  If QMotif created an application context, it is
    // also destroyed.
    virtual ~QMotif();

    // Sets the application context.
    void setApplicationContext( XtAppContext );

    // Returns the application context.  If no application context has
    // been set, then QMotif creates one.  The applicaiton context is
    // destroyed in the QMotif destructor only if QMotif created the
    // application context.
    XtAppContext applicationContext() const;

    // Initialize the application context.  All arguments passed to this
    // function are used to call XtDisplayInitialize().
    void initialize( int *argc, char **argv, char *applicationClass,
		     XrmOptionDescRec *options, int numOptions );

    // Redeliver the given XEvent to Xt.  This is used by QMotifDialog and
    // QMotifWidget.  Rationale: An XEvent handled by Qt does not go through the
    // Xt event handlers, and the internal state of Xt/Motif widgets will
    // not be updated.  This function should only be used if an event
    // delivered by Qt to a QWidget needs to be sent to an Xt/Motif widget.
    //
    // You should not need to call this function.
    static bool redeliverEvent( XEvent * );

private:
    QMotifPrivate *d;

    // Motif widget -> Qt widget mapper.  This is for event delivery of modal events
    // in QMotifDialog/QMotifWidget.
    static QWidgetIntDict *mapper();

    friend class QMotifEventLoop;
    friend class QMotifDialog;
    friend class QMotifWidget;
    friend Boolean qmotif_event_dispatcher( XEvent * );
};

#endif // QMOTIF_H
