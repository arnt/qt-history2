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
#include "propertyeditor.h"
#include <qworkspace.h>
#include <stdlib.h>
#include "designerappiface.h"

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

FormFile::FormFile( const QString &fn, bool temp, Project *p, const char *name )
    : filename( fn ), fileNameTemp( temp ), pro( p ), fw( 0 ), ed( 0 ),
      timeStamp( 0, fn + codeExtension() ), codeEdited( FALSE ), pkg( FALSE ),
      cm( FALSE )
{
    fake = qstrcmp( name, "qt_fakewindow" ) == 0;
    pro->addFormFile( this );
    loadCode();
    if ( !temp )
	checkFileName( FALSE );
}

FormFile::~FormFile()
{
    pro->removeFormFile( this );
    if ( formWindow() )
	formWindow()->setFormFile( 0 );
}

void FormFile::setFormWindow( FormWindow *f )
{
    if ( f == fw )
	return;
    if ( fw )
	fw->setFormFile( 0 );
    fw = f;
    if ( fw )
	fw->setFormFile( this );
    parseCode( cod, FALSE );
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
    return cod;
}

bool FormFile::save( bool withMsgBox, bool ignoreModified, bool exportAsPackage )
{
    if ( !formWindow() )
	return TRUE;
    if ( fileNameTemp )
	return saveAs();
    if ( !ignoreModified && !isModified() )
	return TRUE;
    if ( ed )
	ed->save();
#if defined(PACKAGE_SUPPORT)
    else if ( exportAsPackage && !cm && !ignoreModified )
	loadCode();
#else
    Q_UNUSED( exportAsPackage )
#endif

    if ( isModified( WFormWindow ) ) {
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
		if ( f2.open( IO_WriteOnly | IO_Translate ) ) {
		    QCString data( f.size() );
		    f.readBlock( data.data(), f.size() );
		    f2.writeBlock( data );
		} else {
		    QMessageBox::warning( MainWindow::self, "Save", "The file " + codeFile() + " could not be saved" );
		}
	    }
	}
    }

    cm = FALSE;
    if ( isModified( WFormCode ) ) {
	if ( QFile::exists( pro->makeAbsolute( codeFile() ) ) ) {
	    QString fn( pro->makeAbsolute( codeFile() ) );
#if defined(Q_OS_WIN32)
	    fn += ".bak";
#else
	    fn += "~";
#endif
	    QFile f( pro->makeAbsolute( codeFile() ) );
	    if ( f.open( IO_ReadOnly ) ) {
		QFile f2( fn );
		if ( f2.open( IO_WriteOnly | IO_Translate) ) {
		    QCString data( f.size() );
		    f.readBlock( data.data(), f.size() );
		    f2.writeBlock( data );
		} else {
		    QMessageBox::warning( MainWindow::self, "Save", "The file " + codeFile() + " could not be saved" );
		}
	    }
	}
    }

    Resource resource( MainWindow::self );
    resource.setWidget( formWindow() );
    bool formCodeOnly = isModified( WFormCode ) && !isModified( WFormWindow );
    if ( !resource.save( pro->makeAbsolute( filename ), formCodeOnly ) ) {
	MainWindow::self->statusBar()->message( tr( "Failed to save file '%1'.").arg( filename ), 5000 );
	return saveAs();
    }
    MainWindow::self->statusBar()->message( tr( "'%1' saved.").arg( formCodeOnly ? codeFile() : filename ), 3000 );
    timeStamp.update();
    setModified( FALSE );
    return TRUE;
}

bool FormFile::saveAs( bool ignoreModified, bool exportAsPackage )
{
    QString f = pro->makeAbsolute( fileName() );
    if ( fileNameTemp && formWindow() )
	f = pro->makeAbsolute( QString( formWindow()->name() ).lower() + ".ui" );
    bool saved = FALSE;
    if ( ignoreModified ) {
	QString dir = QStringList::split( ':', pro->iFace()->customSetting( "QTSCRIPT_PACKAGES" ) ).first();
	f = QFileInfo( f ).fileName();
	f.prepend( dir + "/" );
    }
    while ( !saved ) {
	QString fn = QFileDialog::getSaveFileName( f,
					       tr( "Qt User-Interface Files (*.ui)" ) + ";;" +
					       tr( "All Files (*)" ), MainWindow::self, 0,
					       tr( "Save Form '%1' As ...").arg( formName() ),
					       &MainWindow::self->lastSaveFilter );
	if ( fn.isEmpty() )
	    return FALSE;
	QFileInfo fi( fn );
	if ( fi.extension() != "ui" )
	    fn += ".ui";
	fileNameTemp = FALSE;
	filename = pro->makeRelative( fn );
	QFileInfo relfi( filename );
	if ( relfi.exists() ) {
	    if ( QMessageBox::warning( MainWindow::self, tr( "File Already Exists" ),
		tr( "The file already exists. Do you wish to overwrite it?" ),
		QMessageBox::Yes,
		QMessageBox::No ) == QMessageBox::Yes ) {
		saved = TRUE;
	    } else {
		filename = f;
	    }

	} else {
	    saved = TRUE;
	}
    }
    if ( !checkFileName( TRUE ) ) {
	filename = f;
	return FALSE;
    }
    pro->setModified( TRUE );
    timeStamp.setFileName( pro->makeAbsolute( codeFile() ) );
    if ( ed && formWindow() )
	ed->setCaption( tr( "Edit %1" ).arg( formWindow()->name() ) );
    setModified( TRUE );
    return save( TRUE, ignoreModified, exportAsPackage );
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

    switch ( QMessageBox::warning( MainWindow::self, tr( "Save Form" ),
				   tr( "Save changes to form '%1'?" ).arg( filename ),
				   tr( "&Yes" ), tr( "&No" ), tr( "&Cancel" ), 0, 2 ) ) {
    case 0: // save
	if ( !save() )
	    return FALSE;
    case 1: // don't save
	loadCode();
	if ( ed )
	    ed->editorInterface()->setText( cod );
	if ( fileNameTemp )
	    pro->removeFormFile( this );
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
	return cm;
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
    emit somethingChanged( this );
    cm = TRUE;
    if ( !editor() )
	return;
    editor()->setModified( m );
}

void FormFile::showFormWindow()
{
    if ( formWindow() ) {
	if ( ( formWindow()->hasFocus() ||
	      MainWindow::self->qWorkspace()->activeWindow() == formWindow() ) &&
	     MainWindow::self->propertyeditor()->formWindow() != formWindow() ) {
	    MainWindow::self->propertyeditor()->setWidget( formWindow()->currentWidget(), formWindow() );
	    MainWindow::self->objectHierarchy()->setFormWindow( formWindow(),
								formWindow()->currentWidget() );
	}
	formWindow()->setFocus();
	return;
    }
    MainWindow::self->openFormWindow( pro->makeAbsolute( filename ), TRUE, this );
}

SourceEditor *FormFile::showEditor()
{
    showFormWindow();
    bool modify = FALSE;
    if ( !hasFormCode() ) {
	createFormCode();
	modify = TRUE;
    }
    SourceEditor *e = MainWindow::self->openSourceEdior();
    if ( modify )
	setModified( TRUE );
    return e;
}

static int ui_counter = 0;
QString FormFile::createUnnamedFileName()
{
    return QString( "unnamed" ) + QString::number( ++ui_counter ) + QString( ".ui" );
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
"** If you wish to add, delete or rename functions respectively slots use\n"
"** Qt Designer which will update this file, preserving your code. Create an\n"
"** init() function in place of a constructor, and a destroy() function in\n"
"** place of a destructor.\n"
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
    if ( pro->isCpp() )
	cod = comment;
    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow() );
    for ( QValueList<MetaDataBase::Function>::Iterator it = functionList.begin(); it != functionList.end(); ++it ) {
	cod += (!cod.isEmpty() ? "\n\n" : "") +
	       iface->createFunctionStart( formWindow()->name(), make_func_pretty((*it).function),
					   (*it).returnType.isEmpty() ?
					   QString( "void" ) :
					   (*it).returnType, (*it).access ) +
	       "\n" + iface->createEmptyFunction();
    }
    parseCode( cod, FALSE );
}

void FormFile::load()
{
    showFormWindow();
    code();
}

bool FormFile::loadCode()
{
    QFile f( pro->makeAbsolute( codeFile() ) );
    if ( !f.open( IO_ReadOnly ) ) {
	cod = "";
	return FALSE;
    }
    QTextStream ts( &f );
    cod = ts.read();
    parseCode( cod, FALSE );
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

void FormFile::parseCode( const QString &txt, bool allowModify )
{
    if ( !formWindow() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    QValueList<LanguageInterface::Function> functions;
    QValueList<MetaDataBase::Function> newFunctions, oldFunctions;
    oldFunctions = MetaDataBase::functionList( formWindow() );
    iface->functions( txt, &functions );
    QMap<QString, QString> funcs;
    for ( QValueList<LanguageInterface::Function>::Iterator it = functions.begin();
	  it != functions.end(); ++it ) {
	bool found = FALSE;
	for ( QValueList<MetaDataBase::Function>::Iterator fit = oldFunctions.begin();
	      fit != oldFunctions.end(); ++fit ) {
	    QString f( (*fit).function );
	    if ( MetaDataBase::normalizeFunction( f ) ==
		 MetaDataBase::normalizeFunction( (*it).name ) ) {
		found = TRUE;
		MetaDataBase::Function function;
		function.function = make_func_pretty( (*it).name );
		function.specifier = (*fit).specifier;
		function.type = (*fit).type;
		if ( !pro->isCpp() )
		    function.access = (*it).access;
		else
		    function.access = (*fit).access;
		function.language = (*fit).language;
		function.returnType = (*it).returnType;
		newFunctions << function;
		funcs.insert( (*it).name, (*it).body );
		oldFunctions.remove( fit );
		break;
	    }
	}
	if ( !found ) {
	    MetaDataBase::Function function;
	    function.function = make_func_pretty( (*it).name );
	    function.specifier = "virtual";
	    function.access = "public";
	    if ( function.function == "init()" || function.function == "destroy()" )
		function.access = "private";
	    function.type = "function";
	    function.language = pro->language();
	    function.returnType = (*it).returnType;
	    newFunctions << function;
	    funcs.insert( (*it).name, (*it).body );
	    if ( allowModify )
		setFormWindowModified( TRUE );
	}
    }

    if ( allowModify && oldFunctions.count() > 0 )
	setFormWindowModified( TRUE );
	
    MetaDataBase::setFunctionList( formWindow(), newFunctions );
}

void FormFile::syncCode()
{
    if ( !editor() )
	return;
    parseCode( editor()->editorInterface()->text(), TRUE );
    cod = editor()->editorInterface()->text();
}

void FormFile::checkTimeStamp()
{
    if ( timeStamp.isUpToDate() )
	return;
    timeStamp.update();
    if ( codeEdited ) {
	if ( QMessageBox::information( MainWindow::self, tr( "Qt Designer" ),
				       tr( "File '%1' has been changed outside Qt Designer.\n"
					   "Do you want to reload it?" ).arg( timeStamp.fileName() ),
				       tr( "&Yes" ), tr( "&No" ) ) == 0 ) {
	    QFile f( timeStamp.fileName() );
	    if ( f.open( IO_ReadOnly ) ) {
		QTextStream ts( &f );
		editor()->editorInterface()->setText( ts.read() );
		editor()->save();
		MainWindow::self->functionsChanged();
	    }
	}
    } else {
	loadCode();
    }
}

void FormFile::addFunctionCode( MetaDataBase::Function function )
{
    if ( pro->isCpp() && !hasFormCode() && !codeEdited )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;

    QValueList<LanguageInterface::Function> funcs;
    iface->functions( cod, &funcs );
    bool hasFunc = FALSE;
    for ( QValueList<LanguageInterface::Function>::Iterator it = funcs.begin();
	  it != funcs.end(); ++it ) {
	if ( MetaDataBase::normalizeFunction( (*it).name ) == MetaDataBase::normalizeFunction( function.function ) ) {
	    hasFunc = TRUE;
	    break;
	}
    }

    if ( !hasFunc ) {
	if ( !codeEdited && !timeStamp.isUpToDate() )
	    loadCode();
	MetaDataBase::MetaInfo mi = MetaDataBase::metaInfo( formWindow() );
	QString cn;
	if ( mi.classNameChanged )
	    cn = mi.className;
	if ( cn.isEmpty() )
	    cn = formWindow()->name();
	QString body = "\n\n" + iface->createFunctionStart( cn,
							    make_func_pretty( function.function ),
							    function.returnType.isEmpty() ?
							    QString( "void" ) :
							    function.returnType, function.access ) +
		       "\n" + iface->createEmptyFunction();
	cod += body;
	if ( codeEdited ) {
	    setModified( TRUE );
	    emit somethingChanged( this );
	}
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
    FormFile* that = (FormFile*) this;
    if ( formWindow() ) {
	that->cachedFormName = formWindow()->name();
	return cachedFormName;
    }
    if ( !cachedFormName.isNull() )
	return cachedFormName;
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
	that->cachedFormName =  className;
    }
    if ( cachedFormName.isEmpty() )
	that->cachedFormName = filename;
    return cachedFormName;
}

void FormFile::formWindowChangedSomehow()
{
    emit somethingChanged( this );
}

bool FormFile::checkFileName( bool allowBreak )
{
    FormFile *ff = pro->findFormFile( filename, this );
    if ( ff )
	QMessageBox::warning( MainWindow::self, tr( "Invalid Filename" ),
			      tr( "The project already contains a form with a\n"
				  "filename of '%1'. Please choose a new filename." ).arg( filename ) );
    while ( ff ) {
	QString fn;
	while ( fn.isEmpty() ) {
	    fn = QFileDialog::getSaveFileName( pro->makeAbsolute( fileName() ),
					       tr( "Qt User-Interface Files (*.ui)" ) + ";;" +
					       tr( "All Files (*)" ), MainWindow::self, 0,
					       tr( "Save Form '%1' As ...").
					       arg( formWindow()->name() ),
					       &MainWindow::self->lastSaveFilter );
	    if ( allowBreak && fn.isEmpty() )
		return FALSE;
	}
	filename = pro->makeRelative( fn );
	ff = pro->findFormFile( filename, this );
     }
    return TRUE;
}

void FormFile::addConnection( const QString &sender, const QString &signal,
			      const QString &receiver, const QString &slot )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	iface->addConnection( sender, signal, receiver, slot, &cod );
    if ( ed )
	ed->editorInterface()->setText( cod );
}

void FormFile::removeConnection( const QString &sender, const QString &signal,
				 const QString &receiver, const QString &slot )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	iface->removeConnection( sender, signal, receiver, slot, &cod );
    if ( ed )
	ed->editorInterface()->setText( cod );
}

#if defined(PACKAGE_SUPPORT)
bool FormFile::isPackage() const
{
    if ( filename[0] == '/' )
	return TRUE;
    return pkg;
}
#endif
