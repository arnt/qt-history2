#include <qwidget.h>
class Keyboard : public QWidget {
public:
    Keyboard(QWidget* parent=0, const char* name=0, int f=0);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent* e);

    QSize sizeHint() const;
    
private:
    int getKey( int &w, int j = -1 );

    uint shift:1;
    uint lock:1;
    uint ctrl:1;
    uint alt:1;
    
};
