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

FormFile::FormFile( const QString &fn, bool temp, Project *p )
    : filename( fn ), fileNameTemp( temp ), pro( p ), fw( 0 ), ed( 0 ),
      timeStamp( 0, fn + codeExtension() ), hFormCode( FALSE )
{
    pro->addFormFile( this );
    loadCode();
}

void FormFile::setFormWindow( FormWindow *f )
{
    fw = f;
    if ( fw )
	fw->setFormFile( this );
}

void FormFile::setEditor( SourceEditor *e )
{
    ed = e;
}

void FormFile::setFileName( const QString &fn )
{
    filename = fn;
    timeStamp.setFileName( filename + codeExtension() );
    cod = "";
    loadCode();
}

void FormFile::setCode( const QString &c )
{
    cod = c;
    hFormCode = TRUE;
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

QString FormFile::codeFile() const
{
    return filename + codeExtension();
}

QString FormFile::code() const
{
    return cod;
}

bool FormFile::load()
{
    return FALSE;
}

bool FormFile::save()
{
    return FALSE;
}

bool FormFile::saveAs()
{
    return FALSE;
}

bool FormFile::close()
{
    return FALSE;
}

bool FormFile::closeEvent()
{
    return FALSE;
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
    if ( !formWindow() )
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
    if ( !formWindow() )
	return;
    formWindow()->commandHistory()->setModified( m );
}

void FormFile::setCodeModified( bool m )
{
    if ( !editor() )
	return;
    editor()->setModified( m );
}

void FormFile::showFormWindow()
{
    if ( formWindow() ) {
	formWindow()->setFocus();
	return;
    }
    FormWindow *fw = MainWindow::self->openFormWindow( filename );
    if ( fw )
	setFormWindow( fw );
}

void FormFile::showEditor()
{
    showFormWindow();
}

QString FormFile::createUnnamedFileName()
{
    static int count = 0;
    ++count;
    return "unnamed" + QString::number( count ) + ".ui";
}

QString FormFile::codeExtension() const
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	return iface->formCodeExtension();
    return "";
}

bool FormFile::hasFormCode() const
{
    return hFormCode;
}

void FormFile::createFormCode()
{
    hFormCode = TRUE;
    cod =
	"/****************************************************************************\n"
	"** ui.h extension file, included from the uic-generated form implementation.\n"
	"**\n"
	"** If you wish to add, delete or rename slots use Qt Designer which will\n"
	"** update this file, preserving your code. Create an init() slot in place of\n"
	"** a constructor, and a destroy() slot in place of a destructor.\n"
	"*****************************************************************************/\n";
}

bool FormFile::loadCode()
{
    QFile f( pro->makeAbsolute( codeFile() ) );
    if ( !f.open( IO_ReadOnly ) ) {
	hFormCode = FALSE;
	return FALSE;
    }
    hFormCode = TRUE;
    QTextStream ts( &f );
    cod = ts.read();
    timeStamp.update();
    return TRUE;
}
