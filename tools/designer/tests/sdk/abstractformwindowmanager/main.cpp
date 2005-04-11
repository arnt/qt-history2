
#include "tst_abstractformwindowmanager.h"

#include <ideapplication.h>

int main(int argc, char *argv[])
{
    tst_QDesignerFormWindowManagerInterface tc(argc, argv);

    IDEApplication app(argc, argv);
    Q_UNUSED(app);

    return tc.exec();
}
