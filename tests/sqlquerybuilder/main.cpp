#include "qsqlquerybuilder.h"

int main( int argc, char ** argv )
{
    QSqlQueryBuilder qb( "table1 b" );
    
    qb.addField( "a.name" );
    qb.addField( "b.price" );
    qb.join( "table2 a", "a.id = b.id" );
    qWarning( qb.selectQuery() );
    return 1;
}
