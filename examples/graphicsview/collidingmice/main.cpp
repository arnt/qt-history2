#include "mouse.h"

#include <QtGui>

#include <cmath>

static const int MouseCount = 7;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(-300, -300, 600, 600);

    for (int i = 0; i < MouseCount; ++i) {
        Mouse *mouse = new Mouse;
        mouse->setPos(::sin((i * 6.28) / MouseCount) * 200,
                      ::cos((i * 6.28) / MouseCount) * 200);
        scene.addItem(mouse);
    }

    QGraphicsView view(&scene);
    view.setBackgroundBrush(QPixmap(":/images/cheese.jpg"));
    view.setCacheMode(QGraphicsView::CacheBackground);
    view.setDragMode(QGraphicsView::ScrollHandDrag);
    view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Colliding Mice"));
    view.resize(400, 300);
    view.show();

    return app.exec();
}
