/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qwidget.h>

#include <qaxbindable.h>
#include <qaxfactory.h>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qvariant.h>
#include <qpainter.h>

#define PROP(prop) return m_##prop;
#define SET_PROP(prop) m_##prop = prop; repaint();
#define GET_PROP_SLOT(prop) return m_##prop;
#define SET_PROP_SLOT(prop) m_##prop = prop; repaint();
#define GET_AND_SET(prop, type) type old = m_##prop; m_##prop = prop; prop = old; repaint(); return m_##prop;
#define EMIT_REF(prop, type) type old = m_##prop; emit prop##RefSignal( m_##prop ); repaint(); return old;
#define PROP_POINTER(prop) m_##prop = *prop; repaint();

struct IDispatch;

class QSubType : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString unicode READ unicode WRITE setUnicode )
    Q_PROPERTY( int number READ number WRITE setNumber )

public:
    QSubType( QObject *parent = 0, const char *name = 0 )
	: QObject( parent, name )
    {
    }

    QString unicode() const { PROP(unicode) }
    void setUnicode( const QString &unicode ){ SET_PROP(unicode) }

    int number() const { PROP(number) }
    void setNumber( int number ) { SET_PROP(number) }


    void repaint()
    {
	Q_ASSERT( parent() && parent()->isWidgetType() );
	((QWidget*)parent())->repaint();
    }

public slots:
    void emitFire() { emit fire(); }

signals:
    void fire();

private:
    QString m_unicode;
    int m_number;
};

class QTestControl : public QWidget, public QAxBindable
{
    Q_OBJECT
    Q_ENUMS( Alpha )
    Q_PROPERTY( QString unicode READ unicode WRITE setUnicode )
    Q_PROPERTY( QCString text READ text WRITE setText )
    Q_PROPERTY( bool boolval READ boolval WRITE setBoolval )
    Q_PROPERTY( int number READ number WRITE setNumber )
    Q_PROPERTY( uint posnumber READ posnumber WRITE setPosnumber )
    Q_PROPERTY( double real READ real WRITE setReal )
    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( QDate date READ date WRITE setDate )
    Q_PROPERTY( QDateTime time READ time WRITE setTime )
    Q_PROPERTY( QDateTime datetime READ datetime WRITE setDatetime )
    Q_PROPERTY( QFont font READ font WRITE setFont )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( QValueList list READ list WRITE setList )
    Q_PROPERTY( Alpha beta READ beta WRITE setBeta )
    Q_PROPERTY( LongLong currency READ currency WRITE setCurrency )
    Q_PROPERTY( QByteArray bytes READ bytes WRITE setBytes )
    
public:
    QTestControl( QWidget *parent = 0, const char *name = 0 )
    : QWidget( parent, name )
    {
	m_subType = 0;
	// other members not initiliazed on purpose
    }

    enum Alpha {
	AlphaA = 0,
	AlphaB,
	AlphaC,
	AlphaD,
	AlphaE,
	AlphaF
    };

    QString unicode() const { PROP(unicode) }
    void setUnicode( const QString &unicode ){ SET_PROP(unicode) }

    QCString text() const { PROP(text) }
    void setText( const QCString &text ){ SET_PROP(text) }

    bool boolval() const { PROP(boolval) }
    void setBoolval( bool boolval ) { SET_PROP(boolval) }
    
    int number() const { PROP(number) }
    void setNumber( int number ) { SET_PROP(number) }

    uint posnumber() const { PROP(posnumber) }
    void setPosnumber( uint posnumber ) { SET_PROP(posnumber) }

    double real() const { PROP(real) }
    void setReal( double real ) { SET_PROP(real) }

    QColor color() const { PROP(color) }
    void setColor( QColor color ) { SET_PROP(color) }

    QDate date() const { PROP(date) }
    void setDate( const QDate &date ) { SET_PROP(date) }

    QDateTime time() const { PROP(time) }
    void setTime( const QDateTime &time ) { SET_PROP(time) }

    QDateTime datetime() const { PROP(datetime) }
    void setDatetime( const QDateTime &datetime ) { SET_PROP(datetime) }

    QFont font() const { PROP(font) }
    void setFont( const QFont &font ) { SET_PROP(font) }

    QPixmap pixmap() const { PROP(pixmap) }
    void setPixmap( const QPixmap &pixmap ) { SET_PROP(pixmap) }

    QValueList<QVariant> list() const { PROP(list) }
    void setList( QValueList<QVariant> list ) { SET_PROP(list) }

    Alpha beta() const { PROP(beta) }
    void setBeta( Alpha beta ) { SET_PROP(beta) }

    Q_LLONG currency() const { PROP(currency) }
    void setCurrency( Q_LLONG currency ) { SET_PROP(currency) }

    QByteArray bytes() const { PROP(bytes) }
    void setBytes( const QByteArray &bytes ) { SET_PROP(bytes) }

public slots:
    QString getUnicodeSlot() const { GET_PROP_SLOT(unicode) }
    void setUnicodeSlot( const QString &unicode ) { SET_PROP_SLOT(unicode) }
    QString getAndSetUnicodeSlot( QString &unicode ) { GET_AND_SET(unicode, QString) }
    QString emitUnicodeRefSignal() { EMIT_REF(unicode, QString) }

    QCString getTextSlot() const { GET_PROP_SLOT(text) }
    void setTextSlot( const QCString &text ) { SET_PROP_SLOT(text) }
    QCString getAndSetTextSlot( QCString &text ) { GET_AND_SET(text, QCString) }
    QCString emitTextRefSignal() { EMIT_REF(text, QCString) }

    bool getBoolvalSlot() const { GET_PROP_SLOT(boolval) }
    void setBoolvalSlot( bool boolval ) { SET_PROP_SLOT(boolval) }
    bool getAndSetBoolvalSlot( bool& boolval ) { GET_AND_SET(boolval, bool) }
    bool emitBoolvalRefSignal() { EMIT_REF(boolval, bool) }

    int getNumberSlot() const { GET_PROP_SLOT(number) }
    void setNumberSlot( int number ) { SET_PROP_SLOT(number) }
    int getAndSetNumberSlot( int& number ) { GET_AND_SET(number, int) }
    int emitNumberRefSignal() { EMIT_REF(number, int) }

    uint getPosnumberSlot() const { GET_PROP_SLOT(posnumber) }
    void setPosnumberSlot( uint posnumber ) { SET_PROP_SLOT(posnumber) }
    uint getAndSetPosnumberSlot( uint& posnumber ) { GET_AND_SET(posnumber, uint) }
    uint emitPosnumberRefSignal() { EMIT_REF(posnumber, uint) }

    double getRealSlot() const { GET_PROP_SLOT(real) }
    void setRealSlot( double real ) { SET_PROP_SLOT(real) }
    double getAndSetRealSlot( double& real ) { GET_AND_SET(real, double) }
    double emitRealRefSignal() { EMIT_REF(real, double) }

    QColor getColorSlot() const { GET_PROP_SLOT(color) }
    void setColorSlot( QColor color ) { SET_PROP_SLOT(color) }
    QColor getAndSetColorSlot( QColor& color ) { GET_AND_SET(color, QColor) }
    QColor emitColorRefSignal() { EMIT_REF(color, QColor) }

    QDate getDateSlot() const { GET_PROP_SLOT(date) }
    void setDateSlot( const QDate &date ) { SET_PROP_SLOT(date) }
    QDate getAndSetDateSlot( QDate& date ) { GET_AND_SET(date, QDate) }
    QDate emitDateRefSignal() { EMIT_REF(date, QDate) }

    QDateTime getTimeSlot() const { GET_PROP_SLOT(time) }
    void setTimeSlot( const QDateTime &time ) { SET_PROP_SLOT(time) }
    QDateTime getAndSetTimeSlot( QDateTime& time ) { GET_AND_SET(time, QDateTime) }
    QDateTime emitTimeRefSignal() { EMIT_REF(time, QDateTime) }

    QDateTime getDatetimeSlot() const { GET_PROP_SLOT(datetime) }
    void setDatetimeSlot( QDateTime datetime ) { SET_PROP_SLOT(datetime) }
    QDateTime getAndSetDatetimeSlot( QDateTime& datetime ) { GET_AND_SET(datetime, QDateTime) }
    QDateTime emitDatetimeRefSignal() { EMIT_REF(datetime, QDateTime) }

    QFont getFontSlot() const { GET_PROP_SLOT(font) }
    void setFontSlot( QFont font ) { SET_PROP_SLOT(font) }
    QFont getAndSetFontSlot( QFont& font ) { GET_AND_SET(font, QFont) }
    QFont emitFontRefSignal() { EMIT_REF(font, QFont) }

    QPixmap getPixmapSlot() const { GET_PROP_SLOT(pixmap) }
    void setPixmapSlot( QPixmap pixmap ) { SET_PROP_SLOT(pixmap) }
    QPixmap getAndSetPixmapSlot( QPixmap& pixmap ) { GET_AND_SET(pixmap, QPixmap) }
    QPixmap emitPixmapRefSignal() { EMIT_REF(pixmap, QPixmap) }

    QValueList<QVariant> getListSlot() const { GET_PROP_SLOT(list) }
    void setListSlot( QValueList<QVariant> list ) { SET_PROP_SLOT(list) }
    QValueList<QVariant> getAndSetListSlot( QValueList<QVariant>& list ) { GET_AND_SET(list, QValueList<QVariant>) }
    QValueList<QVariant> emitListRefSignal() { EMIT_REF(list, QValueList<QVariant>) }

    Alpha getBetaSlot() const { GET_PROP_SLOT(beta) }
    void setBetaSlot( Alpha beta ) { SET_PROP_SLOT(beta) }
    Alpha getAndSetBetaSlot( Alpha& beta ) { GET_AND_SET(beta, Alpha) }
    Alpha emitBetaRefSignal() { EMIT_REF(beta, Alpha) }

    Q_LLONG getCurrencySlot() const { GET_PROP_SLOT(currency) }
    void setCurrencySlot( Q_LLONG currency ) { SET_PROP_SLOT(currency) }
    Q_LLONG getAndSetCurrencySlot( Q_LLONG& currency ) { GET_AND_SET(currency, Q_LLONG) }
    Q_LLONG emitCurrencyRefSignal() { EMIT_REF(currency, Q_LLONG) }

    QByteArray getBytesSlot() const { GET_PROP_SLOT(bytes) }
    void setBytesSlot( const QByteArray &bytes ) { SET_PROP_SLOT(bytes) }
    QByteArray getAndSetBytesSlot( QByteArray &bytes ) { GET_AND_SET(bytes, QByteArray) }
    QByteArray emitBytesRefSignal() { EMIT_REF(bytes, QByteArray) }

    IDispatch *getDispatchSlot() const { GET_PROP_SLOT(disp) }
    IDispatch *getDispatchSlotRef( IDispatch **d ) const { *d = m_disp; return m_disp; }
    void setDispatchSlot( IDispatch *disp ) { SET_PROP_SLOT(disp) }

    void unicodePointerSlot( QString *unicode ) { PROP_POINTER(unicode) }
    void textPointerSlot( QCString *text ) { PROP_POINTER(text) }
    void boolvalPointerSlot( bool *boolval ) { PROP_POINTER(boolval) }
    void numberPointerSlot( int *number ) { PROP_POINTER(number) }
    void posnumberPointerSlot( uint *posnumber ) { PROP_POINTER(posnumber) }
    void realPointerSlot( double *real ) { PROP_POINTER(real) }
    void colorPointerSlot( QColor *color ) { PROP_POINTER(color) }
    void datePointerSlot( QDate *date ) { PROP_POINTER(date) }
    void timePointerSlot( QDateTime *time ) { PROP_POINTER(time) }
    void datetimePointerSlot( QDateTime *datetime ) { PROP_POINTER(datetime) }
    void fontPointerSlot( QFont *font ) { PROP_POINTER(font) }
    void pixmapPointerSlot( QPixmap *pixmap) { PROP_POINTER(pixmap) }
    void listPointerSlot( QValueList<QVariant> *list) { PROP_POINTER(list) }
    void betaPointerSlot( Alpha *beta ) { PROP_POINTER(beta) }
    void currencyPointerSlot( Q_LLONG *currency ) { PROP_POINTER(currency) }
    void bytesPointerSlot( QByteArray *bytes ) { PROP_POINTER(bytes) }

    QSubType *subType()
    {
	if ( !m_subType )
	    m_subType = new QSubType( this, "subType" );

	repaint();
	return m_subType;
    }

    QSubType *subTypeNum( int i )
    {
	QSubType *sub = new QSubType( this, "subTypeNum" );
	sub->setNumber( i );
	return sub;
    }

    QSubType *subTypeUnicode( const QString &str )
    {
	QSubType *sub = new QSubType( this, "subTypeUnicode" );
	sub->setUnicode( str );
	return sub;
    }

    void throwException( int code, const QString &src, const QString &desc, const QString &context )
    {
	reportError( code, src, desc, context );
    }

signals:
    void unicodeChanged( const QString& );
    void unicodeRefSignal( QString& );

    void textChanged( const QCString& );
    void textRefSignal( QCString& );

    void boolvalChanged( bool );
    void boolvalRefSignal( bool& );

    void numberChanged( int );
    void numberRefSignal( int& );

    void posnumberChanged( uint );
    void posnumberRefSignal( uint& );

    void realChanged( double );
    void realRefSignal( double& );

    void colorChanged( const QColor& );
    void colorRefSignal( QColor& );

    void dateChanged( const QDate& );
    void dateRefSignal( QDate& );
    
    void timeChanged( const QDateTime& );
    void timeRefSignal( QDateTime& );

    void datetimeChanged( const QDateTime& );
    void datetimeRefSignal( QDateTime& );

    void fontChanged( const QFont& );
    void fontRefSignal( QFont& );

    void pixmapChanged( const QPixmap& );
    void pixmapRefSignal( QPixmap& );

    void listChanged( const QValueList<QVariant>& );
    void listRefSignal( QValueList<QVariant>& );

    void betaChanged( Alpha );
    void betaRefSignal( Alpha& );

    void currencyChanged( Q_LLONG );
    void currencyRefSignal( Q_LLONG& );

    void bytesChanged( const QByteArray& );
    void bytesRefSignal( QByteArray& );

protected:
    void paintEvent( QPaintEvent *e )
    {
	QPainter painter( this );
	painter.setFont( m_font );
	painter.setPen( m_color );
	QFontMetrics fm( font() );

	QRect r( rect() );

	if ( !m_pixmap.isNull() )
	    painter.drawPixmap( r, m_pixmap );

	painter.drawText( r, ExpandTabs, "Unicode\t:" + m_unicode );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "C-String\t\t:" + m_text );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "bool\t\t:" + QVariant(m_boolval,42).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "number\t\t:" + QVariant(m_number).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "posnumber\t:" + QVariant(m_posnumber).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "double\t\t:" + QVariant(m_real).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "color\t\t:" + QVariant(m_color).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "date\t\t:" + QVariant(m_date).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "time\t\t:" + QVariant(m_time).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "datetime\t:" + QVariant(m_datetime).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "font\t\t:" + QVariant(m_font).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );

	QStringList list;
	QValueList<QVariant>::Iterator it = m_list.begin();
	while ( it != m_list.end() ) {
	    QVariant var = *it;
	    ++it;
	    list << var.toString();
	}
	painter.drawText( r, ExpandTabs, "list\t\t:" + list.join(", ") );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "enum\t\t:" + QVariant(m_beta).toString() );
	r.addCoords( 0, fm.height(), 0, 0 );
	painter.drawText( r, ExpandTabs, "currency\t:" + QVariant(m_currency).toString() );
	if ( m_subType ) {
	    r.addCoords( 0, fm.height(), 0, 0 );
	    painter.drawText( r, ExpandTabs, "SubType" );
	    r.addCoords( 0, fm.height(), 0, 0 );
	    painter.drawText( r, ExpandTabs, "\tunicode\t\t:" + m_subType->unicode() );
	    r.addCoords( 0, fm.height(), 0, 0 );
	    painter.drawText( r, ExpandTabs, "\tnumber\t\t:" + QString::number( m_subType->number() ) );
	}
    }

private:
    QString m_unicode;
    QCString m_text;
    bool m_boolval;
    int m_number;
    uint m_posnumber;
    double m_real;
    QColor m_color;
    QDate m_date;
    QDateTime m_time;
    QDateTime m_datetime;
    QFont m_font;
    QPixmap m_pixmap;
    QValueList<QVariant> m_list;
    Alpha m_beta;
    Q_LLONG m_currency;
    QByteArray m_bytes;
    IDispatch *m_disp;

    QSubType *m_subType;
};

#include "control.moc"

class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app ) 
	: QAxFactory( lib, app ) 
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << "QTestControl";
	list << "QSubType";
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( key == "QTestControl" )
	    return new QTestControl( parent, name );
	return 0;
    }
    QUuid classID( const QString &key ) const
    {
	if ( key == "QTestControl" )
	    return "{28f6fa1f-49b8-4a77-8b18-2b9b80fc6717}";
	if ( key == "QSubType" )
	    return "{c37ce00f-bcd8-477e-bc84-a90bf3902f0a}";
	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QTestControl" )
	    return "{f2da4629-df7f-4821-b249-30eacbae247f}";
	if ( key == "QSubType" )
	    return "{9f1802a1-7e7c-44e7-b09e-3f15c9cf7b51}";
	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QTestControl" )
	    return "{60743b03-3630-46b7-b2c6-2d1c46c277a4}";
	if ( key == "QSubType" )
	    return "{409b83ea-207a-48c7-94c7-3fcb9084779b}";
	return QUuid();
    }

    QString exposeToSuperClass( const QString &key ) const
    {
	return key;
    }
    bool hasStockEvents( const QString &key ) const
    {
	return FALSE;
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{b934af55-89ec-4b65-8a7d-80a4892fc86a}", "{83cc66df-c867-4c23-b641-9e06d02f3b33}" )

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    return app.exec();
}
