#include <qscrollview.h>

class QComboBox;

class UniScrollview : public QScrollView {
Q_OBJECT
public:
    UniScrollview(QWidget* parent=0, const char* name=0, int f=0);
    int cellSize() const { return cellsize; }
public slots:
    void scrollTo( int unicode );
protected:
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void drawContents( QPainter *, int cx, int cy, int cw, int ch ) ;
private:
    int cellsize;
    QFont smallFont;
    int xoff;
};


class UniKeyboard : public QWidget 
{
Q_OBJECT
public:
    UniKeyboard(QWidget* parent=0, const char* name=0, int f=0);
protected:
    void resizeEvent(QResizeEvent *);
private slots:
    void handleCombo( int );
    void svMove( int, int );
private:
    UniScrollview *sv;
    QComboBox *cb;
    int currentBlock;
};



