/****************************************************************************
** Form interface generated from reading ui file 'mainform.ui'
**
** Created: Tue Feb 20 15:34:46 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef MAINFORM_H
#define MAINFORM_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;

class MainForm : public QDialog
{ 
    Q_OBJECT

public:
    MainForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MainForm();

    QPushButton* quitPushButton;
    QPushButton* creditPushButton;
    QLabel* TextLabel1;
    QLabel* ratingTextLabel;


public slots:
    virtual void creditDialog();

protected slots:
    virtual void init();
    virtual void destroy();

protected:
    QGridLayout* MainFormLayout;
};

#endif // MAINFORM_H
