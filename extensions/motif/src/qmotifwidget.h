#ifndef QMOTIFWIDGET_H
#define QMOTIFWIDGET_H

#include <qwidget.h>
#include <X11/Intrinsic.h>


class QMotifWidgetPrivate;

class QMotifWidget : public QWidget
{
    Q_OBJECT

public:
    QMotifWidget( QWidget *, WidgetClass, ArgList = NULL, Cardinal = 0,
                  const char * = 0, WFlags = 0 );
    virtual ~QMotifWidget();

    Widget motifWidget() const;

    void show();
    void hide();

protected:
    bool x11Event( XEvent * );

private:
    QMotifWidgetPrivate *d;

    void realize( Widget );

    friend void qmotif_widget_shell_realize( Widget, XtValueMask *,
                                             XSetWindowAttributes *);
    friend void qmotif_widget_shell_change_managed( Widget );
};

#endif // QMOTIFWIDGET_H
