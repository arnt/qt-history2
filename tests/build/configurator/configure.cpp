#include "configure.h"

#include <qdir.h>
#include <stdlib.h>
#include <qlistview.h>
#include <qsettings.h>
#include <qprocess.h>

ConfigureQtDialogImpl::ConfigureQtDialogImpl( QWidget* parent, const char* name, WFlags fl )
    : ConfigureQtDialog(parent,name,fl)
{

    listViewOptions->setSorting( -1 );
    listViewAdvanced->setSorting( -1 );
    QCheckListItem* item;

    QString qtdir = getenv( "QTDIR" );

    if ( !qtdir.isEmpty() ) {

	QString mkspecsdir = qtdir + "/mkspecs";
	QString mkspecsenv = getenv( "MKSPEC" );
	QFileInfo mkspecsenvdirinfo( mkspecsenv );
	QString srcdir = qtdir + "/src";
	QFileInfo* fi;

	// general
	modules = new QCheckListItem ( listViewOptions, "Modules" );
	modules->setOpen( TRUE );
	QDir srcDir( srcdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* srcdirs = srcDir.entryInfoList();
	QFileInfoListIterator srcDirIterator( *srcdirs );
	srcDirIterator.toLast();
	while ( ( fi = srcDirIterator.current() ) ) {
	    if ( fi->fileName() != "." && fi->fileName() != ".." ) {
		item = new QCheckListItem( modules, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --srcDirIterator;
	}

	threadModel = new QCheckListItem ( listViewOptions, "Threading" );
	threadModel->setOpen( TRUE );
	item = new QCheckListItem( threadModel, "Threaded", QCheckListItem::RadioButton );
	item = new QCheckListItem( threadModel, "Non-threaded", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	buildType = new QCheckListItem ( listViewOptions, "Build" );
	buildType->setOpen( TRUE );
	item = new QCheckListItem( buildType, "Static", QCheckListItem::RadioButton );
	item = new QCheckListItem( buildType, "Shared", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	debugMode = new QCheckListItem ( listViewOptions, "Mode" );
	debugMode->setOpen( TRUE );
	item = new QCheckListItem( debugMode, "Debug", QCheckListItem::RadioButton );
	item = new QCheckListItem( debugMode, "Release", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	// advanced
	mkspec = new QCheckListItem ( listViewAdvanced, "Platform-Compiler" );
	QDir mkspecDir( mkspecsdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files );
	const QFileInfoList* mkspecs = mkspecDir.entryInfoList();
	QFileInfoListIterator mkspecIterator( *mkspecs );
	mkspecIterator.toLast();
	while ( ( fi = mkspecIterator.current() ) ) {
	    item = new QCheckListItem ( mkspec, fi->fileName(), QCheckListItem::RadioButton );
	    if ( mkspecsenvdirinfo.baseName() ==  fi->fileName() )
		item->setOn( TRUE );
	    --mkspecIterator;
	}

	//sql drivers ###


	loadSettings();

    } else {
	// ## msgbox
    }
}

ConfigureQtDialogImpl::~ConfigureQtDialogImpl()
{
}

void ConfigureQtDialogImpl::loadSettings()
{
    QSettings settings;
    QString entry;

    entry = settings.readEntry( "Mode" );
    set( debugMode, entry );

    entry = settings.readEntry( "Build" );
    set( buildType, entry );

    entry = settings.readEntry( "Threading" );
    set( threadModel, entry );

    QStringList entries = settings.readListEntry( "Modules", ',' );
    set( modules, entries );

    entry = settings.readEntry( "Platform-Compiler" );
    set( mkspec, entry );

}

void ConfigureQtDialogImpl::set( QCheckListItem* parent, const QString& setting )
{
    if ( !setting.isEmpty() ) {
	QCheckListItem* item = (QCheckListItem*)parent->firstChild();
	while( item != 0 ) {
	    if ( item->text() == setting ) {
		item->setOn( TRUE );
		break;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
    }
}

void ConfigureQtDialogImpl::set( QCheckListItem* parent, const QStringList& settings )
{
    QCheckListItem* item = (QCheckListItem*)parent->firstChild();
    while( item != 0 ) {
	if ( settings.contains( item->text() ) ) {
	    item->setOn( TRUE );
	} else {
	    item->setOn( FALSE );
	}
	item = (QCheckListItem*)item->nextSibling();
    }
}


void ConfigureQtDialogImpl::accept()
{
    saveSettings();

    ConfigureQtDialog::accept();
}

void ConfigureQtDialogImpl::execute()
{
    QStringList args;
    args +=  QString( getenv( "QTDIR" ) ) + "/bin/qmake";

    QListViewItem* config = listViewOptions->firstChild();
    while ( config ) {
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->isOn() ) {
		args += config->text(0) + " : " + item->text(); //###
		break;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	config = config->nextSibling();
    }

    QProcess qmake( args );
    qmake.start();
}

void ConfigureQtDialogImpl::saveSettings()
{
    QSettings settings;

    QListViewItem* config = listViewOptions->firstChild();
    while ( config ) {
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->isOn() ) {
		settings.writeEntry( "/General/" + config->text(0), item->text() );
		break;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	config = config->nextSibling();
    }
}
