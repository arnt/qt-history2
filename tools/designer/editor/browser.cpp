#include "browser.h"
#include "editor.h"

EditorBrowser::EditorBrowser( Editor *e )
    : curEditor( e )
{
}

bool EditorBrowser::eventFilter( QObject *o, QEvent *e )
{
    return FALSE;
}

void EditorBrowser::setCurrentEdior( Editor *e )
{
    curEditor = e;
    curEditor->installEventFilter( this );
}

void EditorBrowser::addEditor( Editor *e )
{
    e->installEventFilter( this );
}
