#include "cppcompletion.h"

CppEditorCompletion::CppEditorCompletion( Editor *e )
    : EditorCompletion( e )
{
}

bool CppEditorCompletion::doObjectCompletion( const QString & )
{
    return FALSE;
}

QStringList CppEditorCompletion::functionParameters( const QString &, QChar & )
{
    return QStringList();
}

void CppEditorCompletion::setContext( QObjectList *, QObject * )
{
}
