#include "mouse.h"

#include <QtGui>

#include <cmath>

static const int MouseCount = 5;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setBackgroundBrush(QPixmap(":/images/cheese.jpg"));

    for (int i = 0; i < MouseCount; ++i) {
        Mouse *mouse = new Mouse;
        mouse->setPos(::sin((i * 6.28) / MouseCount) * 200,
                      ::cos((i * 6.28) / MouseCount) * 200);
        scene.addItem(mouse);
    }

    QGraphicsView view(&scene);
    view.setDragMode(QGraphicsView::ScrollHandDrag);
    view.setMinimumSize(400, 300);
    view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Mice"));
    view.show();

    return app.exec();
}
