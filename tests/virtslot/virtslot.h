#include <qwidget.h>



class Base : public QObject
{
    Q_OBJECT    
public:
    Base( QObject *parent = 0) :QObject(parent) {}
public slots:    
    virtual void hello();
};


class Sub : public Base
{
    Q_OBJECT
public:
    Sub( QObject *parent = 0) :Base(parent) {}
    void hello();
};


class Main : public QWidget {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
    void resizeEvent(QResizeEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void paintEvent(QPaintEvent* e);
public slots:
    void bang();
};
