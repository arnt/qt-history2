#ifndef QSQLEDITOR_H
#define QSQLEDITOR_H

#ifndef QT_H
#include "qwidget.h"
#include "qsqlfield.h"
#include "qlineedit.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlEditor : public QWidget
{
public:
    QSqlEditor( QSqlField& field, QWidget * parent=0, const char * name=0, WFlags f=0 );
     ~QSqlEditor();
    void syncToEditor();
    void syncToField();
protected:
    virtual QVariant editorValue() = 0;
    virtual void takeValue( QVariant& value ) = 0;
private:
    QSqlField&       fld;
};


class QSqlLineEdit : public QSqlEditor
{
public:
    QSqlLineEdit ( QWidget * parent, QSqlField& field, const char * name=0 );
    QSqlLineEdit ( const QString & contents, QWidget * parent, QSqlField& field, const char * name=0 );
    ~QSqlLineEdit();
protected:
    QVariant editorValue();
    void takeValue( QVariant& value );
private:
    QLineEdit* ed;
};

#endif	// QT_NO_SQL
#endif
