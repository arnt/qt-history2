#include <qobject.h>
#include <qvariant.h>


// -------------------------

class V {
public:
    virtual ~V();
    virtual void setPasta( int );
    virtual int pasta() const;
protected:
    int my_pasta;
};

V::~V()
{
}

void V::setPasta( int p )
{
    qDebug( "V::setPasta()" );
    qDebug( "\tproperty value: %d", p );
    my_pasta = p;
}

int V::pasta() const
{
    qDebug( "V::pasta()" );
    qDebug( "\tproperty value: %d", my_pasta );
    return my_pasta;
}


// -------------------------

class X : public QObject, virtual public V {
    Q_OBJECT
    Q_PROPERTY( Priority priority READ priority WRITE setPriority )
    Q_PROPERTY( int pasta READ pasta WRITE setPasta )
    Q_ENUMS( Priority )
public:
    virtual ~X();
    enum Priority { Normal, Low, High };
    virtual void setPriority( Priority );
    virtual Priority priority() const;
    virtual void setPasta( int );
    virtual int pasta() const;
protected:
    Priority my_priority;
};

X::~X()
{
}

void X::setPriority( Priority p )
{
    qDebug( "X::setPriority()" );
    qDebug( "\tproperty value: %d", p );
    my_priority = p;
}

X::Priority X::priority() const
{
    qDebug( "X::priority()" );
    qDebug( "\tproperty value: %d", my_priority );
    return my_priority;
}

void X::setPasta( int p )
{
    qDebug( "X::setPasta()" );
    qDebug( "\tproperty value: %d", p );
    my_pasta = p;
}

int X::pasta() const
{
    qDebug( "X::pasta()" );
    qDebug( "\tproperty value: %d", my_pasta );
    return my_pasta;
}


// -------------------------

#include "property_test.moc"


// -------------------------

int main()
{
    X x;
    x.setProperty( "priority", "High" );
    x.setProperty( "pasta", 12345 );
    QVariant v1 = x.property( "priority" );
    QVariant v2 = x.property( "pasta" );

    return 0;
}
