#ifndef MAIN_H
#define MAIN_H

#include <qapplication.h>
#include "workspace.h"

class Application : public  QApplication
{
public:
    Application( int &argc, char **argv );
    ~Application();

protected:
    bool x11EventFilter( XEvent * );

private:
    Workspace* workspace; // will be list of workspaces for MDI support
};



#endif
