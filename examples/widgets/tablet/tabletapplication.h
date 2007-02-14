#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H

#include <QApplication>

#include "tabletcanvas.h"

class TabletApplication : public QApplication
{
    Q_OBJECT

public:
    TabletApplication(int &argv, char **args)
	: QApplication(argv, args) {}

    bool event(QEvent *event);
    void setCanvas(TabletCanvas *canvas)
	{ myCanvas = canvas; }

private:
    TabletCanvas *myCanvas;
};

#endif
