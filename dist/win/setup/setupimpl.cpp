#include "setupimpl.h"

#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qradiobutton.h>
#include <qheader.h>
#include <qlabel.h>

#include <stdlib.h>

#include "check.xpm"

/*
 *  Constructs a Setup which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
Setup::Setup( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SetupBase( parent, name, modal, fl ), finished( FALSE ), isCompiling( FALSE )
{
    initLicensePage();
    initOptionsPage();
    cancelButton()->setText( tr( "&Exit" ) );
    setHelpEnabled( page( 0 ), FALSE );
    setHelpEnabled( page( 1 ), FALSE );
    setHelpEnabled( page( 2 ), FALSE );
}

void Setup::initLicensePage()
{
    QString d = QString( getenv( "QTDIR" ) );
    QStringList l = QDir( d ).entryList();
    l = l.grep( "LICENSE", TRUE );
    QFile f( d + "/" + l.first() );
    if ( l.isEmpty() || !f.exists() || !f.open( IO_ReadOnly ) ) {
	error( NoLicenseAgreement );
	return;
    }
    QTextStream ts( &f );
    viewLicense->setText( ts.read() );

    setNextEnabled( page( 0 ), FALSE );
}

void Setup::initOptionsPage()
{
    QString d = QString( getenv( "QTDIR" ) );
    QStringList l = QDir( d ).entryList();
    l = l.grep( "qt-license", TRUE );
    QFile f( d + "/" + l.first() );
    if ( l.isEmpty() || !f.exists() || !f.open( IO_ReadOnly ) ) {
	error( NoLicenseFile );
	return;
    }
    QTextStream ts( &f );
    QString license = ts.read();

    int i = license.find( "Products=\"" );
    if ( i == -1 ) {
	error( InvalidLicense );
	return;
    }

    int start = i + QString( "Products=\"" ).length();
    QString s = license.mid( start, license.find( '"', start + 1 ) - start );
    l = QStringList::split( ' ', s );
    QStringList modules;
    bool hasEnterprise = FALSE;
    if ( l.first() == "qt-enterprise" )
	hasEnterprise = TRUE;

    listModules->header()->hide();
    QCheckListItem *item = 0;
    if ( hasEnterprise ) {
	QListViewItem *enterprise = new QListViewItem( listModules );
	enterprise->setText( 0, tr( "Qt Enterprise Edition Modules" ) );

	item = new QCheckListItem( enterprise, tr( "Canvas" ), QCheckListItem::CheckBox );
	item->setText( 1, "canvas" );
	item->setOn( TRUE );
	item = new QCheckListItem( enterprise, tr( "Table" ), QCheckListItem::CheckBox );
	item->setText( 1, "table" );
	item->setOn( TRUE );
	item = new QCheckListItem( enterprise, tr( "OpenGL 3D Graphics" ), QCheckListItem::CheckBox );
	item->setText( 1, "opengl" );
	item->setOn( TRUE );
	item = new QCheckListItem( enterprise, tr( "XML/DOM" ), QCheckListItem::CheckBox );
	item->setText( 1, "xml" );
	item->setOn( TRUE );
	item = new QCheckListItem( enterprise, tr( "Network" ), QCheckListItem::CheckBox );
	item->setText( 1, "network" );
	item->setOn( TRUE );

	enterprise->setOpen( TRUE );
    }

    QListViewItem *professional = new QListViewItem( listModules );
    professional->setText( 0, tr( "Qt Professional Edition Modules" ) );

    item = new QCheckListItem( professional, tr( "Workspace" ), QCheckListItem::CheckBox );
    item->setText( 1, "workspace" );
    item->setOn( TRUE );
    item = new QCheckListItem( professional, tr( "Iconview" ), QCheckListItem::CheckBox );
    item->setText( 1, "iconview" );
    item->setOn( TRUE );

    professional->setOpen( TRUE );

    QListViewItem *base = new QListViewItem( listModules );
    listModules->setSorting( -1 );
    base->setText( 0, tr( "Base Modules" ) );

    item = new QCheckListItem( base, tr( "Dialogs" ), QCheckListItem::CheckBox );
    item->setText( 1, "dialogs" );
    item->setOn( TRUE );
    item->setEnabled( FALSE );
    item = new QCheckListItem( base, tr( "Widgets" ), QCheckListItem::CheckBox );
    item->setText( 1, "widgets" );
    item->setOn( TRUE );
    item->setEnabled( FALSE );
    item = new QCheckListItem( base, tr( "Kernel" ), QCheckListItem::CheckBox );
    item->setText( 1, "kernel" );
    item->setOn( TRUE );
    item->setEnabled( FALSE );
    item->setOn( TRUE );
    item = new QCheckListItem( base, tr( "Tools" ), QCheckListItem::CheckBox );
    item->setText( 1, "tools" );
    item->setOn( TRUE );
    item->setEnabled( FALSE );
    listModules->setCurrentItem( item );

    base->setOpen( TRUE );
}


/*
 *  Destroys the object and frees any allocated resources
 */
Setup::~Setup()
{
    // no need to delete child widgets, Qt does it all for us
}

void Setup::licenseAccepted( bool a )
{
    setNextEnabled( currentPage(), a );
}

void Setup::useMSVC( bool b )
{
    if ( !b )
	return;
    comboLib->setEnabled( TRUE );
}

void Setup::useBCC( bool b )
{
    if ( !b )
	return;
    comboLib->setEnabled( FALSE );
    comboLib->setCurrentItem( 1 );
}

void Setup::error( Error e )
{
    switch ( e ) {
    case NoLicenseAgreement:
	QMessageBox::information( this, tr( "No License Agreement" ),
				  tr( "Setup <b>couldn't find</b> a <b>license agreement</b> file in your "
				      "package. Seems you are using an incomplete package. Please "
				      "contact <u>sales@trolltech.com</u> for further information" ) );
	::exit( -1 );
	break;
    case NoLicenseFile:
	QMessageBox::information( this, tr( "No License File" ),
				  tr( "Setup <b>couldn't find</b> a <b>license file (qt-license)</b> in your "
				      "package, which describes the Qt edition you are using. Seems you "
				      "are using am incomplete package. Please "
				      "contact <u>sales@trolltech.com</u> for further information" ) );
	::exit( -1 );
	break;
    case InvalidLicense:
	QMessageBox::information( this, tr( "Invalid License File" ),
				  tr( "The <b>license file</b> in your package seems to be <b>invalid</b>. "
				      "Please contact <u>sales@trolltech.com</u> to get a valid one." ) );
	::exit( -1 );
	break;
    case CannotWriteConfig:
	QMessageBox::information( this, tr( "Couldn't write Configuration" ),
				  tr( "Setup was <b>not able</b> to <b>write your configuration file</b> to the disk. "
				      "Please contact <u>support@trolltech.com</u> to get help." ) );
	::exit( -1 );
	break;
    case CannotWritePlatform:
	QMessageBox::information( this, tr( "Couldn't write Platform file" ),
				  tr( "Setup was <b>not able</b> to <b>write your platform specification file</b> to the disk. "
				      "Please contact <u>support@trolltech.com</u> to get help." ) );
	::exit( -1 );
	break;
    case CannotWriteQtConfig:
	QMessageBox::information( this, tr( "Couldn't write qtconfig.h file" ),
				  tr( "Setup was <b>not able</b> to <b>write your qtconfig.h file</b> to the disk. "
				      "Please contact <u>support@trolltech.com</u> to get help." ) );
	::exit( -1 );
	break;
    }
}

void Setup::pageChanged( const QString &s )
{
    if ( !s.contains( "Compil" ) )
	return;
    writeConfigFile();
    writePlatformFile();
    writeQtConfigFile();
}

void Setup::moduleSelected( QListViewItem *i )
{
    if ( !i->parent() ) {
	labelDescription->setText( "" );
	return;
    }
    if ( i->text( 0 ) == tr( "Tools" ) )
	labelDescription->setText( tr( "<p>Platform-independent Non-GUI API for I/O, encodings, containers, "
				       "strings, time & date, and regular expressions.</p>" ) );
    else if ( i->text( 0 ) == tr( "Kernel"  ) )
	labelDescription->setText( tr( "<p>Platform-independent GUI API, a complete window-system API.</p>" ) );
    else if ( i->text( 0 ) == tr( "Widgets"  ) )
	labelDescription->setText( tr( "<p>Portable GUI controls.</p>" ) );
    else if ( i->text( 0 ) == tr( "Dialogs"  ) )
	labelDescription->setText( tr( "<p>Ready-made common dialogs for selection of colors, files, "
				       "printers, fonts, and basic types, plus a wizard framework, message "
				       "boxes and progress indicator.</p>" ) );
    else if ( i->text( 0 ) == tr( "OpenGL 3D Graphics"  ) )
	labelDescription->setText( tr( "<p>Integration of OpenGL with Qt, making it very "
				       "easy to use OpenGL rendering in a Qt application.</p>" ) );
    else if ( i->text( 0 ) == tr( "Network"  ) )
	labelDescription->setText( tr( "<p>Advanced socket and server-socket handling plus "
				       "asynchronous DNS lookup.</p>" ) );
    else if ( i->text( 0 ) == tr( "Canvas"  ) )
	labelDescription->setText( tr( "<p>A highly optimized 2D graphic area.</p>" ) );
    else if ( i->text( 0 ) == tr( "Table"  ) )
	labelDescription->setText( tr( "<p>A flexible and editable table widget.</p>" ) );
    else if ( i->text( 0 ) == tr( "XML/DOM"  ) )
	labelDescription->setText( tr( "<p>A well-formed XML parser with SAX interface plus an "
				       "implementation of the DOM Level1</p>" ) );
    else if ( i->text( 0 ) == tr( "Workspace"  ) )
	labelDescription->setText( tr( "<p>A workspace window that can contain decorated document "
				       "windows for Multi Document Interfaces (MDI).</p>" ) );
    else if ( i->text( 0 ) == tr( "Iconview"  ) )
	labelDescription->setText( tr( "<p>A powerful visualization widget similar to QListView and "
				       "QListBox. It contains optionally labelled pixmap items that the user "
				       " can select, drag around, rename, delete and more.</p>" ) );
}

void Setup::startCompilation()
{
    setBackEnabled( currentPage(), FALSE );
    buttonCompile->setEnabled( FALSE );

    isCompiling = TRUE;
    // ##### do compilation here

    finished = TRUE;
    setFinishEnabled( currentPage(), TRUE );
    cancelButton()->setEnabled( FALSE );
}

void Setup::gifToggled( bool b )
{
    if ( b ) {
	QMessageBox::information( this, tr( "Builtin GIF Readet" ),
				  tr( "<p>You selected the option to compile Qt with GIF reading support</p>"
				      "<p><b>Warning:</b> <i>Unisys has changed its position regarding GIF. If you are "
				      "in a country where Unisys holds a patent on LZW compression and/or "
				      "decompression and you want to use GIF, Unisys may require you to "
				      "license that technology.  These countries include Canada, Japan, the "
				      "USA, France, Germany, Italy and the UK.</i></p>" ) );
    }
}

void Setup::writeConfigFile()
{
    QString d = QString( getenv( "QTDIR" ) );
    d += "/lib/qt/qt-config.mk";
    QFile f( d );
    if ( !f.open( IO_WriteOnly ) ) {
	error( CannotWriteConfig );
	return;
    }

    QTextStream ts( &f );
    ts << "LIBTYPE=" << ( comboLib->currentItem() == 0 ? "shared" : "static" ) << endl;
    ts << "CONF_CFLAGS=" << ( comboOptimize->currentItem() == 0 ? " $(CFLAGS_OPTIMIZE)" : " $(CFLAGS_DEBUG)" )
       << "-I$(QTDIR)\\src\\3rdparty\\zlib -I$(QTDIR)\\src\\3rdparty\\libpng" << endl;
    ts << "CONF_CXXFLAGS=" << ( comboOptimize->currentItem() == 0 ? " $(CXXFLAGS_OPTIMIZE)" : " $(CXXFLAGS_DEBUG)" )
       << ( checkGIF->isChecked() ? "" : " -DQT_NO_BUILTIN_GIF_READER" ) << ( checkJPEG->isChecked() ? "" : " -DQT_NO_JPEG_SUPPORT" )
       << " -I$(QTDIR)\\src\\3rdparty\\zlib -I$(QTDIR)\\src\\3rdparty\\libpng" << endl;
    ts << "CONF_LIBS=" << endl;
    ts << "MODULE_OBJ=\t$(PNG_OBJECTS) $(ZLIB_OBJECTS) \\" << endl;
    QListViewItemIterator it( listModules );
    while ( it.current() ) {
	QListViewItem *item = it.current();
	++it;
	if ( !item->parent() )
	    continue;
	if ( !( (QCheckListItem*)item )->isOn() )
	    continue;
	QString m = ( (QCheckListItem*)item )->text( 1 );
	ts << "\t\t$(OBJECTS_" << m << ")";
	if ( it.current() )
	    ts << " \\" << endl;
    }
    f.close();
}

void Setup::writePlatformFile()
{
    QString d = QString( getenv( "QTDIR" ) );
    d += "/lib/qt/platform.mk";
    QFile f( d );
    if ( !f.open( IO_WriteOnly ) ) {
	error( CannotWritePlatform );
	return;
    }

    QTextStream ts( &f );
    ts << "SYSTEMTYPE=win32" << endl;
    ts << "SYSTEM=win32" << endl;
    ts << "COMPILER=" << ( radioMSVC->isChecked() ? "msvc" : "borland" ) << endl;
    f.close();
}

void Setup::writeQtConfigFile()
{
    QString d = QString( getenv( "QTDIR" ) );
    d += "/src/tools/qconfig.h";
    QFile f( d );
    if ( !f.open( IO_WriteOnly ) ) {
	error( CannotWriteQtConfig );
	return;
    }

    QTextStream ts( &f );
    ts << "// Empty leaves all features enabled.  See doc/html/features.html for choices." << endl;
    ts << endl;
    ts << "// Note that disabling some features will produce a libqt that is not" << endl;
    ts << "// compatible with other libqt builds. Such modifications are only" << endl;
    ts << "// supported on Qt/Embedded where reducing the library size is important" << endl;
    ts << "// and where the application-suite is often a fixed set." << endl;
    ts << endl;
    ts << "#define QT_NO_IMAGEIO_MNG // not defined by default." << endl;
    if ( comboLib->currentItem() == 0 )
	ts << "#define QT_DLL" << endl;
    f.close();
}

void Setup::reject()
{
    if ( finished ) {
	::exit( 0 );
	return;
    }
    if ( QMessageBox::information( this, tr( "Exit Setup" ),
				   tr( "The Qt Setup is not completed yet. Do yo really "
				       "want to exit?" ),
				   tr( "&Yes" ), tr( "&No" ), QString::null, 1, 1 ) == 0 ) {
	
	if ( isCompiling ) {
	    // ##### kill compilation
	}
	QWizard::reject();
	::exit( -1 );
    } else {
    }
}
