/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtprefdia.cpp#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtprefdia.h"
#include "qtpreferences.h"

#include <qwidget.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

/****************************************************************************
 *
 * Class: QTPrefDia
 *
 ****************************************************************************/

QTPrefDia::QTPrefDia( QWidget *parent, const char *name, QTPreferences *prefs )
    : QTabDialog( parent, name, TRUE ), preferences( prefs ),
      srcAction( Add ), extAction( Add )
{
    setCaption( tr( "Preferences" ) );

    //setupTabGeneral();
    setupTabSources();
    setupTabTranslation();

    //initTabGeneral();
    initTabSources();
    initTabTranslation();

    connect( this, SIGNAL( applyButtonPressed() ),
             this, SLOT( slotApplyButtonPressed() ) );
    connect( this, SIGNAL( helpButtonPressed() ),
             this, SLOT( slotHelpButtonPressed() ) );
    
    setHelpButton();
    setCancelButton();
}

void QTPrefDia::setupTabGeneral()
{
    QWidget *tab = new QWidget( this );
    addTab( tab, tr( "&General" ) );
}

void QTPrefDia::initTabGeneral()
{
}

void QTPrefDia::setupTabSources()
{
    QVBox *tab = new QVBox( this );

    QGroupBox *srcDirs = new QGroupBox( tr( "Source Directories" ), tab );
    QGridLayout *srcLayout = new QGridLayout( srcDirs, 3, 2, 7, 5 );

    QHBox *srcEditLayout = new QHBox( srcDirs );
    srcEditLayout->setSpacing( 10 );

    srcLineEdit = new QLineEdit( srcEditLayout );
    QWhatsThis::add( srcLineEdit, tr( "Into this editbox you can enter a Source Directory, which "
                                      "you can add to the list below with the Add button" ) );

    srcBrowse = new QPushButton( tr( "..." ), srcEditLayout );
    QWhatsThis::add( srcBrowse, tr( "When you press this button, you can choose in "
                                    "a fileselector a Source Directory.") );
    
    srcList = new QListBox( srcDirs );
    QWhatsThis::add( srcList, tr( "In this list all Source Directories are listed, in which "
                                  "QTranslator will search later for messages which you can "
                                  "translate then.\n\n"
                                  "All these messages have to be surrounded by tr( \"..\" ) in the "
                                  "sources." ) );


    QVBox *srcButtonLayout = new QVBox( srcDirs );
    srcButtonLayout->setSpacing( 10 );

    srcAdd = new QPushButton( tr( "&Add" ), srcButtonLayout );
    QWhatsThis::add( srcAdd, tr( "Using this button you can add a Source Directory from "
                                 "the editbox to the list." ) );

    srcDel = new QPushButton( tr( "&Delete" ), srcButtonLayout );
    QWhatsThis::add( srcDel, tr( "Using this button you can delete the selected Source Directory "
                                 "from the list" ) );
    
    srcLayout->addRowSpacing( 0, 15 );
    srcLayout->addWidget( srcEditLayout, 1, 0 );
    srcLayout->addWidget( srcList, 2, 0 );
    srcLayout->addMultiCellWidget( srcButtonLayout, 1, 2, 1, 1, Qt::AlignTop);

    connect( srcLineEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotSrcLineEdit() ) );
    connect( srcBrowse, SIGNAL( clicked() ),
             this, SLOT( slotSrcBrowse() ) );
    connect( srcList, SIGNAL( selected( const QString & ) ),
             this, SLOT( slotSrcList( const QString & ) ) );
    connect( srcList, SIGNAL( highlighted( const QString & ) ),
             this, SLOT( slotSrcListChanged( const QString & ) ) );
    connect( srcAdd, SIGNAL( clicked() ),
             this, SLOT( slotSrcAdd() ) );
    connect( srcDel, SIGNAL( clicked() ),
             this, SLOT( slotSrcDel() ) );


    QGroupBox *extDirs = new QGroupBox( tr( "Extensions" ), tab );
    QGridLayout *extLayout = new QGridLayout( extDirs, 3, 2, 7, 5 );

    QHBox *extEditLayout = new QHBox( extDirs );
    extEditLayout->setSpacing( 10 );

    extLineEdit = new QLineEdit( extEditLayout );
    QWhatsThis::add( extLineEdit, tr( "Into this editbox you can enter a File Extension, which "
                                      "you can add to the list below with the Add button\n\n"
                                      "Enter the extension ONLY, e.g. enter 'cpp' and NOT '*.cpp'" ) );

    extList = new QListBox( extDirs );
    QWhatsThis::add( extList, tr( "In all files in the Source Directories with the Extensions, "
                                  "which are listed here, QTranslator will search later for messages "
                                  "which you can translate then.\n\n"
                                  "All these messages have to be surrounded by tr( \"..\" ) in the "
                                  "sources." ) );

    QVBox *extButtonLayout = new QVBox( extDirs );
    extButtonLayout->setSpacing( 10 );

    extAdd = new QPushButton( tr( "&Add" ), extButtonLayout );
    QWhatsThis::add( extAdd, tr( "Using this button you can add a File Extension from "
                                 "the editbox to the list." ) );

    extDel = new QPushButton( tr( "&Delete" ), extButtonLayout );
    QWhatsThis::add( extDel, tr( "Using this button you can delete the selected File Extension "
                                 "from the list" ) );

    extLayout->addRowSpacing( 0, 15 );
    extLayout->addWidget( extEditLayout, 1, 0 );
    extLayout->addWidget( extList, 2, 0 );
    extLayout->addMultiCellWidget( extButtonLayout, 1, 2, 1, 1, Qt::AlignTop);

    connect( extLineEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotExtLineEdit() ) );
    connect( extList, SIGNAL( selected( const QString & ) ),
             this, SLOT( slotExtList( const QString & ) ) );
    connect( extList, SIGNAL( highlighted( const QString & ) ),
             this, SLOT( slotExtListChanged( const QString & ) ) );
    connect( extAdd, SIGNAL( clicked() ),
             this, SLOT( slotExtAdd() ) );
    connect( extDel, SIGNAL( clicked() ),
             this, SLOT( slotExtDel() ) );


    addTab( tab, tr( "&Sources" ) );

}

void QTPrefDia::initTabSources()
{
    if ( !preferences ) {
        qWarning( "Cannot read/modify preferences!" );
        return;
    }

    QStringList::Iterator it = preferences->sources.directories.begin();
    for ( ; it != preferences->sources.directories.end(); ++it )
        srcList->insertItem( *it, -1 );
    it = preferences->sources.extensions.begin();
    for ( ; it != preferences->sources.extensions.end(); ++it )
        extList->insertItem( *it, -1 );
}

void QTPrefDia::setupTabTranslation()
{
    QVBox *tab = new QVBox( this );
    tab->setMargin( 5 );

    QHBox *dirLayout = new QHBox( tab );
    dirLayout->setSpacing( 5 );

    ( void )new QLabel( tr( "Translation Directory: " ), dirLayout );
    dirLineEdit = new QLineEdit( dirLayout );
    QWhatsThis::add( dirLineEdit, tr( "Here you have to enter the directory, into which "
                                      "the translation files (.po and .qm) should be saved" ) );
    dirBrowse = new QPushButton( tr( "..." ), dirLayout );
    QWhatsThis::add( dirBrowse, tr( "When you press this button, you can choose in "
                                    "a fileselector a directory, into which the translation "
                                    "files will be saved" ) );

    QHBox *nameLayout = new QHBox( tab );
    nameLayout->setSpacing( 5 );

    ( void )new QLabel( tr( "Prefix of Translation Filename: " ), nameLayout );
    nameLineEdit = new QLineEdit( nameLayout );
    QWhatsThis::add( nameLineEdit, tr( "If you want, that the translation files have a prefix, "
                                       "enter this here." ) );

    foldersCheckBox = new QCheckBox( tr( "&Put Translations into Subfolders" ), tab );
    QWhatsThis::add( foldersCheckBox, tr( "If this button is checked, QTranslator creates for each "
                                          "langauge an own directory and puts the translation files "
                                          "int this one. Else all translations are saved into the "
                                          "translation directory directly" ) );

    ( void )new QWidget( tab );
    ( void )new QWidget( tab );

    connect( dirBrowse, SIGNAL( clicked() ),
             this, SLOT( slotDirBrowse() ) );

    addTab( tab, tr( "&Translation" ) );
}

void QTPrefDia::initTabTranslation()
{
    dirLineEdit->setText( preferences->translation.directory );
    nameLineEdit->setText( preferences->translation.prefix );
    foldersCheckBox->setChecked( preferences->translation.folders );
}

void QTPrefDia::slotSrcLineEdit()
{
    if ( srcLineEdit->text().isEmpty() ) {
        srcList->setCurrentItem( -1 );
        srcAction = Add;
        srcAdd->setText( tr( "&Add" ) );
        return;
    }

    if ( srcAction == Change && srcList->currentItem() != -1 )
        srcList->changeItem( srcLineEdit->text(), srcList->currentItem() );
    else
        srcList->insertItem( srcLineEdit->text(), -1 );

    srcLineEdit->setText( "" );
    srcList->setCurrentItem( -1 );
    srcAction = Add;
    srcAdd->setText( tr( "&Add" ) );
}

void QTPrefDia::slotSrcBrowse()
{
    QString dir = QFileDialog::getExistingDirectory();
    if ( !dir.isEmpty() )
        srcLineEdit->setText( dir );
}

void QTPrefDia::slotSrcList( const QString &text )
{
    if ( text.isEmpty() )
        return;

    srcAction = Change;
    srcAdd->setText( tr( "&Change" ) );
    srcLineEdit->setText( text );
}

void QTPrefDia::slotSrcAdd()
{
    slotSrcLineEdit();
}

void QTPrefDia::slotSrcDel()
{
    if ( srcList->currentItem() != -1 ) {
        srcList->removeItem( srcList->currentItem() );
        srcList->setCurrentItem( -1 );
        srcAction = Add;
        srcAdd->setText( tr( "&Add" ) );
        srcLineEdit->setText( "" );
    }
}

void QTPrefDia::slotSrcListChanged( const QString &)
{
    srcAction = Add;
    srcAdd->setText( tr( "&Add" ) );
    srcLineEdit->setText( "" );
}

void QTPrefDia::slotExtLineEdit()
{
    if ( extLineEdit->text().isEmpty() ) {
        extList->setCurrentItem( -1 );
        extAction = Add;
        extAdd->setText( tr( "&Add" ) );
        return;
    }

    if ( extAction == Change && extList->currentItem() != -1 )
        extList->changeItem( extLineEdit->text(), extList->currentItem() );
    else
        extList->insertItem( extLineEdit->text(), -1 );

    extLineEdit->setText( "" );
    extList->setCurrentItem( -1 );
    extAction = Add;
    extAdd->setText( tr( "&Add" ) );
}

void QTPrefDia::slotExtList( const QString &text )
{
    if ( text.isEmpty() )
        return;

    extAction = Change;
    extAdd->setText( tr( "&Change" ) );
    extLineEdit->setText( text );
}

void QTPrefDia::slotExtAdd()
{
    slotExtLineEdit();
}

void QTPrefDia::slotExtDel()
{
    if ( extList->currentItem() != -1 ) {
        extList->removeItem( extList->currentItem() );
        extList->setCurrentItem( -1 );
        extAction = Add;
        extAdd->setText( tr( "&Add" ) );
        extLineEdit->setText( "" );
    }
}

void QTPrefDia::slotExtListChanged( const QString & )
{
    extAction = Add;
    extAdd->setText( tr( "&Add" ) );
    extLineEdit->setText( "" );
}

void QTPrefDia::slotDirBrowse()
{
    QString dir = QFileDialog::getExistingDirectory();
    if ( !dir.isEmpty() )
        dirLineEdit->setText( dir );
}

void QTPrefDia::slotApplyButtonPressed()
{
    if ( !preferences ) {
        qWarning( "Cannot read/modify preferences!" );
        return;
    }

    unsigned int i = 0;

    preferences->sources.directories.clear();
    preferences->sources.extensions.clear();

    for ( i = 0; i < srcList->count(); ++i )
        preferences->sources.directories.append( srcList->text( i ) );
    for ( i = 0; i < extList->count(); ++i )
        preferences->sources.extensions.append( extList->text( i ) );

    preferences->translation.directory = dirLineEdit->text();
    preferences->translation.prefix = nameLineEdit->text();
    preferences->translation.folders = foldersCheckBox->isChecked();
}

void QTPrefDia::slotHelpButtonPressed()
{
    QWhatsThis::enterWhatsThisMode();
}
