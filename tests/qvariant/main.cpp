#include <qapplication.h>
#include <qvariant.h>

extern int get_qv_count();

int count = 0;

class MyValue
{
public:
    MyValue() { qDebug("MyValue()"); count++; }
    MyValue( const MyValue& )  { qDebug("MyValue( copy )"); count++; }
    ~MyValue() { qDebug("destructor MyValue()"); --count; }

    int x;
};

class Vector
{
public:
    Vector() { qDebug("Vector()"); count++; }
    Vector( const Vector& c )  { qDebug("Vector( copy )"); count++; v1=c.v1; v2=c.v2; }
    ~Vector() { qDebug("destructor Vector()"); --count; }

    double v1, v2;
};

class Complex
{
public:
    Complex() { qDebug("Complex()"); count++; }
    Complex( const Complex& c )  { qDebug("Complex( copy )"); count++; r=c.r; i=c.i; }
    ~Complex() { qDebug("destructor Complex()"); --count; }

    double r, i;
};

QDataStream& operator<<( QDataStream& str, const MyValue& )
{
    return str;
}

QDataStream& operator>>( QDataStream& str, MyValue& )
{
    return str;
}

QDataStream& operator<<( QDataStream& str, const Vector& )
{
    return str;
}

QDataStream& operator>>( QDataStream& str, Vector& )
{
    return str;
}

class ComplexType : public QVariantType<Complex>
{
public:
    static const ComplexType* type() { if ( self ) return self; return new ComplexType(); }

    virtual void* castFrom( const QVariant& ) const;
    virtual QVariant castTo( const void* ptr, QVariant::Type ) const;
    virtual void* castTo( const void* ptr, const QVariantTypeBase* type ) const;

    virtual bool canCastTo( const QVariantTypeBase* type ) const;
    virtual bool canCastTo( QVariant::Type ) const;
    virtual bool canCastFrom( const QVariantTypeBase* type ) const;
    virtual bool canCastFrom( QVariant::Type ) const;

private:
    ComplexType() : QVariantType<Complex>( "Complex" ) { self = this; }
    static ComplexType* self;
};

ComplexType* ComplexType::self = 0;

void* ComplexType::castFrom( const QVariant& v ) const
{
    qDebug("Cast from %s", v.typeName() );

    if ( v.type() == QVariant::Double || v.type() == QVariant::Int || v.type() == QVariant::UInt )
    {
	Complex *c = new Complex;
	c->r = v.toDouble();
	c->i = 0;
	return c;
    }

    if ( v.type() == QVariant::Custom && v.customType() == QVariantTypeBase::type( "Vector" ) )
    {
	const Vector* vec = (Vector*)v.asCustom();
	Complex *c = new Complex;
	c->r = vec->v1;
	c->i = vec->v2;
	qDebug("Casting from Vector %f, %f", (float)vec->v1, (float)vec->v2 );
	return c;	
    }

    return 0;
}

QVariant ComplexType::castTo( const void* ptr, const QVariant::Type t ) const
{
    if ( t != QVariant::Double )
	return QVariant();
    return QVariant( ((Complex*)ptr)->r );
}

void* ComplexType::castTo( const void* ptr, const QVariantTypeBase* type ) const
{
    if ( type == 0 || type != QVariantTypeBase::type( "Vector" ) )
	return 0;
    Vector* v = new Vector;
    v->v1 = ((Complex*)ptr)->r;
    v->v2 = ((Complex*)ptr)->i;
    return v;
}
	
bool ComplexType::canCastTo( const QVariantTypeBase* type ) const
{
    if ( type != 0 && type == QVariantTypeBase::type( "Vector" ) )
	return TRUE;
    return FALSE;
}

bool ComplexType::canCastTo( QVariant::Type t ) const
{
    if ( t != QVariant::Double )
	return FALSE;
    return TRUE;
}

bool ComplexType::canCastFrom( const QVariantTypeBase* type ) const
{
    if ( type == QVariantTypeBase::type( "Vector" ) )
	return TRUE;
    return FALSE;
}

bool ComplexType::canCastFrom( QVariant::Type t ) const
{
    if ( t == QVariant::Double )
	return TRUE;
    if ( t == QVariant::Int )
	return TRUE;
    if ( t == QVariant::Int )
	return TRUE;
    return FALSE;
}

typedef QVariantValue<Complex,ComplexType> ComplexValue;

QDataStream& operator<<( QDataStream& str, const Complex& )
{
    return str;
}

QDataStream& operator>>( QDataStream& str, Complex& )
{
    return str;
}


int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QVariantTypeBase* type = new QVariantType<MyValue>( "MyValue" );
    QVariantTypeBase* type2 = new QVariantType<MyValue>( "TestMe" );
    QVariantTypeBase* vectype = new QVariantType<Vector>( "Vector" );

    QVariant v;
    {
    v = QVariant( TRUE );
    ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    ASSERT( v.toDouble() == 1.0 );
    ASSERT( v.toInt() == 1 );
    ASSERT( v.toUInt() == 1 );
    v = QVariant( FALSE );
    ASSERT( v.toDouble() == 0.0 );
    ASSERT( v.toInt() == 0 );
    ASSERT( v.toUInt() == 0 );
	
    v = QVariant( 5.3 );
    ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    ASSERT( v.toBool() == TRUE );
    ASSERT( v.toInt() == 5 );
    ASSERT( v.toUInt() == 5 );
    v = QVariant( 0.0 );
    ASSERT( v.toBool() == FALSE );

    v = QVariant( (int)5 );
    ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    ASSERT( v.toBool() == TRUE );
    ASSERT( v.toDouble() == 5 );
    ASSERT( v.toUInt() == 5 );
    v = QVariant( (int)0 );
    ASSERT( v.toBool() == FALSE );

    v = QVariant( (uint)5 );
    ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    ASSERT( v.toBool() == TRUE );
    ASSERT( v.toDouble() == 5 );
    ASSERT( v.toInt() == 5 );
    v = QVariant( (uint)0 );
    ASSERT( v.toBool() == FALSE );

    v = QVariant( "Hallo" );
    ASSERT( v.canCast( QVariant::String ) && v.canCast( QVariant::CString ) );
    ASSERT( v.toString() == "Hallo" );
    ASSERT( v.toCString() == "Hallo" );

    QValueList<QVariant> vl;
    vl.append( QVariant( "Torben" ) );
    vl.append( QVariant( "Weis" ) );
    v = QVariant( vl );
    ASSERT( v.toList().count() == 2 );
    ASSERT( v.toList()[0].toString() == "Torben" );
    ASSERT( v.toList()[1].toString() == "Weis" );
    ASSERT( v.canCast( QVariant::List ) );
    ASSERT( v.canCast( QVariant::StringList ) );
    QStringList sl = v.toStringList();
    qDebug("ValueList count = %i", v.toList().count() );
    qDebug("StringList count = %i", sl.count() );
    ASSERT( sl.count() == 2 && sl[0] == "Torben" && sl[1] == "Weis" );
    vl.append( QVariant( 5 ) );
    v = QVariant( vl );
    ASSERT( v.canCast( QVariant::List ) );
    ASSERT( !v.canCast( QVariant::StringList ) );
	
    sl.clear();
    sl.append( "Troll" );
    sl.append( "Tech" );
    v = QVariant( sl );
    ASSERT( v.canCast( QVariant::StringList ) && v.canCast( QVariant::List ) );
    vl = v.toList();
    ASSERT( vl.count() == 2 && vl[0].toString() == "Troll" && vl[1].toString() == "Tech" );
    }

    qDebug("------------------ Custom 1 -----------------");

    {
    MyValue* p = new MyValue;
    ASSERT( count == 1 );
    v = QVariant( p, type );
    ASSERT( count == 2 );
    delete p;
    ASSERT( count == 1 );
    v.clear();
    }
    ASSERT( count == 0 );

    qDebug("------------------ Custom 2 -----------------");
	
    {
    MyValue* p = new MyValue;
    v = QVariant( p, type );
    delete p;
    p = (MyValue*)v.toCustom( type2 );
    ASSERT( p == 0 );
    ASSERT( count == 1 );
    p = (MyValue*)v.toCustom( type );
    ASSERT( count == 2 );
    delete p;
    v.clear();
    }
    ASSERT( count == 0 );
	
    qDebug("------------------ Custom 3 -----------------");
    {	
    Complex c;
    c.r = 5.4;
    c.i = -3.2;
    v = QVariant( ComplexValue( c ) );
    ComplexValue cv = v;
    ASSERT( cv->r == 5.4 && cv->i == -3.2 );

    Complex c2;
    c2.r = 2.4;
    c2.i = -1.2;
    cv = c2;
    ASSERT( cv->r == 2.4 && cv->i == -1.2 );
    ComplexValue cv2;
    cv2 = cv;
    ASSERT( cv2->r == 2.4 && cv2->i == -1.2 );
    ComplexValue cv3( cv );
    ASSERT( cv3->r == 2.4 && cv3->i == -1.2 );
    ComplexValue cv4( c2 );
    ASSERT( cv4->r == 2.4 && cv4->i == -1.2 );
    qDebug("------------------ Test done -----------------");
    }
    ASSERT( count == 1 );

    qDebug("------------------ Custom 4 -----------------");
    {
    ASSERT( v.canCast( ComplexType::type() ) );
    ASSERT( v.canCast( QVariant::Double ) );
    ASSERT( !v.canCast( QVariant::Int ) );
    ASSERT( v.toDouble() == 5.4 );
    v = QVariant( (int)5 );
    ComplexValue cv = v;
    ASSERT( !cv.isNull() && cv->r == 5.0 && cv->i == 0.0 );
    v = QVariant( (uint)6 );
    cv = v;
    ASSERT( !cv.isNull() && cv->r == 6.0 && cv->i == 0.0 );
    v = QVariant( 7.8 );
    cv = v;
    ASSERT( !cv.isNull() && cv->r == 7.8 && cv->i == 0.0 );
    v = QVariant( "Hallo" );
    cv = v;
    ASSERT( cv.isNull() );
    ASSERT( !v.canCast( ComplexType::type() ) );

    v.clear();
    qDebug("------------------ Test done -----------------%i", count );
    }
    ASSERT( count == 0 );

    qDebug("------------------ Custom 5 -----------------");
    {
	Complex c;
	c.r = 5.4;
	c.i = -3.2;
	v = QVariant( ComplexValue( c ) );
	ASSERT( v.canCast( vectype ) );
	ASSERT( !v.canCast( type ) );
	Vector* vec = (Vector*)v.toCustom( vectype );
	ASSERT( vec );
	ASSERT( vec->v1 == 5.4 && vec->v2 == -3.2 );
	delete vec;
	v.clear();
    }
    ASSERT( count == 0 );

    qDebug("------------------ Custom 6 -----------------");
    {
	Vector vec;
	vec.v1 = 123.4;
	vec.v2 = 432.1;
	v = QVariant( &vec, vectype );
	ASSERT( v.customType() == vectype );
	ASSERT( ((Vector*)v.asCustom())->v1 == 123.4 && ((Vector*)v.asCustom())->v2 == 432.1 );
	ComplexValue cv( v );
	ASSERT( !cv.isNull() );
	ASSERT( cv->r = 123.4 && cv->i == 432.1 );
	v.clear();
    }
    ASSERT( count == 0 );

    qDebug("------------------ Custom 7 -----------------");
    {
	ASSERT( v.asStringList().isEmpty() );
	v.asStringList().append( "Hallo" );
	v.asStringList().append( "Welt" );

	QStringList::ConstIterator it = v.stringListBegin();
	QStringList::ConstIterator end = v.stringListEnd();
	ASSERT( *it == "Hallo" );
	++it;
	ASSERT( *it == "Welt" );
	++it;
	ASSERT( it == end );

	ASSERT( v.asStringList().count() == 2 );
	ASSERT( v.asList().count() == 2 && v.asList()[0].toString() == "Hallo" );
	
	
	v = QVariant( 6 );
	v.asInt() += 4;
	ASSERT( v.toInt() == 10 );
	
	v = "Hallo";
	ASSERT( v.toString() == "Hallo" );
    }
    ASSERT( count == 0 );

    qDebug("------------------ Shared 1 -----------------");
    {
	v = QVariant( "Torben" );
	QVariant v2;
	v2 = v;
	v.asString() = "Claudia";
	ASSERT( v.toString() == "Claudia" && v2.toString() == "Torben" );
    }
    ASSERT( count == 0 );

    ASSERT( get_qv_count() == 1 );
    
    qDebug("------------------ Finished -----------------");
}

