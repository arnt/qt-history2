#include <qwidget.h>
#include <qtimer.h>




class Obj : public QObject {
    Q_OBJECT
public:
    Obj(QObject* parent=0, const char* name=0 );

signals:
    void tick( int );
    void stopped();
public slots:
    void bang();
private:
    int num;
    QTimer timer;
};


