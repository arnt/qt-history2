#ifndef MAINFORM_H
#define MAINFORM_H

#include "mainformbase.h"
#include <qprocess.h>
#include <qobject.h>

class MainForm : public MainFormBase
{
    Q_OBJECT

public:
    MainForm(bool group=false);
    ~MainForm();

private slots:
    void selectPath();
    void go();
    void currentChanged( QListViewItem * );

    void readyReadStdout();
    void readyReadStderr();
    void processExited();

private:
    void start( const QStringList& args );
    void startChanges( QString label );
    void parseDescribe( const QString& );
    void setDescFilesDiff( const QString&, const QString&, const QString& );

    bool grouped;
    QProcess process;
    QList<int> *changeListFrom;
    QList<int> *changeListTo;
    QMap<int,QString> *changeDateTo;
    QMap<int,QString> *changeWhoTo;
};

#endif // MAINFORM_H
