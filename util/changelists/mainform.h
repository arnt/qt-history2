#ifndef MAINFORM_H
#define MAINFORM_H

#include "mainformbase.h"
#include <qprocess.h>
#include <qobject.h>

class MainForm : public MainFormBase
{
    Q_OBJECT

public:
    MainForm();
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

    QProcess process;
    QValueList<int> *changeListFrom;
    QValueList<int> *changeListTo;
    bool incIntegrates;
#if !defined(USE_READLINE)
    QString changesTmp;
#endif
};

#endif // MAINFORM_H
