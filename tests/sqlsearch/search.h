#include "searchbase.h"

class QSqlCursor;

class Search : public SearchBase 
{
    Q_OBJECT
public:
    Search( QWidget * parent = 0, const char * name = 0 );
    
public slots:
    void close();
    void search();
    
private:
    QSqlCursor * cursor;
};
