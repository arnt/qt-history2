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
    return res;
}

SourceEditor::SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface )
    : QVBox( parent ), iFace( iface ), lIface( liface ), formWindow( 0 )
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

void SourceEditor::setForm( FormWindow *fw )
{
    save();
    bool changed = FALSE;
    if ( &(*formWindow) != fw ) {
	saveBreakPoints();
	changed = TRUE;
    }
    formWindow = fw;
    setCaption( tr( "Edit %1" ).arg( formWindow->name() ) );
    iFace->setText( sourceOfForm( formWindow, lang, iFace, lIface ) );
    if ( fw->project() )
	iFace->setContext( fw->project()->formList(), fw ->mainContainer() );
    if ( changed )
	iFace->setBreakPoints( MetaDataBase::breakPoints( fw ) );
}

QString SourceEditor::sourceOfForm( FormWindow *fw, const QString &lang, EditorInterface *, LanguageInterface *lIface )
{
    QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( fw );
    QMap<QString, QString> bodies = MetaDataBase::functionBodies( fw );
    QString txt;
    for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	if ( (*it).language != lang )
	    continue;
	QString sl( (*it).slot );
	txt += lIface->createFunctionStart( fw->name(), sl );
	QMap<QString, QString>::Iterator bit = bodies.find( MetaDataBase::normalizeSlot( (*it).slot ) );
	if ( bit != bodies.end() )
	    txt += "\n" + *bit + "\n\n";
	else
	    txt += "\n" + lIface->createEmptyFunction() + "\n\n";
    }
    return txt;
}

void SourceEditor::setFunction( const QString &func )
{
    iFace->scrollTo( lIface->createFunctionStart( formWindow->name(), func ) );
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
	    newSlots << slot;
	    funcs.insert( (*it).name, (*it).body );
	}
    }

    MetaDataBase::setSlotList( formWindow, newSlots );
    MetaDataBase::setFunctionBodies( formWindow, funcs, lang );
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
    iFace->setText( sourceOfForm( formWindow, lang, iFace, lIface ) );
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
