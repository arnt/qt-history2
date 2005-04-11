
#include "ideapplication.h"

// components
#include <formeditor.h>

Q_GLOBAL_STATIC(FormEditor, g_formEditor)

IDEApplication::IDEApplication(int &argc, char *argv[])
    : QApplication(argc, argv)
{
}

IDEApplication::~IDEApplication()
{
}

QDesignerFormEditorInterface *IDEApplication::core()
{
    return g_formEditor();
}

