void MainWindow::init()
{
    setWFlags( WDestructiveClose );
    browser = new HelpWindow( this, "qt_assistant_helpwindow" );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken ); 
    setCentralWidget( browser );
    
    // #### hardcoded paths - probably should read the settings from somewhere
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/tools/designer/manual" );
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/doc/html/designer" );
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/tools/linguist/doc/html" );
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/doc/html/" );
    
    connect( actionGoPrev, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( actionGoNext, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( actionEditCopy, SIGNAL( activated() ), browser, SLOT( copy() ) );
    connect( actionFileExit, SIGNAL( activated() ), qApp, SLOT( quit() ) );
    
    QDockWindow *dw = new QDockWindow;
    helpDock = new HelpDialog( dw, this, browser );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addDockWindow( dw, Left );
    dw->setWidget( helpDock );
    dw->setCaption( "Sidebar" );
    dw->setFixedExtentWidth( 250 );    
 
    connect( helpDock, SIGNAL( showLink( const QString&, const QString & ) ),
    		this, SLOT( showLink( const QString&, const QString & ) ) );
    
    goHome();
    
    connect( bookmarkMenu, SIGNAL( activated( int ) ),
		this, SLOT( showBookmark( int ) ) );
    
    setupBookmarkMenu();
    connect( browser, SIGNAL( highlighted( const QString & ) ), statusBar(), SLOT( message( const QString & ) ) );
    connect( actionZoomIn, SIGNAL( activated() ), browser, SLOT( zoomIn() ) );
    connect( actionZoomOut, SIGNAL( activated() ), browser, SLOT( zoomOut() ) ); 
    
    QAccel *a = new QAccel( this );
    a->insertItem( CTRL+Key_L, 100 );
    a->connectItem( 100, helpDock->editIndex, SLOT( setFocus() ) );
             
    QFontDatabase fonts;   
    fontComboBox->insertStringList( fonts.families() );   
    fontComboBox->lineEdit()->setText( browser->QWidget::font().family() );  
    
    QString keybase("/Qt Assistant/3.0/");
    QSettings config;  
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );  
    QFont fnt( browser->QWidget::font() ); 
    fnt.setFamily( config.readEntry( keybase + "Family", fnt.family() ) );
    fnt.setPointSize( config.readNumEntry( keybase + "Size", fnt.pointSize() ) );
    browser->setFont( fnt ); 
    
    PopupMenu->insertItem( tr( "Vie&ws" ), createDockWindowMenu() );
}

void MainWindow::destroy()
{
    QString keybase("/Qt Assistant/3.0/");
    QSettings config; 
    config.insertSearchPath( QSettings::Windows, "/Trolltech" ); 
    config.writeEntry( keybase + "Family",  browser->QWidget::font().family() );
    config.writeEntry( keybase + "Size",  browser->QWidget::font().pointSize() );
}

void MainWindow::about()
{
    static const char *about_text =
    "<p><b>About Qt Assistant</b></p>"
    "<p>The Qt documentation browser.</p>"
    "<p>Version 1.0</p>"
    "<p>(C) 2001 Trolltech AS</p>";
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
    // #### we need a general Qt frontpage with links to Qt Class docu, Designer Manual, Linguist Manual, etc,
    showLink( "index.html", "Qt Reference Documentation" );
}

void MainWindow::showLinguistHelp()
{
    showLink( "qt-translation-tools.html", tr( "Qt Linguist Manual" ) );  
}

void MainWindow::print()
{
    QPrinter printer;
    printer.setFullPage(TRUE);
    if ( printer.setup() ) {
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
	QFont font( "times", 10 );
	QStringList filePaths = browser->mimeSourceFactory()->filePath();
	QString file;
	QStringList::Iterator it = filePaths.begin();
	for ( ; it != filePaths.end(); ++it ) {
	    file = QUrl( *it, QUrl( browser->source() ).path() ).path();
	    if ( QFile::exists( file ) )
		break;
	    else
		file = QString::null;
	}
	if ( file.isEmpty() )
	    return;
	QFile f( file );
	if ( !f.open( IO_ReadOnly ) )
	    return;
	QTextStream ts( &f );
	QSimpleRichText richText( ts.read(), font, browser->context(), browser->styleSheet(),
				  browser->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }
}

void MainWindow::setupBookmarkMenu()
{
    bookmarkMenu->clear();
    bookmarks.clear();
    bookmarkMenu->insertItem( tr( "&Add Bookmark" ), helpDock, SLOT( addBookmark() ) );

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
	showLink( *bookmarks.find( id ), bookmarkMenu->text( id ) );
}

void MainWindow::showDesignerHelp()
{
    showLink( "book1.html", tr( "Qt Designer Manual" ) );
}

void MainWindow::showLink( const QString & link, const QString & title )
{
    browser->setCaption( title );
    browser->setSource( link );
    browser->setFocus();
}

void MainWindow::showQtHelp()
{
    showLink( "index.html", tr( "Qt Reference Documentation" ) ); 
}

void MainWindow::setFamily( const QString & f )
{
    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( f );
    browser->setFont( fnt );
}

