#include <qapplication.h>
#include <qwidget.h>

#include "mypainter.h"


class Simple : public QWidget {
    Q_OBJECT

protected:
    virtual void paintEvent(QPaintEvent *);
};

void Simple::paintEvent(QPaintEvent *)
{
    MyPainter paint(this);

    paint.setBrush( green );
    paint.setPen( QPen( red, 5 ) );
#if 0
    QPointArray pa( 8 );
    pa.setPoint( 0, 10, 10 );
    pa.setPoint( 1, 100, 10 );
    pa.setPoint( 2, 100, 100 );
    pa.setPoint( 3, 10, 100 );
    pa.setPoint( 4, 50, 50 );
    pa.setPoint( 5, 50, 70 );
    pa.setPoint( 6, 70, 70 );
    pa.setPoint( 7, 70, 50 );

    int start[3] = { 0, 4, 8 };
#else
    QPointArray pa( 10 );
    pa.setPoint( 0, 10, 10 );
    pa.setPoint( 1, 100, 10 );
    pa.setPoint( 2, 100, 100 );
    pa.setPoint( 3, 10, 100 );
    pa.setPoint( 4, 10, 10 );
    pa.setPoint( 5, 50, 50 );
    pa.setPoint( 6, 50, 70 );
    pa.setPoint( 7, 70, 70 );
    pa.setPoint( 8, 70, 50 );
    pa.setPoint( 9, 50, 50 );

    int start[3] = { 0, 5, 10 };
#endif
    paint.drawPolyPolygon( pa, start, 2, TRUE );
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *w = new Simple();
    w->setGeometry(50, 50, 400, 400);

    app.setMainWidget(w);
    w->show();
    return app.exec();
}

#include "main.moc"
