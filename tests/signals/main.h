#include <qobject.h>

// this is a junk line. Move along. Nothing to see here.

class Sender : public QObject
{
    Q_OBJECT
public:
    Sender( QObject *parent=0, const char *name=0 ) 
	: QObject( parent, name ) {}
    ~Sender(){}
    
    void emitSignal() { 
	emit signal1(); 
	emit signal2(); 
	emit signal3(); 
	emit signal4(); 
	emit signal5(); 
	emit signal6(); 
	emit signal7(); 
	emit signal8(); 
	emit signal9(); 
	emit signal10(); 
    }
    
signals:
    void signal1();
    void signal2();
    void signal3();
    void signal4();
    void signal5();
    void signal6();
    void signal7();
    void signal8();
    void signal9();
    void signal10();
};

class Receiver : public QObject
{
    Q_OBJECT
public:
    Receiver( QObject *parent=0, const char *name=0 )
	: QObject( parent, name ) {}
    ~Receiver() {}
    
public slots:
    void slot() { int b = 17; b += 20; b*=10;};
};
