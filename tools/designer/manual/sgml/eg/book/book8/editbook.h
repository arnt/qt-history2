/****************************************************************************
** Form interface generated from reading ui file '/home/db/src/qt/main/tools/designer/manual/sgml/eg/book/book8/editbook.ui'
**
** Created: Mon Feb 26 13:06:24 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef EDITBOOKFORM_H
#define EDITBOOKFORM_H

#include <qvariant.h>
#include <qdialog.h>
#include <qsqlrecord.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class QDataBrowser;
class QLabel;
class QLineEdit;
class QPushButton;
class QSqlCursor;
class QSqlDatabase;
class QSqlForm;
class QSqlRecord;

class EditBookForm : public QDialog
{ 
    Q_OBJECT

public:
    EditBookForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~EditBookForm();

    QDataBrowser* BookDataBrowser;
    QLabel* labelPrice;
    QLabel* labelTitle;
    QLineEdit* QLineEditTitle;
    QLineEdit* QLineEditPrice;
    QPushButton* PushButtonInsert;
    QPushButton* PushButtonUpdate;
    QPushButton* PushButtonDelete;
    QPushButton* PushButtonClose;
    QPushButton* PushButtonFirst;
    QPushButton* PushButtonPrev;
    QPushButton* PushButtonNext;
    QPushButton* PushButtonLast;
    QLabel* TextLabel1;
    QComboBox* ComboBoxAuthor;


public slots:
    virtual void beforeUpdateBook( QSqlRecord * buffer );
    virtual void mapAuthor( const QString & name, int & id, bool populate );
    virtual void primeInsertBook( QSqlRecord * buffer );
    virtual void primeUpdateBook( QSqlRecord * buffer );
    void polish();

protected slots:
    virtual void init();
    virtual void destroy();

protected:
    QVBoxLayout* EditBookFormLayout;
    QGridLayout* BookDataBrowserLayout;
    QGridLayout* Layout2;
    QHBoxLayout* Layout6;
    QHBoxLayout* Layout3;
    QHBoxLayout* Layout6_2;

    QMap<QString,int> authorMap;
};

#endif // EDITBOOKFORM_H
