#include "formbase.h"

class QSqlForm;
class QSqlCursor;

class Form : public FormBase 
{
    Q_OBJECT
public:
    Form( QWidget * parent = 0, const char * name = 0 );
    
public slots:
    void update();
    void close();
    void del();
    void insert();
    void clear();
    void prev();
    void next();    
    void first();
    void last();
    
private:
    QSqlForm * form;
    QSqlCursor * cursor;
};
