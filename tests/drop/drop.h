#include <qlabel.h>

class Drag : public QLabel {
    Q_OBJECT
public:
    Drag(QWidget* parent=0, const char* name=0);
protected:
     void mousePressEvent( QMouseEvent* );
};

class Drop : public QLabel {
    Q_OBJECT
public:
    Drop(QWidget* parent=0, const char* name=0);
public slots:
    void trackOn();
protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );

    void mouseMoveEvent( QMouseEvent *);
    void mouseReleaseEvent( QMouseEvent *);

    void timerEvent( QTimerEvent * );
};
