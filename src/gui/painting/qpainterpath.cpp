#include "qpainterpath.h"
#include "qpainterpath_p.h"

#include <private/qobject_p.h>
#include <qlist.h>


/**********************************************************************
 * class: QPainterPathElement
 */
QPoint QPainterPathElement::firstPoint() const
{
    switch (type) {
    case Line:
	return QPoint(lineData.x1, lineData.y1);
    }
    qFatal("QPainterPathElement::firstPoint(), unhandled type: %d", type);
    return QPoint(0, 0);
}

/**********************************************************************
 * class: QPainterSubpath
 */
void QPainterSubpath::connectLast(const QPoint &p)
{
    if (elements.size() > 0 && p != lastPoint) {
	printf(" -> autoconnecting to last point\n");
	addLine(lastPoint, p);
    }
}


void QPainterSubpath::close()
{
    Q_ASSERT(!elements.isEmpty());
    QPainterPathElement firstElement = elements.at(0);
    printf(" -> trying to reconnect to first point\n");
    connectLast(firstElement.firstPoint());
}
    
void QPainterSubpath::addLine(const QPoint &p1, const QPoint &p2)
{
    connectLast(p1);
    QPainterPathElement elm;
    elm.type = QPainterPathElement::Line;
    elm.lineData.x1 = p1.x();
    elm.lineData.y1 = p1.y();
    elm.lineData.x2 = p2.x();
    elm.lineData.y2 = p2.y();
    elements.append(elm);

    printf(" -> added line element (%d, %d) -> (%d, %d)\n",
	   elm.lineData.x1, 
	   elm.lineData.y1, 
	   elm.lineData.x2, 
	   elm.lineData.y2); 
	
    lastPoint = p2;
}

QPointArray QPainterSubpath::toPolygon() const
{
    if (elements.isEmpty())
	return QPointArray();
    QPointArray p; 
    fflush(stdout);
    p << elements.at(0).firstPoint();
    for (int i=0; i<elements.size(); ++i) {
	switch (elements.at(i).type) {
	case QPainterPathElement::Line:
	    p << QPoint(elements.at(i).lineData.x2, elements.at(i).lineData.y2);
	    break;
	default:
	    qFatal("QPainterSubpath::toPolygon() unhandled case...");
	}
    }
    return p;
}

/**********************************************************************
 * class: QPainterPath
 */
QPainterPath::QPainterPath()
{
    d = new QPainterPathPrivate;
    d->subpaths.append(QPainterSubpath());
}

QPainterPath::~QPainterPath()
{
    delete d;
}


/*!
 * Begins a new subpath. A path is already begun when the path
 * is started. If a path is begun and no elements are added to
 * it, the function does nothing.
 * The current subpath is not automatically closed when a new
 * subpath is begun.
 */
void QPainterPath::beginSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
	return;
    printf(" -> beginning a new path\n");
    d->subpaths.append(QPainterSubpath());
}


/*!
 * Closes the current subpath. If the sub path does not contain
 * any elements, the function does nothing. A new subpath is
 * automatically begun when the current is closed.
 */ 
void QPainterPath::closeSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
	return;
    d->subpaths.last().close();
    d->subpaths.append(QPainterSubpath());
}


/*!
 * Adds the line defined by the starting point \a p1 and endpoint
 * \a p2 to the path.
 */
void QPainterPath::addLine(const QPoint &p1, const QPoint &p2)
{
    d->subpaths.last().addLine(p1, p2);    
}


