#ifndef CLIPPING_H
#define CLIPPING_H

#include "demowidget.h"

#include <qpixmap.h>

class Clipping : public DemoWidget
{
public:
    Clipping(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void resizeEvent(QResizeEvent *e);

private:
    QPixmap bgFill;
    int textx;
    int texty;
    double textdirx;
    double textdiry;
};

#endif // CLIPPING_H
