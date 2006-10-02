#include <QtGui>
#include <math.h>

int main(int argv, char *args[])
{
    QApplication app(argv, args);

    QGraphicsItem *ball = new QGraphicsEllipseItem(0, 0, 20, 20);
    
    QTimeLine *timer = new QTimeLine(5000);
    timer->setFrameRange(0, 100);
    
    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(ball);
    animation->setTimeLine(timer);

    for (int i = 0; i < 200; ++i) 
	animation->setPosAt(i / 200.0, QPointF(i, i));
    
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 250, 250);
    scene->addItem(ball);

    QGraphicsView *view = new QGraphicsView(scene);
    view->show();

    timer->start();

    return app.exec();
}
