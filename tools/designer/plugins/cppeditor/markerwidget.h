#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>

class ViewManager;

class MarkerWidget : public QWidget
{
    Q_OBJECT

public:
    MarkerWidget( ViewManager *parent );

public slots:
    void doRepaint() { repaint( FALSE ); }

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );

private:
    QPixmap buffer;
    ViewManager *viewManager;

};

#endif
