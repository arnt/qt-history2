#include <qpoint.h>
#include <qrect.h>

class QTessellatorPrivate;

typedef int Q27Dot5;
#define Q27Dot5ToDouble(i) ((i)/32.)
#define FloatToQ27Dot5(i) (int)((i) * 32)
#define IntToQ27Dot5(i) ((i) << 5)
#define Q27Dot5ToXFixed(i) ((i) << 11)
#define Q27Dot5Factor 32

class Q_GUI_EXPORT QTessellator {
public:
    QTessellator();
    virtual ~QTessellator();

    QRectF tessellate(const QPointF *points, int nPoints);

    void setWinding(bool w);

    struct Vertex {
        Q27Dot5 x;
        Q27Dot5 y;
    };
    struct Trapezoid {
        Q27Dot5 top;
        Q27Dot5 bottom;
        const Vertex *topLeft;
        const Vertex *bottomLeft;
        const Vertex *topRight;
        const Vertex *bottomRight;
    };
    virtual void addTrap(const Trapezoid &trap) = 0;

private:
    friend class QTessellatorPrivate;
    QTessellatorPrivate *d;
};

