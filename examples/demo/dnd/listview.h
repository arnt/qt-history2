#include <qlistview.h>

class ListView : public QListView
{
    Q_OBJECT

public:
    ListView( QWidget* parent = 0, const char* name = 0 );
    ~ListView();

    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    QPoint pressPos;
    bool dragging;
};
