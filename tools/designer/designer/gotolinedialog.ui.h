/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

void GotoLineDialog::init()
{
    editor = 0;
}

void GotoLineDialog::destroy()
{
    if ( editor )
	editor->release();
}

void GotoLineDialog::gotoLine()
{
    if ( editor )
	editor->gotoLine( spinLine->value() - 1 );
    accept();
}

void GotoLineDialog::setEditor( EditorInterface *e )
{
    editor = e;
    editor->addRef();
}

