#ifndef QSQLEDITORFACTORY_H
#define QSQLEDITORFACTORY_H

#ifndef QT_H
#include "qvariant.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QWidget;

class Q_EXPORT QSqlEditorFactory
{
public:
    static  QSqlEditorFactory * instance();
    virtual QWidget * createEditor( QWidget * parent, QSqlField & field );

protected:
    QSqlEditorFactory();

private:
};

#endif // QT_NO_SQL
#endif // QSQLEDITORFACTORY_H
