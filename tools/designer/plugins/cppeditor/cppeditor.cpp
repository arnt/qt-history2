 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "cppeditor.h"
#include "syntaxhighliter_cpp.h"
#include "indent_cpp.h"
#include "cppcompletion.h"
#include <qsettings.h>

CppEditor::CppEditor( const QString &fn, QWidget *parent, const char *name )
    : Editor( fn, parent, name )
{
    document()->setPreProcessor( new SyntaxHighlighter_CPP );
    document()->setIndent( new Indent_CPP );
    completion = new CppEditorCompletion( this );
    int i = 0;
    while ( SyntaxHighlighter_CPP::keywords[ i ] != QString::null )
	    completion->addCompletionEntry( SyntaxHighlighter_CPP::keywords[ i++ ], 0 );
    configChanged();
}

void CppEditor::configChanged()
{
    QMap<QString, Config::Style> styles = Config::readStyles( "/Software/Trolltech/CppEditor" );
    config()->styles = styles;
    ( (SyntaxHighlighter_CPP*)document()->preProcessor() )->updateStyles( config()->styles );
    Editor::configChanged();
}
