/****************************************************************************
** Form interface generated from reading ui file 'creditformbase.ui'
**
** Created: Mon Feb 19 12:01:36 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CREDITFORMBASE_H
#define CREDITFORMBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QPushButton;
class QRadioButton;
class QSpinBox;

class CreditFormBase : public QDialog
{ 
    Q_OBJECT

public:
    CreditFormBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CreditFormBase();

    QButtonGroup* ButtonGroup1;
    QRadioButton* stdRadioButton;
    QRadioButton* noneRadioButton;
    QRadioButton* specialRadioButton;
    QSpinBox* amountSpinBox;
    QPushButton* okPushButton;
    QPushButton* cancelPushButton;


public slots:
    virtual void special(bool);

protected slots:
    virtual void destroy();
    virtual void init();

protected:
    QVBoxLayout* CreditFormBaseLayout;
    QHBoxLayout* ButtonGroup1Layout;
    QVBoxLayout* Layout1;
    QHBoxLayout* Layout4;
};

#endif // CREDITFORMBASE_H
