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

#include "sourcefile.h"
#include <qfile.h>
#include <qtextstream.h>
#include "designerappiface.h"
#include "sourceeditor.h"
#include "metadatabase.h"
#include "../interfaces/languageinterface.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "mainwindow.h"

SourceFile::SourceFile( const QString &fn, bool temp, Project *p )
    : filename( fn ), ed( 0 ), fileNameTemp( temp ), timeStamp( this, fn ), pro( p )
{
    load();
    iface = 0;
    pro->addSourceFile( this );
    MetaDataBase::addEntry( this );
}

SourceFile::~SourceFile()
{
    delete iface;
}

QString SourceFile::text() const
{
    return txt;
}

void SourceFile::setText( const QString &s )
{
    txt = s;
}

bool SourceFile::save()
{
    if ( fileNameTemp )
	return saveAs();
    if ( ed )
	ed->save();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream ts( &f );
    ts << txt;
    timeStamp.update();
    return TRUE;
}

bool SourceFile::saveAs()
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    QMap<QString, QString> extensionFilterMap;
    QString filter;
    if ( iface ) {
	iface->fileFilters( extensionFilterMap );
	for ( QMap<QString,QString>::Iterator it = extensionFilterMap.begin();
	      it != extensionFilterMap.end(); ++it ) {
	    filter += ";;" + *it;
	}
    }

    QString fn = QFileDialog::getSaveFileName( filename, filter );
    if ( fn.isEmpty() )
	return FALSE;
    fileNameTemp = FALSE;
    filename = fn;
    timeStamp.setFileName( filename );
    if ( ed )
	ed->setCaption( tr( "Edit %1" ).arg( filename ) );
    save();
    return TRUE;
}

bool SourceFile::load()
{
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return FALSE;
    QTextStream ts( &f );
    txt = ts.read();
    timeStamp.update();
    return TRUE;
}

DesignerSourceFile *SourceFile::iFace()
{
    if ( !iface )
	iface = new DesignerSourceFileImpl( this );
    return iface;
}

void SourceFile::setEditor( SourceEditor *e )
{
    ed = e;
}

bool SourceFile::isModified() const
{
    if ( !ed )
	return FALSE;
    return ed->isModified();
}

static QMap<QString, int> *extensionCounter;
QString SourceFile::createUnnamedFileName( const QString &extension )
{
    if ( !extensionCounter )
	extensionCounter = new QMap<QString, int>;
    int count = -1;
    QMap<QString, int>::Iterator it;
    if ( ( it = extensionCounter->find( extension ) ) != extensionCounter->end() ) {
	count = *it;
	++count;
	extensionCounter->replace( extension, count );
    } else {
	count = 1;
	extensionCounter->insert( extension, count );
    }
	
    return "unnamed" + QString::number( count ) + "." + extension;
}

void SourceFile::setModified( bool m )
{
    if ( !ed )
	return;
    ed->setModified( m );
}

bool SourceFile::closeEvent()
{
    if ( !isModified() ) {
	// ### remove from project
	return TRUE;
    }

    if ( ed )
	ed->save();

    switch ( QMessageBox::warning( 0, tr( "Save Code" ),
				   tr( "Save changes to '%1'?" ).arg( filename ),
				   tr( "&Yes" ), tr( "&No" ), tr( "&Cancel" ), 0, 2 ) ) {
    case 0: // save
	if ( !save() )
	    return FALSE;
	break;
    case 1: // don't save
	break;
    case 2: // cancel
	return FALSE;
    default:
	break;
    }
    if ( ed ) {
	ed->setModified( FALSE );
	MainWindow::self->setModified( FALSE, ed );
    }
    return TRUE;
}

bool SourceFile::close()
{
    if ( !ed )
	return TRUE;
    ed->close();
}

Project *SourceFile::project() const
{
    return pro;
}

void SourceFile::checkTimeStamp()
{
    if ( timeStamp.isUpToDate() )
	return;
    timeStamp.update();
    if ( QMessageBox::information( 0, tr( "Qt Designer" ),
				   tr( "The file %1 has been changed outside Qt Designer.\n"
				       "Do you want to reload it?" ).arg( filename ),
				   tr( "&Yes" ), tr( "&No" ) ) == 0 ) {
	load();
	if ( ed )
	    ed->editorInterface()->setText( txt );
    }
}
