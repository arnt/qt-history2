#ifndef QXPATH_H
#define QXPATH_H

#ifndef QT_H
#include "qdom.h"
#endif // QT_H

#if 0
class QXPathStep;
#endif
class QXPathDataType;
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


class Q_EXPORT QXPathFunction
{
public:
    QXPathFunction();
    virtual ~QXPathFunction();

    virtual int minNumParameters() = 0;
    virtual int maxNumParameters() = 0;
    virtual int typeParameter( int i ) = 0;
#if 0
    // If I put the function-name -> function mapping in the context, then the
    // whole recursion of the evaluation is hidden.
    virtual void setParameter( int i, QXPathFunction *f );
#else
    virtual void setParameter( int i, QXPathDataType *value );
#endif

#if 0
    // this approach has a nicer API, but it is not that easy to subclass since
    // you need sharing mechanism... ???
    virtual QXPathDataType evaluate() = 0;
#else
    virtual void evaluate( QXPathDataType* ret ) = 0;
#endif
};


class Q_EXPORT QXPathDataType
{
public:
    QXPathDataType();
    virtual ~QXPathDataType();

    virtual bool isType( const QString& ) = 0;
    bool isType( int ) const;
    int type() const;
};


#if 0
class Q_EXPORT QXPathFunctionFactory
{
public:
    QXPathFunctionFactory();
    ~QXPathFunctionFactory();
};
#endif


#if 0
// old stuff that I probably don't need
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
#endif

#endif // QXPATH_H
