/****************************************************************************
** Form interface generated from reading ui file '/home/db/src/qt/main/tools/designer/manual/sgml/eg/book/book8/book.ui'
**
** Created: Mon Feb 26 13:06:45 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef BOOKFORM_H
#define BOOKFORM_H

#include <qvariant.h>
#include <qdatabrowser.h>
#include <qdialog.h>
#include <qsqlrecord.h>
#include "editbook.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QDataTable;
class QPushButton;
class QSplitter;
class QSqlCursor;
class QSqlDatabase;
class QSqlRecord;

class BookForm : public QDialog
{ 
    Q_OBJECT

public:
    BookForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~BookForm();

    QSplitter* Splitter1;
    QDataTable* AuthorDataTable;
    QDataTable* BookDataTable;
    QPushButton* EditPushButton;
    QPushButton* QuitPushButton;


public slots:
    virtual void editClicked();
    virtual void newCurrentAuthor( QSqlRecord *author );
    virtual void primeInsertAuthor( QSqlRecord *buffer );
    void polish();

protected slots:
    virtual void init();
    virtual void destroy();

protected:
    QVBoxLayout* BookFormLayout;
    QHBoxLayout* Layout5;
};

#endif // BOOKFORM_H
