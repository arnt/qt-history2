#ifndef QTEXTOBJECTMANAGER_P_H
#define QTEXTOBJECTMANAGER_P_H

#include <qhash.h>
#include <private/qtextlayout_p.h>

#include "qtextglobal.h"

struct QTextInlineObjectInterface;

class QTextObjectManager : public QObject, 
			 public QTextInlineObjectInterface
{
    Q_OBJECT
public:
    QTextObjectManager(QObject *parent = 0);

    void registerHandler(int formatType, QObject *component);

    virtual void layoutObject(QTextObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *p, const QPoint &position, QTextObject item, const QTextFormat &format, QTextLayout::SelectionType selType);

private slots:
    void handlerDestroyed(QObject *obj);

private:
    struct Handler
    {
	Handler() : iface(0), component(0) {}

	bool isNull() const { return !component; }

	QTextInlineObjectInterface *iface;
	QObject *component;
    };
    typedef QHash<int, Handler> HandlerHash;
    HandlerHash handlers;
};

#endif // QTEXTOBJECTMANAGER_P_H
