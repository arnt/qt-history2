#include "setupwizardimpl.h"
#include "environment.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <qbuttongroup.h>
#include <qsettings.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <zlib/zlib.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qarchive.h>
#include <qvalidator.h>
#include <qdatetime.h>

#if defined(MD4_KEYS)
#include "../md4/qrsync.h"
#endif

#include <keyinfo.h>

#if defined(EVAL)
#include <check-and-patch.h>
#endif

#define FILESTOCOPY 4582

class ResourceLoader
{
public:
    /*
       Tries to load the binary resource \a resourceName. If the resource is
       smaller than \a minimumSize, the resource is not loaded and isValid()
       returns FALSE. isValid() returns also FALSE when the loading failed.
    */
    ResourceLoader( char *resourceName, int minimumSize=0 )
    {
#if defined(Q_OS_WIN32)
	valid = TRUE;

	HMODULE hmodule = GetModuleHandle( 0 );
	HRSRC resource = FindResourceA( hmodule, resourceName, MAKEINTRESOURCEA( 10 ) );
	HGLOBAL hglobal = LoadResource( hmodule, resource );
	arSize = SizeofResource( hmodule, resource );
	if ( arSize == 0 ) {
	    valid = FALSE;
	    return;
	}
	if ( arSize < minimumSize ) {
	    valid = FALSE;
	    return;
	}
	arData = (char*)LockResource( hglobal );
	if ( arData == 0 ) {
	    valid = FALSE;
	    return;
	}
	ba.setRawData( arData, arSize );
#elif defined(Q_OS_MACX)
	valid = FALSE;
	arSize = 0;
	arData = 0;
	QFile f;
	QString path = "install.app/Contents/Qt/";
	path += resourceName;
	f.setName( path );
	if (!f.open( IO_ReadOnly ))
	    return;
	QFileInfo fi(f);
	arSize = fi.size();
	arData = new char[arSize];
	if (f.readBlock( arData, arSize ) != arSize)
	{
	    delete[] arData;
	    return;
	}
	ba.setRawData( arData, arSize );
	valid = TRUE;
	return;
#endif
    }

    ~ResourceLoader()
    {
	if ( isValid() )
	    ba.resetRawData( arData, arSize );
#if defined(Q_OS_MACX)
	delete[] arData;
#endif
    }

    bool isValid() const
    {
	return valid;
    }

    QByteArray data()
    {
	return ba;
    }

private:
    bool valid;
    int arSize;
    char *arData;
    QByteArray ba;
};

static bool createDir( const QString& fullPath )
{
    QStringList hierarchy = QStringList::split( QDir::separator(), fullPath );
    QString pathComponent, tmpPath;
    QDir dirTmp;
    bool success = true;

    for( QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it ) {
	pathComponent = *it + QDir::separator();
	tmpPath += pathComponent;
#if defined(Q_OS_WIN32)
	success = dirTmp.mkdir( tmpPath );
#else
	success = dirTmp.mkdir( QDir::separator() + tmpPath );
#endif
    }
    return success;
}

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f, bool reconfig ) :
    SetupWizard( pParent, pName, modal, f ),
    sysID( 0 ),
    tmpPath( QEnvironment::getTempPath() ),
    filesCopied( false ),
    filesToCompile( 0 ),
    filesCompiled( 0 ),
    reconfigMode( reconfig )
{
#ifdef Q_OS_MACX
    sysID = MACX;
    sysGroup->hide();
#endif
    totalFiles = 0;
    // Disable the HELP button
    setHelpEnabled( introPage, false );
    setHelpEnabled( licensePage, false );
    setHelpEnabled( optionsPage, false );
    setHelpEnabled( foldersPage, false );
    setHelpEnabled( configPage, false );
    setHelpEnabled( progressPage, false );
    setHelpEnabled( buildPage, false );
    setHelpEnabled( finishPage, false );

    setNextEnabled( introPage, false );
    setNextEnabled( licensePage, false );
    setBackEnabled( progressPage, false );
    setNextEnabled( progressPage, false );
    setBackEnabled( buildPage, false );
    setNextEnabled( buildPage, false );
    setFinishEnabled( finishPage, true );
    setBackEnabled( finishPage, false );

    triedToIntegrate = false;

    connect( &autoContTimer, SIGNAL( timeout() ), this, SLOT( timerFired() ) );

    // try to read the archive header information and use them instead of
    // QT_VERSION_STR if possible
    QArchiveHeader *archiveHeader = 0;
    ResourceLoader rcLoader( "QT_ARQ", 500 );	
    if ( rcLoader.isValid() ) {
	// First, try to find qt.arq as a binary resource to the file.
	QArchive ar;
	QDataStream ds( rcLoader.data(), IO_ReadOnly );
	archiveHeader = ar.readArchiveHeader( &ds );
    } else {
	// If the resource could not be loaded or is smaller than 500
	// bytes, we have the dummy qt.arq: try to find and install
	// from qt.arq in the current directory instead.
	QArchive ar;
	QString archiveName = "qt.arq";
# if defined(Q_OS_MACX)
	archiveName  = "install.app/Contents/Qt/qtmac.arq";
# endif
	ar.setPath( archiveName );
	if( ar.open( IO_ReadOnly ) )  {
	    archiveHeader = ar.readArchiveHeader();
	}
    }
    if ( archiveHeader ) {
	qt_version_str = archiveHeader->description();
	if ( qt_version_str.isEmpty() )
	    qt_version_str = QT_VERSION_STR;
#if defined(EVAL)
	if ( archiveHeader->findExtraData( "compiler" ) == "borland" ) {
	    sysGroup->setButton( 1 );
	    clickedSystem( 1 );
	} else
#endif
	{
	    sysGroup->setButton( 0 );
	    clickedSystem( 0 );
	}
	delete archiveHeader;
    } else {
	qt_version_str = QT_VERSION_STR;
    }

    // Optionspage
#if defined(Q_OS_WIN32)
    installPath->setText( QString( "C:\\Qt\\" ) + QString( qt_version_str ).replace( QRegExp("\\s"), "" ).replace( QRegExp("-"), "" ) );
#elif defined(Q_OS_MACX)
    // ### the replace for Windows is done because qmake has problems with
    // spaces and Borland has problems with "-" in the filenames -- I don't
    // think that there is a need for this on Mac (rms)
    installPath->setText( QString( QDir::homeDirPath() + "/" ) + QString( qt_version_str ).replace( QRegExp("-"), "" ) );
#endif
#if defined(EVAL)
    installDocs->setEnabled( FALSE );
    // ### find out which evaluation package (MSVC or Borland) we have
    sysMsvc->setEnabled( FALSE );
    sysBorland->setEnabled( FALSE );

    // Buildpage
    setTitle( buildPage, "Building Qt Examples and Tutorial" );
#endif
    // Folderspage
#if defined(Q_OS_WIN32)
    QByteArray buffer( 256 );
    unsigned long buffSize( buffer.size() );
    GetUserNameA( buffer.data(), &buffSize );
    folderGroups->insertItem( "Anyone who uses this computer (all users)" );
    folderGroups->insertItem( QString( "Only for me (" ) + QString( buffer.data() ) + ")" );
    folderPath->setText( QString( "Qt " ) + qt_version_str );
    if( qWinVersion() & Qt::WV_NT_based )   // On NT we also have a common folder
	folderGroups->setEnabled( true );
    else
	folderGroups->setDisabled( true );
#elif defined(Q_OS_UNIX)
    folderGroups->setDisabled( true );
    removePage( foldersPage );
#endif
#if 0
    //TODO: Confirm with rms these progress display changes are ok
    // ### rms finds them ugly
    filesDisplay->setWordWrap( QTextView::WidgetWidth );
    filesDisplay->setWrapPolicy( QTextView::Anywhere );
    outputDisplay->setWordWrap( QTextView::WidgetWidth );
    outputDisplay->setWrapPolicy( QTextView::Anywhere );
#endif

    if( reconfig ) {
	removePage( introPage );
	removePage( licensePage );
	removePage( foldersPage );
	removePage( optionsPage );
	removePage( progressPage );
	setTitle( configPage, "Reconfigure Qt" );
    }
#if defined(EVAL)
    // ### this page should probably not be removed but all options should be
    // disabled so that the evaluation customer can see how he can configure Qt
    removePage( configPage );
#endif

    // do this rather here than in the showPage() to keep the user's settings
    // when he returns back to the page
    qtDirCheck->setChecked( ( QEnvironment::getEnv( "QTDIR" ).length() == 0 ) );

    // License Page
    setTitle( licensePage, tr("License Information to Install Qt %1").arg(qt_version_str) );
    customerID->setFocus();
#if defined(EVAL)
    // ### improve text
    licenseInfoHeader->setText( tr("Thank you for your intrest in Qt.\n"
		"Please enter the license information you got for this evaluation version of Qt.") );

    customerIDLabel->setText( tr("Name") );
    licenseIDLabel->setText( tr("Company name") );
    licenseeLabel->setText( tr("Serial number") );
    evalName = customerID;
    evalCompany = licenseID;
    evalSerialNumber = licenseeName;

    expiryLabel->hide();
    expiryDate->hide();
    productsLabel->hide();
    productsString->hide();
    keyLabel->hide();
    key->hide();
    readLicenseButton->hide();
#else
    licenseID->setValidator( new QIntValidator( 100000, -1, licenseID ) );
    readLicense( QDir::homeDirPath() + "/.qt-license" );
#  if !defined(MD4_KEYS)
    // expiryDate and productsString comes from the license key
    expiryDate->setEnabled( FALSE );
    productsString->setEnabled( FALSE );
    keyLabel->setText( tr("License key") );
    licenseInfoHeader->setText( tr("Please enter your license information.\n"
		"The License key is required to be able to proceed with the installation process.") );
#  endif
#endif
}

void SetupWizardImpl::stopProcesses()
{
    if( cleaner.isRunning() )
	cleaner.kill();
    if( configure.isRunning() )
	configure.kill();
    if( make.isRunning() )
	make.kill();
    if( integrator.isRunning() )
	integrator.kill();
}

void SetupWizardImpl::clickedPath()
{
    QFileDialog dlg;
    QDir dir( installPath->text() );

#if defined(Q_OS_WIN32)
    if( !dir.exists() )
	dir.setPath( "C:\\" );

    dlg.setDir( dir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	installPath->setText( dlg.dir()->absPath() );
    }
#elif defined(Q_OS_MACX)
    if( !dir.exists() )
	dir.setPath( "/" );

    QString dest =  QFileDialog::getExistingDirectory( installPath->text(), this, NULL, "Select installation directory" );
    if (!dest.isNull())
	installPath->setText( dest );
#endif
}

void SetupWizardImpl::clickedFolderPath()
{
    folderPath->setText( shell.selectFolder( folderPath->text(), ( folderGroups->currentItem() == 0 ) ) );
}

void SetupWizardImpl::clickedDevSysPath()
{
    QFileDialog dlg;
    QDir dir( devSysPath->text() );

    if( !dir.exists() )
	dir.setPath( devSysFolder );

    dlg.setDir( dir );
    dlg.setMode( QFileDialog::DirectoryOnly );
    if( dlg.exec() ) {
	devSysPath->setText( dlg.dir()->absPath() );
    }
}

void SetupWizardImpl::clickedSystem( int sys )
{
    sysID = sys;
}

void SetupWizardImpl::licenseAction( int act )
{
    if( act )
	setNextEnabled( introPage, false );
    else
	setNextEnabled( introPage, true );
}

void SetupWizardImpl::readCleanerOutput()
{
    updateOutputDisplay( &cleaner );
}

void SetupWizardImpl::readConfigureOutput()
{
    updateOutputDisplay( &configure );
}

void SetupWizardImpl::readMakeOutput()
{
    updateOutputDisplay( &make );
}

void SetupWizardImpl::readIntegratorOutput()
{
    updateOutputDisplay( &integrator );
}

void SetupWizardImpl::readCleanerError()
{
    updateOutputDisplay( &cleaner );
}

void SetupWizardImpl::readConfigureError()
{
    updateOutputDisplay( &configure );
}

void SetupWizardImpl::readMakeError()
{
    updateOutputDisplay( &make );
}

void SetupWizardImpl::readIntegratorError()
{
    updateOutputDisplay( &integrator );
}

void SetupWizardImpl::updateOutputDisplay( QProcess* proc )
{
    QString outbuffer;

    outbuffer = QString( proc->readStdout() );
    
    for( int i = 0; i < (int)outbuffer.length(); i++ ) {
	QChar c = outbuffer[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    currentOLine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    if( currentOLine.length() ) {
		if( currentOLine.right( 4 ) == ".cpp" || 
		    currentOLine.right( 2 ) == ".c" ||
		    currentOLine.right( 4 ) == ".pro" ||
		    currentOLine.right( 3 ) == ".ui" )
		    compileProgress->setProgress( ++filesCompiled );

		logOutput( currentOLine );
		currentOLine = "";
	    }
	    break;
	default:
	    currentOLine += c;
	    break;
	}
    }
}

void SetupWizardImpl::updateErrorDisplay( QProcess* proc )
{
    QString outbuffer;

    outbuffer = QString( proc->readStderr() );
    
    for( int i = 0; i < (int)outbuffer.length(); i++ ) {
	QChar c = outbuffer[ i ];
	switch( char( c ) ) {
	case '\r':
	case 0x00:
	    break;
	case '\t':
	    currentELine += "    ";  // Simulate a TAB by using 4 spaces
	    break;
	case '\n':
	    if( currentELine.length() ) {
		if( currentOLine.right( 4 ) == ".cpp" || 
		    currentOLine.right( 2 ) == ".c" || 
		    currentOLine.right( 4 ) == ".pro" )
		    compileProgress->setProgress( ++filesCompiled );

		logOutput( currentELine );
		currentELine = "";
	    }
	    break;
	default:
	    currentELine += c;
	    break;
	}
    }
}

#if defined(Q_OS_WIN32)
void SetupWizardImpl::installIcons( const QString& iconFolder, const QString& dirName, bool common )
{
    QDir dir( dirName );

    dir.setSorting( QDir::Name | QDir::IgnoreCase );
    const QFileInfoList* filist = dir.entryInfoList();
    QFileInfoListIterator it( *filist );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[0] != '.' ) { // Exclude dot-dirs
	    if( fi->isDir() ) {
		installIcons( iconFolder, fi->absFilePath(), common );
	    } else if( fi->fileName().right( 4 ) == ".exe" ) {
		shell.createShortcut( iconFolder, common, fi->baseName(), fi->absFilePath() );
	    }
	}
	++it;
    }
}
#endif

void SetupWizardImpl::doFinalIntegration()
{
    compileProgress->setProgress( compileProgress->totalSteps() );
#if defined(Q_OS_WIN32)
    QString dirName, examplesName, tutorialsName;
    bool common( folderGroups->currentItem() == 0 );
    QString qtDir = QEnvironment::getEnv( "QTDIR" );

    switch( sysID ) {
    case MSVC:
	{
	    QFile autoexp( devSysPath->text() + "\\Common\\MsDev98\\bin\\autoexp.dat" );
	    if ( !autoexp.exists() ) {
		autoexp.open( IO_WriteOnly );
	    } else {
		autoexp.open( IO_ReadOnly );
		QString existingUserType = autoexp.readAll();
		autoexp.close();
		if ( existingUserType.find( "; Trolltech Qt" ) == -1 )
		    autoexp.open( IO_WriteOnly | IO_Append );
	    }

	    if( autoexp.isOpen() ) { // First try to open the file to search for existing installations
		QTextStream outstream( &autoexp );
		outstream << "; Trolltech Qt" << endl;
		outstream << "QString=<d->unicode,su> len=<d->len,u>" << endl;
		outstream << "QCString =<shd->data, s>" << endl;
		outstream << "QPoint =x=<xp> y=<yp>" << endl;
		outstream << "QRect =x1=<x1> y1=<y1> x2=<x2> y2=<y2>" << endl;
		outstream << "QSize =width=<wd> height=<ht>" << endl;
		outstream << "QWMatrix =m11=<_m11> m12=<_m12> m21=<_m21> m22=<_m22> dx=<_dx> dy=<_dy>" << endl;
		outstream << "QVariant =Type=<d->typ> value=<d->value>" << endl;
		outstream << "QValueList<*> =Count=<sh->nodes>" << endl;
		outstream << "QPtrList<*> =Count=<numNodes>" << endl;
		outstream << "QGuardedPtr<*> =ptr=<priv->obj>" << endl;
		outstream << "QEvent =type=<t>" << endl;
		outstream << "QObject =class=<metaObj->classname,s> name=<objname,s>" << endl;
		autoexp.close();
	    }

	    QFile usertype( devSysPath->text() + "\\Common\\MsDev98\\bin\\usertype.dat" );
	    if ( !usertype.exists() ) {
		usertype.open( IO_WriteOnly );
	    } else {
		usertype.open( IO_ReadOnly );
		QString existingUserType = usertype.readAll();
		usertype.close();
		if ( existingUserType.find( "Q_OBJECT" ) == -1 )
		    usertype.open( IO_WriteOnly | IO_Append );
	    }
	    if ( usertype.isOpen() ) {
		QTextStream outstream( &usertype );
		outstream << "Q_OBJECT" << endl;
		outstream << "Q_PROPERTY" << endl;
		outstream << "Q_ENUMS" << endl;
		outstream << "emit" << endl;
		outstream << "TRUE" << endl;
		outstream << "FALSE" << endl;
		outstream << "SIGNAL" << endl;
		outstream << "SLOT" << endl;
		outstream << "signals:" << endl;
		outstream << "slots:" << endl;
		usertype.close();
	    }
	}
	break;
    }
    /*
    ** Set up our icon folder and populate it with shortcuts.
    ** Then move to the next page.
    */
    dirName = shell.createFolder( folderPath->text(), common );
    shell.createShortcut( dirName, common, "Designer", qtDir + "\\bin\\designer.exe", "GUI designer", "", qtDir );
#if !defined(EVAL)
    shell.createShortcut( dirName, common, "Reconfigure Qt", qtDir + "\\bin\\install.exe", "Reconfigure the Qt library", "-reconfig", qtDir );
#endif
    shell.createShortcut( dirName, common, "License agreement", "notepad.exe", "Review the license agreement", QString( "\"" ) + qtDir + "\\LICENSE\"" );
    shell.createShortcut( dirName, common, "On-line documentation", qtDir + "\\bin\\assistant.exe", "Browse the On-line documentation", "", qtDir );
    shell.createShortcut( dirName, common, "Linguist", qtDir + "\\bin\\linguist.exe", "Qt translation utility", "", qtDir );
    if( qWinVersion() & WV_DOS_based ) {
	QString description;
#if defined(EVAL)
	buildQtShortcutText = "Build Qt Examples and Tutorials";
	description = "Build the Qt Examples and Tutorials";
#else
	buildQtShortcutText = "Build Qt " + qt_version_str;
	description = "Build the Qt library";
#endif
	shell.createShortcut( dirName, common,
	    buildQtShortcutText,
	    QEnvironment::getEnv( "QTDIR" ) + "\\build.bat",
	    description );
    }

    if( installTutorials->isChecked() ) {
	if( qWinVersion() & WV_DOS_based ) {
	    shell.createShortcut( dirName, common,
		"Tutorials",
		QEnvironment::getEnv( "QTDIR" ) + "\\tutorial" );
	} else {
	    tutorialsName = shell.createFolder( folderPath->text() + "\\Tutorials", common );
	    installIcons( tutorialsName, QEnvironment::getEnv( "QTDIR" ) + "\\tutorial", common );
	}
    }
    if( installExamples->isChecked() ) {
	if( qWinVersion() & WV_DOS_based ) {
	    shell.createShortcut( dirName, common,
		"Examples",
		QEnvironment::getEnv( "QTDIR" ) + "\\examples" );
	} else {
	    examplesName = shell.createFolder( folderPath->text() + "\\Examples", common );
	    installIcons( examplesName, QEnvironment::getEnv( "QTDIR" ) + "\\examples", common );
	}
    }
    /*
    ** Then record the installation in the registry, and set up the uninstallation
    */
    QStringList uninstaller;
    uninstaller << ( shell.windowsFolderName + "\\quninstall.exe" );
    uninstaller << installPath->text();

    if( common )
	uninstaller << ( QString( "\"" ) + shell.commonProgramsFolderName + QString( "\\" ) + folderPath->text() + QString( "\"" ) );
    else
	uninstaller << ( QString( "\"" ) + shell.localProgramsFolderName + QString( "\\" ) + folderPath->text() + QString( "\"" ) );

    uninstaller << qt_version_str;

    QEnvironment::recordUninstall( QString( "Qt " ) + qt_version_str, uninstaller.join( " " ) );
#endif
}

void SetupWizardImpl::integratorDone()
{
    compileProgress->setTotalSteps( compileProgress->totalSteps() );
    if( ( !integrator.normalExit() || ( integrator.normalExit() && integrator.exitStatus() ) ) && ( triedToIntegrate ) )
	logOutput( "The integration process failed.\n", true );
    else {
	setNextEnabled( buildPage, true );

	/*
	** We still have some more items to do in order to finish all the
	** integration stuff.
	*/
	doFinalIntegration();
	setNextEnabled( buildPage, true );

// The automatic continue feature has been disabled
//	timeCounter = 30;
//	autoContTimer.start( 1000 );
	logOutput( "The build was successful", true );
    }
}

void SetupWizardImpl::makeDone()
{
    QStringList args;

    if( reconfigMode )
	showPage( finishPage );

    if( !make.normalExit() || ( make.normalExit() && make.exitStatus() ) ) {
	logOutput( "The build process failed!\n" );
	QMessageBox::critical( this, "Error", "The build process failed!" );
//	removePage( progressPage );
	setAppropriate( progressPage, false );
    } else {
	compileProgress->setProgress( compileProgress->totalSteps() );

	if( ( sysID != MSVC ) ||
	    ( !findFileInPaths( "atlbase.h", QStringList::split( ";", QEnvironment::getEnv( "INCLUDE" ) ) ) &&
	      !findFileInPaths( "afxwin.h", QStringList::split( ";", QEnvironment::getEnv( "INCLUDE" ) ) ) ) )
	    integratorDone();
	else {
	    connect( &integrator, SIGNAL( processExited() ), this, SLOT( integratorDone() ) );
	    connect( &integrator, SIGNAL( readyReadStdout() ), this, SLOT( readIntegratorOutput() ) );
	    connect( &integrator, SIGNAL( readyReadStderr() ), this, SLOT( readIntegratorError() ) );

	    args << "nmake";

	    integrator.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) + "\\Tools\\Designer\\Integration\\QMsDev" );
	    integrator.setArguments( args );
	    triedToIntegrate = true;
	    if( !integrator.start() )
		logOutput( "Could not start integrator process" );
	}
    }
}

void SetupWizardImpl::configDone()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make" );
    QStringList args;

    if( reconfigMode && !rebuildInstallation->isChecked() )
	showPage( finishPage );

#if !defined(EVAL)
    if( !configure.normalExit() || ( configure.normalExit() && configure.exitStatus() ) )
	logOutput( "The configure process failed.\n" );
    else
#endif
    {
	connect( &make, SIGNAL( processExited() ), this, SLOT( makeDone() ) );
	connect( &make, SIGNAL( readyReadStdout() ), this, SLOT( readMakeOutput() ) );
	connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeError() ) );

	args << makeCmds[ sysID ];

	make.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	make.setArguments( args );

	if( !make.start() ) {
	    logOutput( "Could not start make process" );
	    backButton()->setEnabled( TRUE );
	}
    }
}

void SetupWizardImpl::saveSettings()
{
#if !defined(EVAL)
    QApplication::setOverrideCursor( Qt::waitCursor );
    saveSet( configList );
    saveSet( advancedList );
    QApplication::restoreOverrideCursor();
#endif
}

void SetupWizardImpl::saveSet( QListView* list )
{
    QSettings settings;
    settings.writeEntry( "/Trolltech/Qt/ResetDefaults", "FALSE" );

    QListViewItemIterator it( list );
    while ( it.current() ) {
	QListViewItem *itm = it.current();
	++it;
	if ( itm->rtti() != QCheckListItem::RTTI )
	    continue;
	QCheckListItem *item = (QCheckListItem*)itm;
	if ( item->type() == QCheckListItem::RadioButton ) {
	    if ( item->isOn() ) {
		QString folder;
		QListViewItem *pItem = item;
		while ( (pItem = pItem->parent() ) ) {
		    if ( folder.isEmpty() )
			folder = pItem->text( 0 );
		    else
			folder = pItem->text(0) + "/" + folder;
		}

		settings.writeEntry( "/Trolltech/Qt/" + folder, item->text() );
	    }
	} else if ( item->type() == QCheckListItem::CheckBox ) {
	    QStringList lst;
	    QListViewItem *p = item->parent();
	    if ( p )
		--it;
	    QString c = p->text( 0 );
	    while ( ( itm = it.current() ) &&
		itm->rtti() == QCheckListItem::RTTI &&
		item->type() == QCheckListItem::CheckBox ) {
		item = (QCheckListItem*)itm;
		++it;
		if ( item->isOn() )
		    lst << item->text( 0 );
	    }
	    if ( lst.count() )
		settings.writeEntry( "/Trolltech/Qt/" + p->text(0), lst, ',' );
	}
    }
}

#if defined(Q_OS_UNIX)
static bool copyFile( const QString& src, const QString& dest )
{
    int len;
    const int buflen = 4096;
    char buf[buflen];
    QFileInfo info( src );
    QFile srcFile( src ), destFile( dest );
    if (!srcFile.open( IO_ReadOnly ))
	return false;
    destFile.remove();
    if (!destFile.open( IO_WriteOnly )) {
	srcFile.close();
	return false;
    }
    while (!srcFile.atEnd()) {
	len = srcFile.readBlock( buf, buflen );
	if (len <= 0)
	    break;
	if (destFile.writeBlock( buf, len ) != len)
	    return false;
    }
    destFile.flush();
    return true;
}
#endif

void SetupWizardImpl::showPage( QWidget* newPage )
{
    SetupWizard::showPage( newPage );

    if( newPage == licensePage ) {
	setInstallStep( 1 );
	showPageLicense();
    } else if( newPage == introPage ) {
	setInstallStep( 2 );
	readLicenseAgreement();
    } else if( newPage == optionsPage ) {
	setInstallStep( 3 );
    } else if( newPage == foldersPage ) {
	setInstallStep( 4 );
	showPageFolders();
    } else if( newPage == configPage ) {
	if( reconfigMode )
	    setInstallStep( 1 );
	else
	    setInstallStep( 5 );
	showPageConfig();
    } else if( newPage == progressPage ) {
#if defined(EVAL)
	setInstallStep( 5 );
#else
	setInstallStep( 6 );
#endif
	showPageProgress();
    } else if( newPage == buildPage ) {
#if defined(EVAL)
	setInstallStep( 6 );
#else
	if( reconfigMode )
	    setInstallStep( 2 );
	else
	    setInstallStep( 7 );
#endif
	showPageBuild();
    } else if( newPage == finishPage ) {
#if defined(EVAL)
	setInstallStep( 7 );
#else
	if( reconfigMode )
	    setInstallStep( 3 );
	else
	    setInstallStep( 8 );
#endif
	showPageFinish();
    }
}

void SetupWizardImpl::showPageLicense()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake.exe make.exe gmake.exe make" );
#if defined(Q_OS_WIN32)
    QStringList paths = QStringList::split( QRegExp("[;,]"), QEnvironment::getEnv( "PATH" ) );
#elif defined(Q_OS_UNIX)
    QStringList paths = QStringList::split( QRegExp("[:]"), QEnvironment::getEnv( "PATH" ) );
#endif
    if( !findFileInPaths( makeCmds[ sysID ], paths ) ) {
	setNextEnabled( licensePage, false );
	QMessageBox::critical( this, "Environment problems",
				     "The installation program can't find the make command '"
				     + makeCmds[ sysID ] +
				     "'.\nMake sure the path to it "
				     "is present in the PATH environment variable.\n"
				     "The installation can't continue." );
    } else {
	licenseChanged();
    }
}

void SetupWizardImpl::showPageFolders()
{
    QStringList devSys = QStringList::split( ';',"Microsoft Visual Studio path;Borland C++ Builder path;GNU C++ path" );

    devSysLabel->setText( devSys[ sysID ] );
    devSysPath->setEnabled( sysID == 0 );
    devSysPathButton->setEnabled( sysID == 0 );
#if defined(Q_OS_WIN32)
    if( sysID == 0 )
	devSysPath->setText( QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual Studio", "ProductDir", QEnvironment::LocalMachine ) );
#endif
}

void SetupWizardImpl::showPageProgress()
{
    saveSettings();
    int totalSize = 0;
    QFileInfo fi;
    totalRead = 0;
    bool copySuccessful = true;

    if( !filesCopied ) {
	createDir( installPath->text() );
	filesDisplay->append( "Installing files...\n" );
#if !defined(MD4_KEYS)
	// install the right LICENSE file
	QDir installDir( installPath->text() );
	QFile licenseFile( installDir.filePath("LICENSE") );
	if ( licenseFile.open( IO_WriteOnly ) ) {
	    ResourceLoader *rcLoader;
	    if ( usLicense ) {
		rcLoader = new ResourceLoader( "LICENSE-US" );
	    } else {
		rcLoader = new ResourceLoader( "LICENSE" );
	    }
	    if ( rcLoader->isValid() ) {
		licenseFile.writeBlock( rcLoader->data() );
	    } else {
		QMessageBox::critical( this, tr("Package corrupted"),
			tr("Could not find the LICENSE file in the package.\nThe package might be corrupted.") );
	    }
	    delete rcLoader;
	    licenseFile.close();
	} else {
	    // ### error handling -- we could not write the LICENSE file
	}

	// Install the files -- use different fallbacks if one method failed.
	QArchive ar;
	ar.setVerbosity( QArchive::Destination | QArchive::Verbose | QArchive::Progress );
	connect( &ar, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( archiveMsg( const QString& ) ) );
	connect( &ar, SIGNAL( operationFeedback( int ) ), operationProgress, SLOT( setProgress( int ) ) );
	// First, try to find qt.arq as a binary resource to the file.
	ResourceLoader rcLoader( "QT_ARQ", 500 );	
	if ( rcLoader.isValid() ) {
	    operationProgress->setTotalSteps( rcLoader.data().count() );
	    QDataStream ds( rcLoader.data(), IO_ReadOnly );
	    ar.readArchive( &ds, installPath->text(), key->text() );
	} else {
	    // If the resource could not be loaded or is smaller than 500
	    // bytes, we have the dummy qt.arq: try to find and install
	    // from qt.arq in the current directory instead.
	    QString archiveName = "qt.arq";
# if defined(Q_OS_MACX)
	    archiveName  = "install.app/Contents/Qt/qtmac.arq";
# endif
	    fi.setFile( archiveName );
	    if( fi.exists() )
		totalSize = fi.size();
	    operationProgress->setTotalSteps( totalSize );

	    ar.setPath( archiveName );
	    if( ar.open( IO_ReadOnly ) )  {
		ar.readArchive( installPath->text(), key->text() );
# if defined(Q_OS_MACX)
 		QString srcName  = "install.app/Contents/Qt/LICENSE";
    		QString destName = "/.LICENSE";
    		QString srcName2 = srcName;
    		if (featuresForKeyOnUnix( key->text() ) & Feature_US)
        	    srcName2 += "-US";
    		if((!copyFile( srcName, installPath->text() + destName )) ||
       		   (!copyFile( srcName + "-US", installPath->text() + destName + "-US" )) ||
       		   (!copyFile( srcName2, installPath->text() + "/LICENSE" )))
		    QMessageBox::critical( this, "Installation Error",
					   "License files could not be copied." );
# endif
	    } else {
		// We were not able to find any qt.arq -- so assume we have
		// the old fashioned zip archive and simply copy the files
		// instead.
#endif
		operationProgress->setTotalSteps( FILESTOCOPY );
		copySuccessful = copyFiles( QDir::currentDirPath(), installPath->text(), true );

#if 0
		// ### what is the purpose of this? Can't we put it right in
		// the package in first place?
		{
		    QFile inFile( installPath->text() + "\\bin\\quninstall.exe" );
		    QFile outFile( shell.windowsFolderName + "\\quninstall.exe" );
		    QFileInfo fi( inFile );
		    QByteArray buffer( fi.size() );

		    if( buffer.size() ) {
			if( inFile.open( IO_ReadOnly ) ) {
			    if( outFile.open( IO_WriteOnly ) ) {
				inFile.readBlock( buffer.data(), buffer.size() );
				outFile.writeBlock( buffer.data(), buffer.size() );
				outFile.close();
			    }
			    inFile.close();
			}
		    }
		}
#endif

		/*These lines are only to be used when changing the filecount estimate
		  QString tmp( "%1" );
		  tmp = tmp.arg( totalFiles );
		  QMessageBox::information( this, tmp, tmp );
		 */
		operationProgress->setProgress( FILESTOCOPY );
	    }
#if !defined(MD4_KEYS)
	}
#endif
#if 0
	// ### what is the purpose of this? Can't we put it right in the
	// package in first place?
	{
	    QFile inFile( "install.exe" );
	    QFile outFile( installPath->text() + "\\bin\\Install.exe" );
	    QFileInfo fi( inFile );
	    QByteArray buffer( fi.size() );

	    if( buffer.size() ) {
		if( inFile.open( IO_ReadOnly ) ) {
		    if( outFile.open( IO_WriteOnly ) ) {
			inFile.readBlock( buffer.data(), buffer.size() );
			outFile.writeBlock( buffer.data(), buffer.size() );
			outFile.close();
		    }
		    inFile.close();
		}
	    }
	}
#endif
	filesCopied = copySuccessful;

	timeCounter = 30;
	if( copySuccessful ) {
#if defined(EVAL)
	    // patch qt-mt.lib (or qtmt.lib under Borland)
	    QString qtLib;
	    if ( sysID == MSVC ) {
		qtLib = "\\lib\\qt-mt.lib";
	    } else {
		qtLib = "\\lib\\qtmt.lib";
	    }
	    int ret = trDoIt( installPath->text() + qtLib,
		    evalName->text().latin1(),
		    evalCompany->text().latin1(),
		    evalSerialNumber->text().latin1() );
	    if ( ret != 0 ) {
		copySuccessful = FALSE;
		QMessageBox::critical( this,
			tr( "Error patching Qt library" ),
			tr( "Could not patch the Qt library with the evaluation\n"
			    "license information. You will not be able to execute\n"
			    "any program linked against qt-mt.lib." ) );
	    }
#endif
	    logFiles( tr("All files have been installed.\n"
			 "This log has been saved to the installation directory.\n"
			 "The build will start automatically in 30 seconds."), true );
	} else {
	    logFiles( tr("One or more errors occurred during file installation.\n"
			 "Please review the log and try to amend the situation.\n"), true );
	}
    }
    if ( copySuccessful )
	autoContTimer.start( 1000 );
    setNextEnabled( progressPage, copySuccessful );
}

void SetupWizardImpl::showPageFinish()
{
    autoContTimer.stop();
    nextButton()->setText( "Next >" );
    QString finishMsg;
#if defined(Q_OS_WIN32)
    if( qWinVersion() & WV_NT_based ) {
#elif defined(Q_OS_UNIX)
    if( true ) {
#endif
	if( reconfigMode ) {
	    if( !rebuildInstallation->isChecked() )
		finishMsg = "Qt has been reconfigured, and is ready to be rebuilt.";
	    else
		finishMsg = "Qt has been reconfigured and rebuilt, and is ready for use.";
	}
	else {
	    finishMsg = QString( "Qt has been installed to " ) + installPath->text() + " and is ready to use.";
#if defined(Q_OS_WIN32)
            finishMsg += "\nYou may need to reboot, or open";
	    finishMsg += " the environment editing dialog to make changes to the environment visible";
#endif
	}
    } else {
	if( reconfigMode ) {
		finishMsg = "The new configuration has been written.\nThe library needs to be rebuilt to activate the ";
		finishMsg += "new configuration.";
#if defined(Q_OS_WIN32)
                finishMsg += "To rebuild it, use the \"Build Qt ";
		finishMsg += qt_version_str;
		finishMsg += "\" icon in the Qt program group in the start menu.";
#endif
	}
	else {
	    finishMsg = QString( "The Qt files have been installed to " ) + installPath->text() + " and is ready to be compiled.\n";
#if defined(Q_OS_WIN32)
	    if( persistentEnv ) {
		finishMsg += "The environment variables needed to use Qt have been recorded into your AUTOEXEC.BAT file.\n";
		finishMsg += "Please review this file, and take action as appropriate depending on your operating system to get them into the persistent environment. (Windows Me users, run MsConfig)\n\n";
	    }
#  if defined(EVAL)
	    finishMsg += QString( "To build the examples and tutorials, use the"
				  "\"Build the Qt Examples and Tutorials\""
				  "icon which has been installed into your Start-Menu." );
#  else
	    finishMsg += QString( "To build Qt, use the"
				  "\"Build Qt " ) + qt_version_str + "\""
				  "icon which has been installed into your Start-Menu.";
#  endif
#endif
	}
    }
    finishText->setText( finishMsg );
}

void SetupWizardImpl::setStaticEnabled( bool se )
{
    bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";
    if ( se ) {
	if ( accOn->isOn() ) {
	    accOn->setOn( false );
	    accOff->setOn( true );
	}
	if ( bigCodecsOff->isOn() ) {
	    bigCodecsOn->setOn( true );
	    bigCodecsOff->setOn( false );
	}
	if ( mngPlugin->isOn() ) {
	    mngDirect->setOn( true );
	    mngPlugin->setOn( false );
	    mngOff->setOn( false );
	}
	if ( pngPlugin->isOn() ) {
	    pngDirect->setOn( true );
	    pngPlugin->setOn( false );
	    pngOff->setOn( false );
	}
	if ( jpegPlugin->isOn() ) {
	    jpegDirect->setOn( true );
	    jpegPlugin->setOn( false );
	    jpegOff->setOn( false );
	}
	if ( sgiPlugin->isOn() ) {
	    sgiPlugin->setOn( false );
	    sgiDirect->setOn( true );
	}
	if ( cdePlugin->isOn() ) {
	    cdePlugin->setOn( false );
	    cdeDirect->setOn( true );
	}
	if ( motifplusPlugin->isOn() ) {
	    motifplusPlugin->setOn( false );
	    motifplusDirect->setOn( true );
	}
	if ( motifPlugin->isOn() ) {
	    motifPlugin->setOn( false );
	    motifDirect->setOn( true );
	}
	if ( platinumPlugin->isOn() ) {
	    platinumPlugin->setOn( false );
	    platinumDirect->setOn( true );
	}
	if ( enterprise ) {
	    if ( mysqlPlugin->isOn() ) {
		mysqlPlugin->setOn( false );
		mysqlDirect->setOn( true );
	    }
	    if ( ociPlugin->isOn() ) {
		ociPlugin->setOn( false );
		ociDirect->setOn( true );
	    }
	    if ( odbcPlugin->isOn() ) {
		odbcPlugin->setOn( false );
		odbcDirect->setOn( true );
	    }
	    if ( psqlPlugin->isOn() ) {
		psqlPlugin->setOn( false );
		psqlDirect->setOn( true );
	    }
	    if ( tdsPlugin->isOn() ) {
		tdsPlugin->setOn( false );
		tdsDirect->setOn( true );
	    }
	}

	accOn->setEnabled( false );
	bigCodecsOff->setEnabled( false );
	mngPlugin->setEnabled( false );
	pngPlugin->setEnabled( false );
	jpegPlugin->setEnabled( false );
	sgiPlugin->setEnabled( false );
	cdePlugin->setEnabled( false );
	motifPlugin->setEnabled( false );
	motifplusPlugin->setEnabled( false );
	motifPlugin->setEnabled( false );
	platinumPlugin->setEnabled( false );
	if ( enterprise ) {
	    mysqlPlugin->setEnabled( false );
	    ociPlugin->setEnabled( false );
	    odbcPlugin->setEnabled( false );
	    psqlPlugin->setEnabled( false );
	    tdsPlugin->setEnabled( false );
	}
    } else {
	accOn->setEnabled( true );
	bigCodecsOff->setEnabled( true );
	mngPlugin->setEnabled( true );
	pngPlugin->setEnabled( true );
	jpegPlugin->setEnabled( true );
	sgiPlugin->setEnabled( true );
	cdePlugin->setEnabled( true );
	motifplusPlugin->setEnabled( true );
	motifPlugin->setEnabled( true );
	platinumPlugin->setEnabled( true );
	if ( enterprise ) {
	    mysqlPlugin->setEnabled( true );
	    ociPlugin->setEnabled( true );
	    odbcPlugin->setEnabled( true );
	    psqlPlugin->setEnabled( true );
	    tdsPlugin->setEnabled( true );
	}
    }
    setJpegDirect( mngDirect->isOn() );
}

void SetupWizardImpl::setJpegDirect( bool jd )
{
    // direct MNG support requires also direct JPEG support
    if ( jd ) {
	jpegOff->setOn( FALSE );
	jpegPlugin->setOn( FALSE );
	jpegDirect->setOn( TRUE );

	jpegOff->setEnabled( FALSE );
	jpegPlugin->setEnabled( FALSE );
	jpegDirect->setEnabled( TRUE );
    } else {
	jpegOff->setEnabled( TRUE );
	if ( !staticItem->isOn() )
	    jpegPlugin->setEnabled( TRUE );
	jpegDirect->setEnabled( TRUE );
    }
}

void SetupWizardImpl::optionClicked( QListViewItem *i )
{
    if ( !i || i->rtti() != QCheckListItem::RTTI )
	return;

    QCheckListItem *item = (QCheckListItem*)i;
    if ( item->type() != QCheckListItem::RadioButton )
	return;

    if ( item->text(0) == "Static" && item->isOn() ) {
	setStaticEnabled( TRUE );
	if ( !QMessageBox::information( this, "Are you sure?", "It will not be possible to build components "
				  "or plugins if you select the static build of the Qt library.\n"
				  "New features, e.g souce code editing in Qt Designer, will not "
				  "be available, "
				  "\nand you or users of your software might not be able "
				  "to use all or new features, e.g. new styles.\n\n"
				  "Are you sure you want to build a static Qt library?",
				  "No, I want to use the cool new stuff", "Yes" ) ) {
		item->setOn( FALSE );
		if ( ( item = (QCheckListItem*)configList->findItem( "Shared", 0, 0 ) ) ) {
		item->setOn( TRUE );
		configList->setCurrentItem( item );
		setStaticEnabled( FALSE );
	    }
	}
	return;
    } else if ( item->text( 0 ) == "Shared" && item->isOn() ) {
	setStaticEnabled( FALSE );
	if( ( (QCheckListItem*)configList->findItem( "Non-threaded", 0, 0 ) )->isOn() ) {
	    if( QMessageBox::information( this, "Are you sure?", "Single-threaded, shared configurations "
								 "may cause instabilities because of runtime "
								 "library conflicts.", "OK", "Revert" ) ) {
		item->setOn( FALSE );
		if( ( item = (QCheckListItem*)configList->findItem( "Static", 0, 0 ) ) ) {
		    item->setOn( TRUE );
		    configList->setCurrentItem( item );
		    setStaticEnabled( TRUE );
		}
	    }
	}
    }
    else if( item->text( 0 ) == "Non-threaded" && item->isOn() ) {
	if( ( (QCheckListItem*)configList->findItem( "Shared", 0, 0 ) )->isOn() ) {
	    if( QMessageBox::information( this, "Are you sure?", "Single-threaded, shared configurations "
								 "may cause instabilities because of runtime "
								 "library conflicts.", "OK", "Revert" ) ) {
		item->setOn( FALSE );
		if( (item = (QCheckListItem*)configList->findItem( "Threaded", 0, 0 ) ) ) {
		    item->setOn( TRUE );
		    configList->setCurrentItem( item );
		}
	    }
	}
    } else if ( item==mngDirect || item==mngPlugin || item==mngOff ) {
	setJpegDirect( mngDirect->isOn() );
    }
}

void SetupWizardImpl::configPageChanged()
{
    if ( configList->isVisible() ) {
	configList->setSelected( configList->currentItem(), true );
	optionSelected( configList->currentItem() );
    } else if ( advancedList->isVisible() ) {
	advancedList->setSelected( advancedList->currentItem(), true );
	optionSelected( advancedList->currentItem() );
    }
}

void SetupWizardImpl::licenseChanged()
{
#if defined(EVAL)
    int ret = trCheckIt( evalName->text().latin1(),
	    evalCompany->text().latin1(),
	    evalSerialNumber->text().latin1() );

    if ( ret == 0 )
	setNextEnabled( licensePage, TRUE );
    else
	setNextEnabled( licensePage, FALSE );
    return;
#else
#  if defined(MD4_KEYS)
    QString proKey = key->text();
    if ( proKey.length() < 19 ) {
	setNextEnabled( licensePage, FALSE );
	return;
    }
    QString customer = customerID->text();
    QString license = licenseID->text();
    QString name = licenseeName->text();
    QString date = expiryDate->text();
    QString products = productsString->currentText();
    int i = proKey.find( '-' );
    if ( i <= 0 || i >= (int)proKey.length()-1 ) {
	setNextEnabled( licensePage, FALSE );
	return;
    }

    QString platforms = proKey.left( i );
    if ( platforms.contains('W') <= 0 ) {
	setNextEnabled( licensePage, FALSE );
	return;
    }

    QString keyString = customer + license + products + date + platforms;
    QMd4Sum sum( keyString );
    setNextEnabled( licensePage, sum.scrambled() == proKey.mid( i+1, 17 ) );
#  else
    QDate date;
    uint features;
    QString licenseKey = key->text();
    if ( licenseKey.length() != 14 ) {
	goto rejectLicense;
    }
    features = featuresForKey( licenseKey.upper() );
    date = decodedExpiryDate( licenseKey.mid(9) );
    if ( !date.isValid() ) {
	goto rejectLicense;
    }
#    ifdef Q_OS_MACX
    if ( !(features&Feature_Mac) ) {
	if ( features & (Feature_Unix|Feature_Windows|Feature_Windows|Feature_Embedded) ) {
	    int ret = QMessageBox::information( this,
		    tr("No Mac OS X license"),
		    tr("You are not licensed for the Mac OS X platform.\n"
		       "Please contact sales@trolltech.com to upgrade\n"
		       "your license to include the Mac OS X platform."),
		    tr("Try again"),
		    tr("Abort installation")
		    );
	    if ( ret == 1 ) {
		QApplication::exit();
	    } else {
		key->setText( "" );
	    }
	}
	goto rejectLicense;
    }
#    elif defined(Q_OS_WIN32)
    if ( !(features&Feature_Windows) ) {
	if ( features & (Feature_Unix|Feature_Windows|Feature_Mac|Feature_Embedded) ) {
	    int ret = QMessageBox::information( this,
		    tr("No Windows license"),
		    tr("You are not licensed for the Windows platform.\n"
		       "Please contact sales@trolltech.com to upgrade\n"
		       "your license to include the Windows platform."),
		    tr("Try again"),
		    tr("Abort installation")
		    );
	    if ( ret == 1 ) {
		QApplication::exit();
	    } else {
		key->setText( "" );
	    }
	}
	goto rejectLicense;
    }
#    endif
    if ( date < QDate::currentDate() ) {
	static bool alreadyShown = FALSE;
	if ( !alreadyShown ) {
	    QMessageBox::warning( this,
		    tr("Support and upgrade period has expired"),
		    tr("Your support and upgrade period has expired.\n"
			"\n"
			"You may continue to use your last licensed release\n"
			"of Qt under the terms of your existing license\n"
			"agreement. But you are not entitled to technical\n"
			"support, nor are you entitled to use any more recent\n"
			"Qt releases.\n"
			"\n"
			"Please contact sales@trolltech.com to renew your\n"
			"support and upgrades for this license.")
		    );
	    alreadyShown = TRUE;
	}
    }
    if ( features & Feature_US )
	usLicense = TRUE;
    else
	usLicense = FALSE;

    expiryDate->setText( date.toString( Qt::ISODate ) );
    if( features & Feature_Enterprise )
	productsString->setCurrentItem( 1 );
    else
	productsString->setCurrentItem( 0 );
    setNextEnabled( licensePage, TRUE );
    return;

rejectLicense:
    expiryDate->setText( "" );
#    if defined(Q_OS_WIN32)
//TODO: Is this a bug? It bus errors on MACX, ask rms.
     productsString->setCurrentItem( -1 );
#    endif
    setNextEnabled( licensePage, FALSE );
    return;
#  endif
#endif
}

void SetupWizardImpl::logFiles( const QString& entry, bool close )
{
    if( !fileLog.isOpen() ) {
	fileLog.setName( installPath->text() + QDir::separator() + "install.log" );
	if( !fileLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    QTextStream outstream( &fileLog );

    // ######### At the moment, there is a bug in QTextEdit. The log output
    // will cause the application to grow that much that we can't install the
    // evaluation packages on lusa -- for now, disable the output that we get
    // the packages out.
#if 0
    filesDisplay->append( entry + "\n" );
#else
    filesDisplay->setText( "Installing files...\n" + entry + "\n" );
#endif
    outstream << ( entry + "\n" );

    if( close )
	fileLog.close();
}

void SetupWizardImpl::logOutput( const QString& entry, bool close )
{
    static QTextStream outstream;
    if( !outputLog.isOpen() ) {
	outputLog.setName( installPath->text() + QDir::separator() +  "build.log" );
	if( !outputLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    outstream.setDevice( &outputLog );

    outputDisplay->append( entry + "\n" );
    outstream << ( entry + "\n" );

    if( close )
	outputLog.close();
}

void SetupWizardImpl::archiveMsg( const QString& msg )
{
#if defined(MD4_KEYS)
    Q_UNUSED(msg)
#else
    if( msg.right( 7 ) == ".cpp..." || msg.right( 5 ) == ".c..." || msg.right( 7 ) == ".pro..." || msg.right( 6 ) == ".ui..." )
	filesToCompile++;
    qApp->processEvents();
    logFiles( msg );
#endif
}

bool SetupWizardImpl::copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel )
{
    QDir dir( sourcePath );
    const QFileInfoList* list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo* fi;
    bool doCopy;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[ 0 ] != '.' ) {
	    QString entryName = sourcePath + QDir::separator() + fi->fileName();
	    QString targetName = destPath + QDir::separator() + fi->fileName();
	    doCopy = true;
	    if( fi->isDir() ) {
		if( !dir.exists( targetName ) )
		    createDir( targetName );
		if( topLevel ) {
		    if( fi->fileName() == "doc" )
			doCopy = installDocs->isChecked();
		    else if( fi->fileName() == "tutorial" )
			doCopy = installTutorials->isChecked();
		    else if( fi->fileName() == "examples" )
			doCopy = installExamples->isChecked();
		}
		if( doCopy )
		    if( !copyFiles( entryName, targetName, false ) )
			return false;
	    } else {
		if( qApp && !isHidden() ) {
		    qApp->processEvents();
		    operationProgress->setProgress( totalFiles );
		    logFiles( targetName );
		} else {
		    return false;
		}
		if( entryName.right( 4 ) == ".cpp" || 
		    entryName.right( 2 ) == ".c" ||
		    entryName.right( 4 ) == ".pro" ||
		    entryName.right( 3 ) == ".ui" )
		    filesToCompile++;
		bool res = true;
#if defined(Q_OS_WIN32)
		if ( !QFile::exists( targetName ) )
		    res = CopyFileA( entryName.local8Bit(), targetName.local8Bit(), false );

		if ( res ) {
		    totalFiles++;
		    HANDLE inFile, outFile;
		    if( inFile = ::CreateFileA( entryName.latin1(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ){
			if( outFile = ::CreateFileA( targetName.latin1(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ){
			    FILETIME createTime, accessTime, writeTime;
			    ::GetFileTime( inFile, &createTime, &accessTime, &writeTime );
			    ::SetFileTime( outFile, &createTime, &accessTime, &writeTime );
			    ::CloseHandle( outFile );
			}
			::CloseHandle( inFile );
		    }
		} else {
		    QString error = QEnvironment::getLastError();
		    logFiles( QString( "   ERROR: " ) + error + "\n" );
		    if( QMessageBox::warning( this, "File copy error", entryName + ": " + error, "Continue", "Cancel", QString::null, 0 ) )
			return false;
		}
#elif defined(Q_OS_UNIX)
		if ( !QFile::exists( targetName ) )
		    res = copyFile( entryName.local8Bit(), targetName.local8Bit() );
		// TODO: keep file date the same, handle errors
#endif
	    }
	}
	++it;
    }
    return true;
}

void SetupWizardImpl::setInstallStep( int step )
{
#if defined(EVAL)
    setCaption( QString( "Qt Evaluation Version Installation Wizard - Step %1 of 7" ).arg( step ) );
#else
    if( reconfigMode )
	setCaption( QString( "Qt Configuration Wizard - Step %1 of 3" ).arg( step ) );
    else
	setCaption( QString( "Qt Installation Wizard - Step %1 of 8" ).arg( step ) );
#endif
}

void SetupWizardImpl::timerFired()
{
    QString tmp( "Next %1 >" );

    timeCounter--;

    if( timeCounter )
	nextButton()->setText( tmp.arg( timeCounter ) );
    else {
	next();
	autoContTimer.stop();
    }
}

void SetupWizardImpl::readLicense( QString filePath)
{
#if !defined(EVAL)
    QFile licenseFile( filePath );

    if( licenseFile.open( IO_ReadOnly ) ) {
	QString buffer;

	while( licenseFile.readLine( buffer, 1024 ) != -1 ) {
	    if( buffer[ 0 ] != '#' ) {
		QStringList components = QStringList::split( '=', buffer );
		QStringList::Iterator it = components.begin();
		QString keyString = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null ).upper();
		QString value = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null );

		licenseInfo[ keyString ] = value;
	    }
	}
	licenseFile.close();

	customerID->setText( licenseInfo[ "CUSTOMERID" ] );
	licenseID->setText( licenseInfo[ "LICENSEID" ] );
	licenseeName->setText( licenseInfo[ "LICENSEE" ] );
	if( licenseInfo[ "PRODUCTS" ] == "qt-enterprise" ) {
	    productsString->setCurrentItem( 1 );
	} else {
	    productsString->setCurrentItem( 0 );
	}
	expiryDate->setText( licenseInfo[ "EXPIRYDATE" ] );
#  if defined(MD4_KEYS)
	key->setText( licenseInfo[ "PRODUCTKEY" ] );
#  else
	key->setText( licenseInfo[ "LICENSEKEY" ] );
#  endif
    }
#endif
}

void SetupWizardImpl::writeLicense( QString filePath )
{
#if !defined(EVAL)
    QFile licenseFile( filePath );

    if( licenseFile.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream licStream( &licenseFile );
	
	licenseInfo[ "CUSTOMERID" ] = customerID->text();
	licenseInfo[ "LICENSEID" ] = licenseID->text();
	licenseInfo[ "LICENSEE" ] = licenseeName->text();
	if( productsString->currentItem() == 0 )
	    licenseInfo[ "PRODUCTS" ] = "qt-professional";
	else
	    licenseInfo[ "PRODUCTS" ] = "qt-enterprise";

	licenseInfo[ "EXPIRYDATE" ] = expiryDate->text();
#  if defined(MD4_KEYS)
	licenseInfo[ "PRODUCTKEY" ] = key->text();
#  else
	licenseInfo[ "LICENSEKEY" ] = key->text();
#  endif

	licStream << "# Toolkit license file" << endl;
	licStream << "CustomerID=\"" << licenseInfo[ "CUSTOMERID" ].latin1() << "\"" << endl;
	licStream << "LicenseID=\"" << licenseInfo[ "LICENSEID" ].latin1() << "\"" << endl;
	licStream << "Licensee=\"" << licenseInfo[ "LICENSEE" ].latin1() << "\"" << endl;
	licStream << "Products=\"" << licenseInfo[ "PRODUCTS" ].latin1() << "\"" << endl;
	licStream << "ExpiryDate=" << licenseInfo[ "EXPIRYDATE" ].latin1() << endl;
#  if defined(MD4_KEYS)
	licStream << "ProductKey=" << licenseInfo[ "PRODUCTKEY" ].latin1() << endl;
#  else
	licStream << "LicenseKey=" << licenseInfo[ "LICENSEKEY" ].latin1() << endl;
#  endif

	licenseFile.close();
    }
#endif
}

void SetupWizardImpl::clickedLicenseFile()
{
    QString licensePath = QFileDialog::getOpenFileName( installPath->text(), QString::null, this, NULL, "Browse for license file" );

    if( !licensePath.isEmpty() )
	readLicense( licensePath );

}

bool SetupWizardImpl::findFileInPaths( QString fileName, QStringList paths )
{
	QDir d;
	for( QStringList::Iterator it = paths.begin(); it != paths.end(); ++it ) {
	    if( d.exists( (*it) +QDir::separator() + fileName ) )
		return true;
	}
	return false;
}

void SetupWizardImpl::readLicenseAgreement()
{
    // Intropage
#if defined(MD4_KEYS)
    QFile licenseFile( "LICENSE" );
    if( licenseFile.open( IO_ReadOnly ) ) {
	QFileInfo fi( licenseFile );
	QByteArray fileData( fi.size() + 2 );
	licenseFile.readBlock( fileData.data(), fi.size() );
	fileData.data()[ fi.size() ] = 0;
	fileData.data()[ fi.size() + 1 ] = 0;
	introText->setText( QString( fileData.data() ) );
    }
#else
    ResourceLoader *rcLoader;
    if ( usLicense ) {
	rcLoader = new ResourceLoader( "LICENSE-US" );
    } else {
	rcLoader = new ResourceLoader( "LICENSE" );
    }
    if ( rcLoader->isValid() ) {
	introText->setText( rcLoader->data() );
	acceptLicense->setEnabled( TRUE );
    } else {
	QMessageBox::critical( this, tr("Package corrupted"),
		tr("Could not find the LICENSE file in the package.\nThe package might be corrupted.") );
	acceptLicense->setEnabled( FALSE );
    }
    delete rcLoader;
#endif
}
