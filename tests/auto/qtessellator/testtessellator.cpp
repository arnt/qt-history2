#include <testtessellator.h>
#include <private/qtessellator_p.h>

class TestTessellator : public QTessellator
{
public:
    QVector<XTrapezoid> *traps;
    void addTrap(const Trapezoid &trap);
};

void TestTessellator::addTrap(const Trapezoid &trap)
{
    XTrapezoid xtrap;
    xtrap.top = Q27Dot5ToXFixed(trap.top);
    xtrap.bottom = Q27Dot5ToXFixed(trap.bottom);
    xtrap.left.p1.x = Q27Dot5ToXFixed(trap.topLeft->x);
    xtrap.left.p1.y = Q27Dot5ToXFixed(trap.topLeft->y);
    xtrap.left.p2.x = Q27Dot5ToXFixed(trap.bottomLeft->x);
    xtrap.left.p2.y = Q27Dot5ToXFixed(trap.bottomLeft->y);
    xtrap.right.p1.x = Q27Dot5ToXFixed(trap.topRight->x);
    xtrap.right.p1.y = Q27Dot5ToXFixed(trap.topRight->y);
    xtrap.right.p2.x = Q27Dot5ToXFixed(trap.bottomRight->x);
    xtrap.right.p2.y = Q27Dot5ToXFixed(trap.bottomRight->y);
    traps->append(xtrap);
}


void test_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                         bool winding)
{
    TestTessellator t;
    t.traps = traps;
    t.setWinding(winding);
    t.tessellate(points, nPoints);
}
