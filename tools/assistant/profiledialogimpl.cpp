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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
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
#include <qlabel.h>

#include "docuparser.h"
#include "profiledialogimpl.h"
#include "config.h"

class IconPreview : public QLabel, public QFilePreview
{
public:
    IconPreview( QWidget *parent=0 ) : QLabel( parent )
    {
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setBackgroundColor( QColor( "white" ) );
	setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    }
    void previewUrl( const QUrl &u )
    {
	QString path = u.path();
	QPixmap pix( path );
	if ( !pix.isNull() )
	    setPixmap( pix );
	else
	    setText( "" );
    }
};

ProfileDialog::ProfileDialog( QWidget *parent, QString pN )
    : ProfileDialogBase( parent ), profName( pN )
{
    changed = FALSE;
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
	insertProfileData();
    } else {
	profile = new Profile();
	mode = Add;
    }
    if ( lePath->text().isEmpty() )
	lePath->setText( QDir::homeDirPath() );
    oldProfile = new Profile( profile );
}

void ProfileDialog::insertProfileData()
{
    leName->setText( profile->props["name"] );
    leIcon->setText( profile->props["applicationicon"] );
    leAboutMenuText->setText( profile->props["aboutmenutext"] );
    leAboutUrl->setText( profile->props["abouturl"] );
    leTitle->setText( profile->props["title"] );
    lePath->setText( profile->props["basepath"] );
    leHome->setText( profile->props["startpage"] );

    QStringList docList = profile->docs;
    QListBoxPixmap *item;
    for ( QStringList::Iterator it = docList.begin(); it != docList.end(); ++it ) {
	QPixmap pix( iconAbsFilePath( *it, profile->icons[*it] ) );
	if ( pix.width() > 22 )
	    pix.resize( 22, 22 );
	item = new QListBoxPixmap( docListView, pix, profile->titles[*it] );
    }
}

void ProfileDialog::okClicked()
{
    if ( leName->text().isEmpty() ) {
	QMessageBox::critical( this, tr( "Qt Assistant - Edit Profile" ),
	    tr( "A profile name must be specified!" ) );
	return;
    }
    profile->props["name"] = leName->text();
    profile->props["applicationicon"] = leIcon->text();
    profile->props["aboutmenutext"] = leAboutMenuText->text();
    profile->props["abouturl"] = leAboutUrl->text();
    profile->props["basepath"] = lePath->text();
    profile->props["title"] = leTitle->text();
    profile->props["startpage"] = leHome->text();

    if ( profile->props["name"] != oldProfile->props["name"] ||
	 profile->props["applicationicon"] != oldProfile->props["applicationicon"] ||
	 profile->props["aboutmenutext"] != oldProfile->props["aboutmenutext"] ||
	 profile->props["abouturl"] != oldProfile->props["abouturl"] ||
	 profile->props["title"] != oldProfile->props["title"] ||
	 profile->props["startpage"] != oldProfile->props["startpage"] ||
	 profile->docs != oldProfile->docs ) {
	changed = TRUE;
	if ( mode == Modify )
	    Config::configuration()->removeProfile( oldProfile->props["name"] );
	if ( removedDocFiles.count() ) {
	    QValueListConstIterator<QString> it = removedDocFiles.begin();
	    for ( ; it != removedDocFiles.end(); ++it )
		profile->removeDocFileEntry( *it );
	    removedDocFiles.clear();
	}
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

void ProfileDialog::chooseProfileIcon()
{
    IconPreview* ip = new IconPreview;

    QFileDialog fd( this );
    fd.setCaption( tr( "Choose an Application Icon" ) );
    fd.setFilter( "Images (*.png *.xpm *.jpg)" );
    fd.setDir( QDir::homeDirPath() );
    fd.setContentsPreviewEnabled( TRUE );
    fd.setContentsPreview( ip, ip );
    fd.setPreviewMode( QFileDialog::Contents );
    if ( fd.exec()== QDialog::Accepted ) {
	QString sf = fd.selectedFile();
	if ( !sf.isEmpty() )
	    leIcon->setText( sf );
    }
    delete ip;
}

void ProfileDialog::addDocFile()
{
    QFileDialog *fd = new QFileDialog( QDir::homeDirPath(),
	"*.xml", this );
    fd->addFilter( "Documentation Content Files *.dcf" );
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

	QPixmap pix( iconAbsFilePath( file, handler.getIconName() ) );
	if ( pix.width() > 22 )
	    pix.resize( 22, 22 );
	QListBoxPixmap *item = new QListBoxPixmap( docListView,
			       pix, handler.getDocumentationTitle() );
	profile->addDocFile( file );
	profile->addDocFileTitle( file, handler.getDocumentationTitle() );
	profile->addDocFileIcon( file, handler.getIconName() );
	profile->addDocFileImageDir( file, handler.getImageDir() );
    }
}

QString ProfileDialog::iconAbsFilePath( const QString &docFileName, const QString &iconName )
{
    QFileInfo iconInfo( iconName );
    QString iconFile = iconName;
    if ( iconInfo.isRelative() ) {
	QFileInfo docInfo( docFileName );
	QString basePath = docInfo.dirPath( TRUE );
	QFileInfo buf( basePath + "/" + iconName );
	iconFile = buf.absFilePath();
    }
    return iconFile;
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
    removedDocFiles << docListView->currentText();
    docListView->removeItem( docListView->currentItem() );
}

void ProfileDialog::saveProfileInFile()
{
    QString fileName = QFileDialog::getSaveFileName( QDir::homeDirPath(),
	"Assistant Profiles (*.adp)", this, "saveProfileAs",
	tr( "Qt Assistant - Save Profile As" ) );
    if ( !fileName.isEmpty() ) {
	if ( !fileName.endsWith( ".adp" ) )
	    fileName += ".adp";
	if ( removedDocFiles.count() ) {
	    QValueListConstIterator<QString> it = removedDocFiles.begin();
	    for ( ; it != removedDocFiles.end(); ++it )
		profile->removeDocFileEntry( *it );
	    removedDocFiles.clear();
	}
	profile->props["name"] = leName->text();
	profile->props["aboutmenutext"] = leAboutMenuText->text();
	profile->props["title"] = leTitle->text();
	profile->props["applicationicon"] = leIcon->text();
	profile->props["abouturl"] = leAboutUrl->text();
	profile->props["basepath"] = lePath->text();
	profile->props["startpage"] = leHome->text();
	profile->save( fileName );
    }
}

bool ProfileDialog::profileChanged() const
{
    return changed;
}

void ProfileDialog::setUrl()
{
    QString file = QFileDialog::getOpenFileName( QDir::homeDirPath(), "*", this, "setUrl",
	    tr( "Qt Assistant - Edit About Url" ) );
    if ( !file.isNull() )
	leAboutUrl->setText( file );
}

void ProfileDialog::setPath()
{
    QString dir = QFileDialog::getExistingDirectory( QDir::homeDirPath(), this, "setPath",
	    tr( "Qt Assistant - Edit Base Path" ) );
    if ( !dir.isNull() )
	lePath->setText( dir );
}

void ProfileDialog::setHome()
{
    QString file = QFileDialog::getOpenFileName( QDir::homeDirPath(), "*.html *.htm", this, "setHome",
	    tr( "Qt Assistant - Edit Homepage" ) );
    if ( !file.isNull() )
	leHome->setText( file );
}
