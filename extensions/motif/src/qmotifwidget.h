#ifndef QMOTIFWIDGET_H
#define QMOTIFWIDGET_H

#include <qwidget.h>
#include <X11/Intrinsic.h>


class QMotifWidgetPrivate;

class QMotifWidget : public QWidget
{
    Q_OBJECT

public:
    // Creates a Motif widget with a QWidget parent.  Any kind of Xt/Motif
    // widget can be used.
    //
    // Creates a Shell widget is created if the QWidget
    // parent is zero.  The Shell is a special subclass of ApplicationShell.
    // This allows applications that use QDialogs to have proper modality
    // handling through the QMotif extension.
    //
    // Future development will make it possible to use an Xt/Motif widget inside
    // a QWidget parent, which is useful for setting the center widget in a
    // QMainWindow.  Using Xt/Motif widgets with other QWidget parents is
    // beyond the scope of this implementation at the moment, and if this
    // functionality is needed, it will be added later.
    QMotifWidget( QWidget *, WidgetClass, ArgList = NULL, Cardinal = 0,
                  const char * = 0, WFlags = 0 );
    // Destroys the QWidget and Xt/Motif widget.  If a Shell widget was created
    // by the constructor, it is also destroyed.
    virtual ~QMotifWidget();

    // Returns the embedded Xt/Motif widget.  If a Shell widget was created by
    // the constructor, you can access it with XtParent().
    Widget motifWidget() const;

    // Manages the embedded Xt/Motif widget and shows the widget.
    void show();
    // Unmanaged the embedded Xt/Motif widget and hides the widget.
    void hide();

protected:
    // Delivers events to both Xt/Motif and Qt.
    bool x11Event( XEvent * );

private:
    QMotifWidgetPrivate *d;

    void realize( Widget );

    friend void qmotif_widget_shell_realize( Widget, XtValueMask *,
                                             XSetWindowAttributes *);
    friend void qmotif_widget_shell_change_managed( Widget );
};

#endif // QMOTIFWIDGET_H
