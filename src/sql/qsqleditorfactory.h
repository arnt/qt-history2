#ifndef QSQLEDITORFACTORY_H
#define QSQLEDITORFACTORY_H

#ifndef QT_H
#include "qobject.h"
#include "qvariant.h"
#include "qsqlfield.h"
#include "qlineedit.h"
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
/*
class QSqlDateTimeEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY( QDateTime dateTime READ dateTime WRITE setDateTime )
public:
};

class QSqlDateEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY( QDate date READ date WRITE setDate )
public:
};

class QSqlTimeEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY( QTime time READ time WRITE setTime )
public:
};*/
#endif // QT_NO_SQL
#endif // QSQLEDITORFACTORY_H
