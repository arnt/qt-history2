#ifndef HELPFINDDIALOG_H
#define HELPFINDDIALOG_H

#include <qdialog.h>

class QComboBox;
class QPushButton;
class QCheckBox;
class QCloseEvent;

class HelpFindDialog : public QDialog
{
    Q_OBJECT
    
public:
    HelpFindDialog( QWidget *parent );

protected:
    void closeEvent( QCloseEvent *e );
    
signals:
    void closed();
    
private:
    QComboBox *findEdit;
    QPushButton *findButton;
    QCheckBox *findCaseSensitive, *findWholeWords;
    
};

#endif
