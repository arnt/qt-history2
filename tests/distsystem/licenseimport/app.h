#include <qapplication.h>
#include <qsqldatabase.h>

class ImportApp : public QApplication
{
public:
    ImportApp( int argc, char** argv );

    void doImport();

    QSqlDatabase* axaptaDB;
};
