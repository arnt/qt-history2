#include <qapplication.h>

#include <qaxobject.h>
#include <qmetaobject.h>
#include <limits.h>
#include <qcolor.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qimage.h>

static int errorcount = 0;

#define PROP(prop) return m_##prop;
#define SET_PROP(prop) m_##prop = prop;

#define VERIFY_EQUAL( value, expected ) { \
    QVariant valvar = value; \
    QVariant expvar = expected; \
    if ( valvar != expvar ) { \
	if ( valvar.type() != expvar.type() ) \
	    qDebug( "Error in type conversion!" ); \
	bool reallyWrong = TRUE; \
	if ( valvar.type() == QVariant::Pixmap ) { \
	    QImage img1 = valvar.toPixmap().convertToImage(); \
	    QImage img2 = expvar.toPixmap().convertToImage(); \
	    reallyWrong = img1 != img2; \
	} \
	if ( reallyWrong ) { \
	    errorcount++; \
	    qDebug( "\t\tfailed in line %d!", __LINE__ ); \
	    if ( valvar.canCast( QVariant::String ) ) { \
		qDebug( "\t\t\tobject value: %s\n\t\t\texpvar: %s", \
		    valvar.toString().latin1(), expvar.toString().latin1() ); \
	    } \
	} \
    } \
} \


static inline QString constRefify( const QString& type )
{
    QString crtype;

    if ( type == "QString" )
	crtype = "const QString&";
    else if ( type == "QDateTime" )
	crtype = "const QDateTime&";
    else if ( type == "QVariant" )
	crtype = "const QVariant&";
    else if ( type == "QColor" )
	crtype = "const QColor&";
    else if ( type == "QFont" )
	crtype = "const QFont&";
    else if ( type == "QPixmap" )
	crtype = "const QPixmap&";
    else if ( type == "QValueList<QVariant>" )
	crtype = "const QValueList<QVariant>&";
    else
	crtype = type;

    return crtype;
}

class QTestContainer : public QObject
{
    Q_OBJECT
    Q_ENUMS( Alpha )
    Q_PROPERTY( QString unicode READ unicode WRITE setUnicode )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( bool boolval READ boolval WRITE setBoolval )
    Q_PROPERTY( int number READ number WRITE setNumber )
    Q_PROPERTY( uint posnumber READ posnumber WRITE setPosnumber )
    Q_PROPERTY( double real READ real WRITE setReal )
    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( QDateTime date READ date WRITE setDate )
    Q_PROPERTY( QDateTime time READ time WRITE setTime )
    Q_PROPERTY( QDateTime datetime READ datetime WRITE setDatetime )
    Q_PROPERTY( QFont font READ font WRITE setFont )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( QValueList list READ list WRITE setList )
    Q_PROPERTY( Alpha beta READ beta WRITE setBeta )

/*
    Q_PROPERTY( short shortnumber READ shortnumber WRITE setShortnumber )
    Q_PROPERTY( long longnumber READ longnumber WRITE setLongnumber )
*/
public:
    enum Alpha {
	AlphaA = 0,
	AlphaB,
	AlphaC,
	AlphaD,
	AlphaE,
	AlphaF
    };


    QTestContainer( const QString &control )
	: QObject( 0, "Test Container" )
    {
	m_unicode = "unicode";
	m_text = "c-string";
	m_boolval = TRUE;
	m_number = 42;
	m_posnumber = UINT_MAX;
	m_real = 3.1415927;
	m_color = red;
	m_date = QDate( 2002, 8, 7 );
	m_time = QDateTime( QDate( 1,1,1 ), QTime( 10, 30, 45 ) );
	m_datetime = QDateTime( QDate( 2001, 12, 31 ), QTime( 23, 59, 59 ) );
	m_font = QFont( "Times New Roman", 12, 75, TRUE );
	m_pixmap = QPixmap( 100, 100 );
	m_pixmap.fill( red );
	m_list << 1.5 << 2.6 << 3.7 << 4.8;
	m_beta = AlphaC;

/*
	m_shortnumber = 23;
	m_longnumber = 12345678;
*/
	object = new QAxObject( control, this, "Test Control" );
	Q_ASSERT( !object->isNull() );
	if ( object->isNull() )
	    return;

	QString str = object->generateDocumentation();
	qDebug( "%s\n\n", str.latin1() );

	QCString sigcode = "2";
	QCString slotcode = "1";

	const QMetaObject *mo = object->metaObject();
	int isig;
	for ( isig = 3; isig < mo->numSignals(); ++isig ) {
	    const QMetaData *signal = mo->signal( isig );
	    Q_ASSERT( connect( object, sigcode + signal->name, this, slotcode + signal->name ) );
	}
	mo = metaObject();
	for ( isig = 0; isig < mo->numSignals(); ++isig ) {
	    const QMetaData *signal = mo->signal( isig );
	    Q_ASSERT( connect( this, sigcode + signal->name, object, slotcode + signal->name ) );
	}
    }

    int run()
    {
	if ( object->isNull() )
	    return -1;

	QVariant containerValue;

	const QMetaObject *mo = object->metaObject();
	for ( int iprop = 1; iprop < mo->numProperties(); ++iprop ) {
	    // Turn of "changed" signals from object
	    object->blockSignals(TRUE);

	    const QMetaProperty *prop = mo->property( iprop );
	    Q_ASSERT( prop );
	    if ( !prop )
		continue;

	    qDebug( "\nTesting property %s of type %s", prop->name(), prop->type() );
	    QVariant defvalue;
	    if ( !prop->isEnumType() ) {
		QVariant::Type proptype = QVariant::nameToType( prop->type() );
		defvalue.cast( proptype );
		switch( defvalue.type() ) {
		case QVariant::Color:
		    defvalue = green;
		    break;
		case QVariant::Font:
		    defvalue = QFont( "Arial", 10 );
		    break;
		case QVariant::Pixmap:
		    {
			QPixmap pm( 5, 5 );
			pm.fill( green );
			defvalue = pm;
		    }
		    break;
		default:
		    break;
		}
		Q_ASSERT( defvalue.type() == proptype );
	    } else {
		defvalue.cast( QVariant::Int );
	    }

	    // Get container's value
	    containerValue = property( prop->name() );
	    Q_ASSERT( containerValue.isValid() );
	    Q_ASSERT( defvalue != containerValue );

	    // Initialize property with default value, and verify initialization
	    qDebug( "\tInitializing object with default values..." );
	    Q_ASSERT( object->setProperty( prop->name(), defvalue ) );
	    VERIFY_EQUAL( object->property( prop->name() ), defvalue );

	    qDebug( "\tsetProperty and property..." );
	    // Verify setProperty and property
	    Q_ASSERT( object->setProperty( prop->name(), containerValue ) );
	    VERIFY_EQUAL( object->property( prop->name() ), containerValue );
	    // Reset property
	    Q_ASSERT( object->setProperty( prop->name(), defvalue ) );

	    qDebug( "\tProperty handling via dynamicCall..." );
	    // Verify property setting and getting with dynamicCall
	    object->dynamicCall( prop->name(), containerValue );
	    VERIFY_EQUAL( object->dynamicCall( prop->name() ), containerValue );
	    // Reset property
	    Q_ASSERT( object->setProperty( prop->name(), defvalue ) );

	    qDebug( "\tProperty setting for generated setter-slot..." );
	    // Verify dynamicCall for generated setter-slots
	    QCString slotname = prop->name();
	    QChar oldfirst = slotname[0];
	    slotname[0] = oldfirst.upper();
	    if ( oldfirst == slotname[0] )
		slotname = "Set" + slotname + "(";
	    else
		slotname = "set" + slotname + "(";
	    slotname += constRefify( prop->type() );
	    slotname += ")";
	    object->dynamicCall( slotname, containerValue );
	    VERIFY_EQUAL( object->property( prop->name() ), containerValue );
	    // Reset property
	    Q_ASSERT( object->setProperty( prop->name(), defvalue ) );

	    // Generate Slot-names
	    QCString ftemplate = prop->name();
	    ftemplate[0] = QChar(ftemplate[0]).upper();

	    qDebug( "\tget%sSlot...", (const char*)ftemplate );
	    // Call and verify get<Prop>Slot
	    QCString getPropSlot = "get" + ftemplate + "Slot()";
	    VERIFY_EQUAL( object->dynamicCall( getPropSlot ), defvalue );

	    qDebug( "\tset%sSlot...", (const char*)ftemplate );
	    // Call and verify set<Prop>Slot
	    QCString setPropSlot = "set" + ftemplate + "Slot(";
	    setPropSlot += constRefify( prop->type() ) + ")";
	    //** if this crashes, see QStringToQUType and VARIANTToQUObject
	    object->dynamicCall( setPropSlot, containerValue );
	    VERIFY_EQUAL( object->property( prop->name() ), containerValue );

	    qDebug( "\tgetAndSet%sSlot...", (const char*)ftemplate );
	    // Call and verify getAndSet<Prop>Slot checking out-parameter
	    // (getAndSet returns and sets the new value, and sets the out-parameter to the old value)
	    QCString getAndSetPropSlot = "getAndSet" + ftemplate + "Slot(";
	    getAndSetPropSlot += prop->type();
	    getAndSetPropSlot += "&)";
	    QValueList<QVariant> varlist;
	    varlist << defvalue;
	    //** if this crashes, see QUObjectToVARIANT for QVariant
	    VERIFY_EQUAL( object->dynamicCall( getAndSetPropSlot, varlist ), defvalue );
	    //** if this fails, see VARIANTToQVariant
	    VERIFY_EQUAL( varlist[0], containerValue );

	    // Verify states
	    Q_ASSERT( defvalue != containerValue );
	    VERIFY_EQUAL( object->property( prop->name() ), defvalue );
	    VERIFY_EQUAL( property( prop->name() ), containerValue );

	    // Turn on signals from object
	    object->blockSignals( FALSE );

	    qDebug( "\temit%sRefSignal...", (const char*)ftemplate );
	    // Call and verify emit<Prop>GetSignal
	    // (the signal calls the <prop>RefSignal slot of "this" and updates the value, and returns the old value)
	    QCString emitPropRefSignal = "emit" + ftemplate + "RefSignal()";
	    //** if this crashes, see... QUObjectToVARIANT and makeReference
	    VERIFY_EQUAL( object->dynamicCall( emitPropRefSignal ), defvalue );

	    qDebug( "\tSynchronizing values..." );
	    // Synchronize values in container - both are "containerValue"
	    QVariant objvalue = object->property( prop->name() );
	    VERIFY_EQUAL( objvalue, containerValue );
	    setProperty( prop->name(), objvalue );
	    VERIFY_EQUAL( property( prop->name() ), containerValue );

	    qDebug( "\tSynchronizing values via %sChanged...", prop->name() );
	    // Synchronize values in container - both are "containerValue"
	    // If this crashes, see QVariantToQUObject
	    Q_ASSERT( object->setProperty( prop->name(), defvalue ) );
	    VERIFY_EQUAL( property( prop->name() ), defvalue );
	    VERIFY_EQUAL( object->property( prop->name() ), defvalue );

	    // Set the container back to containerValue
	    setProperty( prop->name(), containerValue );
	}
	// here, all properties of object are defvalue, and of the container containerValue

	emit setUnicodeSlot( m_unicode );
	VERIFY_EQUAL( object->property( "unicode" ), m_unicode );
	QString unicodeSlot;
	emit getAndSetUnicodeSlot( unicodeSlot );
	VERIFY_EQUAL( object->property( "unicode" ).toString(), QString::null );

	qDebug( "\nEmitting signals to pointer slots..." );
	emit unicodePointerSlot( m_unicode );
	VERIFY_EQUAL( object->property( "unicode" ), m_unicode );
	emit textPointerSlot( m_text );
	VERIFY_EQUAL( object->property( "text" ), m_text );
	emit boolPointerSlot( m_boolval );
	VERIFY_EQUAL( object->property( "boolval" ), m_boolval );
	emit numberPointerSlot( m_number );
	VERIFY_EQUAL( object->property( "number" ), m_number );
	emit posnumberPointerSlot( m_posnumber );
	VERIFY_EQUAL( object->property( "posnumber" ), m_posnumber );
	emit realPointerSlot( m_real );
	VERIFY_EQUAL( object->property( "real" ), m_real );
	emit datePointerSlot( m_date );
	VERIFY_EQUAL( object->property( "date" ), m_date );
	emit timePointerSlot( m_time );
	VERIFY_EQUAL( object->property( "time" ), m_time );
	emit datetimePointerSlot( m_datetime );
	VERIFY_EQUAL( object->property( "datetime" ), m_datetime );
	emit fontPointerSlot( m_font );
	VERIFY_EQUAL( object->property( "font" ), m_font );
	emit pixmapPointerSlot( m_pixmap );
	VERIFY_EQUAL( object->property( "pixmap" ), m_pixmap );
	emit listPointerSlot( m_list );
	VERIFY_EQUAL( object->property( "list" ), m_list );

/*	Difficult to support
	emit betaPointerSlot( m_beta );
	VERIFY_EQUAL( object->property( "beta" ), m_beta );
*/
	return errorcount;
    }

    QString unicode() const { PROP(unicode) }
    void setUnicode( const QString &unicode ){ SET_PROP(unicode) }

    QString text() const { PROP(text) }
    void setText( const QString &text ){ SET_PROP(text) }

    bool boolval() const { PROP(boolval) }
    void setBoolval( bool boolval ) { SET_PROP(boolval) }
    
    int number() const { PROP(number) }
    void setNumber( int number ) { SET_PROP(number) }

    uint posnumber() const { PROP(posnumber) }
    void setPosnumber( uint posnumber ) { SET_PROP(posnumber) }

    double real() const { PROP(real) }
    void setReal( double real ) { SET_PROP(real) }

    QColor color() const { PROP(color) }
    void setColor( const QColor &color ) { SET_PROP(color) }

    QDateTime date() const { PROP(date) }
    void setDate( QDateTime date ) { SET_PROP(date) }

    QDateTime time() const { PROP(time) }
    void setTime( QDateTime time ) { SET_PROP(time) }

    QDateTime datetime() const { PROP(datetime) }
    void setDatetime( const QDateTime &datetime ) { SET_PROP(datetime) }

    QFont font() const { PROP(font) }
    void setFont( QFont font ) { SET_PROP(font) }

    QPixmap pixmap() const { PROP(pixmap) }
    void setPixmap( QPixmap pixmap ) { SET_PROP(pixmap) }

    QValueList<QVariant> list() const { PROP(list) }
    void setList( QValueList<QVariant> list ) { SET_PROP(list) }

    Alpha beta() const { PROP(beta) }
    void setBeta( Alpha beta ) { SET_PROP(beta) }

/*
    void setShortnumber( short shortnumber ) { m_shortnumber = shortnumber; }
    short shortnumber() const { return m_shortnumber; }

    void setLongnumber( long longnumber ) { m_longnumber = longnumber; }
    long longnumber() const { return m_longnumber; }
*/
public slots:
    // <prop>Changed is called by the object's property notification
    // get<Prop>Signal is called by the object's emitGetPropSignal slot

    void unicodeChanged( const QString &unicode ) { m_unicode = unicode; }
    void unicodeRefSignal( QString &unicode ) { unicode = m_unicode; }

    void textChanged( const QString &text ) { m_text = text; }
    void textRefSignal( QString &text ) { text = m_text; }

    void boolvalChanged( bool boolval ) { m_boolval = boolval; }
    void boolvalRefSignal( bool &boolval ) { boolval = m_boolval; }

    void numberChanged( int number ) { m_number = number; }
    void numberRefSignal( int &number ) { number = m_number; }

    void posnumberChanged( uint posnumber ) { m_posnumber = posnumber; }
    void posnumberRefSignal( uint &posnumber ) { posnumber = m_posnumber; }

    void realChanged( double real ) { m_real = real; }
    void realRefSignal( double &real ) { real = m_real; }

    void colorChanged( const QColor &color ) { m_color = color; }
    void colorRefSignal( QColor &color ) { color = m_color; }

    void dateChanged( const QDateTime &date ) { m_date = date; }
    void dateRefSignal( QDateTime &date ) { date = m_date; }
    
    void timeChanged( const QDateTime &time ) { m_time = time; }
    void timeRefSignal( QDateTime &time ) { time = m_time; }

    void datetimeChanged( const QDateTime &datetime) { m_datetime = datetime; }
    void datetimeRefSignal( QDateTime &datetime ) { datetime = m_datetime; }

    void fontChanged( const QFont &font ) { m_font = font; }
    void fontRefSignal( QFont &font ) { font = m_font; }

    void pixmapChanged( const QPixmap &pixmap ) { m_pixmap = pixmap; }
    void pixmapRefSignal( QPixmap &pixmap ) { pixmap = m_pixmap; }

    void listChanged( const QValueList<QVariant> &list ) { m_list = list; }
    void listRefSignal( QValueList<QVariant> &list ) { list = m_list; }

    void betaChanged( Alpha beta ) { m_beta = beta; }
    void betaRefSignal( Alpha &beta ) { beta = m_beta; }

/*
    void shortnumberChanged( short shortnumber ) { m_shortnumber = shortnumber; }
    void shortnumberRefSignal( short &shortnumber ) { shortnumber = m_shortnumber; }

    void longnumberChanged( long longnumber ) { m_longnumber = longnumber; }
    void longnumberRefSignal( long &longnumber ) { longnumber = m_longnumber; }
*/
signals:
    void setUnicodeSlot( const QString &string );
    void getAndSetUnicodeSlot( QString &string );

    void unicodePointerSlot( const QString &string );
    void textPointerSlot( const QString &string );
    void boolPointerSlot( bool boolval );
    void numberPointerSlot( int number );
    void posnumberPointerSlot( uint posnumber );
    void realPointerSlot( double real );
    void colorPointerSlot( const QColor &color );
    void datePointerSlot( const QDateTime &date );
    void timePointerSlot( const QDateTime &time );
    void datetimePointerSlot( const QDateTime &datetime );
    void fontPointerSlot( const QFont &font );
    void pixmapPointerSlot( const QPixmap &pixmap );
    void listPointerSlot( const QValueList<QVariant> &list );
    void betaPointerSlot( Alpha beta );

private:
    QAxObject *object;

    QString m_unicode;
    QString m_text;
    bool m_boolval;
    int m_number;
    uint m_posnumber;
    double m_real;
    QColor m_color;
    QDateTime m_date;
    QDateTime m_time;
    QDateTime m_datetime;
    QFont m_font;
    QPixmap m_pixmap;
    QValueList<QVariant> m_list;
    Alpha m_beta;

/*
    short m_shortnumber;
    long m_longnumber;  
*/
};

#include "container.moc"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QTestContainer container( "testcontrol.QTestControl" );
    return container.run();
}
