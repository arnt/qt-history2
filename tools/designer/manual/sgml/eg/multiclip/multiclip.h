/****************************************************************************
** Form interface generated from reading ui file 'multiclip.ui'
**
** Created: Wed Feb 14 13:43:47 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef MULTICLIPFORM_H
#define MULTICLIPFORM_H

#include <qvariant.h>
#include <qclipboard.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QLCDNumber;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;

class MulticlipForm : public QDialog
{ 
    Q_OBJECT

public:
    MulticlipForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MulticlipForm();

    QLabel* TextLabel1;
    QLineEdit* currentLineEdit;
    QPushButton* addPushButton;
    QPushButton* quitPushButton;
    QLabel* TextLabel3;
    QLabel* TextLabel2;
    QCheckBox* autoCheckBox;
    QLCDNumber* lengthLCDNumber;
    QPushButton* deletePushButton;
    QListBox* clippingsListBox;
    QPushButton* copyPushButton;


public slots:
    virtual void init();
    virtual void destroy();
    virtual void addClipping();
    virtual void copyPrevious();
    virtual void dataChanged();
    virtual void deleteClipping();
    virtual void selectionChanged();
    virtual void clippingChanged( const QString & s );

protected:
    QVBoxLayout* MulticlipFormLayout;
    QHBoxLayout* Layout1;
    QGridLayout* Layout19;

    QClipboard *cb;
};

#endif // MULTICLIPFORM_H
