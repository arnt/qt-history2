/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "settingsdialogimpl.h"
#include "docuparser.h"
#include "config.h"
#include "profiledialogimpl.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qptrstack.h>
#include <qsettings.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qmap.h>

int ProfileCheckItem::RTTI = 7391;

ProfileCheckItem::ProfileCheckItem( QListView *parent, const QString &name )
    : QCheckListItem( parent, name, QCheckListItem::RadioButtonController )
{

}

ProfileCheckItem::ProfileCheckItem( ProfileCheckItem *parent,
    const QString &pN )
    : QCheckListItem( parent, pN, QCheckListItem::RadioButton )
{
    profName = pN;
}

int ProfileCheckItem::rtti() const
{
    return ProfileCheckItem::RTTI;
}

void ProfileCheckItem::activate()
{
    setState( QCheckListItem::On );
}

QString ProfileCheckItem::profileName() const
{
    return profName;
}



SettingsDialog::SettingsDialog( QWidget *parent, const char* name )
    : SettingsDialogBase( parent, name )
{
    if ( Config::configuration()->startedWithProfile() )
	settingsTab->removePage( settingsTab->page( settingsTab->count()-1 ) );

    init();
}

void SettingsDialog::init()
{
    Config *config = Config::configuration();

    browserApp->setText( config->webBrowser() );
    homePage->setText( config->homePage() );
    pdfApp->setText( config->pdfReader() );
    oldProfile = config->profileName();
    profileAttributesChanged = FALSE;
    setupProfiles();
}

void SettingsDialog::setCurrentProfile()
{
    oldProfile = Config::configuration()->profileName();
    setupProfiles();
}

ProfileCheckItem* SettingsDialog::currentCheckedProfile()
{
    QPtrList<QListViewItem> lst;
    QListViewItemIterator it( profileView );
    ProfileCheckItem *item;
    while ( it.current() ) {
	if( (it.current())->rtti() != ProfileCheckItem::RTTI )
	    continue;
	item = (ProfileCheckItem*)(it.current());
	if ( item->state() == QCheckListItem::On )
	    return item;
	++it;
    }
    return 0;
}

void SettingsDialog::selectColor()
{
    QColor c = QColorDialog::getColor( colorButton->paletteBackgroundColor(), this );
    colorButton->setPaletteBackgroundColor( c );
}

void SettingsDialog::setupProfiles()
{
    Config *config = Config::configuration();
    config->reloadProfiles();
    QStringList profs = config->profiles();

    QString oldProfile;
    if (  currentCheckedProfile() )
	oldProfile = currentCheckedProfile()->profileName();
    else
	oldProfile = config->profileName();

    profileView->clear();
    ProfileCheckItem *root = new ProfileCheckItem( profileView, tr( "Profiles" ) );
    root->setOpen( TRUE );

    ProfileCheckItem *ci;
    QStringList::ConstIterator it = profs.begin();
    for ( ; it != profs.end(); ++it ) {
	ci = new ProfileCheckItem( root, *it );
	ci->setSelected( FALSE );
	if ( *it == oldProfile ) {
	    ci->activate();
	    ci->setSelected( TRUE );
	}
    }
    if ( profs.count() > 1 )
	buttonDelete->setEnabled( TRUE );
    else
	buttonDelete->setEnabled( FALSE );
}

void SettingsDialog::addProfile()
{
    ProfileDialog pd( this );
    if ( pd.exec() == QDialog::Accepted )
	setupProfiles();
}

void SettingsDialog::removeProfile()
{
    ProfileCheckItem *item = currentCheckedProfile();
    if ( !item )
	return;
    deleteProfilesList << item->profileName();
    delete item;
    item = (ProfileCheckItem*)(profileView->firstChild()->firstChild());
    if ( item ) {
	item->activate();
	item->setSelected( TRUE );
    }
    if ( profileView->firstChild()->childCount() < 2 )
	buttonDelete->setDisabled( TRUE );
}

void SettingsDialog::modifyProfile()
{
    ProfileCheckItem *item = currentCheckedProfile();
    ProfileDialog pd( this, item->profileName() );
    if ( pd.exec() == QDialog::Accepted )
	setupProfiles();
    profileAttributesChanged = pd.profileChanged();
}

void SettingsDialog::browseWebApp()
{
    setFile( browserApp, tr( "Qt Assistant - Set Web Browser" ) );
}

void SettingsDialog::browsePDFApplication()
{
    setFile( pdfApp, tr( "Qt Assistant - Set PDF Browser" ) );
}

void SettingsDialog::browseHomepage()
{
    setFile( homePage, tr( "Qt Assistant - Set Homepage" ) );
}

void SettingsDialog::setFile( QLineEdit *le, const QString &caption )
{
    QFileDialog *fd = new QFileDialog( this );
    fd->setCaption( caption );
    fd->setMode( QFileDialog::AnyFile );
    fd->setDir( QDir::homeDirPath() );

    if ( fd->exec() == QDialog::Accepted ) {
	if ( !fd->selectedFile().isEmpty() )
	   le->setText( fd->selectedFile() );
    }
}

void SettingsDialog::accept()
{
    Config *config = Config::configuration();

    config->setWebBrowser( browserApp->text() );
    config->setHomePage( homePage->text() );
    config->setPdfReader( pdfApp->text() );

    hide();

    QStringList::ConstIterator it = deleteProfilesList.begin();
    for ( ; it != deleteProfilesList.end(); ++it )
	Config::removeProfile( *it );

    ProfileCheckItem *item = currentCheckedProfile();
    if ( item )
	newProfile = item->profileName();

    if ( newProfile != oldProfile || profileAttributesChanged ) {
	config->setCurrentProfile( newProfile );
	emit profileChanged();
    }

    done( Accepted );
}

void SettingsDialog::reject()
{
    init();
    done( Rejected );
}
