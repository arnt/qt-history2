#include <qlabel.h>

class Drop : public QLabel {
    Q_OBJECT
public:
    Drop(QWidget* parent=0, const char* name=0);
public slots:
    void bang();
protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );

    void mouseMoveEvent( QMouseEvent *);
    void mouseReleaseEvent( QMouseEvent *);
private:
    bool active;
};
