#ifndef QMOTIF_H
#define QMOTIF_H

#include <qeventloop.h>

#include <X11/Intrinsic.h>

class QMotifPrivate;

class QMotif : public QEventLoop
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
    static XEvent* lastEvent();

protected:
    bool processEvents( ProcessEventsFlags flags );

private:
    void appStartingUp();
    void appClosingDown();
    QMotifPrivate *d;

};



#endif // QMOTIF_H
