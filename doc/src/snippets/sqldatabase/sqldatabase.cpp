#include <QSqlDatabase>

void QSqlDatabase_snippet()
{
    {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("customdb");
    db.setUserName("mojito");
    db.setPassword("J0a1m8");
    bool ok = db.open();
    }

    {
    QSqlDatabase db = QSqlDatabase::database();
    }
}

int main()
{
    QSqlDatabase_snippet();
}
