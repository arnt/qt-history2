#include "sqlinterpreter.h"
#include <qvariant.h>
#include <qvaluelist.h>

#define FILENAME "test.dbf"

int main( int /*argc*/, char** /*argv*/ )
{

    Environment env;

#if 0
    /* create a table */
    QVariant name = "id";
    QVariant value;
    value.cast( QVariant::Int );
    QValueList<QVariant> field;
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    name = "name";
    value.cast( QVariant::String );
    field[0] = name;
    field[1] = value;
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Create( FILENAME ) );
#endif

#if 0
    /* insert some records */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Push( 12 ) );
    env.program().append( new Push( QString("db") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 11 ) );
    env.program().append( new Push( QString("jasmin") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 10 ) );
    env.program().append( new Push( QString("trolltech") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 9 ) );
    env.program().append( new Push( QString("gnome") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 8 ) );
    env.program().append( new Push( QString("linux") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 7 ) );
    env.program().append( new Push( QString("boogers") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 6 ) );
    env.program().append( new Push( QString("junk") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 5 ) );
    env.program().append( new Push( QString("knicks") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 4 ) );
    env.program().append( new Push( QString("oslo") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 3 ) );
    env.program().append( new Push( QString("norway") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 2 ) );
    env.program().append( new Push( QString("europe") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );

    env.program().append( new Push( 1 ) );
    env.program().append( new Push( QString("canada") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Insert( 0 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* delete records with id = 2 */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0, 6 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new Push( 2 ) );
    env.program().append( new Ne( 1 ) );
    env.program().append( new Mark( 0 ) );
    //env.program().append( new Goto( 1 ) );
    //env.program().append( new DeleteMarked( 0 ) );
    env.program().append( new RewindMarked( 0 ) );
    env.program().append( new NextMarked( 0 , 12 ) );
    env.program().append( new Push( 99 ) );
    env.program().append( new Push( QString("blarging") ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Update( 0 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* update record with id = 2 */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0, 7 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new Push( 2 ) );
    env.program().append( new Ne( 1 ) );
    env.program().append( new Mark( 0 ) );
    env.program().append( new Goto( 1 ) );
    QVariant name = "name";
    QVariant value = QString("barf");
    QValueList<QVariant> field;
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new UpdateMarked( 0 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* select some records */
    QValueList<QVariant> field;
    QVariant name = "id";
    QVariant value;
    value.cast( QVariant::Int );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    field.clear();
    name = "name";
    value.cast( QVariant::String );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new CreateResult() );
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0, 14 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new Push( 2 ) );
    env.program().append( new Ne( 5 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new PushField( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 5 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* create an index on id field */
    QValueList<QVariant> field;
    QVariant name = "id";
    QVariant value;
    value.cast( QVariant::Int );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new CreateIndex( 0, QVariant(FALSE,1) ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* create an index on name field */
    QValueList<QVariant> field;
    QVariant name = "name";
    QVariant value;
    value.cast( QVariant::String );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new CreateIndex( 0, QVariant(FALSE,1) ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* select some records using a range scan */
    QValueList<QVariant> field;
    QVariant name = "id";
    QVariant value;
    value.cast( QVariant::Int );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    field.clear();
    name = "name";
    value.cast( QVariant::String );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new CreateResult() );
    env.program().append( new Open( 0, FILENAME ) );
    name = "name";
    value = QString("trolltech");
    field.clear();
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new RangeScan( 0 ) );
    env.program().append( new RewindMarked( 0 ) );
    env.program().append( new NextMarked( 0 , 15 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new PushField( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 9 ) );
    env.program().append( new Close( 0 ) );
#endif

    /* select all records and sort */
    QValueList<QVariant> field;
    QVariant name = "id";
    QVariant value;
    value.cast( QVariant::Int );
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    name = "name";
    value.cast( QVariant::String );
    field.clear();
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new CreateResult() ); // ## perhaps give the result an id??
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0 , 11 ) );
    env.program().append( new PushField( 0, 0 ) );
    env.program().append( new PushField( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 5 ) );
    name = "id";
    value.cast( QVariant::Int );
    field.clear();
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    name = "name";
    value.cast( QVariant::String );
    field.clear();
    field.append( name );
    field.append( value );
    env.program().append( new Push( field ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Sort( 0 ) ); //## to be used here.
    env.program().append( new Close( 0 ) );

#if 0
    /* drop a table */
    env.program().append( new Drop( 0, FILENAME ) );
#endif

    env.execute();
    env.saveListing( "programlisting" );

    return 0;
}
