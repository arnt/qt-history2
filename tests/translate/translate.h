#include <qwidget.h>
#include <qvbox.h>
class Main : public QVBox {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0);
    void resizeEvent(QResizeEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void paintEvent(QPaintEvent* e);
public slots:
    void bang();
};
