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

#include "config.h"

#include <qtabwidget.h>
#include <qfileinfo.h>
#include <qaccel.h>
#include <qobjectlist.h>
#include <qtimer.h>
#include <qdragobject.h>
#include <qfontinfo.h>
#include <qaccel.h>

QPtrList<MainWindow> *MainWindow::windows = 0;

void MainWindow::init()
{
#ifndef Q_WS_MACX
    setIcon( QPixmap::fromMimeSource( "appicon.png" ) );
#endif
    setupCompleted = FALSE;

    if ( !windows )
	windows = new QPtrList<MainWindow>;
    windows->append( this );
    setWFlags( WDestructiveClose );
    tabs = new TabbedBrowser( this, "qt_assistant_tabbedbrowser" );
    setCentralWidget( tabs );
    settingsDia = 0;

    Config *config = Config::configuration();

    dw = new QDockWindow( QDockWindow::InDock, this );
    helpDock = new HelpDialog( dw, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addDockWindow( dw, DockLeft );
    dw->setWidget( helpDock );
    dw->setCaption( "Sidebar" );
    dw->setFixedExtentWidth( 320 );

    setObjectsEnabled( FALSE );

    // read geometry configuration
    setupGoActions( config->docFiles(), config->selectedCategories() );

    if ( !config->isMaximized() ) {
	QRect geom = config->geometry();
	if( geom.isValid() ) {
	    QRect desktop =  QApplication::desktop()->geometry();
	    setGeometry( geom.intersect( desktop ) );
	}
    }

    QString mainWindowLayout = config->mainWindowLayout();

    QTextStream ts( &mainWindowLayout, IO_ReadOnly );
    ts >> *this;

//     QTimer::singleShot( 0, this, SLOT( setup() ) );
    setup();
}

void MainWindow::setup()
{
    if( setupCompleted )
	return;
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

    tabs->setup();

    setupBookmarkMenu();
    PopupMenu->insertItem( tr( "Vie&ws" ), createDockWindowMenu() );
    helpDock->tabWidget->setCurrentPage( config->sideBarPage() );

    setObjectsEnabled( TRUE );

    setupCompleted = TRUE;
}

void MainWindow::setupGoActions( const QStringList &docList, const QStringList &catList )
{
    Config * config = Config::configuration();
    QStringList docFiles = config->docFiles();
    QAction * action = 0;

    bool separatorInserted = FALSE;

    QStringList::ConstIterator it = docFiles.begin();
    for ( ; it != docFiles.end(); ++it ) {
	QString cat = config->docCategory( *it );
	QString title = config->docTitle( *it );
	QPixmap pix = config->docIcon( *it );
	if ( catList.contains( cat ) && !pix.isNull() ) {
	    if( !separatorInserted )
		separatorInserted = insertActionSeparator();
	    action = new QAction( title, QIconSet( pix ), title, 0, 0 );
	    action->addTo( goMenu );
	    action->addTo( Toolbar );
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
    QString about_text =
	"<center><img src=\"splash.png\">"
	"<p>Version " + QString(QT_VERSION_STR) + "</p>"
	"<p>Copyright (C) 2001-2002 Trolltech AS. All rights reserved.</p>"
	"</center><p></p>"
	"<p>This program is licensed to you under the terms of the GNU General "
	"Public License Version 2 as published by the Free Software Foundation. This "
	"gives you legal permission to copy, distribute and/or modify this software "
	"under certain conditions. For details, see the file 'LICENSE.GPL' that came with "
	"this software distribution. If you did not get the file, send email to "
	"info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY "
	"OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
	"FOR A PARTICULAR PURPOSE.</p>";
    QMessageBox box( this );
    box.setText( about_text );
    box.setCaption( tr( "Qt Assistant" ) );
    box.setIcon( QMessageBox::NoIcon );
    box.exec();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this, tr( "Qt Assistant" ) );
}

void MainWindow::find()
{
    if ( !findDialog ) {
	findDialog = new FindDialog( this );
//	findDialog->setBrowser( browser );
    }
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->setSelection(
        0, findDialog->comboFind->lineEdit()->text().length() );
    findDialog->show();
}

void MainWindow::goHome()
{
    QString home = Config::configuration()->homePage();

    if ( home.isEmpty() )
	showLink( QString( qInstallPathDocs() ) + "/html/index.html" );
    else
	showLink( QString( home ) );
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
    // ### introduce a default-not-found site
    if ( !fi.exists() ) {
	statusBar()->message( tr( "Failed to open link: '%1'" ).arg( link ) );
	tabs->setSource( "index.html" );
    }
    else {
	tabs->setSource( link );
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
	connect( settingsDia, SIGNAL( docuFilesChanged() ), helpDock, SLOT( generateNewDocu() ));
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
    /*
    actionGoQt->removeFrom( goMenu );
    actionGoQt->removeFrom( Toolbar );
    actionGoDesigner->removeFrom( goMenu );
    actionGoDesigner->removeFrom( Toolbar );
    actionGoAssistant->removeFrom( goMenu );
    actionGoAssistant->removeFrom( Toolbar );
    actionGoLinguist->removeFrom( goMenu );
    actionGoLinguist->removeFrom( Toolbar );
    */
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
    config->setFontSize( tabs->font().pointSize() );
    config->setFontFixedFamily( tabs->styleSheet()->item( "pre" )->fontFamily() );
    config->setLinkUnderline( tabs->linkUnderline() );
    config->setLinkColor( tabs->palette().color( QPalette::Active, QColorGroup::Link ).name() );
    config->setSource( tabs->currentBrowser()->source() );
    config->setSideBarPage( helpDock->tabWidget->currentPageIndex() );
    config->setGeometry( geometry() );
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
