#ifndef EDITWIDGET_H
#define EDITWIDGET_H

#include <qwidget.h>

class EditWidgetPrivate;

class EditWidget : public QWidget
{
    Q_OBJECT
public:
    EditWidget( QWidget *parent,  const char * name );
    ~EditWidget();

    void setText( const QString &t );
    void setFont( const QFont &f );

protected:
    void mousePressEvent( QMouseEvent *e );
    void keyPressEvent ( QKeyEvent *e );

    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

    void timerEvent( QTimerEvent * );

private:
    void recalculate();

    EditWidgetPrivate *d;
};


#endif
