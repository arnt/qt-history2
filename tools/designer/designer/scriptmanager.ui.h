/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qtextedit.h>
#if defined(APP_SCRIPTING)
#include "../../../../quick/src/qfa/qfa.h"
#endif

void ScriptManager::init()
{
#if defined(APP_SCRIPTING)
    editorGroup->setColumnLayout( 1, Qt::Vertical );
    QuickEngine e;
    e.createEditor( editorGroup, &editor );
#endif
}


void ScriptManager::buttonPlayClicked()
{
#if defined(APP_SCRIPTING)
    QuickEngine e;
    e.exec( editor->text() );
#endif
}

void ScriptManager::buttonNewClicked()
{
#if defined(APP_SCRIPTING)
    editor->clear();
#endif
}
