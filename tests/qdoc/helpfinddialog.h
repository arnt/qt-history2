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
    void find( const QString &query, bool cs, bool ww, bool fromStart );

private slots:
    void find();
    void checkQuery( const QString &s );
    
private:
    QComboBox *findEdit;
    QPushButton *findButton;
    QCheckBox *findCaseSensitive, *findWholeWords;
    QString lastQuery;
    
};

#endif
