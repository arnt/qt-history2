#include <qwidget.h>

class QMenuBar;

class Main : public QWidget {
    Q_OBJECT
    QMenuBar* mb;
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
    void resizeEvent(QResizeEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void paintEvent(QPaintEvent* e);
    void timerEvent(QTimerEvent* e);
public slots:
    void bang();
};
