#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>

#include "qabstracttextdocumentlayout_p.h"
#define d d_func()
#define q q_func()

QAbstractTextDocumentLayout::QAbstractTextDocumentLayout()
    : QObject(*new QAbstractTextDocumentLayoutPrivate, 0)
{
}


void QAbstractTextDocumentLayout::registerHandler(int formatType, QObject *component)
{
    QTextObjectInterface *iface = qt_cast<QTextObjectInterface *>(component);
    if (!iface)
	return; // ### print error message on terminal?

    connect(component, SIGNAL(destroyed(QObject *)), this, SLOT(handlerDestroyed(QObject *)));

    QTextObjectHandler h;
    h.iface = iface;
    h.component = component;
    d->handlers.insert(formatType, h);
}

void QAbstractTextDocumentLayout::layoutObject(QTextObject item, const QTextFormat &format)
{
    QTextObjectHandler handler = d->handlers.value(format.type());
    if (!handler.component)
	return;
    handler.iface->layoutObject(item, format);
}

void QAbstractTextDocumentLayout::drawObject(QPainter *p, const QPoint &position, QTextObject item,
					     const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextObjectHandler handler = d->handlers.value(format.type());
    if (!handler.component)
	return;
    handler.iface->drawObject(p, position, item, format, selType);
}

void QAbstractTextDocumentLayout::handlerDestroyed(QObject *obj)
{
    HandlerHash::Iterator it = d->handlers.begin();
    while (it != d->handlers.end())
	if ((*it).component == obj)
	    it = d->handlers.erase(it);
	else
	    ++it;
}


void QAbstractTextDocumentLayout::invalidate(const QRect &r)
{
}

void QAbstractTextDocumentLayout::invalidate(const QRegion &r)
{
}

QTextLayout *QAbstractTextDocumentLayout::layoutAt(int position) const
{
    return 0;
}


void QAbstractTextDocumentLayout::setPageSize(const QSize &size)
{
    d->pageSize = size;
}

QSize QAbstractTextDocumentLayout::pageSize() const
{
    return d->pageSize;
}
