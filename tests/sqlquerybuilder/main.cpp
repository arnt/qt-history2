#include "qsqlquerybuilder.h"

int main( int argc, char ** argv )
{
    QSqlQueryBuilder qb( "table1 b" );
    
    qb.addField( "a", "name" );
    qb.addField( "a", "address" );
    qb.addField( "b", "price" );
    qb.join( "table2 a", "a.id = b.id" ).leftOuterJoin( "table3", "a.id = table3.name_id" ).rightOuterJoin( "table4 c", "c.id = table3.name2_id" );
    qWarning( qb.selectQuery() );
    qWarning( qb.insertQueries().join(";\n") );
    qWarning( qb.updateQueries().join(";\n") );
    qWarning( qb.deleteQueries().join(";\n") );
    return 1;
}
