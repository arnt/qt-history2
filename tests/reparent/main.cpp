#include "widget.h"
#include <qapplication.h>
#include <qframe.h>

class RealWidget : public Widget
{
    Q_OBJECT

public:
    RealWidget() : Widget()
    {
	BLUE->setAcceptDrops(TRUE);
    }

public slots:
    void blueToZero();
    void blueToThis();
    void blueToGreen();
    void blueToRed();
    void greenToZero();
    void greenToThis();
    void greenToBlue();
    void greenToRed();
    void redToZero();
    void redToThis();
    void redToGreen();
    void redToBlue();
    void toggleGreyDND();
    void reparentBelow();
};

void RealWidget::blueToZero()
{
    BLUE->reparent(0, QPoint(30, 30), TRUE);
}

void RealWidget::greenToZero()
{
    GREEN->reparent(0, QPoint(20, 20), TRUE);
}

void RealWidget::redToZero()
{
    RED->reparent(0, QPoint(10, 10), TRUE);
}

void RealWidget::blueToThis()
{
    BLUE->reparent(this, QPoint(30, 120), TRUE);
}

void RealWidget::greenToThis()
{
    GREEN->reparent(this, QPoint(20, 110), TRUE);
}

void RealWidget::redToThis()
{
    RED->reparent(this, QPoint(10, 100), TRUE);
}

void RealWidget::blueToGreen()
{
    BLUE->reparent(GREEN, QPoint(10, 10), TRUE);
}

void RealWidget::redToGreen()
{
    RED->reparent(GREEN, QPoint(50, 50), TRUE);
}

void RealWidget::blueToRed()
{
    BLUE->reparent(RED, QPoint(20, 20), TRUE);
}

void RealWidget::greenToRed()
{
    GREEN->reparent(RED, QPoint(10, 10), TRUE);
}

void RealWidget::greenToBlue()
{
    GREEN->reparent(BLUE, QPoint(40, 40), TRUE);
}

void RealWidget::redToBlue()
{
    RED->reparent(BLUE, QPoint(50, 50), TRUE);
}

void RealWidget::toggleGreyDND()
{
    GREY->setAcceptDrops(! GREY->acceptDrops());
    qDebug("GREY: drops %d", GREY->acceptDrops());
}

void RealWidget::reparentBelow()
{
    RED->reparent(BELOW, QPoint(10, 10), TRUE);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    RealWidget w;
    app.setMainWidget(&w);
    w.show();
    return app.exec();
}

#include "main.moc"
