#include <qapplication.h>
#include <qlabel.h>
#include <qthread.h>

class MyWidget : public QLabel {


public:

    MyWidget() : QLabel(0,0,0) {}
    ~MyWidget() {}

protected:

    void customEvent(QCustomEvent * c) {
      qDebug("Got customevent! %d %d",c->type(),QThread::currentThread());
      setText(QString::number((int)c->data()));
    }

};

MyWidget * mywidget;

class MyThread : public QThread {
    
public:
    
    virtual void run();
    
};

void MyThread::run()
{
    int n=0;
    while(1) {
	QCustomEvent * wibble=new QCustomEvent(6666);
	wibble->setData((void *)n);
	QThread::postEvent(mywidget,wibble);
	n++;
    }
}

int main(int argc,char ** argv)
{
  QApplication app(argc,argv);
  mywidget=new MyWidget();
  mywidget->show();
  qDebug("Main thread is %d",QThread::currentThread());
  MyThread foo;
  foo.start();
  app.exec();
}
