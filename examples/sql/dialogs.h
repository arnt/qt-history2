#ifndef DIALOGS_H
#define DIALOGS_H

#include "cursors.h"

#include <qdialog.h>
#include <qsqltable.h>
#include <qsqleditorfactory.h>
#include <qframe.h>

class QSqlForm;
class QSqlTable;
class QLabel;
class QPushButton;
class QComboBox;

class ProductEditor : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( int productid READ productId WRITE setProductId )    
public:
    ProductEditor( QWidget * parent = 0,
		       const char * name = "ProductEditor" );
    int productId() const;
    void setProductId( int productId );
 
protected slots:
    void slotSetProductId( int productId );
    
private:
    QComboBox* cb;
};


class InvoiceEditorFactory : public QSqlEditorFactory
{
public:
    InvoiceEditorFactory ( QObject * parent=0, const char * name=0 );    
    QWidget * createEditor( QWidget * parent, const QSqlField * f );
};


class GenericDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    GenericDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		   const char * name = 0 );
public slots:
    void close();
    void execute();

private:
    Mode mMode;
    QSqlForm * form;
};

#endif // DIALOGS_H

