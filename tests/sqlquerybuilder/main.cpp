#include "qsqlquerybuilder.h"

int main( int argc, char ** argv )
{
    QSqlQueryBuilder qb( "table1 b" );
    
    qb.addField( "a.name" );
    qb.addField( "b.price" );
    qb.join( "table2 a", "a.id = b.id" ).leftOuterJoin( "table3", "a.id = table3.name_id" ).rightOuterJoin( "table4 c", "c.id = table3.name2_id" );
    qWarning( qb.selectQuery() );
    return 1;
}
