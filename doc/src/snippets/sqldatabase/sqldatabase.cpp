#include <QtSql>
#include <QPixmap>

void QSqlDatabase_snippets()
{
    {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("customdb");
    db.setUserName("mojito");
    db.setPassword("J0a1m8");
    bool ok = db.open();
    Q_UNUSED(ok);
    }

    {
    QSqlDatabase db = QSqlDatabase::database();
    }
}

void QSqlField_snippets()
{
#if 0
    {
    QSqlField field("age", QCoreVariant::Int);
    field.setValue(QPixmap());  // WRONG
    }
#endif

    {
    QSqlField field("age", QCoreVariant::Int);
    field.setValue(QString("123"));  // casts QString to int
    }

    {
    QSqlQuery query;
    QSqlRecord record = query.record();
    QSqlField field = record.field("country");
    }
}

int main()
{
    QSqlDatabase_snippets();
    QSqlField_snippets();
}
