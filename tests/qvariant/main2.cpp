#include <qapplication.h>
#include <qvariant.h>
#include <qdatetime.h>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    QVariant v;
    qDebug("Beginning test.");
    
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

    v = QVariant( "ciao" );
    ASSERT( v.canCast( QVariant::String ) && v.canCast( QVariant::CString ) );
    ASSERT( v.toString() == "ciao" );
    ASSERT( v.toCString() == "ciao" );

    v = QVariant( QString("1234") );
    ASSERT( v.canCast( QVariant::String ) );
    ASSERT( v.canCast( QVariant::CString ) );
    ASSERT( v.canCast( QVariant::Int ) );
    ASSERT( v.canCast( QVariant::UInt ) ); 
    ASSERT( v.canCast( QVariant::Double ) );
    ASSERT( v.canCast( QVariant::Date ) );
    ASSERT( v.canCast( QVariant::Time ) );
    ASSERT( v.canCast( QVariant::DateTime ) );
    ASSERT( v.toInt() == 1234 );
    ASSERT( v.toUInt() == (uint)1234 );
    ASSERT( v.toDouble() == (double)1234 );

    QDate d( 1972,4,18 );
    v = QVariant( d );
    ASSERT( v.canCast( QVariant::String ) );
    ASSERT( v.toString() == d.toString( Qt::ISODate ) );
    QDate dfs = QDate::fromString( d.toString( Qt::ISODate ), Qt::ISODate );
    ASSERT( dfs.isValid() );        
    ASSERT( dfs.toString() == d.toString() );
    ASSERT( dfs.toString( Qt::ISODate ) == d.toString( Qt::ISODate ) );
    dfs = QDate::fromString( d.toString() );
    ASSERT( dfs.isValid() );    
    ASSERT( dfs.toString() == d.toString() );
    ASSERT( dfs.toString( Qt::ISODate ) == d.toString( Qt::ISODate ) );
    v = QVariant( d.toString( Qt::ISODate ) );
    ASSERT( d == v.toDate() );
    
    QTime t( 12,5,45 );
    v = QVariant( t );
    ASSERT( v.canCast( QVariant::String ) );
    ASSERT( v.toString() == t.toString( Qt::ISODate ) );
    QTime tfs = QTime::fromString( t.toString( Qt::ISODate ), Qt::ISODate );
    ASSERT( tfs.isValid() );    
    ASSERT( tfs.toString() == t.toString() );
    ASSERT( tfs.toString( Qt::ISODate ) == t.toString( Qt::ISODate ) );
    tfs = QTime::fromString( t.toString() );
    ASSERT( tfs.isValid() );    
    ASSERT( tfs.toString() == t.toString() );
    ASSERT( tfs.toString( Qt::ISODate ) == t.toString( Qt::ISODate ) );
    v = QVariant( t.toString( Qt::ISODate ) );
    ASSERT( t == v.toTime() );

    QDateTime dt( d, t );
    v = QVariant( dt );
    ASSERT( v.canCast( QVariant::String ) );
    ASSERT( v.toString() == dt.toString( Qt::ISODate ) );
    QDateTime dtfs = QDateTime::fromString( dt.toString( Qt::ISODate ), Qt::ISODate );
    ASSERT( dtfs.isValid() );
    ASSERT( dtfs.toString() == dt.toString() );
    ASSERT( dtfs.toString( Qt::ISODate ) == dt.toString( Qt::ISODate ) );
    dtfs = QDateTime::fromString( dt.toString() );
    ASSERT( dtfs.isValid() );    
    ASSERT( dtfs.toString() == dt.toString() );
    ASSERT( dtfs.toString( Qt::ISODate ) == dt.toString( Qt::ISODate ) );
    v = QVariant( dt.toString( Qt::ISODate ) );
    ASSERT( dt == v.toDateTime() );
   
    qDebug("Done.");
}

