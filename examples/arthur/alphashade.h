#ifndef ALPHASHADE_H
#define ALPHASHADE_H

#include "demowidget.h"

#include <qpixmap.h>

class AlphaShade : public DemoWidget
{
public:
    AlphaShade(QWidget *parent=0);
    void paintEvent(QPaintEvent *e);
    QString description() const;

private:
    int iterations;
    int spread;
    QPixmap pattern;
};

#endif // ALPHASHADE_H
