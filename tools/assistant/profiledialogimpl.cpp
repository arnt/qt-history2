/**********************************************************************
** Copyright (C) 2000-2003 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
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

#include <qlineedit.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>

#include "docuparser.h"
#include "profiledialogimpl.h"
#include "config.h"


ProfileDialog::ProfileDialog( QWidget *parent, QString pN )
    : ProfileDialogBase( parent ), profName( pN )
{
    changed = FALSE;
    modifyIconButton->hide();
    initDialog();
}

ProfileDialog::~ProfileDialog()
{

}

void ProfileDialog::initDialog()
{
    if ( !profName.isEmpty() ) {
	profile = Config::configuration()->loadProfile( profName );
	mode = Modify;
	if ( !profile )
	    return;
	leName->setText( profile->props["name"] );
	iconName = profile->props["applicationicon"];
	setIcon();
	leAboutMenuText->setText( profile->props["aboutmenutext"] );
	leAboutUrl->setText( profile->props["abouturl"] );
	leTitle->setText( profile->props["title"] );

	QStringList docList = profile->docs;
	QListBoxPixmap *item;
	for ( QStringList::Iterator it = docList.begin(); it != docList.end(); ++it ) {
	    //qDebug( "icon: " + profile->icons[ *it ] );
	    item = new QListBoxPixmap( docListView, QPixmap( profile->icons[ *it ] ), *it );
	}
    } else {
	profile = new Profile();
	mode = Add;
    }
    oldProfile = new Profile( profile );
}

void ProfileDialog::setIcon()
{
    /* todo: accept not only pngs, also in open dialog */
    iconButton->setIconSet( QIconSet( QPixmap( iconName, "PNG" ) ) );
}

void ProfileDialog::okClicked()
{
    if ( leName->text().isEmpty() ) {
	QMessageBox::critical( this, tr( "Qt Assistant - Edit Profile" ),
	    tr( "A profile name must be specified!" ) );
	return;
    }
    profile->props["name"] = leName->text();
    profile->props["applicationicon"] = iconName;
    profile->props["aboutmenutext"] = leAboutMenuText->text();
    profile->props["abouturl"] = leAboutUrl->text();
    profile->props["title"] = leTitle->text();

    if ( profile->props["name"] != oldProfile->props["name"] ||
	 profile->props["applicationicon"] != oldProfile->props["applicationicon"] ||
	 profile->props["aboutmenutext"] != oldProfile->props["aboutmenutext"] ||
	 profile->props["abouturl"] != oldProfile->props["abouturl"] ||
	 profile->props["title"] != oldProfile->props["title"] ||
	 profile->docs != oldProfile->docs ) {
	changed = TRUE;
	if ( mode == Modify )
	    Config::configuration()->removeProfile( oldProfile->props["name"] );
	Config::configuration()->saveProfile( profile, TRUE );
	delete oldProfile;
	oldProfile = 0;
    }

    done( Accepted );
}

void ProfileDialog::cancelClicked()
{
    changed = FALSE;
    done( Rejected );
}

void ProfileDialog::setProfileIcon()
{
    iconName = QFileDialog::getOpenFileName( QDir::homeDirPath(), "*", this, "saveProfileAs",
	tr( "Qt Assistant - Choose Profile Icon" ) );
    setIcon();
}

void ProfileDialog::setDocIcon()
{

}

void ProfileDialog::addDocFile()
{
    QFileDialog *fd = new QFileDialog( QDir::homeDirPath(), "xml Files (*.xml)", this );
    fd->setCaption( tr( "Qt Assistant - Add Documentation" ) );

    if ( fd->exec() == QDialog::Accepted ) {
	QString file = fd->selectedFile();
	QFileInfo fi( file );
	if ( !fi.isReadable() ) {
	    QMessageBox::warning( this, tr( "Qt Assistant" ),
		    tr( "File " + file + " is not readable!" ) );
	    return;
	}
	QFile f( file );
	DocuParser handler;
	QXmlInputSource source( f );
	QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
	reader.setErrorHandler( &handler );
	setCursor( waitCursor );
	qApp->processEvents();
	bool ok = reader.parse( source );
	setCursor( arrowCursor );
	f.close();
	if ( !ok ) {
	    QString msg = QString( "In file %1:\n%2" )
		.arg( QFileInfo( file ).absFilePath() )
		.arg( handler.errorProtocol() );
	    QMessageBox::critical( this, tr( "Parse Error" ), tr( msg ) );
	    return;
	}
	QString title = handler.getDocumentationTitle();
	if ( title.isEmpty() )
	    title = file;

	QListBoxPixmap *item = new QListBoxPixmap( docListView, QPixmap( handler.getIconName() ), file );
	profile->addDocFile( file );
	profile->addDocFileTitle( file, handler.getDocumentationTitle() );
	profile->addDocFileIcon( file, handler.getIconName() );
	profile->addDocFileImageDir( file, handler.getImageDir() );
    }
}

void ProfileDialog::removeDocFile()
{
    QStringList::iterator it = profile->docs.begin();

    while ( it != profile->docs.end() ) {
	if ( *it == docListView->currentText() ) {
	    profile->icons.remove( *it );
	    profile->docs.remove( it );
	    break;
	}
	++it;
    }
    docListView->removeItem( docListView->currentItem() );
}

void ProfileDialog::saveProfileInFile()
{
    QString fileName = QFileDialog::getSaveFileName( QDir::homeDirPath(), "*", this, "saveProfileAs",
	tr( "Qt Assistant - Save Profile As" ) );
    if ( !fileName.isEmpty() ) {
	profile->props["name"] = leName->text();
	//profile->props["applicationicon"] = ...
	profile->props["aboutmenutext"] = leAboutMenuText->text();
	profile->props["abouturl"] = leAboutUrl->text();
	profile->props["title"] = leTitle->text();
	profile->save( fileName );
    }
}

void ProfileDialog::checkForChanges()
{

}

bool ProfileDialog::profileChanged() const
{
    return changed;
}

void ProfileDialog::setUrl()
{
    leAboutUrl->setText( QFileDialog::getOpenFileName( QDir::homeDirPath(), "*", this, "saveProfileAs",
	tr( "Qt Assistant - Edit About Url" ) ) );
}
