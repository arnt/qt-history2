#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include "mainwindow.h"

class QWorkspace;
class MDIWindow : public MainWindow
{
    Q_OBJECT
public:
    MDIWindow( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );

public slots:
    void fileNew();

protected:
    QWorkspace *mdi;
};

#endif // MDIWINDOW_H