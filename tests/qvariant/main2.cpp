#include <qapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qpen.h>
#include <qcolor.h>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    QVariant v;
    qDebug("Beginning test.");

    v = QVariant( TRUE );
    Q_ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    Q_ASSERT( v.toDouble() == 1.0 );
    Q_ASSERT( v.toInt() == 1 );
    Q_ASSERT( v.toUInt() == 1 );
    v = QVariant( FALSE );
    Q_ASSERT( v.toDouble() == 0.0 );
    Q_ASSERT( v.toInt() == 0 );
    Q_ASSERT( v.toUInt() == 0 );

    v = QVariant( 5.3 );
    Q_ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    Q_ASSERT( v.toBool() == TRUE );
    Q_ASSERT( v.toInt() == 5 );
    Q_ASSERT( v.toUInt() == 5 );
    v = QVariant( 0.0 );
    Q_ASSERT( v.toBool() == FALSE );

    v = QVariant( (int)5 );
    Q_ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    Q_ASSERT( v.toBool() == TRUE );
    Q_ASSERT( v.toDouble() == 5 );
    Q_ASSERT( v.toUInt() == 5 );
    v = QVariant( (int)0 );
    Q_ASSERT( v.toBool() == FALSE );

    v = QVariant( (uint)5 );
    Q_ASSERT( v.canCast( QVariant::Double ) && v.canCast( QVariant::Int ) && v.canCast( QVariant::UInt ) && v.canCast( QVariant::Bool ) );
    Q_ASSERT( v.toBool() == TRUE );
    Q_ASSERT( v.toDouble() == 5 );
    Q_ASSERT( v.toInt() == 5 );
    v = QVariant( (uint)0 );
    Q_ASSERT( v.toBool() == FALSE );

    v = QVariant( "ciao" );
    Q_ASSERT( v.canCast( QVariant::String ) && v.canCast( QVariant::CString ) );
    Q_ASSERT( v.toString() == "ciao" );
    Q_ASSERT( v.toCString() == "ciao" );

    v = QVariant( QString("1234") );
    Q_ASSERT( v.canCast( QVariant::String ) );
    Q_ASSERT( v.canCast( QVariant::CString ) );
    Q_ASSERT( v.canCast( QVariant::Int ) );
    Q_ASSERT( v.canCast( QVariant::UInt ) );
    Q_ASSERT( v.canCast( QVariant::Double ) );
    Q_ASSERT( v.canCast( QVariant::Date ) );
    Q_ASSERT( v.canCast( QVariant::Time ) );
    Q_ASSERT( v.canCast( QVariant::DateTime ) );
    Q_ASSERT( v.toInt() == 1234 );
    Q_ASSERT( v.toUInt() == (uint)1234 );
    Q_ASSERT( v.toDouble() == (double)1234 );

    QDate d( 1972,4,18 );
    v = QVariant( d );
    Q_ASSERT( v.canCast( QVariant::String ) );
    Q_ASSERT( v.toString() == d.toString( Qt::ISODate ) );
    QDate dfs = QDate::fromString( d.toString( Qt::ISODate ), Qt::ISODate );
    Q_ASSERT( dfs.isValid() );
    Q_ASSERT( dfs.toString() == d.toString() );
    Q_ASSERT( dfs.toString( Qt::ISODate ) == d.toString( Qt::ISODate ) );
    dfs = QDate::fromString( d.toString() );
    Q_ASSERT( dfs.isValid() );
    Q_ASSERT( dfs.toString() == d.toString() );
    Q_ASSERT( dfs.toString( Qt::ISODate ) == d.toString( Qt::ISODate ) );
    v = QVariant( d.toString( Qt::ISODate ) );
    Q_ASSERT( d == v.toDate() );

    QTime t( 12,5,45 );
    v = QVariant( t );
    Q_ASSERT( v.canCast( QVariant::String ) );
    Q_ASSERT( v.toString() == t.toString( Qt::ISODate ) );
    QTime tfs = QTime::fromString( t.toString( Qt::ISODate ), Qt::ISODate );
    Q_ASSERT( tfs.isValid() );
    Q_ASSERT( tfs.toString() == t.toString() );
    Q_ASSERT( tfs.toString( Qt::ISODate ) == t.toString( Qt::ISODate ) );
    tfs = QTime::fromString( t.toString() );
    Q_ASSERT( tfs.isValid() );
    Q_ASSERT( tfs.toString() == t.toString() );
    Q_ASSERT( tfs.toString( Qt::ISODate ) == t.toString( Qt::ISODate ) );
    v = QVariant( t.toString( Qt::ISODate ) );
    Q_ASSERT( t == v.toTime() );

    QDateTime dt( d, t );
    v = QVariant( dt );
    Q_ASSERT( v.canCast( QVariant::String ) );
    Q_ASSERT( v.toString() == dt.toString( Qt::ISODate ) );
    QDateTime dtfs = QDateTime::fromString( dt.toString( Qt::ISODate ), Qt::ISODate );
    Q_ASSERT( dtfs.isValid() );
    Q_ASSERT( dtfs.toString() == dt.toString() );
    Q_ASSERT( dtfs.toString( Qt::ISODate ) == dt.toString( Qt::ISODate ) );
    dtfs = QDateTime::fromString( dt.toString() );
    Q_ASSERT( dtfs.isValid() );
    Q_ASSERT( dtfs.toString() == dt.toString() );
    Q_ASSERT( dtfs.toString( Qt::ISODate ) == dt.toString( Qt::ISODate ) );
    v = QVariant( dt.toString( Qt::ISODate ) );
    Q_ASSERT( dt == v.toDateTime() );

    QPen pen( Qt::black, 2);
    v = QVariant( pen );
    Q_ASSERT( !v.canCast( QVariant::Double ) &&
	      !v.canCast( QVariant::Int ) &&
	      !v.canCast( QVariant::UInt ) &&
	      !v.canCast( QVariant::Bool ) );
    Q_ASSERT( v.canCast( QVariant::Pen ) );
    Q_ASSERT( v.toPen() == pen );
    Q_ASSERT( v.toDouble() != 1.0 );
    Q_ASSERT( v.toInt() != 1 );
    Q_ASSERT( v.toUInt() != 1 );

    qDebug("Done.");
}

