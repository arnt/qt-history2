#include "setupwizardimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <windows.h>
#include <qbuttongroup.h>
#include <qsettings.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <zlib/zlib.h>
#include <qtextstream.h>

#define BUFFERSIZE 65536

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f ), tmpPath( 256 )
{
    sysID = 0;
    app = NULL;
    setNextEnabled( introPage, false );
//    setFinishEnabled( finishPage, true );
    setBackEnabled( progressPage, false );
//    setBackEnabled( finishPage, false );
    setNextEnabled( progressPage, false );

    // Read the installation control files
    GetTempPath( tmpPath.size(), tmpPath.data() );

    readArchive( "sys.arq", tmpPath.data() );
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg;
    QDir installDir( installPath->text() );

    if( !installDir.exists() )
	installDir.setPath( "C:\\" );

    dlg.setDir( installDir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	installPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedSystem( int sys )
{
    sysID = sys;
}

void SetupWizardImpl::licenseAccepted( )
{
    setNextEnabled( introPage, true );
}

void SetupWizardImpl::readConfigureOutput()
{
    outputDisplay->append( QString( configure.readStdout() ) );
    outputDisplay->append( QString( configure.readStderr() ) );
}

void SetupWizardImpl::readMakeOutput()
{
    outputDisplay->append( QString( make.readStdout() ) );
    outputDisplay->append( QString( make.readStderr() ) );
}

void SetupWizardImpl::makeDone()
{
}

void SetupWizardImpl::configDone()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake" );
    QStringList args;

    connect( &make, SIGNAL( processExited() ), this, SLOT( makeDone() ) );
    connect( &make, SIGNAL( readyReadStdout() ), this, SLOT( readMakeOutput() ) );
    connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeOutput() ) );

    args << makeCmds[ sysID ];

    make.setWorkingDirectory( QString( getenv( "QTDIR" ) ) );
    make.setArguments( args );

    make.start();
}

void SetupWizardImpl::saveSettings()
{
    QApplication::setOverrideCursor( Qt::waitCursor );
    saveSet( configList );
    saveSet( advancedList );
    QApplication::restoreOverrideCursor();
}

void SetupWizardImpl::saveSet( QListView* list )
{
    QSettings settings;
    settings.writeEntry( "/Software/Trolltech/Qt/ResetDefaults", "FALSE" );
    // radios
    QListViewItem* config = list->firstChild();
    while ( config ) {
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->type() == QCheckListItem::RadioButton ) {
		if ( item->isOn() ) {
		    settings.writeEntry( "/Software/Trolltech/Qt/" + config->text(0), item->text() );
		    break;
		}
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	config = config->nextSibling();
    }

    // checks
    config = list->firstChild();
    QStringList lst;
    while ( config ) {
	bool foundChecks = FALSE;
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    if ( item->type() == QCheckListItem::CheckBox ) {
		if ( item->isOn() )
		    lst += item->text();
		foundChecks = TRUE;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	if ( foundChecks )
	    settings.writeEntry( "/Software/Trolltech/Qt/" + config->text(0), lst, ',' );
	config = config->nextSibling();
	lst.clear();
    }
}

void SetupWizardImpl::pageChanged( const QString& pageName )
{
    HKEY environmentKey;

    if( pageName == title( introPage ) ) {
	QFile licenseFile( QString( tmpPath.data() ) + "\\LICENSE" );
	if( licenseFile.open( IO_ReadOnly ) ) {
	    QByteArray fileData;
	    QFileInfo fi( licenseFile );
	    
	    if( !fileData.resize( fi.size() ) )
		qFatal( "Could not allocate memory for license text!" );
	    licenseFile.readBlock( fileData.data(), fileData.size() );
	    introText->setText( QString( fileData.data() ) );
	}
    }
    else if( pageName == title( optionsPage ) ) {
	installPath->setText( QString( "C:\\Qt\\" ) + QString( DISTVER ) );
	sysGroup->setButton( 0 );
    }
    else if( pageName == title( progressPage ) ) {
	int totalSize( 0 );
	QFileInfo fi;
	totalRead = 0;

	fi.setFile( "qt.arq" );
	if( fi.exists() )
	    totalSize = fi.size();

	fi.setFile( "build.arq" );
	if( fi.exists() )
	    totalSize += fi.size();

	if( installDocs->isChecked() ) {
	    fi.setFile( "doc.arq" );
	    if( fi.exists() ) 
		totalSize += fi.size();
	}

	if( installExamples->isChecked() ) {
	    fi.setFile( "examples.arq" );
	    if( fi.exists() )
		totalSize += fi.size();
	}

	if( installTutorials->isChecked() ) {
	    fi.setFile( "tutorial.arq" );
	    if( fi.exists() )
		totalSize += fi.size();
	}

	operationProgress->setTotalSteps( totalSize );

/*
	readArchive( "qt.arq", installPath->text() );
	readArchive( "build.arq", installPath->text() );
	if( installDocs->isChecked() )
	    readArchive( "doc.arq", installPath->text() );
	if( installExamples->isChecked() )
	    readArchive( "examples.arq", installPath->text() );
	if( installTutorials->isChecked() )
	    readArchive( "tutorial.arq", installPath->text() );
*/
	next();
    }
    else if( pageName == title( configPage ) ) {
	QStringList mkSpecs = QStringList::split( ' ', "win32-msvc win32-borland win32-g++" );

	putenv( QString( "QTDIR=" ) + installPath->text().latin1() );
	putenv( QString( "MKSPEC=" ) + mkSpecs[ sysID ].latin1() );

	if( ( RegOpenKeyEx( HKEY_CURRENT_USER, "Environment", 0, KEY_WRITE, &environmentKey ) ) == ERROR_SUCCESS ) {
	    RegSetValueEx( environmentKey, "QTDIR", 0, REG_SZ, (const unsigned char*)installPath->text().latin1(), installPath->text().length() + 1 );
	    RegSetValueEx( environmentKey, "MKSPEC", 0, REG_SZ, (const unsigned char*)mkSpecs[ sysID ].latin1(), mkSpecs[ sysID ].length() + 1 );
	}
	configList->setSorting( -1 );
	advancedList->setSorting( -1 );
	QCheckListItem* item;
	connect( &configure, SIGNAL( processExited() ), this, SLOT( configDone() ) );
	connect( &configure, SIGNAL( readyReadStdout() ), this, SLOT( readConfigureOutput() ) );
	connect( &configure, SIGNAL( readyReadStderr() ), this, SLOT( readConfigureOutput() ) );

	QString qtdir = getenv( "QTDIR" );

	QString mkspecsdir = qtdir + "/mkspecs";
	QString mkspecsenv = getenv( "MKSPEC" );
	QFileInfo mkspecsenvdirinfo( mkspecsenv );
	QString srcdir = qtdir + "/src";
	QFileInfo* fi;

	// general
	modules = new QCheckListItem ( configList, "Modules" );
	modules->setOpen( TRUE );
	QDir srcDir( srcdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* srcdirs = srcDir.entryInfoList();
	QFileInfoListIterator srcDirIterator( *srcdirs );
	srcDirIterator.toLast();
	while ( ( fi = srcDirIterator.current() ) ) {
	    if ( fi->fileName()[0] != '.' && // fi->fileName() != ".." &&
		 fi->fileName() != "tmp" &&
		 fi->fileName() != "compat" &&
		 fi->fileName() != "3rdparty" &&
		 fi->fileName() != "Debug" && // MSVC directory
		 fi->fileName() != "Release" && // MSVC directory
		 fi->fileName() != "moc" ) {
		item = new QCheckListItem( modules, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --srcDirIterator;
	}
	threadModel = new QCheckListItem ( configList, "Threading" );
	threadModel->setOpen( TRUE );
	item = new QCheckListItem( threadModel, "Threaded", QCheckListItem::RadioButton );
	item = new QCheckListItem( threadModel, "Non-threaded", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	buildType = new QCheckListItem ( configList, "Build" );
	buildType->setOpen( TRUE );
	item = new QCheckListItem( buildType, "Static", QCheckListItem::RadioButton );
	item = new QCheckListItem( buildType, "Shared", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	debugMode = new QCheckListItem ( configList, "Mode" );
	debugMode->setOpen( TRUE );
	item = new QCheckListItem( debugMode, "Debug", QCheckListItem::RadioButton );
	item = new QCheckListItem( debugMode, "Release", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	// advanced
	sqldrivers = new QCheckListItem ( advancedList, "SQL Drivers" );
	sqldrivers->setOpen( true );
	QDir sqlsrcDir( srcdir + "/sql/src", QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* sqlsrcdirs = sqlsrcDir.entryInfoList();
	QFileInfoListIterator sqlsrcDirIterator( *sqlsrcdirs );
	sqlsrcDirIterator.toLast();
	while ( ( fi = sqlsrcDirIterator.current() ) ) {
	    if ( fi->fileName() != "." && fi->fileName() != ".." && fi->fileName() != "tmp" ) {
		item = new QCheckListItem( sqldrivers, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --sqlsrcDirIterator;
	}
    }
    else if( pageName == title( buildPage ) ) {
	QStringList args;
	QStringList entries;
	QSettings settings;
	QString entry;
	QStringList::Iterator it;
	QFile tmpFile;
	QTextStream tmpStream;
	bool settingsOK;

	settings.writeEntry( "/Software/Microsoft/Command Processor/DelayedExpansion", 1 );
	saveSettings();
    
	outputDisplay->setText( "Execute configure...\n" );

	args << QString( getenv( "QTDIR" ) ) + "\\configure.bat";
	entry = settings.readEntry( "/Software/Trolltech/Qt/Mode", &settingsOK );
	if ( entry == "Debug" )
	    args += "-debug";
	else
	    args += "-release";

	entry = settings.readEntry( "/Software/Trolltech/Qt/Build", &settingsOK );
	if ( entry == "Static" )
	    args += "-static";
	else
	    args += "-shared";

	entry = settings.readEntry( "/Software/Trolltech/Qt/Threading", &settingsOK );
	if ( entry == "Threaded" )
	    args += "-thread";

	entries = settings.readListEntry( "/Software/Trolltech/Qt/Modules", ',', &settingsOK );
	for( it = entries.begin(); it != entries.end(); ++it ) {
	    entry = *it;
	    args += QString( "-enable-" ) + entry;
	}

	entries = settings.readListEntry( "/Software/Trolltech/Qt/SQL Drivers", ',', &settingsOK );
	for( it = entries.begin(); it != entries.end(); ++it ) {
	    entry = *it;
	    args += QString( "-sql-" ) + entry;
	}

	tmpFile.setName( QString( getenv( "QTDIR" ) ) + "\\configcmd.bat" );
	if( tmpFile.open( IO_WriteOnly ) ) {
	    tmpStream.setDevice( &tmpFile );
	    tmpStream << args.join( " " );

	    args.clear();
	    args << QString( "CMD /V:ON /C " ) + tmpFile.name();
	    tmpFile.close();
	}

	outputDisplay->append( args.join( " " ) + "\n" );
	configure.setWorkingDirectory( QString( getenv( "QTDIR" ) ) );
	configure.setArguments( args );
	// Start the configure process
	configure.start();
	// The configuration is now started.
	// The build will be done in the configDone() slot
    }
}
bool SetupWizardImpl::createDir( QString fullPath )
{
    QStringList hierarchy = QStringList::split( QString( "\\" ), fullPath );
    QString pathComponent, tmpPath;
    QDir dirTmp;
    bool success;

    for( QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it ) {
	pathComponent = *it + "\\";
	tmpPath += pathComponent;
	success = dirTmp.mkdir( tmpPath );
    }
    return success;
}

void SetupWizardImpl::readArchive( QString arcname, QString installPath )
{
    QDataStream inStream, outStream;
    QFile inFile, outFile;
    QDir outDir;
    QByteArray inBuffer;
    QByteArray outBuffer( BUFFERSIZE );
    z_stream ztream;
    int totalOut;
    bool continueDeCompressing;
    QString entryName, dirName;
    int entryLength;

    inFile.setName( arcname );
    outDir.setPath( QDir::convertSeparators( installPath ) );
    if( inFile.open( IO_ReadOnly ) ) {
	inStream.setDevice( &inFile );
	while( !inStream.atEnd() ) {
	    inStream >> entryLength;
	    totalRead += sizeof( entryLength );
	    inBuffer.resize( entryLength );
	    inStream.readRawBytes( inBuffer.data(), entryLength );
	    totalRead += entryLength;
	    entryName = inBuffer.data();
	    if( entryName.right( 1 ) == "\\" ) {
		if( entryName == "..\\" )
		    outDir.cdUp();
		else {
		    dirName = QDir::convertSeparators( outDir.absPath() + QString( "\\" ) + entryName.left( entryName.length() - 1 ) );
		    if( outDir.exists( dirName ) )
			outDir.cd( dirName );
		    else {
			if( createDir( dirName ) )
			    outDir.cd( dirName );
			else
			    break;
		    }
		}
	    }
	    else
	    {
		QString fileName = outDir.absPath() + QString( "\\" ) + entryName;
		totalOut = 0;
		outFile.setName( fileName );
		
		if( outFile.open( IO_WriteOnly ) ) {

		    if( app ) {
			app->processEvents();
			operationProgress->setProgress( totalRead );
			filesDisplay->append( fileName );
		    }
		    outStream.setDevice( &outFile );
		    inStream >> entryLength;
		    totalRead += sizeof( entryLength );
		    inBuffer.resize( entryLength );
		    inStream.readRawBytes( inBuffer.data(), entryLength );
		    totalRead += entryLength;
		    ztream.next_in = (unsigned char*)inBuffer.data();
		    ztream.avail_in = entryLength;
		    ztream.total_in = 0;
		    ztream.msg = NULL;
		    ztream.zalloc = (alloc_func)0;
		    ztream.zfree = (free_func)0;
		    ztream.opaque = (voidpf)0;
		    ztream.data_type = Z_BINARY;
		    inflateInit( &ztream );
		    continueDeCompressing = true;
		    while( continueDeCompressing ) {
			ztream.next_out = (unsigned char*)outBuffer.data();
			ztream.avail_out = outBuffer.size();
			ztream.total_out = 0;
			continueDeCompressing = ( inflate( &ztream, Z_NO_FLUSH ) == Z_OK );
			outStream.writeRawBytes( outBuffer.data(), ztream.total_out );
			totalOut += ztream.total_out;
		    }
		    inflateEnd( &ztream );
		    outFile.close();

		}
	    }
	}

	inFile.close();
    }
}

