/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
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

#include <qtabwidget.h>
#include <qfileinfo.h>
#include <qaccel.h>
#include <qobjectlist.h>
#include <qtimer.h>
#include <qdragobject.h>
#include <qfontinfo.h>

QPtrList<MainWindow> *MainWindow::windows = 0;

void MainWindow::init()
{
#ifndef Q_WS_MACX
    setIcon( QPixmap::fromMimeSource( "appicon.png" ) );
#endif

    if ( !windows )
	windows = new QPtrList<MainWindow>;
    windows->append( this );
    setWFlags( WDestructiveClose );
    browser = new HelpWindow( this, this, "qt_assistant_helpwindow" );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setCentralWidget( browser );
    settingsDia = 0;

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    dw = new QDockWindow;
    helpDock = new HelpDialog( dw, this, browser );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addDockWindow( dw, DockLeft );
    dw->setWidget( helpDock );
    dw->setCaption( "Sidebar" );
    dw->setFixedExtentWidth( 320 );

    setObjectsEnabled( FALSE );

    // read geometry configuration
    QString keybase("/Qt Assistant/3.1/");

    setupGoActions( settings.readListEntry( keybase + "AdditionalDocFiles" ),
		    settings.readListEntry( keybase + "CategoriesSelected" ) );
    if ( !settings.readBoolEntry( keybase  + "GeometryMaximized", FALSE ) ) {
	QRect r( pos(), size() );
	r.setX( settings.readNumEntry( keybase + "GeometryX", r.x() ) );
	r.setY( settings.readNumEntry( keybase + "GeometryY", r.y() ) );
	r.setWidth( settings.readNumEntry( keybase + "GeometryWidth", r.width() ) );
	r.setHeight( settings.readNumEntry( keybase + "GeometryHeight", r.height() ) );

	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}
    }

    QString mainWindowLayout = settings.readEntry( keybase + "MainwindowLayout" );
    QTextStream ts( &mainWindowLayout, IO_ReadOnly );
    ts >> *this;

    QTimer::singleShot( 0, this, SLOT( setup() ) );
}

void MainWindow::setup()
{
    helpDock->initialize();
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QString base( qInstallPathDocs() );
    browser->mimeSourceFactory()->addFilePath( base + "/html/" );

    connect( actionGoPrevious, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( actionGoNext, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( actionEditCopy, SIGNAL( activated() ), browser, SLOT( copy() ) );
    connect( actionFileExit, SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );
    connect( actionAddBookmark, SIGNAL( activated() ),
	     helpDock, SLOT( addBookmark() ) );
    connect( helpDock, SIGNAL( showLink( const QString& ) ),
	     this, SLOT( showLink( const QString& ) ) );
    connect( bookmarkMenu, SIGNAL( activated( int ) ),
	     this, SLOT( showBookmark( int ) ) );
    connect( browser, SIGNAL( highlighted( const QString & ) ),
	     statusBar(), SLOT( message( const QString & ) ) );
    connect( actionZoomIn, SIGNAL( activated() ), browser, SLOT( zoomIn() ) );
    connect( actionZoomOut, SIGNAL( activated() ), browser, SLOT( zoomOut() ) );

    QAccel *acc = new QAccel( this );
    acc->connectItem( acc->insertItem( Key_F5 ), browser, SLOT( reload() ) );

    QAccel *a = new QAccel( this, dw );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+T") ) ),
		    helpDock, SLOT( toggleContents() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+I") ) ),
		    helpDock, SLOT( toggleIndex() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+B") ) ),
		    helpDock, SLOT( toggleBookmarks() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+S") ) ),
		    helpDock, SLOT( toggleSearch() ) );

    // read configuration
    QString keybase("/Qt Assistant/3.1/");

    QFont fnt( browser->QWidget::font() );
    QFontInfo fntInfo( fnt );
    fnt.setFamily( settings.readEntry( keybase + "Family", fntInfo.family() ) );
    fnt.setPointSize( settings.readNumEntry( keybase + "Size", fntInfo.pointSize() ) );
    browser->setFont( fnt );
    browser->setLinkUnderline( settings.readBoolEntry( keybase + "LinkUnderline", TRUE ) );

    QPalette pal = browser->palette();
    QColor lc( settings.readEntry( keybase + "LinkColor",
	       pal.color( QPalette::Active, QColorGroup::Link ).name() ) );
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    browser->setPalette( pal );

    QString family = settings.readEntry( keybase + "FixedFamily",
			browser->styleSheet()->item( "pre" )->fontFamily() );

    QStyleSheet *sh = browser->styleSheet();
    sh->item( "pre" )->setFontFamily( family );
    sh->item( "code" )->setFontFamily( family );
    sh->item( "tt" )->setFontFamily( family );
    browser->setStyleSheet( sh );

    setupBookmarkMenu();
    PopupMenu->insertItem( tr( "Vie&ws" ), createDockWindowMenu() );
    helpDock->tabWidget->setCurrentPage( settings.readNumEntry( keybase
					 + "SideBarPage", 0 ) );

    setObjectsEnabled( TRUE );

    if ( settings.readBoolEntry( "/Qt Assistant/3.1/NewDoc/", FALSE ) ) {
	QTimer::singleShot( 0, helpDock, SLOT( generateNewDocu() ));
	settings.writeEntry( "/Qt Assistant/3.1/NewDoc/", FALSE );
    }

}

void MainWindow::setupGoActions( const QStringList &docList, const QStringList &catList )
{
    QStringList::ConstIterator it = docList.begin();
    bool separatorInserted = FALSE;
    for ( ; it != docList.end(); ++it ) {
	if ( (*it).lower().contains( "qt.xml" ) &&
	     catList.find( "qt/reference" ) != catList.end() ) {
	    if ( !separatorInserted )
		separatorInserted = insertActionSeparator();
	    actionGoQt->addTo( goMenu );
	    actionGoQt->addTo( Toolbar );
	} else if ( (*it).lower().contains( "designer.xml" ) &&
		    catList.find( "qt/designer" ) != catList.end() ) {
	    if ( !separatorInserted )
		separatorInserted = insertActionSeparator();
	    actionGoDesigner->addTo( goMenu );
	    actionGoDesigner->addTo( Toolbar );
	} else if ( (*it).lower().contains( "assistant.xml" ) &&
		    catList.find( "qt/assistant" ) != catList.end() ) {
	    if ( !separatorInserted )
		separatorInserted = insertActionSeparator();
	    actionGoAssistant->addTo( goMenu );
	    actionGoAssistant->addTo( Toolbar );
	} else if ( (*it).lower().contains( "linguist.xml" ) &&
		    catList.find( "qt/linguist" ) != catList.end() ) {
	    if ( !separatorInserted )
		separatorInserted = insertActionSeparator();
	    actionGoLinguist->addTo( goMenu );
	    actionGoLinguist->addTo( Toolbar );
	}
    }
}

bool MainWindow::insertActionSeparator()
{
    goMenu->insertSeparator();
    Toolbar->addSeparator();
    return TRUE;
}

void MainWindow::setObjectsEnabled( bool b )
{
    if ( b ) {
	qApp->restoreOverrideCursor();
    } else {
	qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );
	statusBar()->message( tr( "Initializing Qt Assistant..." ) );
    }
    QObjectList *l = queryList( "QAction" );
    QObject *obj;
    QObjectListIt it( *l );
    while ( (obj = it.current()) != 0 ) {
        ++it;
        ((QAction*)obj)->setEnabled( b );
    }
    delete l;
    menubar->setEnabled( b );
    helpDock->setEnabled( b );
}

bool MainWindow::close( bool alsoDelete )
{
    saveSettings();
    return QMainWindow::close( alsoDelete );
}

void MainWindow::destroy()
{
    windows->removeRef( this );
    if ( windows->isEmpty() ) {
	delete windows;
	windows = 0;
    }
}

void MainWindow::about()
{
    static const char *about_text =
    "<p><b><font size=+2>Qt Assistant</font></b></p>"
    "<p>The Qt documentation browser</p>"
    "<p>Version 2.0</p>"
    "<p>Copyright (C) 2001-2002 Trolltech AS. All rights reserved.</p>"
    "<p></p>"
    "<p>This program is licensed to you under the terms of the GNU General "
    "Public License Version 2 as published by the Free Software Foundation. This "
    "gives you legal permission to copy, distribute and/or modify this software "
    "under certain conditions. For details, see the file 'LICENSE.GPL' that came with "
    "this software distribution. If you did not get the file, send email to "
    "info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY "
    "OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
    "FOR A PARTICULAR PURPOSE.</p>";
    QMessageBox::about( this, tr("Qt Assistant"), tr( about_text ) );
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, tr( "Qt Assistant" ) );
}

void MainWindow::find()
{
    if ( !findDialog ) {
	findDialog = new FindDialog( this );
	findDialog->setBrowser( browser );
    }
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->setSelection(
        0, findDialog->comboFind->lineEdit()->text().length() );
    findDialog->show();
}

void MainWindow::goHome()
{
    showLink( QString( qInstallPathDocs() ) + "/html/index.html" );
}

void MainWindow::showAssistantHelp()
{
    showLink( QString( qInstallPathDocs() ) +
	      "/html/assistant.html" );
}

void MainWindow::showLinguistHelp()
{
    showLink( QString( qInstallPathDocs() ) +
	      "/html/linguist-manual.html" );
}

void MainWindow::print()
{
    QPrinter printer;
    printer.setFullPage( TRUE );
    if ( printer.setup( this ) ) {
	QPaintDeviceMetrics screen( this );
	printer.setResolution( screen.logicalDpiY() );
	QPainter p( &printer );
	QPaintDeviceMetrics metrics(p.device());
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
	const int margin = 72; // pt
	QRect body( margin*dpix/72, margin*dpiy/72,
		    metrics.width()-margin*dpix/72*2,
		    metrics.height()-margin*dpiy/72*2 );
	QSimpleRichText richText( browser->text(), browser->QWidget::font(), browser->context(), browser->styleSheet(),
				  browser->mimeSourceFactory(), body.height(),
				  Qt::black, FALSE );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top() >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }
}

void MainWindow::updateBookmarkMenu()
{
    for ( MainWindow *mw = windows->first(); mw; mw = windows->next() )
	mw->setupBookmarkMenu();
}

void MainWindow::setupBookmarkMenu()
{
    bookmarkMenu->clear();
    bookmarks.clear();
    actionAddBookmark->addTo( bookmarkMenu );

    QFile f( QDir::homeDirPath() + "/.bookmarks" );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    bookmarkMenu->insertSeparator();
    while ( !ts.atEnd() ) {
	QString title = ts.readLine();
	QString link = ts.readLine();
	bookmarks.insert( bookmarkMenu->insertItem( title ), link );
    }
}

void MainWindow::showBookmark( int id )
{
    if ( bookmarks.find( id ) != bookmarks.end() )
	showLink( *bookmarks.find( id ) );
}

void MainWindow::showDesignerHelp()
{
    showLink( QString( qInstallPathDocs() ) +
	      "/html/designer-manual.html" );
}

void MainWindow::showLinkFromClient( const QString &link )
{
    raise();
    setActiveWindow();
    showLink( link );
}

void MainWindow::showLink( const QString &link )
{
    QString filename = link.left( link.find( '#' ) );
    QFileInfo fi( filename );
    // introduce a default-not-found site
    if ( !fi.exists() )
	browser->setSource( "index.html" );
    else {
	browser->setSource( link );
    }
    browser->setFocus();
}

void MainWindow::showQtHelp()
{
    showLink( QString( qInstallPathDocs() ) + "/html/index.html" );
}

void MainWindow::setFamily( const QString & f )
{
    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( f );
    browser->setFont( fnt );
}

void MainWindow::showSettingsDialog()
{
    if ( !settingsDia ){
	settingsDia = new SettingsDialog( this );
	connect( settingsDia, SIGNAL( docuFilesChanged() ), helpDock, SLOT( generateNewDocu() ));
    }
    QFontDatabase fonts;
    settingsDia->fontCombo->insertStringList( fonts.families() );
    settingsDia->fontCombo->lineEdit()->setText( browser->QWidget::font().family() );
    settingsDia->fixedfontCombo->insertStringList( fonts.families() );
    settingsDia->fixedfontCombo->lineEdit()->setText( browser->styleSheet()->item( "pre" )->fontFamily() );
    settingsDia->linkUnderlineCB->setChecked( browser->linkUnderline() );
    settingsDia->colorButton->setPaletteBackgroundColor( browser->palette().color( QPalette::Active, QColorGroup::Link ) );

    int ret = settingsDia->exec();

    if ( ret != QDialog::Accepted )
	return;

    actionGoQt->removeFrom( goMenu );
    actionGoQt->removeFrom( Toolbar );
    actionGoDesigner->removeFrom( goMenu );
    actionGoDesigner->removeFrom( Toolbar );
    actionGoAssistant->removeFrom( goMenu );
    actionGoAssistant->removeFrom( Toolbar );
    actionGoLinguist->removeFrom( goMenu );
    actionGoLinguist->removeFrom( Toolbar );
    goMenu->removeItemAt( goMenu->count() - 1 );
    QObjectList *lst = (QObjectList*)Toolbar->children();
    QObject *obj;
    for ( obj = lst->last(); obj; obj = lst->prev() ) {
	if ( obj->isA( "QToolBarSeparator" ) ) {
	    delete obj;
	    obj = 0;
	    break;
	}
    }

    setupGoActions( settingsDia->documentationList(), settingsDia->selCategoriesList() );

    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( settingsDia->fontCombo->currentText() );
    browser->setFont( fnt );
    browser->setLinkUnderline( settingsDia->linkUnderlineCB->isChecked() );

    QPalette pal = browser->palette();
    QColor lc = settingsDia->colorButton->paletteBackgroundColor();
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    browser->setPalette( pal );

    QString family = settingsDia->fixedfontCombo->currentText();

    QStyleSheet *sh = browser->styleSheet();
    sh->item( "pre" )->setFontFamily( family );
    sh->item( "code" )->setFontFamily( family );
    sh->item( "tt" )->setFontFamily( family );
    browser->setStyleSheet( sh );
}

void MainWindow::hide()
{
    saveToolbarSettings();
    QMainWindow::hide();
}


MainWindow* MainWindow::newWindow()
{
    saveSettings();
    saveToolbarSettings();
    MainWindow *mw = new MainWindow;
    mw->move( geometry().topLeft() );
    if ( isMaximized() )
	mw->showMaximized();
    else
	mw->show();
    return mw;
}

void MainWindow::saveSettings()
{
    QString keybase("/Qt Assistant/3.1/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    config.writeEntry( keybase + "Family",  browser->QWidget::font().family() );
    config.writeEntry( keybase + "Size",  browser->QWidget::font().pointSize() );
    config.writeEntry( keybase + "FixedFamily", browser->styleSheet()->item( "pre" )->fontFamily() );
    config.writeEntry( keybase + "LinkUnderline", browser->linkUnderline() );
    config.writeEntry( keybase + "LinkColor", browser->palette().color( QPalette::Active, QColorGroup::Link ).name() );
    config.writeEntry( keybase + "Source", browser->source() );
    config.writeEntry( keybase + "Title", browser->caption() );
    config.writeEntry( keybase + "SideBarPage", helpDock->tabWidget->currentPageIndex() );
    config.writeEntry( keybase + "GeometryX", x() );
    config.writeEntry( keybase + "GeometryY", y() );
    config.writeEntry( keybase + "GeometryWidth", width() );
    config.writeEntry( keybase + "GeometryHeight", height() );
    config.writeEntry( keybase + "GeometryMaximized", isMaximized() );
}

void MainWindow::saveToolbarSettings()
{
    QString mainWindowLayout;
    QTextStream ts( &mainWindowLayout, IO_WriteOnly );
    ts << *this;
    QSettings config;
    config.writeEntry( "/Qt Assistant/3.1/MainwindowLayout", mainWindowLayout );
}
