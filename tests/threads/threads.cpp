#include <qapplication.h>
#include <qlabel.h>
#include <qthread.h>

#ifdef _WS_WIN_
#include <qt_windows.h>
#endif

class MyWidget : public QLabel {


public:

    MyWidget() : QLabel(0,0,0) {}
    ~MyWidget() {}

protected:

    void customEvent(QCustomEvent * c) {
      QString * s=(QString *)c->data();
      setText(*s);
      delete s;
    }

};

MyWidget * mywidget;

class MyThread : public QThread {

    QString myname;
    int pause;

public:

    MyThread(QString s,int p) { myname=s; pause=p; }
    virtual void run();
};

class WaitThread : public QThread {

    QThread * thread1;
    QThread * thread2;
    QCondition * done_condition;

public:

    WaitThread(QThread *,QThread *,QCondition *);
    virtual void run();

};

class FinalThread : public QThread {

    QCondition * done_condition;

public:

    FinalThread(QCondition * t) { done_condition=t; }
    virtual void run();

};

WaitThread::WaitThread(QThread * a,QThread * b,QCondition * c)
{
  thread1=a;
  thread2=b;
  done_condition=c;
}

void MyThread::run()
{
    int n=0;
    while(n<5) {
	QCustomEvent * wibble=new QCustomEvent(6666);
	QString * message=new QString();
	*message=myname+" ";
	*message=*message+QString::number(n);
	wibble->setData((void *)message);
	QThread::postEvent(mywidget,wibble);
	n++;
	sleep( pause );
    }
}

void WaitThread::run()
{
    thread1->wait();
    thread2->wait();
    QString * message=new QString;
    *message="First two threads done!";
    QCustomEvent * wibble=new QCustomEvent(6666);
    wibble->setData((void *)message);
    QThread::postEvent(mywidget,wibble);
    sleep(1);
    done_condition->wakeOne();
}

void FinalThread::run()
{
    done_condition->wait();
    QString * message=new QString;
    *message="Final message!";
    QCustomEvent * wibble=new QCustomEvent(6666);
    wibble->setData((void *)message);
    QThread::postEvent(mywidget,wibble);
}

int main(int argc,char ** argv)
{
    QApplication app(argc,argv);
    mywidget=new MyWidget();
    mywidget->show();
    MyThread foo("Thread one",2);
    MyThread bar("Thread two",3);
    QCondition qte;
    WaitThread frobnitz(&foo,&bar,&qte);
    FinalThread fooble(&qte);
    foo.start();
    bar.start();
    frobnitz.start();
    fooble.start();
    app.setMainWidget(mywidget);
    return app.exec();
}