#ifndef QEVENTLOOP_H
#define QEVENTLOOP_H

#include <qobject.h>

class QEventLoopPrivate;

#if defined(QT_THREAD_SUPPORT)
class QMutex;
#endif // QT_THREAD_SUPPORT


class Q_EXPORT QEventLoop : public QObject
{
    Q_OBJECT

public:
    // usually created as a child of QApplication
    QEventLoop( QObject *parent, const char *name = 0 );
    virtual ~QEventLoop();

    /*
      Xt API analysis
      ----------------------------------------

      DONE  -> we have matching functionality
      INC   -> functionality incorporated into our functionality.  to get matching
      functionality, certain arguments have to be passed to our functions
      DONE+ -> we have matching functionality (plus extra features).
      NOEQ  -> there is no equivalent functionality available, usually because
      this requires platform dependence

      ----------------------------------------

      DONE+ - XtAppMainLoop

      DONE  - XtAppAddTimeOut
      DONE  - XtRemoveTimeOut

      DONE  - XtAppAddInput
      DONE  - XtRemoveInput
      DONE  - XtAppNextEvent

      DONE  - XtAppPending

      NOEQ  - XtAppPeekEvent

      INC   - XtAppProcessEvent

      INC   - XtDispatchEvent

      NOEQ  - XtAppAddSignal
      NOEQ  - XtRemoveSignal

            - XtAppAddBlockHook
	    - XtAppRemoveBlockHook

	    - XtAppAddWorkProc
	    - XtAppRemoveWorkProc

    */

    enum EventType {
	User		= 1, // timers and user input events (button/key/wheel
	                     // events)

	System		= 2, // system events - timers and all other gui
	                     // (paint, configure, etc.) events

	External	= 4, // timers and socket notifiers and other input
	                     // events (uses file descriptors)

	All		= 7  // all event types
    };

    // returns the number of milliseconds that Qt needs to handle it's timers.
    // event loop reimplementations that do their own timer handling need to
    // use this to make sure that Qt's timers continue to work. returns -1 if
    // Qt has no timers running
    int timeToWait();

    // activate all Qt timers and return the number of timers that were activated.
    // event loop reimplementations that do their own timer handling need to call
    // this after the time returned by timeToWait() has elapsed.
    int activateTimers();

    virtual void addExternal( int fd );
    virtual void removeExternal( int fd );

    // main event loop - QApplication calls this function from its exec()
    //
    // this is XtAppMainLoop behavior with added functionality
    virtual int exec();

    // exit the event loop... the exec() function returns the exitcode specified
    // by the \a retcode arguement.
    //
    // no Xt equivalent
    virtual void exit( int retcode = 0 );

    // event loop stack - no Xt equivalent
    int enterLoop();
    void exitLoop();
    int loopLevel() const;

    // process one, and exactly one event that matches \a eventTypes.  The
    // passed eventTypes can be any combination of theif there are no events
    // available, this function waits for the next event.
    //
    // calls processNextEvent( eventTypes, TRUE )
    virtual void processOneEvent( int eventTypes );

    // process pending events that match \a eventTypes for a maximum of
    // \a maxtime milliseconds, or until there are no more events to process,
    // which ever is shorter. if this function is called without any
    // arguments, then all event types are processed for a maximum of 3
    // seconds (3000 milliseconds).
    //
    // no Xt equivalent
    virtual void processEvents( int eventTypes = All , int maxtime = 3000 );

    // returns true if there is an event waiting that matches \a eventTypes,
    // otherwise it returns false.
    //
    // this is XtAppPending, but checking the Qt event types, not the Xt
    // input masks
    virtual bool hasPendingEvents( int eventTypes );

    // this function wakes up the event loop
    virtual void wakeUp();

#if defined(QT_THREAD_SUPPORT)
    QMutex *mutex() const;
#endif // QT_THREAD_SUPPORT

signals:
    // emitted when the eventloop is awake
    void awake();

protected:
    // process the next event received that matches \a eventType.  when there are
    // no available events matching \a eventType, this function will wait for the
    // next event if \a canWait is true, otherwise it returns immediately.
    //
    // returns true if an event matching \a eventType was processed, otherwise
    // returns false.
    //
    // very similar to XtAppProcessEvent, but with different event mask/types
    //
    // NOTE: Xt has more input types, but doesn't distinguish between
    // user input and other gui events.  this is a trade off designed to
    // integrate more into the Qt way of doing things.  therefore, the
    // XtAppNextEvent, XtPeekEvent, XtDispatchEvent and XtAppProcessEvent
    // functionality is encompased in this one function.  behavior similar
    // to the above Xt functions can be somewhat acheived by the \a eventTypes
    // argument.
    //
    // if ( eventTypes == All ) {
    //    XtAppNextEvent
    //    XtDispatchEvent
    // } else {
    //     if ( eventTypes & User ) {
    //         XtPeekEvent or get queued event - order undefined
    //         if ( event->type matches User ) {
    //             XtDispatchEvent
    //         } else {
    //             queue event for later processing
    //         }
    //     }
    //     if ( eventTypes & System ) {
    //         XtPeekEvent or get queued event - order undefined
    //         if ( event->type matches System ) {
    //             XtDispatchEvent
    //         } else {
    //             queue for later processing
    //         }
    //         XtProcessEvent mask = ( XtIMTimer | XtIMSignal )
    //     }
    //     if ( eventTypes & External ) {
    //         XtProcessEvent mask = XtIMAlternateInput
    //     }
    // }
    virtual bool processNextEvent( int eventType, bool canWait );


#if defined(Q_WS_X11)
    // process \a event.  this function returns true if the event was process,
    // otherwise it returns false.
    //
    // NOTE: The default implementation translates and delivers \a event to
    // the appropriate Qt widget.  Therefore, when overriding this function,
    // you must call the superclass implementation for Qt to continue working.
    // Usually you do this if your reimplementation can't deliver the current
    // event, something similar to:
    //
    // bool MyEventLoop::x11ProcessEvent( XEvent *event )
    // {
    //     if ( findSomething( event->xany.window ) {
    //         // process the event, since it is for one of my windows
    //         ...
    //         ...
    //         ...
    //         return TRUE;
    //     }
    //
    //     return QEventLoop::x11ProcessEvent( event );
    // }
    virtual bool x11ProcessEvent( XEvent *event );
#endif


    //
    // This is the current QApplication event API:
    //
    // got it - int exec(); // platform dependent for some gui thread  stuff
    // got it - void processEvents(); // cross platform, calls processEvents( 3000 )
    // got it - void processOneEvent(); // cross platform, calls processNextEvent( TRUE )
    // got it - int enter_loop(); // cross platform, starts a new loop around
    //                            // processNextEvent( TRUE )
    // got it - void exit_loop(); // cross platform, sets the app_exit_loop flag
    // got it - int loopLevel() const; // cross platforms, returns loop_level
    // got it - static void exit( int retcode=0 ); // cross platforms, sets
    //                                             // app_exit_loop, quit_now and
    //                                             // quit_code

    // got it - void processEvents( int maxtime ); // platform dependent (to get the
    //                                             // time faster on windows), calls
    //                                             // processNextEvent( FALSE )
    // got it - bool hasPendingEvents(); // platform dependent, winodow system events
    //                                   // plus posted events, does not care about
    //                                   // socket notifiers
    // private:
    // got it - bool processNextEvent( bool ); // platform dependent

private:
    // internal initialization/cleanup - implemented in various platform specific files
    void init();
    void cleanup();

    // data for the default implementation - other implementations should not
    // use/need this data
    QEventLoopPrivate *d;

    friend class QApplication;
};

#endif // QEVENTLOOP_H
