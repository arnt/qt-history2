
#include "qtextobjectmanager_p.h"
#include <qtextformat.h>

QTextObjectManager::QTextObjectManager(QObject *parent)
    : QObject(parent)
{
}

void QTextObjectManager::registerHandler(int formatType, QObject *component)
{
    QTextInlineObjectInterface *iface = qt_cast<QTextInlineObjectInterface *>(component);
    if (!iface)
	return; // ### print error message on terminal?

    connect(component, SIGNAL(destroyed(QObject *)), this, SLOT(handlerDestroyed(QObject *)));

    Handler h;
    h.iface = iface;
    h.component = component;
    handlers.insert(formatType, h);
}

void QTextObjectManager::layoutObject(QTextObject item, const QTextFormat &format)
{
    Handler handler = handlers.value(format.type());
    if (handler.isNull())
	return;
    handler.iface->layoutObject(item, format);
}

void QTextObjectManager::drawObject(QPainter *p, const QPoint &position, QTextObject item, const QTextFormat &format, QTextLayout::SelectionType selType)
{
    Handler handler = handlers.value(format.type());
    if (handler.isNull())
	return;
    handler.iface->drawObject(p, position, item, format, selType);
}

void QTextObjectManager::handlerDestroyed(QObject *obj)
{
    HandlerHash::Iterator it = handlers.begin();
    while (it != handlers.end())
	if ((*it).component == obj)
	    it = handlers.erase(it);
	else
	    ++it;
}

