#ifndef QXPATH_H
#define QXPATH_H

#ifndef QT_H
#include "qdom.h"
#endif // QT_H

class QXPathStep;
class QXPathPrivate;

class Q_EXPORT QXPath
{
public:
    QXPath();
    QXPath( const QString& expr );
    virtual ~QXPath();

    virtual void setExpression( const QString& expr );
    QString expression() const;
    bool isValid() const;

private:
    QXPathPrivate *d;
};


class Q_EXPORT QXPathStep
{
public:
    enum Axis {
	Child,
	Descendant,
	Parent,
	Ancestor,
	FollowingSibling,
	PrecedingSibling,
	Following,
	Preceding,
	Attribute,
	Namespace,
	Self,
	DescendantOrSelf,
	AncestorOrSelf
    };

    QXPathStep();
    QXPathStep( Axis axis ); // nodeTest, predicates );
    QXPathStep( const QXPathStep& step );
    ~QXPathStep();

    void setAxis( Axis axis );
    Axis axis() const;

private:
    Axis stepAxis;
};

#endif // QXPATH_H
