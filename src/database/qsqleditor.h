#ifndef QSQLEDITOR_H
#define QSQLEDITOR_H

#ifndef QT_H
#include "qwidget.h"
#include "qlayout.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlEditor : public QWidget
{
    Q_OBJECT 
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


class QLineEdit;
class QSqlLineEdit : public QSqlEditor
{
public:
    QSqlLineEdit ( QWidget * parent, QSqlField& field, const char * name=0 );
    QSqlLineEdit ( const QString & contents, QWidget * parent, QSqlField& field, const char * name=0 );
    ~QSqlLineEdit();
    QLineEdit* lineEdit();
protected:
    QVariant editorValue();
    void takeValue( QVariant& value );
private:
    QLineEdit* ed;
    QGridLayout* grid;
};

class QSpinBox;
class QSqlSpinBox : public QSqlEditor
{
public:
    QSqlSpinBox ( QWidget * parent, QSqlField& field, const char * name = 0 );
    QSqlSpinBox ( int minValue, int maxValue, int step, QWidget * parent, QSqlField& field, const char * name = 0 );
    ~QSqlSpinBox();
    QSpinBox* spinBox();
protected:
    QVariant editorValue();
    void takeValue( QVariant& value );
private:
    QSpinBox* ed;
    QGridLayout* grid;
};


#endif	// QT_NO_SQL
#endif
