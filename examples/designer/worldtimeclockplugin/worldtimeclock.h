#ifndef WORLDTIMECLOCK_H
#define WORLDTIMECLOCK_H

#include <QTime>
#include <QWidget>

class WorldTimeClock : public QWidget
{
    Q_OBJECT

public:
    WorldTimeClock(QWidget *parent = 0);

public slots:
    void setTimeZone(int hourOffset);

signals:
    void updated(QTime currentTime);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int timeZoneOffset;
};

#endif
