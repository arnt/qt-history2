#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include <QWidget>

class CircleWidget : public QWidget
{
    Q_OBJECT

public:
    CircleWidget(QWidget *parent = 0);

    void setFloatBased(bool floatBased);
    void setAntialiased(bool antialiased);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void nextAnimationFrame();

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool floatBased;
    bool antialiased;
    int frameNo;
};

#endif
