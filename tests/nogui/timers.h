#include <qwidget.h>
#include <qtimer.h>




class Obj : public QObject {
    Q_OBJECT
public:
    Obj(QObject* parent=0, const char* name=0 );

signals:
    void tick();
    void stopped();
public slots:
    void bang();
private:
    int num;
    QTimer timer;
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

private:
    int fucks;
};
