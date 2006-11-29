#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include <QLabel>
#include <QPixmap>
#include <QTimer>

class LEDWidget : public QLabel
{
    Q_OBJECT
public:
    LEDWidget(QWidget *parent = 0);
public slots:
    void flash();

private slots:
    void extinguish();

private:
    QPixmap onPixmap, offPixmap;
    QTimer flashTimer;
};

#endif // LEDWIDGET_H
