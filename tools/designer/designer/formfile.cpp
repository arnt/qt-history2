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

#include "formfile.h"
#include "timestamp.h"
#include "project.h"
#include "formwindow.h"
#include "command.h"
#include "sourceeditor.h"
#include "mainwindow.h"
#include "../interfaces/languageinterface.h"
#include "resource.h"
#include "workspace.h"
#include <qmessagebox.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstatusbar.h>

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
    res.replace( QRegExp( ":" ), " : " );
    res = res.simplifyWhiteSpace();
    return res;
}

FormFile::FormFile( const QString &fn, bool temp, Project *p )
    : filename( fn ), fileNameTemp( temp ), pro( p ), fw( 0 ), ed( 0 ),
      timeStamp( 0, fn + codeExtension() ), codeEdited( FALSE )
{
    pro->addFormFile( this );
    loadCode();
}

FormFile::~FormFile()
{
    pro->removeFormFile( this );
    if ( formWindow() )
	formWindow()->setFormFile( 0 );
}

void FormFile::setFormWindow( FormWindow *f )
{
    if ( fw )
	fw->setFormFile( 0 );
    fw = f;
    if ( fw )
	fw->setFormFile( this );
    parseCode( cod );
}

void FormFile::setEditor( SourceEditor *e )
{
    ed = e;
}

void FormFile::setFileName( const QString &fn )
{
    if ( fn == filename )
	return;
    if ( fn.isEmpty() ) {
	fileNameTemp = TRUE;
	if ( filename.find( "unnamed" ) != 0 )
	    filename = createUnnamedFileName();
	return;
    }
    filename = fn;
    timeStamp.setFileName( filename + codeExtension() );
    cod = "";
    loadCode();
}

void FormFile::setCode( const QString &c )
{
    cod = c;
    parseCode( cod );
}

FormWindow *FormFile::formWindow() const
{
    return fw;
}

SourceEditor *FormFile::editor() const
{
    return ed;
}

QString FormFile::fileName() const
{
    return filename;
}

QString FormFile::absFileName() const
{
    return pro->makeAbsolute( filename );
}

QString FormFile::codeFile() const
{
    return filename + codeExtension();
}

QString FormFile::code()
{
    bool createSource = TRUE;
    bool setSource = FALSE;
    QString txt;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface && iface->supports( LanguageInterface::StoreFormCodeSeperate ) ) {
	createSource = FALSE;
	txt = cod;
	if ( txt.isEmpty() ) {
	    createSource = TRUE;
	    setSource = TRUE;
	}
    }
    if ( createSource ) {
	QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( formWindow() );
	QMap<QString, QString> bodies = MetaDataBase::functionBodies( formWindow() );
	for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	    if ( (*it).language != pro->language() )
		continue;
	    QString sl( (*it).slot );
	    QString comments = MetaDataBase::functionComments( formWindow(), sl );
	    if ( !comments.isEmpty() )
		txt += comments + "\n";
	    txt += iface->createFunctionStart( formWindow()->name(), make_func_pretty( sl ),
					       ( (*it).returnType.isEmpty() ?
						 QString( "void" ) :
						 (*it).returnType ) );
	    QMap<QString, QString>::Iterator bit = bodies.find( MetaDataBase::normalizeSlot( (*it).slot ) );
	    if ( bit != bodies.end() )
		txt += "\n" + *bit + "\n\n";
	    else
		txt += "\n" + iface->createEmptyFunction() + "\n\n";
	}
	if ( setSource )
	    cod = txt;
    }
    return txt;
}

bool FormFile::load()
{
    return FALSE;
}

bool FormFile::save( bool withMsgBox )
{
    if ( !formWindow() )
	return TRUE;
    if ( fileNameTemp )
	return saveAs();
    if ( !isModified() )
	return TRUE;
    if ( ed )
	ed->save();

    if ( withMsgBox ) {
	if ( !formWindow()->checkCustomWidgets() )
	    return FALSE;
    }

    if ( QFile::exists( pro->makeAbsolute( filename ) ) ) {
	QString fn( pro->makeAbsolute( filename ) );
#if defined(Q_OS_WIN32)
	fn += ".bak";
#else
	fn += "~";
#endif
	QFile f( pro->makeAbsolute( filename ) );
	if ( f.open( IO_ReadOnly ) ) {
	    QFile f2( fn );
	    if ( f2.open( IO_WriteOnly ) ) {
		QCString data( f.size() );
		f.readBlock( data.data(), f.size() );
		f2.writeBlock( data );
	    }
	}
    }

    Resource resource( MainWindow::self );
    resource.setWidget( formWindow() );
    if ( !resource.save( pro->makeAbsolute( filename ) ) ) {
	MainWindow::self->statusBar()->message( tr( "Failed to save file %1.").arg( filename ), 5000 );
	if ( withMsgBox )
	    QMessageBox::warning( MainWindow::self, tr( "Save" ), tr( "Couldn't save file %1" ).arg( filename ) );
	return FALSE;
    }
    MainWindow::self->statusBar()->message( tr( "%1 saved.").arg( filename ), 3000 );
    timeStamp.update();
    setModified( FALSE );
    return TRUE;
}

bool FormFile::saveAs()
{
    QString fn = QFileDialog::getSaveFileName( pro->makeAbsolute( filename ),
					       tr( "Qt User-Interface Files (*.ui)" ) + ";;" +
					       tr( "All Files (*)" ), MainWindow::self, 0,
					       tr( "Save form '%1' as ....").arg( name() ),
					       &MainWindow::self->lastSaveFilter );
    if ( fn.isEmpty() )
	return FALSE;
    QFileInfo fi( fn );
    if ( fi.extension() != "ui" )
	fn += ".ui";
    fileNameTemp = FALSE;
    filename = pro->makeRelative( fn );
    pro->setModified( TRUE );
    timeStamp.setFileName( pro->makeAbsolute( codeFile() ) );
    if ( ed )
	ed->setCaption( tr( "Edit %1" ).arg( filename ) );
    setModified( TRUE );
    return save();
}

bool FormFile::close()
{
    if ( editor() ) {
	editor()->save();
	editor()->close();
    }
    if ( formWindow() )
	return formWindow()->close();
    return TRUE;
}

bool FormFile::closeEvent()
{
    if ( !isModified() && fileNameTemp ) {
	pro->removeFormFile( this );
	return TRUE;
    }

    if ( !isModified() )
	return TRUE;

    if ( editor() )
	editor()->save();

    switch ( QMessageBox::warning( 0, tr( "Save Form" ),
				   tr( "Save changes to the form '%1'?" ).arg( filename ),
				   tr( "&Yes" ), tr( "&No" ), tr( "&Cancel" ), 0, 2 ) ) {
    case 0: // save
	if ( !save() )
	    return FALSE;
    case 1: // don't save
	loadCode();
	if ( ed )
	    ed->editorInterface()->setText( cod );
	MainWindow::self->workspace()->update();
	break;
    case 2: // cancel
	return FALSE;
    default:
	break;
    }

    setModified( FALSE );
    MainWindow::self->updateFunctionList();
    setCodeEdited( FALSE );
    return TRUE;
}

void FormFile::setModified( bool m, int who )
{
    if ( ( who & WFormWindow ) == WFormWindow )
	setFormWindowModified( m );
    if ( ( who & WFormCode ) == WFormCode )
	setCodeModified( m );
}

bool FormFile::isModified( int who )
{
    if ( who == WFormWindow )
	return isFormWindowModified();
    if ( who == WFormCode )
	return isCodeModified();
    return isCodeModified() || isFormWindowModified();
}

bool FormFile::isFormWindowModified() const
{
    if ( !formWindow()  || !formWindow()->commandHistory() )
	return FALSE;
    return formWindow()->commandHistory()->isModified();
}

bool FormFile::isCodeModified() const
{
    if ( !editor() )
	return FALSE;
    return editor()->isModified();
}

void FormFile::setFormWindowModified( bool m )
{
    bool b = isFormWindowModified();
    if ( m == b )
	return;
    if ( !formWindow() || !formWindow()->commandHistory() )
	return;
    formWindow()->commandHistory()->setModified( m );
    emit somethingChanged( this );
}

void FormFile::setCodeModified( bool m )
{
    bool b = isCodeModified();
    if ( m == b )
	return;
    if ( !editor() )
	return;
    editor()->setModified( m );
    emit somethingChanged( this );
}

void FormFile::showFormWindow()
{
    if ( formWindow() ) {
	formWindow()->setFocus();
	return;
    }
    MainWindow::self->openFormWindow( pro->makeAbsolute( filename ), TRUE, this );
}

void FormFile::showEditor()
{
    showFormWindow();
    if ( !hasFormCode() )
	createFormCode();
    MainWindow::self->editSource();
}

static int count = 0;
QString FormFile::createUnnamedFileName()
{
    return QString( "unnamed" ) + QString::number( ++count ) + QString( ".ui" );
}

QString FormFile::codeExtension() const
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	return iface->formCodeExtension();
    return "";
}

static const char * const comment =
"/****************************************************************************\n"
"** ui.h extension file, included from the uic-generated form implementation.\n"
"**\n"
"** If you wish to add, delete or rename slots use Qt Designer which will\n"
"** update this file, preserving your code. Create an init() slot in place of\n"
"** a constructor, and a destroy() slot in place of a destructor.\n"
"*****************************************************************************/\n";


bool FormFile::hasFormCode() const
{
    return !cod.isEmpty() && cod != QString( comment );
}

void FormFile::createFormCode()
{
    if ( !formWindow() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    cod = comment;
    QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( formWindow() );
    for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	cod += "\n\n" + iface->createFunctionStart( formWindow()->name(), (*it).slot,
						    (*it).returnType.isEmpty() ?
						    QString( "void" ) :
						    (*it).returnType ) +
	       "\n" + iface->createEmptyFunction();
    }
    parseCode( cod );
}

bool FormFile::loadCode()
{
    QFile f( pro->makeAbsolute( codeFile() ) );
    if ( !f.open( IO_ReadOnly ) )
	return FALSE;
    QTextStream ts( &f );
    cod = ts.read();
    parseCode( cod );
    timeStamp.update();
    return TRUE;
}

bool FormFile::isCodeEdited() const
{
    return codeEdited;
}

void FormFile::setCodeEdited( bool b )
{
    codeEdited = b;
}

void FormFile::parseCode( const QString &txt )
{
    if ( !formWindow() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    QValueList<LanguageInterface::Function> functions;
    QValueList<MetaDataBase::Slot> newSlots, oldSlots;
    oldSlots = MetaDataBase::slotList( formWindow() );
    iface->functions( txt, &functions );
    QMap<QString, QString> funcs;
    for ( QValueList<LanguageInterface::Function>::Iterator it = functions.begin();
	  it != functions.end(); ++it ) {
	bool found = FALSE;
	for ( QValueList<MetaDataBase::Slot>::Iterator sit = oldSlots.begin(); sit != oldSlots.end(); ++sit ) {
	    QString s( (*sit).slot );
	    if ( MetaDataBase::normalizeSlot( s ) == MetaDataBase::normalizeSlot( (*it).name ) ) {
		found = TRUE;
		MetaDataBase::Slot slot;
		slot.slot = make_func_pretty( (*it).name );
		slot.specifier = (*sit).specifier;
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
	    slot.specifier = "virtual";
	    slot.access = "public";
	    slot.language = pro->language();
	    slot.returnType = (*it).returnType;
	    newSlots << slot;
	    funcs.insert( (*it).name, (*it).body );
	}
	MetaDataBase::setFunctionComments( formWindow(), (*it).name, (*it).comments );
    }

    MetaDataBase::setSlotList( formWindow(), newSlots );
    MetaDataBase::setFunctionBodies( formWindow(), funcs, pro->language(), QString::null );
}

void FormFile::syncCode()
{
    if ( !editor() )
	return;
    parseCode( editor()->editorInterface()->text() );
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface && iface->supports( LanguageInterface::StoreFormCodeSeperate ) )
	cod = editor()->editorInterface()->text();
}

void FormFile::checkTimeStamp()
{
    if ( timeStamp.isUpToDate() )
	return;
    timeStamp.update();
    if ( QMessageBox::information( 0, tr( "Qt Designer" ),
				   tr( "The file %1 has been changed outside Qt Designer.\n"
				       "Do you want to reload it?" ).arg( timeStamp.fileName() ),
				   tr( "&Yes" ), tr( "&No" ) ) == 0 ) {
	QFile f( timeStamp.fileName() );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    editor()->editorInterface()->setText( ts.read() );
	    editor()->save();
	    MainWindow::self->slotsChanged();
	}
    }
}

void FormFile::addSlotCode( MetaDataBase::Slot slot )
{
    if ( !hasFormCode() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    QMap<QString, QString> functionBodies = MetaDataBase::functionBodies( formWindow() );
    QMap<QString, QString>::Iterator it = functionBodies.find( MetaDataBase::normalizeSlot( slot.slot ) );
    if ( it == functionBodies.end() ) {
	if ( codeEdited && !timeStamp.isUpToDate() )
	    loadCode();
	QString body = "\n\n" + iface->createFunctionStart( formWindow()->name(),
							    make_func_pretty( slot.slot ),
							    slot.returnType.isEmpty() ?
							    QString( "void" ) :
							    slot.returnType ) +
		       "\n" + iface->createEmptyFunction();
	cod += body;
	functionBodies.insert( MetaDataBase::normalizeSlot( slot.slot ), iface->createEmptyFunction() );
	MetaDataBase::setFunctionBodies( formWindow(), functionBodies, pro->language(), slot.returnType );
    }
}

void FormFile::functionNameChanged( const QString &oldName, const QString &newName )
{
    if ( !cod.isEmpty() ) {
	QString funcStart = QString( formWindow()->name() ) + QString( "::" );
	int i = cod.find( funcStart + oldName );
	if ( i != -1 ) {
	    cod.remove( i + funcStart.length(), oldName.length() );
	    cod.insert( i + funcStart.length(), newName );
	}
    }
}

QString FormFile::formName() const
{
    if ( formWindow() )
	return formWindow()->name();
    QFile f( pro->makeAbsolute( filename ) );
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	QString line;
	QString className;
	while ( !ts.eof() ) {
	    line = ts.readLine();
	    if ( !className.isEmpty() ) {
		int end = line.find( "</class>" );
		if ( end == -1 ) {
		    className += line;
		} else {
		    className += line.left( end );
		    break;
		}
		continue;
	    }
	    int start;
	    if ( ( start = line.find( "<class>" ) ) != -1 ) {
		int end = line.find( "</class>" );
		if ( end == -1 ) {
		    className = line.mid( start + 7 );
		} else {
		    className = line.mid( start + 7, end - ( start + 7 ) );
		    break;
		}
	    }
	}
	if ( !className.isEmpty() )
	    return className;
    }
    return filename;
}

void FormFile::formWindowChangedSomehow()
{
    emit somethingChanged( this );
}

