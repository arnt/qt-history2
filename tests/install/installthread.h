/*
** InstallThread
*/

#include <qthread.h>
#include <qlabel.h>
#include <qprogressbar.h>

#ifndef INSTALLTHREAD_H
#define INSTALLTHREAD_H

class SetupWizardImpl;

class InstallThread : public QThread
{
public:
    InstallThread();

    virtual void run();

    void readArchive( QString arcname, QString installPath );
    bool createDir( QString fullPath );

    SetupWizardImpl* GUI;

    int totalRead;

private:
//    QProcess extproc;
};

#endif
