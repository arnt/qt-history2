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

#include "sourceeditor.h"

#include "formwindow.h"
#include "metadatabase.h"
#include "project.h"
#include "mainwindow.h"
#include "../interfaces/languageinterface.h"
#include <qregexp.h>
#include "project.h"
#include "sourcefile.h"

static QString make_func_pretty( const QString &s )
{
    QString res = s;
    if ( res.find( ")" ) - res.find( "(" ) == 1 )
	return res;
    res.replace( QRegExp( "[(]" ), "( " );
    res.replace( QRegExp( "[)]" ), " )" );
    res.replace( QRegExp( "&" ), " &" );
    res.replace( QRegExp( "[*]" ), " *" );
    res.replace( QRegExp( "," ), ", " );
    res = res.simplifyWhiteSpace();
    return res;
}

SourceEditor::SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface )
    : QVBox( parent ), iFace( iface ), lIface( liface ), formWindow( 0 ), pro( 0 )
{
    iFace->addRef();
    lIface->addRef();
    editor = iFace->editor( this, MainWindow::self->designerInterface() );
    iFace->onBreakPointChange( MainWindow::self, SLOT( breakPointsChanged() ) );
    resize( 600, 400 );
}

SourceEditor::~SourceEditor()
{
    saveBreakPoints();
    iFace->release();
    lIface->release();
    MainWindow::self->editorClosed( this );
    editor = 0;
}

void SourceEditor::setObject( QObject *fw, Project *p )
{
    save();
    bool changed = FALSE;
    if ( &(*formWindow) != fw ) {
	saveBreakPoints();
	changed = TRUE;
    }
    formWindow = fw;
    pro = p;
    setCaption( tr( "Edit %1" ).arg( formWindow->inherits( "FormWindow" ) ? QString( formWindow->name() ) : ( (SourceFile*)fw )->fileName() ) );
    iFace->setText( sourceOfObject( formWindow, lang, iFace, lIface ) );
    if ( pro && fw->inherits( "FormWindow" ) )
	iFace->setContext( pro->formList(), ( (FormWindow*)fw ) ->mainContainer() );
    if ( changed || fw->inherits( "SourceFile" ) ) // #### ?
	iFace->setBreakPoints( MetaDataBase::breakPoints( fw ) );
}

QString SourceEditor::sourceOfObject( QObject *fw, const QString &lang, EditorInterface *, LanguageInterface *lIface )
{
    QString txt;
    if ( fw->inherits( "FormWindow" ) ) {
	QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( fw );
	QMap<QString, QString> bodies = MetaDataBase::functionBodies( fw );
	for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	    if ( (*it).language != lang )
		continue;
	    QString sl( (*it).slot );
	    txt += lIface->createFunctionStart( fw->name(), sl, ( (*it).returnType.isEmpty() ? QString( "void" ) : (*it).returnType ) );
	    QMap<QString, QString>::Iterator bit = bodies.find( MetaDataBase::normalizeSlot( (*it).slot ) );
	    if ( bit != bodies.end() )
		txt += "\n" + *bit + "\n\n";
	    else
		txt += "\n" + lIface->createEmptyFunction() + "\n\n";
	}
    } else if ( fw->inherits( "SourceFile" ) ) {
	txt = ( (SourceFile*)fw )->text();
    }
    return txt;
}

void SourceEditor::setFunction( const QString &func )
{
    iFace->scrollTo( lIface->createFunctionStart( formWindow->name(), func, "" ) );
}

void SourceEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void SourceEditor::save()
{
    if ( !formWindow )
	return;
    if ( formWindow->inherits( "FormWindow" ) ) {
	QValueList<LanguageInterface::Function> functions;
	QValueList<MetaDataBase::Slot> newSlots, oldSlots;
	oldSlots = MetaDataBase::slotList( formWindow );
	lIface->functions( iFace->text(), &functions );
	QMap<QString, QString> funcs;
	for ( QValueList<LanguageInterface::Function>::Iterator it = functions.begin(); it != functions.end(); ++it ) {
	    bool found = FALSE;
	    for ( QValueList<MetaDataBase::Slot>::Iterator sit = oldSlots.begin(); sit != oldSlots.end(); ++sit ) {
		QString s( (*sit).slot );
		if ( MetaDataBase::normalizeSlot( s ) == MetaDataBase::normalizeSlot( (*it).name ) ) {
		    found = TRUE;
		    MetaDataBase::Slot slot;
		    slot.slot = make_func_pretty( (*it).name );
		    slot.access = (*sit).access;
		    slot.language = (*sit).language;
		    slot.returnType = (*it).returnType;
		    newSlots << slot;
		    funcs.insert( (*it).name, (*it).body );
		    oldSlots.remove( sit );
		    break;
		}
	    }
	    if ( !found ) {
		MetaDataBase::Slot slot;
		slot.slot = make_func_pretty( (*it).name );
		slot.access = "public";
		slot.language = lang;
		slot.returnType = (*it).returnType;
		newSlots << slot;
		funcs.insert( (*it).name, (*it).body );
	    }
	}

	MetaDataBase::setSlotList( formWindow, newSlots );
	MetaDataBase::setFunctionBodies( formWindow, funcs, lang, QString::null );
    } else if ( formWindow->inherits( "SourceFile" ) ) {
	( (SourceFile*)(QObject*)formWindow )->setText( iFace->text() );
    }
}

QString SourceEditor::language() const
{
    return lang;
}

void SourceEditor::setLanguage( const QString &l )
{
    lang = l;
}

void SourceEditor::editCut()
{
    iFace->cut();
}

void SourceEditor::editCopy()
{
    iFace->copy();
}

void SourceEditor::editPaste()
{
    iFace->paste();
}

void SourceEditor::editUndo()
{
    iFace->undo();
}

void SourceEditor::editRedo()
{
    iFace->redo();
}

void SourceEditor::editSelectAll()
{
    iFace->selectAll();
}

void SourceEditor::configChanged()
{
    iFace->readSettings();
}

void SourceEditor::setModified( bool b )
{
    iFace->setModified( b );
}

void SourceEditor::refresh()
{
    iFace->setText( sourceOfObject( formWindow, lang, iFace, lIface ) );
}

void SourceEditor::setFocus()
{
    if ( editor )
	editor->setFocus();
}

int SourceEditor::numLines() const
{
    return iFace->numLines();
}

void SourceEditor::saveBreakPoints()
{
    if ( !formWindow )
	return;
    QValueList<int> l;
    iFace->breakPoints( l );
    MetaDataBase::setBreakPoints( formWindow, l );
}

void SourceEditor::clearStep()
{
    iFace->clearStep();
}
