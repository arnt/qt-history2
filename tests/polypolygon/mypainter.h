#include <qpainter.h>

class MyPainter : public QPainter
{
public:
    MyPainter();
    MyPainter( const QPaintDevice * pd, bool unclipped = FALSE );
    MyPainter( const QPaintDevice * pd, const QWidget * copyAttributes, bool unclipped = FALSE );
    ~MyPainter();

    void drawPolyPolygon( const QPointArray &pa, const int *start, int ncomp, bool winding );
};
