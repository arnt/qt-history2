/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "config.h"

#include <qtabwidget.h>
#include <qfileinfo.h>
#include <qaccel.h>
#include <qtimer.h>
#include <qdragobject.h>
#include <qfontinfo.h>
#include <qaccel.h>
#include <qmetaobject.h>
#include <qeventloop.h>
#include <qdesktopwidget.h>

QPtrList<MainWindow> *MainWindow::windows = 0;

void MainWindow::init()
{
    setupCompleted = FALSE;

    goActions = new QPtrList<QAction>;
    goActionDocFiles = new QMap<QAction*,QString>;
    goActions->setAutoDelete( TRUE );

    if ( !windows )
	windows = new QPtrList<MainWindow>;
    windows->append( this );
    tabs = new TabbedBrowser( this, "qt_assistant_tabbedbrowser" );
    setCentralWidget( tabs );
    settingsDia = 0;

    Config *config = Config::configuration();

    updateProfileSettings();

    dw = new QDockWindow( QDockWindow::InDock, this );
    helpDock = new HelpDialog( dw, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addDockWindow( dw, DockLeft );
    dw->setWidget( helpDock );
    dw->setCaption( "Sidebar" );
    dw->setFixedExtentWidth( 320 );

    // read geometry configuration
    setupGoActions();

    if ( !config->isMaximized() ) {
	QRect desktop =  QApplication::desktop()->availableGeometry();
	QRect geom = config->geometry() & desktop;
	if( config->geometry().isValid() && geom.isValid() ) {
	    resize(geom.size());
	    move(geom.topLeft());
	}
    }

    QString mainWindowLayout = config->mainWindowLayout();

    QTextStream ts( &mainWindowLayout, IO_ReadOnly );
    ts >> *this;

    if ( config->sideBarHidden() )
	dw->hide();

    setObjectsEnabled( FALSE );
    tabs->setup();
    QTimer::singleShot( 0, this, SLOT( setup() ) );
}

void MainWindow::setup()
{
    if( setupCompleted )
	return;

    setupCompleted = TRUE;
    helpDock->initialize();
    connect( actionGoPrevious, SIGNAL( activated() ), tabs, SLOT( backward() ) );
    connect( actionGoNext, SIGNAL( activated() ), tabs, SLOT( forward() ) );
    connect( actionEditCopy, SIGNAL( activated() ), tabs, SLOT( copy() ) );
    connect( actionFileExit, SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );
    connect( actionAddBookmark, SIGNAL( activated() ),
	     helpDock, SLOT( addBookmark() ) );
    connect( helpDock, SIGNAL( showLink( const QString& ) ),
	     this, SLOT( showLink( const QString& ) ) );
    connect( helpDock, SIGNAL( showSearchLink( const QString&, const QStringList& ) ),
	     this, SLOT( showSearchLink( const QString&, const QStringList&) ) );

    connect( bookmarkMenu, SIGNAL( activated( int ) ),
	     this, SLOT( showBookmark( int ) ) );
    connect( actionZoomIn, SIGNAL( activated() ), tabs, SLOT( zoomIn() ) );
    connect( actionZoomOut, SIGNAL( activated() ), tabs, SLOT( zoomOut() ) );

    connect( actionOpenPage, SIGNAL( activated() ), tabs, SLOT( newTab() ) );
    connect( actionClosePage, SIGNAL( activated() ), tabs, SLOT( closeTab() ) );
    connect( actionNextPage, SIGNAL( activated() ), tabs, SLOT( nextTab() ) );
    connect( actionPrevPage, SIGNAL( activated() ), tabs, SLOT( previousTab() ) );



#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
    QAccel *acc = new QAccel( this );
//     acc->connectItem( acc->insertItem( Key_F5 ), browser, SLOT( reload() ) );
    acc->connectItem( acc->insertItem( QKeySequence("SHIFT+CTRL+=") ), actionZoomIn, SIGNAL(activated()) );
#endif

    QAccel *a = new QAccel( this, dw );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+T") ) ),
		    helpDock, SLOT( toggleContents() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+I") ) ),
		    helpDock, SLOT( toggleIndex() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+B") ) ),
		    helpDock, SLOT( toggleBookmarks() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+S") ) ),
		    helpDock, SLOT( toggleSearch() ) );

    Config *config = Config::configuration();

    setupBookmarkMenu();
    PopupMenu->insertItem( tr( "Vie&ws" ), createDockWindowMenu() );
    helpDock->tabWidget->setCurrentPage( config->sideBarPage() );

    setObjectsEnabled( TRUE );
    actionGoPrevious->setEnabled( FALSE );
    actionGoNext->setEnabled( FALSE );
}

void MainWindow::setupGoActions()
{
    Config *config = Config::configuration();
    QStringList titles = config->docTitles();   
    QAction *action = 0;

    static bool separatorInserted = FALSE;

    QAction *cur = goActions->first();
    while( cur ) {
	cur->removeFrom( goMenu );
	cur->removeFrom( goActionToolbar );
	cur = goActions->next();
    }
    goActions->clear();
    goActionDocFiles->clear();

    int addCount = 0;

    QStringList::ConstIterator it = titles.begin();
    for ( ; it != titles.end(); ++it ) {
	QString title = *it;
	QPixmap pix = config->docIcon( title );	
	if( !pix.isNull() ) {
	    if( !separatorInserted ) {
		goMenu->insertSeparator();
		separatorInserted = TRUE;
	    }
	    action = new QAction( title, QIconSet( pix ), title, 0, 0 );
	    action->addTo( goMenu );
	    action->addTo( goActionToolbar );
	    goActions->append( action );
	    goActionDocFiles->insert( action, config->indexPage( title ) );
	    connect( action, SIGNAL( activated() ),
		     this, SLOT( showGoActionLink() ) );
	    ++addCount;
	}
    }
    if( !addCount )
	goActionToolbar->hide();
    else
	goActionToolbar->show();

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
    QObjectList l = queryList( "QAction" );
    for (int i = 0; i < l.size(); ++i) {
	QObject *obj = l.at(i);
        ((QAction*)obj)->setEnabled( b );
    }
    QPtrListIterator<QAction> ait( *goActions );
    while ( ait.current() != 0 ) {
	(*ait)->setEnabled( b );
        ++ait;
    }
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
    delete goActions;
    delete goActionDocFiles;
}

void MainWindow::about()
{
    QMessageBox box( this );
    box.setText( "<center><img src=\"splash.png\">"
		 "<p>Version " + QString(QT_VERSION_STR) + "</p>"
		 "<p>Copyright (C) 2000-2003 Trolltech AS. All rights reserved."
		 "</p></center><p></p>"
		 "<p>Qt Commercial Edition license holders: This program is"
		 " licensed to you under the terms of the Qt Commercial License"
		 " Agreement. For details, see the file LICENSE that came with"
		 " this software distribution.</p><p></p>"
		 "<p>Qt Free Edition users: This program is licensed to you"
		 " under the terms of the GNU General Public License Version 2."
		 " For details, see the file LICENSE.GPL that came with this"
		 " software distribution.</p><p>The program is provided AS IS"
		 " with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF"
		 " DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."
		 "</p>" );
    box.setCaption( tr( "About Qt Assistant" ) );
    box.setIcon( QMessageBox::NoIcon );
    box.exec();
}

void MainWindow::aboutApplication()
{
    QString url = Config::configuration()->aboutURL();
    if ( url == "about_qt" ) {
	QMessageBox::aboutQt( this, "Qt Assistant" );
	return;
    }
    QString text;
    QFile file( url );
    if( file.exists() && file.open( IO_ReadOnly ) )
	text = QString( file.readAll() );
    if( text.isNull() )
	text = tr( "Failed to open about application contents in file: '%1'" ).arg( url );

    QMessageBox box( this );
    box.setText( text );
    box.setCaption( Config::configuration()->aboutApplicationMenuText() );
    box.setIcon( QMessageBox::NoIcon );
    box.exec();
}

void MainWindow::find()
{
    if ( !findDialog )
	findDialog = new FindDialog( this );
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->setSelection(
        0, findDialog->comboFind->lineEdit()->text().length() );
    findDialog->show();
}

void MainWindow::goHome()
{
    showLink( Config::configuration()->homePage() );
}

void MainWindow::print()
{
    QPrinter printer;
    printer.setFullPage( TRUE );
    if ( printer.setup( this ) ) {
	QPaintDeviceMetrics screen( this );
	printer.setResolution( screen.logicalDpiY() );
	QPainter p;
	if ( !p.begin( &printer ) )
	    return;

	qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );
	qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

	QPaintDeviceMetrics metrics(p.device());
	QTextBrowser *browser = tabs->currentBrowser();
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
	    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

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

	qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
	qApp->restoreOverrideCursor();
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

    QFile f( QDir::homeDirPath() + "/.assistant/bookmarks." +
	Config::configuration()->profileName() );
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

void MainWindow::showLinkFromClient( const QString &link )
{
    raise();
    setActiveWindow();
    showLink( link );
}

void MainWindow::showLink( const QString &link )
{
    if( link.isEmpty() ) {
	qWarning( "The link is empty!" );
    }

    int find = link.find( '#' );
    QString name = find >= 0 ? link.left( find ) : link;

    QString absLink = link;
    QFileInfo fi( name );
    if ( fi.isRelative() ) {
	if ( find >= 0 )
	    absLink = fi.absFilePath() + link.right( link.length() - find );
	else
	    absLink = fi.absFilePath();
    }
    if( fi.exists() ) {
	tabs->setSource( absLink );
	tabs->currentBrowser()->setFocus();
    } else {
	// ### Default 404 site!
	statusBar()->message( tr( "Failed to open link: '%1'" ).arg( link ), 5000 );
	tabs->currentBrowser()->setText( tr( "<div align=\"center\"><h1>The page could not be found!</h1><br>"
					     "<h3>'%1'</h3></div>").arg( link ) );
	tabs->updateTitle( tr( "Error..." ) );
    }
}

void MainWindow::showQtHelp()
{
    showLink( QString( qInstallPathDocs() ) + "/html/index.html" );
}

void MainWindow::setFamily( const QString & f )
{
    QFont fnt( tabs->font() );
    fnt.setFamily( f );
    tabs->setFont( fnt );
}

void MainWindow::showSettingsDialog()
{
    showSettingsDialog( -1 );
}

void MainWindow::showWebBrowserSettings()
{
    showSettingsDialog( 1 );
}

void MainWindow::showSettingsDialog( int page )
{
    if ( !settingsDia ){
	settingsDia = new SettingsDialog( this );
    }
    QFontDatabase fonts;
    settingsDia->fontCombo->insertStringList( fonts.families() );
    settingsDia->fontCombo->lineEdit()->setText( tabs->QWidget::font().family() );
    settingsDia->fixedfontCombo->insertStringList( fonts.families() );
    settingsDia->fixedfontCombo->lineEdit()->setText( tabs->styleSheet()->item( "pre" )->fontFamily() );
    settingsDia->linkUnderlineCB->setChecked( tabs->linkUnderline() );
    settingsDia->colorButton->setPaletteBackgroundColor( tabs->palette().color( QPalette::Active, QColorGroup::Link ) );
    if ( page != -1 )
	settingsDia->settingsTab->setCurrentPage( page );

    int ret = settingsDia->exec();

    if ( ret != QDialog::Accepted )
	return;

    QObjectList lst = Toolbar->children();
    for (int i = 0; i < lst.size(); ++i) {
	QObject *obj = lst.at(i);
	if ( obj->isA( "QToolBarSeparator" ) ) {
	    delete obj;
	    break;
	}
    }

    setupGoActions();

    QFont fnt( tabs->QWidget::font() );
    fnt.setFamily( settingsDia->fontCombo->currentText() );
    tabs->setFont( fnt );
    tabs->setLinkUnderline( settingsDia->linkUnderlineCB->isChecked() );

    QPalette pal = tabs->palette();
    QColor lc = settingsDia->colorButton->paletteBackgroundColor();
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    tabs->setPalette( pal );

    QString family = settingsDia->fixedfontCombo->currentText();

    QStyleSheet *sh = tabs->styleSheet();
    sh->item( "pre" )->setFontFamily( family );
    sh->item( "code" )->setFontFamily( family );
    sh->item( "tt" )->setFontFamily( family );
    tabs->currentBrowser()->setText( tabs->currentBrowser()->text() );
    showLink( tabs->currentBrowser()->source() );
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
    mw->goHome();
    return mw;
}

void MainWindow::saveSettings()
{
    Config *config = Config::configuration();
    config->setFontFamily( tabs->font().family() );
    config->setFontSize( tabs->currentBrowser()->font().pointSize() );
    config->setFontFixedFamily( tabs->styleSheet()->item( "pre" )->fontFamily() );
    config->setLinkUnderline( tabs->linkUnderline() );
    config->setLinkColor( tabs->palette().color( QPalette::Active, QColorGroup::Link ).name() );
    config->setSource( tabs->currentBrowser()->source() );
    config->setSideBarPage( helpDock->tabWidget->currentPageIndex() );
    config->setGeometry( QRect( x(), y(), width(), height() ) );
    config->setMaximized( isMaximized() );
    config->save();
}

void MainWindow::saveToolbarSettings()
{
    QString mainWindowLayout;
    QTextStream ts( &mainWindowLayout, IO_WriteOnly );
    ts << *this;
    Config::configuration()->setMainWindowLayout( mainWindowLayout );
}

TabbedBrowser* MainWindow::browsers()
{
    return tabs;
}

void MainWindow::showSearchLink( const QString &link, const QStringList &terms )
{
    HelpWindow * hw = tabs->currentBrowser();
    hw->blockScrolling( TRUE );
    hw->setCursor( waitCursor );
    if ( hw->source() == link )
	hw->reload();
    else
	showLink( link );
    hw->sync();
    hw->setCursor( arrowCursor );

    hw->viewport()->setUpdatesEnabled( FALSE );
    int minPar = INT_MAX;
    int minIndex = INT_MAX;
    QStringList::ConstIterator it = terms.begin();
    for ( ; it != terms.end(); ++it ) {
	int para = 0;
	int index = 0;
	bool found = hw->find( *it, FALSE, TRUE, TRUE, &para, &index );
	while ( found ) {
	    if ( para < minPar ) {
		minPar = para;
		minIndex = index;
	    }
	    hw->setColor( red );
	    found = hw->find( *it, FALSE, TRUE, TRUE );
	}
    }
    hw->blockScrolling( FALSE );
    hw->viewport()->setUpdatesEnabled( TRUE );
    hw->setCursorPosition( minPar, minIndex );
    hw->updateContents();
}


void MainWindow::showGoActionLink()
{
    const QObject *origin = sender();
    if( !origin ||
	origin->metaObject()->className() != QString( "QAction" ) )
	return;

    QAction *action = (QAction*) origin;
    QString docfile = *( goActionDocFiles->find( action ) );    
    showLink( docfile );
}

void MainWindow::showAssistantHelp()
{
    showLink( Config::configuration()->assistantDocPath() + "/assistant.html" );
}

HelpDialog* MainWindow::helpDialog()
{
    return helpDock;
}

void MainWindow::backwardAvailable( bool enable )
{
    actionGoPrevious->setEnabled( enable );
}

void MainWindow::forwardAvailable( bool enable )
{
    actionGoNext->setEnabled( enable );
}

void MainWindow::updateProfileSettings()
{
    Config *config = Config::configuration();
#ifndef Q_WS_MACX
    setIcon( config->applicationIcon() );
#endif
    helpMenu->clear();
    actionHelpAssistant->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAbout_Qt_AssistantAction->addTo( helpMenu );
    if ( !config->aboutApplicationMenuText().isEmpty() )
	actionAboutApplication->addTo( helpMenu );
    helpMenu->insertSeparator();
    actionHelpWhatsThis->addTo( helpMenu );

    actionAboutApplication->setMenuText( config->aboutApplicationMenuText() );

    if( !config->title().isNull() )
	setCaption( config->title() );
}
