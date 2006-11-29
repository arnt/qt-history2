#include "ledwidget.h"

LEDWidget::LEDWidget(QWidget *parent)
    : QLabel(parent), onPixmap(":/ledon.png"), offPixmap(":/ledoff.png")
{
    setPixmap(offPixmap);
    flashTimer.setInterval(200);
    flashTimer.setSingleShot(true);
    connect(&flashTimer, SIGNAL(timeout()), this, SLOT(extinguish()));
};

void LEDWidget::extinguish()
{
    setPixmap(offPixmap);
}

void LEDWidget::flash()
{
    setPixmap(onPixmap);
    flashTimer.start();
}

