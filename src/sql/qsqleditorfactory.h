#ifndef QSQLEDITORFACTORY_H
#define QSQLEDITORFACTORY_H

#ifndef QT_H
#include "qobject.h"
#include "qvariant.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlEditorFactory : public QObject
{
public:
    QSqlEditorFactory ( QObject * parent=0, const char * name=0 );
    ~QSqlEditorFactory();
    virtual QWidget * createEditor( QWidget * parent, const QVariant & v );
    virtual QWidget * createEditor( QWidget * parent, const QSqlField & f );
};

#endif // QT_NO_SQL
#endif // QSQLEDITORFACTORY_H
