#include "qsqlquerybuilder.h"

int main( int argc, char ** argv )
{
    QSqlQueryBuilder qb( "table1 b" );
    QStringList key;
    
    qb.addField( "a", "name" );
    qb.addField( "a", "address" );
    qb.addField( "b", "price" );
    key << "p_id" << "id" << "c_id";
    qb.setPrimaryKey( "a", key );
    key.clear();
    key << "id";
    qb.setPrimaryKey( "b", key );
    qb.join( "table2 a", "a.id = b.id" ).leftOuterJoin( "table3", "a.id = table3.name_id" ).rightOuterJoin( "table4 c", "c.id = table3.name2_id" );
    qWarning( qb.selectQuery() );
    qWarning( qb.insertQueries().join("\n") );
    qWarning( qb.updateQueries().join("\n") );
    qWarning( qb.deleteQueries().join("\n") );
    return 1;
}
