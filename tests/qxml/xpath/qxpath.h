#include "qdom.h"
#include "qvaluelist.h"

class QXPathStep;
class QXPathPrivate;

class Q_EXPORT QXPath
{
public:
    QXPath();
    QXPath( const QString& path );
    virtual ~QXPath();

    virtual void setPath( const QString& p );
    QString path() const;
    bool isValid() const;
    bool isAbsolutePath() const;

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
	DescendentOrSelf,
	AncestorOrSelf
    };

protected:
    virtual bool parse( const QString& path );

private:
    QXPathPrivate *d;
    bool absPath;
    QValueList<QXPathStep> steps;
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
