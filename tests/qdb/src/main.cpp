#include "sqlinterpreter.h"
#include <qvariant.h>
#include <qvaluelist.h>

#define FILENAME "test.dbf"

int main( int /*argc*/, char** /*argv*/ )
{

    Environment env;

#if 0
    /* create a table :

       create table test(
       id numeric(10),
       name character(30)
       );

     */
    env.program().append( new Push( "id" ) );
    env.program().append( new Push( QVariant::Int ) );
    env.program().append( new Push( 10 ) );
    env.program().append( new Push( 0 ) );
    env.program().append( new PushList( 4 ) );
    env.program().append( new Push( "name" ) );
    env.program().append( new Push( QVariant::String ) );
    env.program().append( new Push( 30 ) );
    env.program().append( new Push( 0 ) );
    env.program().append( new PushList( 4 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Create( FILENAME ) );
#endif

#if 0
    /* insert some records */
    env.program()->append( new Open( 0, FILENAME ) );
    env.program()->append( new Push( "id" ) );
    env.program()->append( new Push( 12 ) );
    env.program()->append( new PushList( 2 ) );
    env.program()->append( new Push( "name" ) );
    env.program()->append( new Push( QString("db") ) );
    env.program()->append( new PushList( 2 ) );
    env.program()->append( new PushList( 2 ) );
    env.program()->append( new Insert( 0 ) );
    env.program()->append( new Close( 0 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* create an index on id field */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new PushFieldDesc( 0, "id" ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new CreateIndex( 0, QVariant(FALSE,1) ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* delete records with id = 2 */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0, 6 ) );
    env.program().append( new PushFieldValue( 0, 0 ) );
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
    /* update a record */
    /* update a record */
    env.program()->append( new Open( 0, FILENAME ) );
    env.program()->append( new Next( 0, 6 ) );
    env.program()->append( new Push( QString("name") ) );
    env.program()->append( new Push( QString("blarg") ) );
    env.program()->append( new PushList( 2 ) );
    env.program()->append( new Update( 0 ) );
    env.program()->append( new Close( 0 ) );
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
    env.program().append( new CreateResult( 0 ) );
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new Next( 0, 14 ) );
    env.program().append( new PushFieldValue( 0, 0 ) );
    env.program().append( new Push( 2 ) );
    env.program().append( new Ne( 5 ) );
    env.program().append( new PushFieldValue( 0, 0 ) );
    env.program().append( new PushFieldValue( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 5 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* create an index on name field */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new PushFieldDesc( 0, "name" ) );
    env.program().append( new PushList( 1 ) );
    env.program().append( new CreateIndex( 0, QVariant(FALSE,1) ) );
    env.program().append( new Close( 0 ) );
#endif

    /* select some records using a range scan */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new PushFieldDesc( 0, "id" ) );
    env.program().append( new PushFieldDesc( 0, "name" ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new CreateResult( 0 ) );
    env.program().append( new PushFieldDesc( 0, "name" ) );
    env.program().append( new Push( "trolltech" ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new RangeScan( 0 ) );
    env.program().append( new RewindMarked( 0 ) );
    env.program().append( new NextMarked( 0 , 16 ) );
    env.program().append( new PushFieldValue( 0, 0 ) );
    env.program().append( new PushFieldValue( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 10 ) );
    env.program().append( new PushFieldDesc( 0, "id" ) );
    env.program().append( new Push( QVariant( TRUE, 0 )  ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new Sort( 0 ) );
    env.program().append( new Close( 0 ) );

#if 0
    /* select all records and sort */
    env.program().append( new Open( 0, FILENAME ) );
    env.program().append( new PushFieldDesc( 0, "id" ) );
    env.program().append( new PushFieldDesc( 0, "name" ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new CreateResult( 0 ) );
    env.program().append( new Next( 0 , 11 ) );
    env.program().append( new PushFieldValue( 0, 0 ) );
    env.program().append( new PushFieldValue( 0, 1 ) );
    env.program().append( new PushList( 2 ) );
    env.program().append( new SaveResult( 0 ) );
    env.program().append( new Goto( 5 ) );
    env.program().append( new PushFieldDesc( 0, "id" ) );
    env.program().append( new Push( QVariant( FALSE, 0 )  ) );
    env.program().append( new PushFieldDesc( 0, "name" ) );
    env.program().append( new Push( QVariant( TRUE, 0 ) ) );
    env.program().append( new PushList( 4 ) );
    env.program().append( new Sort( 0 ) );
    env.program().append( new Close( 0 ) );
#endif

#if 0
    /* drop a table */
    env.program().append( new Drop( 0, FILENAME ) );
#endif

    env.execute();
    env.saveListing( "programlisting" );

    return 0;
}
