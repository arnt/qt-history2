/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qwidget.h>

#include <qaxbindable.h>
#include <qaxfactory.h>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qvariant.h>

#define PROP(prop) return m_##prop;
#define SET_PROP(prop) m_##prop = prop;
#define GET_PROP_SLOT(prop) return m_##prop;
#define SET_PROP_SLOT(prop) m_##prop = prop;
#define GET_AND_SET(prop, type) type old = m_##prop; m_##prop = prop; prop = old; return m_##prop;
#define EMIT_REF(prop, type) type old = m_##prop; emit prop##RefSignal( m_##prop ); return old;

class QTestControl : public QWidget, public QAxBindable
{
    Q_OBJECT
    Q_PROPERTY( QString unicode READ unicode WRITE setUnicode )
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
    
//  QVariant does not support short or long
//    Q_PROPERTY( short shortnumber READ shortnumber WRITE setShortnumber )
//    Q_PROPERTY( long longnumber READ longnumber WRITE setLongnumber )

public:
    QTestControl( QWidget *parent = 0, const char *name = 0 )
    : QWidget( parent, name )
    {
	// members not initiliazed on purpose
    }

    QString unicode() const { PROP(unicode) }
    void setUnicode( const QString &unicode ){ SET_PROP(unicode) }

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

    QDateTime date() const { PROP(date) }
    void setDate( QDateTime date ) { SET_PROP(date) }

    QDateTime time() const { PROP(time) }
    void setTime( QDateTime time ) { SET_PROP(time) }

    QDateTime datetime() const { PROP(datetime) }
    void setDatetime( QDateTime datetime ) { SET_PROP(datetime) }

    QFont font() const { PROP(font) }
    void setFont( QFont font ) { SET_PROP(font) }

    QPixmap pixmap() const { PROP(pixmap) }
    void setPixmap( QPixmap pixmap ) { SET_PROP(pixmap) }

    QValueList<QVariant> list() const { PROP(list) }
    void setList( QValueList<QVariant> list ) { SET_PROP(list) }

/*
    short shortnumber() const { PROP(shortnumber) }
    void setShortnumber( short shortnumber ) { SET_PROP(shortnumber) }

    long longnumber() const { PROP(longnumber) }
    void setLongnumber( long longnumber ) { SET_PROP(longnumber) }
*/
public slots:
    QString getUnicodeSlot() const { GET_PROP_SLOT(unicode) }
    void setUnicodeSlot( const QString &unicode ) { SET_PROP_SLOT(unicode) }
    QString getAndSetUnicodeSlot( QString &unicode ) { GET_AND_SET(unicode, QString) }
    QString emitUnicodeRefSignal() { EMIT_REF(unicode, QString) }

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

    QDateTime getDateSlot() const { GET_PROP_SLOT(date) }
    void setDateSlot( QDateTime date ) { SET_PROP_SLOT(date) }
    QDateTime getAndSetDateSlot( QDateTime& date ) { GET_AND_SET(date, QDateTime) }
    QDateTime emitDateRefSignal() { EMIT_REF(date, QDateTime) }

    QDateTime getTimeSlot() const { GET_PROP_SLOT(time) }
    void setTimeSlot( QDateTime time ) { SET_PROP_SLOT(time) }
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

/*
    short getShortnumberSlot() const { GET_PROP_SLOT(shortnumber) }
    void setShortnumberSlot( short shortnumber ) { SET_PROP_SLOT(shortnumber) }
    short getAndSetShortnumberSlot( short& shortnumber ) { GET_AND_SET(shortnumber, short) }
    short emitShortnumberRefSignal() { EMIT_REF(shortnumber, short) }

    long getLongnumberSlot() const { GET_PROP_SLOT(longnumber) }
    void setLongnumberSlot( long longnumber ) { SET_PROP_SLOT(longnumber) }
    long getAndSetLongnumberSlot( long& longnumber ) { GET_AND_SET(longnumber, long) }
    long emitLongnumberRefSignal() { EMIT_REF(longnumber, long) }
*/
signals:
    void unicodeChanged( const QString& );
    void unicodeRefSignal( QString& );

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

    void dateChanged( const QDateTime& );
    void dateRefSignal( QDateTime& );
    
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

/*
    void shortnumberChanged( short );
    void shortnumberRefSignal( short& );

    void longnumberChanged( long );
    void longnumberRefSignal( long& );
*/
private:
    QString m_unicode;
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

/*
    short m_shortnumber;
    long m_longnumber;
*/
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
	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QTestControl" )
	    return "{f2da4629-df7f-4821-b249-30eacbae247f}";
	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QTestControl" )
	    return "{60743b03-3630-46b7-b2c6-2d1c46c277a4}";
	return QUuid();
    }

    QString exposeToSuperClass( const QString &key ) const
    {
	return key;
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{b934af55-89ec-4b65-8a7d-80a4892fc86a}", "{83cc66df-c867-4c23-b641-9e06d02f3b33}" )

void main()
{}
