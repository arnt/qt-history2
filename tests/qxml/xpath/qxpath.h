#include "qdom.h"

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

protected:
    virtual bool parse( const QString& path );

private:
    QXPathPrivate *d;
};


class Q_EXPORT QXPathStep
{
public:
    QXPathStep();
    QXPathStep( QXPath::Axis axis ); // nodeTest, predicates );
    QXPathStep( const QXPathStep& step );
    ~QXPathStep();

    void setAxis( QXPath::Axis axis );
    QXPath::Axis axis() const;

private:
    QXPath::Axis stepAxis;
};
