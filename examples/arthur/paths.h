#ifndef PATHS_H
#define PATHS_H

#include "demowidget.h"

class Paths : public DemoWidget
{
public:
    Paths(QWidget *parent=0);

    void paintEvent(QPaintEvent *e);
};

#endif // PATHS_H
