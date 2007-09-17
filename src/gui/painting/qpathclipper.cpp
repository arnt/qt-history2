/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpathclipper_p.h"

#include <private/qbezier_p.h>
#include <private/qmath_p.h>

/**
   The algorithm used here is a little complicated. It's based on
   the most recent research related to set operations/clipping of
   polygons and extended a lot for paths. Introduction
   of curves makes the algorithm quite complex. Removal of all the
   special/corner cases of curves would make it a lot simpler.

   To understand the basics please read the following research
   papers:
   "Efficient clipping of arbitrary polygons"
   http://portal.acm.org/citation.cfm?id=274364&coll=portal&dl=ACM

   And

   "An Extension of Polygon Clipping To Resolve Degenerate Cases"
   http://cadanda.com/CAD_A_3_1-4_48.PDF

   The most involved parts of the algorithm are finding
   intersections, marking them and finally pathFromList method
   which is meant to combine them. Optimizations should start
   with the findIntersections method which is quadratic atm.
 */

#include <qdebug.h>

QT_BEGIN_NAMESPACE

//#define QDEBUG_CLIPPER
#ifdef QDEBUG_CLIPPER
static QDebug operator<<(QDebug str, const QBezier &b)
{
    QString out = QString::fromLatin1("Bezier([%1, %2],[%3, %4],[%5, %6],[%7, %8])")
                  .arg(b.x1)
                  .arg(b.y1)
                  .arg(b.x2)
                  .arg(b.y2)
                  .arg(b.x3)
                  .arg(b.y3)
                  .arg(b.x4)
                  .arg(b.y4);
    str.nospace()<<out;
    return str;
}
#endif

class QPathVertex
{
public:
    enum Degeneracy {
        DNone,
        DIntersect,
        DDegenerate
    };

    enum TraversalFlag {
        TNone,
        TEnEx,
        TExEn,
        TEn,
        TEx
    };

    enum CoupleFlag {
        NoCouple,
        FrontElement,
        RearElement
    };

    enum Direction {
        Stop,
        ForwardGo,
        BackwardGo,
        ForwardTurn,
        BackwardTurn
    };

    enum Type {
        MoveTo,
        LineTo,
        CurveTo,
        MoveCurveTo, //used when the path starts and ends at this vertex
        MoveLineTo,  //used when the part starts and ends at this vertex
        BezierIntersection,
        LineIntersection
    };
public:
    inline QPathVertex()
        : heap_allocated(true)
    {
#ifdef QDEBUG_CLIPPER
        Q_ASSERT(0);
#endif
    }
    inline QPathVertex(qreal xi, qreal yi, Type t)
        : next(0), prev(0), intersect(DNone),
          code(TNone), neighbor(0),
          cross_transfer(false),
          x(xi), y(yi), alpha(0), type(t),
          heap_allocated(true)
    {
    }
    ~QPathVertex();

    Direction eat();

    Direction forwardEat(QPathVertex *prev);

    Direction backwardEat(QPathVertex *prev);

    Direction turnForwardEat();

    Direction turnBackwardEat();

    void setIntersect(Degeneracy d);

    void setCode(TraversalFlag c)
    {
        code = c;
    }

    bool isCoupled() const
    {
        return (couple.info != NoCouple);
    }

    QPointF getPoint() const
    {
        return QPointF(x, y);
    }

    void  setCouple(CoupleFlag which, QPathVertex *a)
    {
	couple.info = which;
	couple.link = a;
    }
public:
    QPathVertex *next;
    QPathVertex *prev;

    Degeneracy    intersect;
    TraversalFlag code;

    QPathVertex *neighbor;

    struct CoupleInfo {
        CoupleInfo()
            : info(NoCouple),
              link(0)
        {}
        CoupleFlag  info;
        QPathVertex *link;
    };
    CoupleInfo couple;

    bool cross_transfer;

    qreal x, y;
    qreal alpha;

    inline void setType(Type t)
    {
        type = t;
    }
    inline Type getType(const QPathVertex *prev) const
    {
        if (type == MoveLineTo || type == MoveCurveTo) {
            if (!prev || prev == next)
                return MoveTo;
            else if (type == MoveLineTo)
                return LineTo;
            else
                return CurveTo;
        } else
            return type;
    }
    inline bool isCurveTo() const
    {
        return type == CurveTo || type == MoveCurveTo;
    }
    inline Type getRawType() const
    {
        return type;
    }
    inline bool isMoveTo() const
    {
        return (type == QPathVertex::MoveTo ||
                type == QPathVertex::MoveLineTo ||
                type == QPathVertex::MoveCurveTo);
    }
private:
    Type type;
public:
    //used only by curveto/movecurveto elements
    QPointF ctrl1, ctrl2;

    bool heap_allocated;
};

#ifdef QDEBUG_CLIPPER
static QDebug operator<<(QDebug str, const QPathVertex &b)
{
    QString out = QString::fromLatin1("Vertex(%1 - (%2, %3),inter=%4,tf=%5)")
                  .arg(b.getRawType())
                  .arg(b.x)
                  .arg(b.y)
                  .arg(b.intersect)
                  .arg(b.code);
    str.nospace()<<out;
    return str;
}

static QDebug operator<<(QDebug str, const QPathVertex::TraversalFlag &b)
{
    QString out;

    switch (b) {
    case QPathVertex::TNone:
        out = QString::fromLatin1("TNone");
        break;
    case QPathVertex::TEnEx:
        out = QString::fromLatin1("TEnEx");
        break;
    case QPathVertex::TExEn:
        out = QString::fromLatin1("TExEn");
        break;
    case QPathVertex::TEn:
        out = QString::fromLatin1("TEn");
        break;
    case QPathVertex::TEx:
        out = QString::fromLatin1("TEx");
        break;
    }
    str.nospace()<<out;
    return str;
}
#endif

QPathVertex::Direction QPathVertex::eat()
{
    switch (code) {
    case TEnEx:
        setCode(TNone);
        if (cross_transfer)
            return ForwardTurn;
        else
            return BackwardTurn;
    case TExEn :
        setCode(TEn);
        return BackwardGo;
    case TEn :
        setCode(TNone);
        return ForwardGo;
    case TEx :
        setCode(TNone);
        return BackwardGo;
    case TNone :
        Q_ASSERT(!"QPathVertex::eat: is this state possible?");
    }
    return ForwardGo;
}


QPathVertex::Direction QPathVertex::forwardEat(QPathVertex *prev)
{
    if (prev == 0)
        Q_ASSERT(!"clip_vertex::forward_eat: is this state possible?");

    switch (code) {
    case TEnEx :
        setCode(TEx);
        if (cross_transfer)
            return BackwardTurn;
        else
            return ForwardTurn;
    case TExEn :
        setCode(TEn);
        if (cross_transfer)
            return BackwardTurn;
        else
            return ForwardTurn;
    case TEn:
        setCode(TNone);
        if (isCoupled() && prev->isCoupled()) {
            if (prev->couple.link == this)
                return ForwardGo;
            else
                return ForwardTurn;
        }
        break;
    case TEx:
        setCode(TNone);
        break;

    case TNone :
        Q_ASSERT(!"QPathVertex::forwardEat: is this state possible?");
    }

    if (cross_transfer)
        return BackwardTurn;
    else
        return ForwardTurn;
}


QPathVertex::Direction QPathVertex::backwardEat(QPathVertex *prev)
{
    if (prev == 0)
        Q_ASSERT(!"QPathVertex::backward_eat: is this state possible?");

    switch (code) {
    case TEnEx :
        setCode(TEn);
        if (cross_transfer)
            return ForwardTurn;
        else
            return BackwardTurn;
    case TExEn :
        setCode(TEx);
        if (cross_transfer)
            return ForwardTurn;
        else
            return BackwardTurn;
    case TEx:
        setCode(TNone);
        if (isCoupled() && prev->isCoupled()) {
            if (prev->couple.link == this)
                return BackwardGo;
            else return BackwardTurn;
        }
        break;
    case TEn:
        setCode(TNone);
        break;
    case TNone:
        Q_ASSERT(!"QPathVertex::backward_eat: is this state possible?");;
    }
    if (cross_transfer)
        return ForwardTurn;
    else
        return BackwardTurn;
}


QPathVertex::Direction QPathVertex::turnForwardEat()
{
    switch (code) {
    case TEnEx:
        setCode(TNone);
        if (cross_transfer)
            return ForwardTurn;
        else
            return BackwardTurn;
    case TExEn:
        setCode(TEn);
        return BackwardGo;
    case TEn:
        setCode(TNone);
        return ForwardGo;
    case TEx:
        setCode(TNone);
        return BackwardGo;
    case TNone :
        Q_ASSERT(!"QPathVertex::turnForwardEat: is this state possible?");
    }
    return ForwardGo;
}


QPathVertex::Direction QPathVertex::turnBackwardEat()
{
    switch (code) {
    case TEnEx:
        setCode(TNone);
        if (cross_transfer)
            return BackwardTurn;
        else
            return ForwardTurn;
    case TExEn:
        setCode(TEx);
        return ForwardGo;
    case TEn:
        setCode(TNone);
        return ForwardGo;
    case TEx:
        setCode(TNone);
        return BackwardGo;
    case TNone :
        Q_ASSERT(!"QPathVertex::turnBackwardEat: is this state possible?");
    }
    return ForwardGo;
}


void QPathVertex::setIntersect(Degeneracy d)
{
    //don't want to reset the degenerate flag
    if (intersect == DNone || intersect == DIntersect)
        intersect = d;
}


QPathVertex::~QPathVertex()
{
}


struct QVertexList {
public:
    static QVertexList *fromPainterPath(const QPainterPath &path);
public:
    QPathVertex *node;

    QPathVertex *first_node;
    QPathVertex *last_node;
    QPathVertex *current_node;

    const int PATH_CACHE_SIZE;
    QPathVertex *cache;
    int current_cache;

    inline QPathVertex *allocateVertex(qreal x, qreal y, QPathVertex::Type t)
    {
        if (current_cache <= (PATH_CACHE_SIZE - 1)) {
            cache[current_cache] = QPathVertex(x, y, t);
            QPathVertex *vtx = &cache[current_cache];
            vtx->heap_allocated = false;
            ++current_cache;
            return vtx;
        } else
            return new QPathVertex(x, y, t);
    }

    QVertexList(int size)
        : node(0), first_node(0),
          last_node(0), current_node(0),
          PATH_CACHE_SIZE(size), current_cache(0)
    {
        cache = new QPathVertex[PATH_CACHE_SIZE];
    }

    ~QVertexList()
    {
        reset();
        delete [] cache;
    }

    void setCurrentNode(QPathVertex *a)
    {
	if (a)
            current_node = a;
	else
            Q_ASSERT(!"QVertexList:: will crash!");
    }

    void reset()
    {
	QPathVertex *a = first_node;

	while (a != 0) {
	    QPathVertex *n = a->next;
            if (a->heap_allocated)
                delete a;
	    a = n;
	}

	current_node = 0;
	first_node = 0;
	last_node = 0;
        current_cache = 0;
    }

    void makeRing()
    {
	if (!first_node || !last_node)
            return;

	first_node->prev = last_node;
	last_node->next = first_node;
    }

    void breakRing()
    {
	if (!first_node || !last_node) return;

	first_node->prev = 0;
	last_node->next = 0;
    }

    void delNode(QPathVertex *a)
    {
	if (a == 0) return;

	if (a == current_node)
	    current_node = a->prev;
	if (a->next)
	    a->next->prev = a->prev;
	if (a->prev)
	    a->prev->next = a->next;
	if (a == last_node)
	    last_node = a->prev;
	if (a == first_node)
	    first_node = a->next;
        if (a->heap_allocated)
            delete a;
    }

    void appendNode(QPathVertex *a)
    {
	a->prev = last_node;
	if (last_node)
	    last_node->next = a;
	if (first_node == 0)
	    first_node = a;
        last_node = a;
	current_node = a;
    }

    void insertNode(QPathVertex *a, QPathVertex *b)
    {
	setCurrentNode(b);

	if (current_node == b)
	    insertNode(a);
    }

    void insertNode(QPathVertex *a)
    {
	if (current_node == 0) {
	    appendNode(a);
	} else {
	    a->next = current_node;

	    if (current_node) {
		a->prev = current_node->prev;
		current_node->prev = a;
	    }

	    if (a->prev) a->prev->next = a;
	    if (current_node == first_node) first_node  = a;

	    current_node = a;
	}
    }

#ifdef QDEBUG_CLIPPER
    void dump();
#endif
};

struct QVertexListNavigate {

    const QVertexList &h;

    QPathVertex *cur;
    QPathVertex *first;
    QPathVertex *last;
    QPathVertex *prev;

    QPathVertex *lastMove;

    QVertexListNavigate(const QVertexList &hh)
	: h(hh), cur(hh.first_node),
          first(hh.first_node), last(hh.last_node),
          prev(hh.first_node->prev)
    {
        lastMove = cur;
    }

    ~QVertexListNavigate()
    {
    }

    inline operator bool() const
    {
	return cur ? true : false;
    }

    inline void forward()
    {
	cur = cur->next ? cur->next : first;
    }

    inline void backward()
    {
	cur = cur->prev ? cur->prev : last;
    }

    inline void next()
    {
        prev = cur;
        if (cur && cur->isMoveTo())
            lastMove = cur;
	cur = cur ? cur->next : 0;
    }

    inline QPathVertex *getNextNode() const
    {
	QPathVertex *nn = cur ? cur->next: 0;

        if (nn && nn->isMoveTo())
            return lastMove;
        if (!nn && lastMove) {
            if (lastMove->getRawType() == QPathVertex::MoveLineTo ||
                lastMove->getRawType() == QPathVertex::MoveCurveTo)
                return lastMove;
        }

	return nn;
    }

    inline QPathVertex *getPrevNode() const
    {
        return prev;
    }

    inline QPathVertex *getNode() const
    {
	return cur;
    }

    inline QPathVertex *getLastMove() const
    {
        return lastMove;
    }
};

QVertexList *QVertexList::fromPainterPath(const QPainterPath &path)
{
    QVertexList *lst = new QVertexList(path.elementCount());

    bool multipleMoves = false;
    QPathVertex *firstMove = 0;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch(e.type) {
        case QPainterPath::MoveToElement: {
            QPathVertex *lstMove = lst->allocateVertex(e.x, e.y,
                                                      QPathVertex::MoveTo);
            lst->appendNode(lstMove);
            multipleMoves = firstMove;
            if (!firstMove)
                firstMove = lstMove;
            break;
        }
        case QPainterPath::LineToElement: {
            if (i == (path.elementCount() - 1) && !multipleMoves &&
                qFuzzyCompare(firstMove->x, e.x) && qFuzzyCompare(firstMove->y, e.y)) {
                firstMove->setType(QPathVertex::MoveLineTo);
            } else {
                lst->appendNode(lst->allocateVertex(e.x, e.y,
                                                    QPathVertex::LineTo));
            }
            break;
        }
        case QPainterPath::CurveToElement: {
#ifdef QDEBUG_CLIPPER
            Q_ASSERT(path.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(path.elementAt(i+2).type == QPainterPath::CurveToDataElement);
#endif
            if (i == (path.elementCount() - 3) && !multipleMoves &&
                qFuzzyCompare(firstMove->x, e.x) && qFuzzyCompare(firstMove->y, e.y)) {
                firstMove->setType(QPathVertex::MoveCurveTo);
                firstMove->ctrl1 = QPointF(e.x, e.y);
                firstMove->ctrl2 = QPointF(path.elementAt(i+1).x, path.elementAt(i+1).y);
            } else {
                QPathVertex *vtx = lst->allocateVertex(path.elementAt(i+2).x,
                                                      path.elementAt(i+2).y,
                                                      QPathVertex::CurveTo);
                vtx->ctrl1 = QPointF(e.x, e.y);
                vtx->ctrl2 = QPointF(path.elementAt(i+1).x, path.elementAt(i+1).y);
                lst->appendNode(vtx);
            }
            i += 2;
            break;
        }
        case QPainterPath::CurveToDataElement:
            Q_ASSERT(!"nodesFromPath(), bad element type");
            break;
        }
    }

    return lst;
}

#ifdef QDEBUG_CLIPPER
void QVertexList::dump()
{
    QPathVertex *itr = first_node;

    int i = 0;
    while (itr) {
        qDebug()<<i<<") ["<<itr->x<<", "<<itr->y<<"] "
                <<", t="<<itr->getRawType()
                <<", d="<<itr->intersect
                <<", tf="<<itr->code
                <<", cf="<<itr->couple.info
                <<", alpha="<<itr->alpha
                <<", self="<<itr
                <<", neig="<<itr->neighbor;

        ++i;
        itr = itr->next;
        if (itr == first_node)
            break;
    }
}
#endif

static qreal dist(qreal x1, qreal y1, qreal x2, qreal y2)
{
    return qSqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}


static inline QBezier bezierFromNodes(QPathVertex *p1, QPathVertex *p2)
{
    if (p2->isCurveTo()) {
        return QBezier::fromPoints(QPointF(p1->x, p1->y),
                                   p2->ctrl1,
                                   p2->ctrl2,
                                   QPointF(p2->x, p2->y));
    } else {
        return QBezier::fromPoints(QPointF(p1->x, p1->y),
                                   p1->getPoint(),
                                   p2->getPoint(),
                                   QPointF(p2->x, p2->y));
    }
}

static inline QBezier reverseBezier(const QBezier &b)
{
    return QBezier::fromPoints(QPointF(b.x4, b.y4),
                               QPointF(b.x3, b.y3),
                               QPointF(b.x2, b.y2),
                               QPointF(b.x1, b.y1));
}

static inline bool isBezierBetween(const QPathVertex *prev,
                                   const QPathVertex *v,
                                   QBezier &bezier)
{
    const QPathVertex *start = prev;
    const QPathVertex *end = v;

    bool reverse = false;
    if (v->next == prev) {
        reverse = true;
        start = v;
        end = prev;
    } else if (v->next == prev->neighbor) {
        reverse = true;
        start = v;
        end = prev->neighbor;
    } else if (v->prev == prev->neighbor) {
        start = prev->neighbor;
    }

    const QPathVertex *origStart = start;
    const QPathVertex *origEnd = end;
    while (origStart->prev && origStart->getRawType() == QPathVertex::BezierIntersection)
        origStart = origStart->prev;
    while (origEnd->next && origEnd->getRawType() == QPathVertex::BezierIntersection)
        origEnd = origEnd->next;

    if (!origEnd->isCurveTo())
        return false;

    QBezier origBez = QBezier::fromPoints(origStart->getPoint(),
                                          origEnd->ctrl1,
                                          origEnd->ctrl2,
                                          origEnd->getPoint());
#ifdef QDEBUG_CLIPPER
    qDebug()<<"************* OrigOrigBez("
            <<origBez.x1<<", "<<origBez.y1 <<"  ,  "
            <<origBez.x2<<", "<<origBez.y2 <<"  ,  "
            <<origBez.x3<<", "<<origBez.y3 <<"  ,  "
            <<origBez.x4<<", "<<origBez.y4 <<")";
#endif

    qreal palpha = 0, nalpha = 1;
    if (start->getRawType() == QPathVertex::BezierIntersection)
        palpha = start->alpha;
    if (end->getRawType() == QPathVertex::BezierIntersection)
        nalpha = end->alpha;
#ifdef QDEBUG_CLIPPER
    qDebug()<<"\t"<<palpha<<nalpha;
    Q_ASSERT(palpha < nalpha);
#endif
    bezier = origBez.bezierOnInterval(palpha, nalpha);

    if (reverse)
        bezier = reverseBezier(bezier);

    return true;
}

static inline bool vertexAlreadyIntersected(QPathVertex *v, qreal alpha)
{
    return (v->intersect != QPathVertex::DNone &&
            (qFuzzyCompare(alpha, qreal(1.)) ||
             qFuzzyCompare(alpha, qreal(0.))));
}

static inline bool tryInjectingBezier(const QPathVertex *prev,
                                      const QPathVertex *v,
                                      QPainterPath &path)
{
    QBezier bezier;
    if (!isBezierBetween(prev, v, bezier))
        return false;
    //qDebug()<<"************* Bezier("
    //        <<bezier.x1<<", "<<bezier.y1 <<"  ,  "
    //        <<bezier.x2<<", "<<bezier.y2 <<"  ,  "
    //        <<bezier.x3<<", "<<bezier.y3 <<"  ,  "
    //        <<bezier.x4<<", "<<bezier.y4 <<")";

    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                 QPointF(bezier.x3, bezier.y3),
                 QPointF(bezier.x4, bezier.y4));

    return true;
}

static inline QBezier bezierOutOfIntersection(const QPathVertex *prev,
                                              const QPathVertex *curr)
{
#ifdef QDEBUG_CLIPPER
    Q_ASSERT(curr &&
             (curr->getRawType() == QPathVertex::BezierIntersection ||
              curr->getRawType() == QPathVertex::CurveTo ||
              curr->getRawType() == QPathVertex::MoveCurveTo));
#endif

    QBezier bezier = QBezier::fromPoints(prev->getPoint(),
                                         curr->ctrl1,
                                         curr->ctrl2,
                                         curr->getPoint());
    qreal alpha = (curr->getType(prev) == QPathVertex::CurveTo)
                  ? 1.0 : curr->alpha;

    bezier = bezier.bezierOnInterval(prev->alpha,
                                     alpha);

    if (qFuzzyCompare(bezier.x4, prev->x) &&
        qFuzzyCompare(bezier.y4, prev->y))
        return reverseBezier(bezier);
    else
        return bezier;
}

class QPathClipper::Private
{
public:
    Private()
        : subject(0),
          clipper(0)
    {
    }
    Private(const QPainterPath &s,
            const QPainterPath &c)
        : subjectPath(s),
          clipPath(c),
          subject(0), clipper(0)
    {
    }
    ~Private()
    {
        delete subject;
        delete clipper;
    }

    inline bool isEdgeIn(QPathVertex *a, QPathVertex *b)
    {
	QPathVertex *c = a->neighbor;
	QPathVertex *d = b->neighbor;

	if (!c || !d) return 0;
        //qDebug()<<"isedge in "<<a<<b;
	if ((c->next == d && d->prev == c) ||
	    (c->prev == d && d->next == c)) {
	    return true;
        }
	return false;
    }

    void makeRing()
    {
        subject->makeRing();
        clipper->makeRing();
    }
    void breakRing()
    {
        subject->breakRing();
        clipper->breakRing();
    }

    QPathVertex *getUnprocessed()
    {
        for (QVertexListNavigate dh(*subject); dh ; dh.next()) {
            QPathVertex *cur = dh.getNode();
            QPathVertex::TraversalFlag now = cur->code;

            if (now != QPathVertex::TNone) {
                if (cur->isCoupled()) {

                    QPathVertex::CoupleFlag which = cur->couple.info;
                    QPathVertex *link = cur->couple.link;

                    if (link->code == QPathVertex::TNone) continue;

                    if (which == QPathVertex::FrontElement &&
                        now == QPathVertex::TEn) continue;
                    if (which == QPathVertex::RearElement &&
                        now == QPathVertex::TEx) continue;

                    return cur;
                } else
                    return cur;
            }
        }
        return 0;
    }


    bool walkResultingPath(QPathVertex  *start,
                           QPathVertex  *&prev_code_owner,
                           QPathVertex  *&current,
                           QPathVertex::Direction &traversal_stat,
                           QList<QPathVertex*> &notebook)
    {
        if (current == start && traversal_stat != QPathVertex::Stop) {
            traversal_stat = QPathVertex::Stop;
            return false;
        }

        if (current->code != QPathVertex::TNone) {

            switch (traversal_stat) {
            case QPathVertex::Stop:
                traversal_stat = current->eat();
                notebook.append(current);
                prev_code_owner = current;
                break;
            case QPathVertex::ForwardTurn:
                traversal_stat = current->turnForwardEat();
                prev_code_owner = current;
                break;
            case QPathVertex::BackwardTurn:
                traversal_stat = current->turnBackwardEat();
                prev_code_owner = current;
                break;
            case QPathVertex::ForwardGo:
                traversal_stat = current->forwardEat(prev_code_owner);
                prev_code_owner = current;
                break;
            case QPathVertex::BackwardGo:
                traversal_stat = current->backwardEat(prev_code_owner);
                prev_code_owner = current;
                break;
            default:
                Q_ASSERT(!"PathClipper::walkPat: unexpected state!!");
            }
        }

        //qDebug()<<"current is "<<current<<traversal_stat<<current->code
        //        <<current->getPoint();
        switch (traversal_stat) {
        case QPathVertex::BackwardTurn:
        case QPathVertex::ForwardTurn:
            current = current->neighbor;
            break;
        case QPathVertex::ForwardGo:
            current = current->next;
            notebook.append(current);
            break;
        case QPathVertex::BackwardGo:
            current = current->prev;
            notebook.append(current);
            break;
        default:
            qWarning("ignoring flag...");
        }

        return true;
    }

    enum PointTest {
        AinsideB,
        AoutsideB
    };

    void getExpressions(PointTest &op1,
                        PointTest &op2)
    {
        if (op == BoolAnd) {
            op1 = AinsideB;
            op2 = AinsideB;
        } else if (op == BoolOr) {
            op1 = AoutsideB;
            op2 = AoutsideB;
        } else if (op == BoolSub) {
            op1 = AoutsideB;
            op2 = AinsideB;
        } else if (op == BoolInSub) {
            op1 = AinsideB;
            op2 = AoutsideB;
        }
    }

    enum PointLocation {
        LOut,
        LIn,
        LOn
    };

    PointLocation classifyPointLocation(const QPointF &point,
                                        const QPainterPath &B,
                                        PointTest op)
    {
        bool isContained = B.contains(point);
        //qDebug()<<"\tB  "<<point<<", contained = "<<isContained;
        if (op == AinsideB) {
            if (isContained)
                return LIn;
            else
                return LOut;
        }

        if (op == AoutsideB) {
            if (isContained)
                return LOut;
            else
                return LIn;
        }
        Q_ASSERT(!"Should never get here!");
        return LOut;
    }

    static inline QPointF midPoint(QPathVertex *one, QPathVertex *two)
    {
        if (two->getRawType() != QPathVertex::BezierIntersection &&
            !two->isCurveTo()) {
            qreal xMid = (one->x+two->x)/2;
            qreal yMid = (one->y+two->y)/2;
            return QPointF(xMid, yMid);
        } else {
            QBezier bezier;
            if (!isBezierBetween(one, two, bezier)) {
                //This happens when a line intersects a bezier. the intersection
                //on the line is still marked as BezierIntersection.
                //qWarning("Couldn't form a bezier curve out of bezier-vertices!");
                return QPointF((one->x+two->x)/2,
                               (one->y+two->y)/2);
            }
            QPointF pt = bezier.midPoint();
            //qDebug()<<"\t"<<bezier;
            //qDebug()<<qSetRealNumberPrecision(12)<<"\t point between"<<one<<two<<pt;
            return pt;
        }
    }

    QPathVertex::TraversalFlag generateCode(QPathVertex::TraversalFlag code,
                                           QPathVertex *prev,
                                           QPathVertex *cur,
                                           QPathVertex *next,
                                           const QPainterPath & B_p,
                                           PointTest op)
    {
        if (cur->intersect == QPathVertex::DNone)
            return QPathVertex::TNone;

        PointLocation  prev_s;
        PointLocation  next_s;

        QPointF  prev_p  = prev->getPoint();
        QPointF  cur_p   = cur->getPoint();
        QPointF  next_p  = next->getPoint();


        if (isEdgeIn(prev, cur)) {
            prev_s = LOn;
        } else {
            if (code == QPathVertex::TEx || code == QPathVertex::TEnEx)
                prev_s = LOut;
            else if (code == QPathVertex::TEn || code == QPathVertex::TExEn)
                prev_s = LIn;
            else
                prev_s = classifyPointLocation(midPoint(prev, cur), B_p, op);

            //Q_ASSERT(prev_s == classifyPointLocation(midPoint(prev, cur), B_p, op));
        }

        if (isEdgeIn(cur, next))
            next_s = LOn;
        else {
            next_s = classifyPointLocation(midPoint(cur, next), B_p, op);
        }
#ifdef QDEBUG_CLIPPER
        qDebug()<<"XXX Generating code = "<<prev_p<<cur_p<<next_p<<prev_s<<next_s;
#endif
        if (prev_s == LOn  && next_s == LOn)  return QPathVertex::TNone;
        if (prev_s == LOn  && next_s == LOut) return QPathVertex::TEx;
        if (prev_s == LOn  && next_s == LIn)  return QPathVertex::TEn;
        if (prev_s == LOut && next_s == LOn)  return QPathVertex::TEn;
        if (prev_s == LIn  && next_s == LOn)  return QPathVertex::TEx;
        if (prev_s == LIn  && next_s == LOut) return QPathVertex::TEx;
        if (prev_s == LOut && next_s == LIn)  return QPathVertex::TEn;
        if (prev_s == LIn  && next_s == LIn)  return QPathVertex::TExEn;
        if (prev_s == LOut && next_s == LOut) return QPathVertex::TEnEx;

        return QPathVertex::TNone;
    }

    void markForBooleanOperation()
    {
        PointTest which1=AinsideB, which2=AinsideB;//and is the default
        getExpressions(which1, which2);

        makeRing();

        QPathVertex *start, *cur, *prev, *next;

        QVertexListNavigate subjItr(*subject);
        start = cur = subjItr.getNode();
        prev = subjItr.getPrevNode();
        next = subjItr.getNextNode();

        QPathVertex::TraversalFlag prev_code = QPathVertex::TNone;

        ///////////////////////////////////////////////////////////////////
        /// subject of the coding is subject against the region of clipper
        while (true) {
            QPathVertex::TraversalFlag code = generateCode(prev_code, prev,
                                                          cur, next, clipPath, which1);
            if (code != QPathVertex::TNone)
                prev_code = code;

            cur->setCode(code);

            subjItr.next();
            prev = cur;
            cur	 = subjItr.getNode();
            next = subjItr.getNextNode();

            if (cur == start) break;
        }
        QVertexListNavigate clipItr(*clipper);
        start = cur = clipItr.getNode();
        prev = clipItr.getPrevNode();
        next = clipItr.getNextNode();

        prev_code = QPathVertex::TNone;
        //qDebug()<<"//////////////////////////////////////////////////////";
        ///////////////////////////////////////////////////////////////////
        /// subject of the coding is clipper against the region of subject
        while (true) {
            QPathVertex::TraversalFlag code = generateCode(prev_code, prev,
                                                          cur, next, subjectPath, which2);

            if (code != QPathVertex::TNone) prev_code = code;

            cur->setCode(code);

            clipItr.next();
            prev = cur;
            cur	 = clipItr.getNode();
            next = clipItr.getNextNode();

            if (cur == start) break;
        }

        breakRing();
    }

    inline bool getNextWhichHasCode(QVertexListNavigate &ln,
                                    QPathVertex *start)
    {
	while (1) {
	    ln.forward();
	    QPathVertex *cur = ln.getNode();
	    QPathVertex::TraversalFlag code = cur->code;

	    if (cur == start)                return false;
	    if (code != QPathVertex::TNone)   return true;
	}
    }

    inline bool triArea(const QPointF &a,
                        const QPointF &b,
                        const QPointF &c)
    {
        return 0.5 * (a.x() * (b.y() - c.y()) +
                      b.x() * (c.y() - a.y()) +
                      c.x() * (a.y() - b.y()));
    }

    void encodeCrossTransfer(QPathVertex *c)
    {
        QPointF p  = c->getPoint();
        QPointF p1 = c->prev->getPoint();
        QPointF p2 = c->next->getPoint();
        QPointF q1 = c->neighbor->prev->getPoint();
        QPointF q2 = c->neighbor->next->getPoint();

        qreal t1 = triArea(q2, p1, q1);
        qreal t2 = triArea(p2, p1, q1);

        if (t1 * t2 < 0) {
            c->cross_transfer = true;
            c->neighbor->cross_transfer = true;
        }
    }

    void findCouplesAndCrossTransfers()
    {
        makeRing();

        QPathVertex *cur, *start, *next;
        QVertexListNavigate ln(*subject);

        start = ln.getNode();

        if (getNextWhichHasCode(ln, start)) {

            start = cur = ln.getNode();

            QPathVertex::TraversalFlag cur_code = cur->code, next_code;

            if (cur_code == QPathVertex::TEnEx || cur_code == QPathVertex::TExEn)
                encodeCrossTransfer(cur);

            while (true) {
                int will_be_continued = getNextWhichHasCode(ln, start);
                next = ln.getNode();
                next_code = next->code;

                if (cur == next) break;

                if (cur_code == QPathVertex::TEn || cur_code == QPathVertex::TEx) {
                    if (cur_code == next_code) {
                        cur->setCouple(QPathVertex::FrontElement, next);
                        next->setCouple(QPathVertex::RearElement, cur);
                    }
                }

                cur = next;
                cur_code = next_code;

                if (!will_be_continued) break;

                if (cur_code == QPathVertex::TEnEx || cur_code == QPathVertex::TExEn)
                    encodeCrossTransfer(cur);
            }
        }

        QVertexListNavigate ln1(*clipper);

        start = ln1.getNode();

        if (getNextWhichHasCode(ln1, start)) {

            start = cur = ln1.getNode();

            QPathVertex::TraversalFlag cur_code = cur->code, next_code;

            while (true) {
                int will_be_continued = getNextWhichHasCode(ln1, start);
                next = ln1.getNode();
                next_code = next->code;

                if (cur == next) break;

                if (cur_code == QPathVertex::TEn || cur_code == QPathVertex::TEx) {
                    if (cur_code == next_code) {
                        cur->setCouple(QPathVertex::FrontElement, next);
                        next->setCouple(QPathVertex::RearElement, cur);
                    }
                }
                cur = next;
                cur_code = next_code;

                if (!will_be_continued) break;
            }
        }


        breakRing();
    }

    struct Intersection
    {
        Intersection(qreal xx, qreal yy,
                     QPathVertex::Type t,
                     qreal ap, qreal aq,
                     QPathVertex *pp1, QPathVertex *pp2,
                     QPathVertex *qq1, QPathVertex *qq2,
                     QVertexList *&pLst, QVertexList *&qLst)
            : x(xx), y(yy), type(t),
              alpha_p(ap), alpha_q(aq),
              p1(pp1), p2(pp2), q1(qq1), q2(qq2),
              pList(pLst), qList(qLst)
        {
#ifdef QDEBUG_CLIPPER
            qDebug()<<qSetRealNumberPrecision(12)<<"Intersection at "<<xx<<yy
                    <<"between : "<<p1->getPoint()<<" and "<<p2->getPoint()
                    <<" --- "<<q1->getPoint()<<" and "<<q2->getPoint()
                    <<" at "<<alpha_p<<alpha_q;
#endif
        }
        qreal x, y;
        QPathVertex::Type type;
        qreal alpha_p, alpha_q;

        QPathVertex *p1, *p2;
        QPathVertex *q1, *q2;

        QVertexList *pList;
        QVertexList *qList;

        QPathVertex *intersection(qreal alpha,
                                 QPathVertex *one, QPathVertex *two,
                                 QVertexList &lst, bool &created) const
        {
            QPathVertex *v = 0;
            created = false;
            if (qFuzzyCompare(alpha, 0)) {
                v = one;
            } else if (qFuzzyCompare(alpha, 1)) {
                v = two;
            } else {
                v = lst.allocateVertex(x, y, type);
                v->alpha = alpha;
                one = one->next;
                while (one && one != two &&
                       (one->intersect != QPathVertex::DNone &&
                        alpha > one->alpha))
                    one = one->next;
                if (one)
                    lst.insertNode(v, one);
                else
                    lst.appendNode(v);
                created = true;
            }
            return v;
        }
        void insert() const
        {
            QPathVertex::Degeneracy d = (qFuzzyCompare(alpha_p, 0) ||
                                        qFuzzyCompare(alpha_q, 0) ||
                                        qFuzzyCompare(alpha_p, 1) ||
                                        qFuzzyCompare(alpha_q, 1))
                                       ? QPathVertex::DDegenerate
                                       : QPathVertex::DIntersect;
            bool  newlyCreated1 = false, newlyCreated2 = false;
            QPathVertex *sinter = intersection(alpha_p, p1, p2, *pList, newlyCreated1);
            QPathVertex *cinter = intersection(alpha_q, q1, q2, *qList, newlyCreated2);

            if (!newlyCreated1) {
                if (vertexAlreadyIntersected(sinter, alpha_p)) {
                    if (newlyCreated2) {
                        //qDebug()<<"----- 1 already intersected = "<<sinter->getPoint();
                        qList->delNode(cinter);
                        return;
                    }
                }
            }
            if (!newlyCreated2) {
                if (vertexAlreadyIntersected(cinter, alpha_q)) {
                    if (newlyCreated1) {
                        //qDebug()<<"----- 2 already intersected = "<<cinter->getPoint();
                        pList->delNode(sinter);
                        return;
                    }
                }
            }

            sinter->neighbor = cinter;
            cinter->neighbor = sinter;
            sinter->intersect = d;
            cinter->intersect = d;

            if (p2->isCurveTo()) {
                sinter->ctrl1 = p2->ctrl1;
                sinter->ctrl2 = p2->ctrl2;
            }
            if (q2->isCurveTo()) {
                cinter->ctrl1 = q2->ctrl1;
                cinter->ctrl2 = q2->ctrl2;
            }
        }
    };
    QList<Intersection> intersections;

    bool intersectBeziers(QPathVertex *p1, QPathVertex *p2,
                          QPathVertex *q1, QPathVertex *q2)
    {
        QBezier one = bezierFromNodes(p1, p2);
        QBezier two = bezierFromNodes(q1, q2);

        QVector< QList<qreal> > inters = QBezier::findIntersections(one, two);

        //qDebug()<<"\tIntersecting: one = "<<one
        //        <<"\n\t\t two = "<<two
        //        <<"\n\t\t == "<<inters[0]<<inters[1];
        if (inters[0].isEmpty())
            return false;
        const QList<qreal> &alpha_ps = inters[0];
        const QList<qreal> &alpha_qs = inters[1];
        int count = alpha_ps.size();

        for (int i = 0; i < count; ++i) {
            qreal alpha_p = alpha_ps[i];
            qreal alpha_q = alpha_qs[i];
            QPointF pt = one.pointAt(alpha_p);

            intersections.append(
                Intersection(pt.x(), pt.y(), QPathVertex::BezierIntersection,
                             alpha_p, alpha_q,
                             p1, p2, q1, q2,
                             subject, clipper));
        }
        return true;
    }

    bool intersectLines(QPathVertex *p1, QPathVertex *p2,
                        QPathVertex *q1, QPathVertex *q2)
    {
        qreal x, y, tp, tq, par;

        par = ((p2->x - p1->x)*(q2->y - q1->y) -
               (p2->y - p1->y)*(q2->x - q1->x));

        if (qFuzzyCompare(par, qreal(0.0))) {
            //FIXME
            //qDebug("possibly skipping coinciding edges...");
            return false;        /* parallel lines */
        }

        tp = ((q1->x - p1->x)*(q2->y - q1->y) -
              (q1->y - p1->y)*(q2->x - q1->x))/par;
        tq = ((p2->y - p1->y)*(q1->x - p1->x) -
              (p2->x - p1->x)*(q1->y - p1->y))/par;

        if (tp<0 || tp>1 || tq<0 || tq>1)
            return false;

        //qDebug().nospace()<<"........ ["<<QPointF(p1->x, p1->y)<<QPointF(p2->x, p2->y)
        //                  <<"], ["<<QPointF(q1->x, q1->y)<<QPointF(q2->x, q2->y)<<"]"
        //                  <<", p = "<<par<<", tp = "<<tp<<", tq = "<<tq;

        x = p1->x + tp*(p2->x - p1->x);
        y = p1->y + tp*(p2->y - p1->y);


        //qDebug()<<"1 Vertex"<<p1->x<<p1->y<<tp;
        //qDebug()<<"2 Vertex"<<p2->x<<p2->y<<tp;
        //qDebug()<<"+++++++++++++++ "<<x<<y;

        qreal nalpha_p = dist(p1->x, p1->y, x, y) /
                         dist(p1->x, p1->y, p2->x, p2->y);
        qreal nalpha_q = dist(q1->x, q1->y, x, y) /
                         dist(q1->x, q1->y, q2->x, q2->y);

        intersections.append(
            Intersection(x, y, QPathVertex::LineIntersection, nalpha_p, nalpha_q,
                         p1, p2, q1, q2, subject, clipper));

        return true;
    }

    void intersectEdges(QPathVertex *a, QPathVertex *b,
                        QPathVertex *c, QPathVertex *d)
    {
#ifdef QDEBUG_CLIPPER
        Q_ASSERT(a->intersect == QPathVertex::DNone);
        Q_ASSERT(b->intersect == QPathVertex::DNone);
        Q_ASSERT(c->intersect == QPathVertex::DNone);
        Q_ASSERT(d->intersect == QPathVertex::DNone);
#endif

        if (b->isCurveTo() || d->isCurveTo()) {
            intersectBeziers(a, b, c, d);
        } else {
            intersectLines(a, b, c, d);
        }
    }
    bool doEdgesIntersect(QPathVertex *a, QPathVertex *b,
                          QPathVertex *c, QPathVertex *d)
    {
#ifdef QDEBUG_CLIPPER
        Q_ASSERT(a->intersect == QPathVertex::DNone);
        Q_ASSERT(b->intersect == QPathVertex::DNone);
        Q_ASSERT(c->intersect == QPathVertex::DNone);
        Q_ASSERT(d->intersect == QPathVertex::DNone);
#endif

        if (b->isCurveTo() || d->isCurveTo()) {
            return intersectBeziers(a, b, c, d);
        } else {
            return intersectLines(a, b, c, d);
        }
    }

    void findIntersections()
    {
        QRectF subjControl = subjectPath.controlPointRect();
        QRectF clipControl = clipPath.controlPointRect();

        if (!subjControl.intersects(clipControl)) {
            // no way we could intersect
            return;
        }

        for (QVertexListNavigate subj(*subject); subj ; subj.next()) {
            QPathVertex *a = subj.getNode();
            QPathVertex *b = (subj.getNextNode())?subj.getNextNode():subj.getLastMove();
            if (!b)
                break;

            for (QVertexListNavigate obj(*clipper); obj ; obj.next()) {
                QPathVertex *c = obj.getNode();
                QPathVertex *d = (obj.getNextNode())?obj.getNextNode():obj.getLastMove();;
                if (!d)
                    break;

                intersectEdges(a, b,
                               c, d);
            }
        }
        for (QList<Intersection>::const_iterator itr = intersections.constBegin();
             itr != intersections.constEnd(); ++itr) {
            const Intersection &inter = (*itr);
            inter.insert();
        }
    }

    bool areIntersecting()
    {
        QRectF subjControl = subjectPath.controlPointRect();
        QRectF clipControl = clipPath.controlPointRect();

        bool intersects = false;
        QRectF r1 = subjControl.normalized();
        QRectF r2 = clipControl.normalized();
        if (qMax(r1.x(), r2.x()) > qMin(r1.x() + r1.width(), r2.x() + r2.width()) ||
            qMax(r1.y(), r2.y()) > qMin(r1.y() + r1.height(), r2.y() + r2.height())) {
            // no way we could intersect
#ifdef QDEBUG_CLIPPER
            qDebug()<<"Boundries not intersecting : " << subjControl <<clipControl;
            qDebug()<<"max x = "<<qMax(r1.x(), r2.x())<< " < "
                    <<qMin(r1.x() + r1.width(), r2.x() + r2.width());
            qDebug()<<"max y = "<<qMax(r1.y(), r2.y()) << " < "
                    <<qMin(r1.y() + r1.height(), r2.y() + r2.height());
#endif
            return intersects;
        }

#ifdef QDEBUG_CLIPPER
        qDebug("---- Subject and clipper state ---");
        subject->dump();
        clipper->dump();
        qDebug("---- end state info ----");
#endif
        for (QVertexListNavigate subj(*subject); subj ; subj.next()) {
            QPathVertex *a = subj.getNode();
            QPathVertex *b = (subj.getNextNode());
            if (!a || !b)
                break;

            for (QVertexListNavigate obj(*clipper); obj ; obj.next()) {
                QPathVertex *c = obj.getNode();
                QPathVertex *d = (obj.getNextNode());
                if (!c || !d)
                    break;
                //qDebug()<<"intersecting = ";
                //qDebug()<< "\t1) "<<(*a) << " and " << (*b);
                //qDebug()<< "\t2) "<<(*c) << " and " << (*d);
                intersects = doEdgesIntersect(a, b,
                                              c, d);
                if (intersects) {
                    //qDebug()<<"-------- Found intersection";
                    return true;
                }
            }
        }
        return intersects;
    }

    QPainterPath pathFromList(const QList<QPathVertex*> &lst)
    {
        QList<QPathVertex*>::const_iterator itr;
        QPainterPath path;

        int i = 0;
        const QPathVertex *prev = 0;
        //qDebug()<<lst;
        for (itr = lst.constBegin(); itr != lst.constEnd(); ++itr) {
            const QPathVertex *const v = *itr;
            QList<QPathVertex*>::const_iterator nextItr = itr;
            ++nextItr;
#ifdef QDEBUG_CLIPPER
            qDebug()<<i<<")Vtx = "<<v->x<<","<<v->y<<" | "
                    <<v->getRawType() <<", "<<v;
#endif
            ++i;
            if (!path.elementCount()) {
                path.moveTo(v->getPoint());
                prev = v;
                continue;
            }
            if (prev && qFuzzyCompare(prev->x, v->x) &&
                qFuzzyCompare(prev->y, v->y)) {
                prev = v;
                continue;
            }
            if (tryInjectingBezier(prev, v, path)) {
                prev = v;
                continue;
            }
            bool traversingReverse = (prev &&
                                      (prev == v->next || (prev->neighbor && prev->neighbor == v->next)));
            bool prevWasBezier = (prev && prev->getRawType() == QPathVertex::CurveTo &&
                                  v->next == prev);
#ifdef QDEBUG_CLIPPER
            qDebug()<<"\t TR = "<<traversingReverse<<", wasBezier = "
                    <<prevWasBezier;
            qDebug()<<prev<<v->next<<prev->neighbor<<v->next;
#endif
            switch(v->getType(prev)) {
            case QPathVertex::MoveTo:
                if (prevWasBezier && traversingReverse) {
                    QBezier bezier = bezierOutOfIntersection(v, prev);
                    bezier = reverseBezier(bezier);
                    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                                 QPointF(bezier.x3, bezier.y3),
                                 QPointF(bezier.x4, bezier.y4));
                } else {
                    path.lineTo(v->getPoint());
                }
                break;
            case QPathVertex::LineTo:
                if (prevWasBezier && traversingReverse) {
                    QBezier bezier = bezierOutOfIntersection(v, prev);
                    bezier = reverseBezier(bezier);
                    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                                 QPointF(bezier.x3, bezier.y3),
                                 QPointF(bezier.x4, bezier.y4));
                } else {
                    path.lineTo(v->getPoint());
                }
                break;
            case QPathVertex::CurveTo: {
                if (prevWasBezier && traversingReverse) {
                    QBezier bezier = bezierOutOfIntersection(v, prev);
                    bezier = reverseBezier(bezier);
                    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                                 QPointF(bezier.x3, bezier.y3),
                                 QPointF(bezier.x4, bezier.y4));
                } else if (traversingReverse) {
                    path.lineTo(v->getPoint());
                } else {
                    QBezier bezier = bezierOutOfIntersection(prev, v);
                    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                                 QPointF(bezier.x3, bezier.y3),
                                 QPointF(bezier.x4, bezier.y4));
                }
            }
                break;
            case QPathVertex::LineIntersection:
                if (prevWasBezier && traversingReverse) {
                    QBezier bezier = bezierOutOfIntersection(v, prev);
                    bezier = reverseBezier(bezier);
                    path.cubicTo(QPointF(bezier.x2, bezier.y2),
                                 QPointF(bezier.x3, bezier.y3),
                                 QPointF(bezier.x4, bezier.y4));
                } else {
                    path.lineTo(v->getPoint());
                }
                break;
            case QPathVertex::BezierIntersection: {
                //since injecting failed, this should mean
                //we're traversing a region of lines
                path.lineTo(v->getPoint());
            }
                break;
            case QPathVertex::MoveLineTo:
            case QPathVertex::MoveCurveTo:
                Q_ASSERT(!"Unhandled element");
                break;
            default:
                Q_ASSERT(!"Unrecognized Vertex type");
            }
            prev = v;
        }
        return path;
    }


    QPainterPath subjectPath;
    QPainterPath clipPath;
    Operation    op;

    QVertexList *subject;
    QVertexList *clipper;
};

QPathClipper::QPathClipper()
    : d(new Private)
{
}


QPathClipper::QPathClipper(const QPainterPath &subject,
                           const QPainterPath &clip)
    : d(new Private)
{
    setSubjectPath(subject);
    setClipPath(clip);
}

QPathClipper::~QPathClipper()
{
    delete d;
    d = 0;
}

void QPathClipper::setSubjectPath(const QPainterPath &path)
{
    d->subjectPath = path;
    delete d->subject;
    d->subject = QVertexList::fromPainterPath(path);
}


QPainterPath QPathClipper::subjectPath() const
{
    return d->subjectPath;
}


void QPathClipper::setClipPath(const QPainterPath &path)
{
    d->clipPath = path;
    delete d->clipper;
    d->clipper = QVertexList::fromPainterPath(path);
}


QPainterPath QPathClipper::clipPath() const
{
    return d->clipPath;
}

QPainterPath QPathClipper::clip(Operation op)
{
    d->op = op;

#ifdef QDEBUG_CLIPPER
    qDebug("--- subject clipper state ----");
    d->subject->dump();
    d->clipper->dump();
    qDebug("---- subject clipper state end ----");
#endif

    d->findIntersections();

    if (d->intersections.isEmpty()) { //no intersections
        bool clipInSubject = d->subjectPath.contains(d->clipPath.elementAt(0));
        bool subjectInClip = d->clipPath.contains(d->subjectPath.elementAt(0));
        QPainterPath result;
        switch(d->op) {
        case QPathClipper::BoolAnd:
            if (clipInSubject)
                result = d->clipPath;
            else if (subjectInClip)
                result = d->subjectPath;
            break;
        case QPathClipper::BoolOr:
            if (clipInSubject)
                result = d->subjectPath;
            else if (subjectInClip)
                result = d->clipPath;
            else {
                result.addPath(d->subjectPath);
                result.addPath(d->clipPath);
            }
            break;
        case QPathClipper::BoolSub:
            if (subjectInClip) {
                return QPainterPath();
            } else if (clipInSubject) {
                result = d->subjectPath;
                result.addPath(d->clipPath);
            } else {
                result = d->subjectPath;
            }
            break;
        case QPathClipper::BoolInSub:
            if (clipInSubject || subjectInClip) {
                result = d->clipPath;
                result.addPath(d->subjectPath);
            } else {
                result = d->clipPath;
            }
            break;
        }
        return result;
    }

    d->markForBooleanOperation();

    d->findCouplesAndCrossTransfers();

#ifdef QDEBUG_CLIPPER
    d->subject->dump();
    d->clipper->dump();
#endif

    d->makeRing();

    QPathVertex *current, *start, *prev_code_owner = 0;

    QPainterPath result;

    while (true) {
        current = 0;

        d->breakRing();
        current = d->getUnprocessed();
        d->makeRing();

        if (!current) break;

        start = current;
        bool not_over = true;

        QPathVertex::Direction traversal_stat = QPathVertex::Stop;

        QList<QPathVertex*> vertices;
        while (not_over)
            not_over = d->walkResultingPath(start, prev_code_owner,
                                            current, traversal_stat, vertices);

        result.addPath(d->pathFromList(vertices));
    }

    d->breakRing();

    return result;
}

bool QPathClipper::intersect()
{
    return d->areIntersecting();
}

bool QPathClipper::contains()
{
    bool intersect = d->areIntersecting();

    //we have an intersection clearly we can't be fully contained
    if (intersect)
        return false;

    //if there's no intersections the path is already completely outside
    //or fully inside. if the first element of the clip is inside then
    //due to no intersections, the rest will be inside as well...
    return d->subjectPath.contains(d->clipPath.elementAt(0));
}

QT_END_NAMESPACE
