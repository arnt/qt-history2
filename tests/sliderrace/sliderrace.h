#ifndef SLIDERRACE_H
#define SLIDERRACE_H

#include <qthread.h>
#include <qvbox.h>
#include <qptrlist.h>

class QSlider;
class QPushButton;
class Commentator;

class Race : public QVBox
{
    Q_OBJECT
public:
    Race();
    ~Race();

    void waitForStart() { startEvent->wait(); }
    void start() { startEvent->wakeAll(); }
    void waitForFinish() { finishEvent->wait(); }
    void finished() { 
	over = TRUE;
	finishEvent->wakeOne(); 
    }
    void waitForCeremony() { stopEvent->wait(); }
    void celebrate() { stopEvent->wakeAll(); }

    bool* isFinished() { return &over; }

private:
    QThreadEvent* startEvent;
    QThreadEvent* finishEvent;
    QThreadEvent* stopEvent;
    bool over;

    Commentator* comment;
};

class Horse : public QThread
{
public:
    Horse( Race* target, const QString& message );
    ~Horse();

    QString name() { return msg; }
    int toGo();
    void celebrate();

    bool exit;

protected:
    void run();
    Race* race;
    int speed;

    QSlider* horse;
    QString msg;
};

class Commentator : public QObject, public QThread
{
    Q_OBJECT
public:
    Commentator( Race* target );
    ~Commentator();

    void run();

private:
    QPushButton* action;
    Race* race;
    bool exit;

    QPtrList<Horse> horses;

private slots:
    void placeHorses();
    void startRace();
    void quit();
};


#endif // SLIDERRACE_H
