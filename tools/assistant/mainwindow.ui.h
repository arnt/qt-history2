#include <qtabwidget.h>
#include <qaccel.h>


static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};


QPtrList<MainWindow> *MainWindow::windows = 0;

void MainWindow::init()
{
    setIcon( logo_xpm );
    if ( !windows )
	windows = new QPtrList<MainWindow>;
    windows->append( this );
    setWFlags( WDestructiveClose );
    browser = new HelpWindow( this, this, "qt_assistant_helpwindow" );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    QAccel *acc = new QAccel( this );
    acc->connectItem( acc->insertItem( Key_F5 ),
		      browser,
		      SLOT( reload() ) );
    setCentralWidget( browser );

    settings = 0L;

#ifdef QT_PALMTOPCENTER_DOCS
    QSettings settings;
    settings.insertSearchPath( QSettings::Unix,
			       QDir::homeDirPath() + "/.palmtopcenter/" );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString dir = settings.readEntry( "/palmtopcenter/qtopiadir" );
    if ( dir.isEmpty() )
	dir = getenv( "PALMTOPCENTERDIR" );
    QString lang = settings.readEntry( "/palmtopcenter/language" );
    if ( lang.isEmpty() )
	lang = getenv( "LANG" );
    browser->mimeSourceFactory()->addFilePath( dir + "/doc/" + lang );
    browser->mimeSourceFactory()->addFilePath( dir + "/doc/en/" );
    browser->mimeSourceFactory()->setExtensionType("html","text/html;charset=UTF-8");
    actionGoLinguist->removeFrom( goMenu );
    actionGoLinguist->removeFrom( Toolbar );
    actionGoQt->removeFrom( goMenu );
    actionGoQt->removeFrom( Toolbar );
    actionGoDesigner->removeFrom( goMenu );
    actionGoDesigner->removeFrom( Toolbar );
#else
    // #### hardcoded paths - probably should read the settings from somewhere
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/tools/linguist/doc/html" );
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/doc/html/" );    
    browser->mimeSourceFactory()->addFilePath( QString( getenv( "QTDIR" ) ) + "/doc/html/" );
#endif

    connect( actionGoPrev, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( actionGoNext, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( actionEditCopy, SIGNAL( activated() ), browser, SLOT( copy() ) );
    connect( actionFileExit, SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );

    QDockWindow *dw = new QDockWindow;
    helpDock = new HelpDialog( dw, this, browser );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addDockWindow( dw, DockLeft );
    dw->setWidget( helpDock );
    dw->setCaption( "Sidebar" );
    dw->setFixedExtentWidth( 250 );

    connect( helpDock, SIGNAL( showLink( const QString&, const QString & ) ),
	     this, SLOT( showLink( const QString&, const QString & ) ) );

    connect( bookmarkMenu, SIGNAL( activated( int ) ),
	     this, SLOT( showBookmark( int ) ) );

    setupBookmarkMenu();
    connect( browser, SIGNAL( highlighted( const QString & ) ), statusBar(), SLOT( message( const QString & ) ) );
    connect( actionZoomIn, SIGNAL( activated() ), browser, SLOT( zoomIn() ) );
    connect( actionZoomOut, SIGNAL( activated() ), browser, SLOT( zoomOut() ) );

    PopupMenu->insertItem( tr( "Vie&ws" ), createDockWindowMenu() );

    QAccel *a = new QAccel( this, dw );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+T") ) ), helpDock, SLOT( toggleContents() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+I") ) ), helpDock, SLOT( toggleIndex() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+B") ) ), helpDock, SLOT( toggleBookmarks() ) );
    a = new QAccel( dw );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+T") ) ), helpDock, SLOT( toggleContents() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+I") ) ), helpDock, SLOT( toggleIndex() ) );
    a->connectItem( a->insertItem( QAccel::stringToKey( tr("Ctrl+B") ) ), helpDock, SLOT( toggleBookmarks() ) );

    // read configuration
    QString keybase("/Qt Assistant/3.1/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( config.readEntry( keybase + "Family", fnt.family() ) );
    fnt.setPointSize( config.readNumEntry( keybase + "Size", fnt.pointSize() ) );
    browser->setFont( fnt );
    browser->setLinkUnderline( config.readBoolEntry( keybase + "LinkUnderline", TRUE ) );

    QPalette pal = browser->palette();
    QColor lc( config.readEntry( keybase + "LinkColor", pal.color( QPalette::Active, QColorGroup::Link ).name() ) );
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    browser->setPalette( pal );

    QString family = config.readEntry( keybase + "FixedFamily", browser->styleSheet()->item( "pre" )->fontFamily() );

    QStyleSheet *sh = browser->styleSheet();
    sh->item( "pre" )->setFontFamily( family );
    sh->item( "code" )->setFontFamily( family );
    sh->item( "tt" )->setFontFamily( family );
    browser->setStyleSheet( sh );

    QApplication::sendPostedEvents();
    QString fn = QDir::homeDirPath() + "/.assistanttbrc";
    QFile f( fn );
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	ts >> *this;
	f.close();
    }

    helpDock->tabWidget->setCurrentPage( config.readNumEntry( keybase + "SideBarPage", 0 ) );

    if ( !config.readBoolEntry( keybase  + "GeometryMaximized", FALSE ) ) {
	QRect r( pos(), size() );
	r.setX( config.readNumEntry( keybase + "GeometryX", r.x() ) );
	r.setY( config.readNumEntry( keybase + "GeometryY", r.y() ) );
	r.setWidth( config.readNumEntry( keybase + "GeometryWidth", r.width() ) );
	r.setHeight( config.readNumEntry( keybase + "GeometryHeight", r.height() ) );

	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}
    }

    QString source = config.readEntry( keybase + "Source", QString::null );
    QString title = config.readEntry( keybase + "Title", source );
    if ( !source.isEmpty() )
	showLink( source, title );
    else
	goHome();
}

void MainWindow::destroy()
{
    windows->removeRef( this );
    if ( windows->isEmpty() ) {
	delete windows;
	windows = 0;
    }
    saveSettings();
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
#ifdef QT_PALMTOPCENTER_DOCS
    showLink( "qtopiadesktop.html", "Qtopia Desktop Documentation" );
#else
    // #### we need a general Qt frontpage with links to Qt Class docu, Designer Manual, Linguist Manual, etc,
    showLink( "index.html", "Qt Reference Documentation" );
#endif
}

void MainWindow::showLinguistHelp()
{
    showLink( "linguist-manual.html", tr( "Qt Linguist Manual" ) );
}

void MainWindow::print()
{
    QPrinter printer;
    printer.setFullPage(TRUE);
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
	QSimpleRichText richText( browser->text(), browser->font(), browser->context(), browser->styleSheet(),
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
	    if ( view.top()  >= richText.height() )
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
    showLink( "designer-manual.html", tr( "Qt Designer Manual" ) );
}

void MainWindow::showLink( const QString & link, const QString & title )
{    
    browser->setCaption( title );
    browser->setSource( link );
    browser->setFocus();
}

void MainWindow::showQtHelp()
{
#ifdef QT_PALMTOPCENTER_DOCS
    showLink( "palmtopcenter.html", tr( "Qt Palmtopcenter Documentation" ) );
#else
    showLink( "index.html", tr( "Qt Reference Documentation" ) );
#endif
}

void MainWindow::setFamily( const QString & f )
{
    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( f );
    browser->setFont( fnt );
}

void MainWindow::showSettingsDialog()
{
    if ( !settings ){
	settings = new SettingsDialog( this );
	connect( settings, SIGNAL( changedPath() ), helpDock, SLOT( generateNewDoc() ));
	connect( settings, SIGNAL( changedCategory() ), helpDock, SLOT( showCatDoc() ));
    }
    QFontDatabase fonts;
    settings->fontCombo->insertStringList( fonts.families() );
    settings->fontCombo->lineEdit()->setText( browser->QWidget::font().family() );
    settings->fixedfontCombo->insertStringList( fonts.families() );
    settings->fixedfontCombo->lineEdit()->setText( browser->styleSheet()->item( "pre" )->fontFamily() );
    settings->linkUnderlineCB->setChecked( browser->linkUnderline() );
    settings->colorButton->setPaletteBackgroundColor( browser->palette().color( QPalette::Active, QColorGroup::Link ) );

    int ret = settings->exec();

    if ( ret != QDialog::Accepted )
	return;

    QFont fnt( browser->QWidget::font() );
    fnt.setFamily( settings->fontCombo->currentText() );
    browser->setFont( fnt );
    browser->setLinkUnderline( settings->linkUnderlineCB->isChecked() );

    QPalette pal = browser->palette();
    QColor lc = settings->colorButton->paletteBackgroundColor();
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    browser->setPalette( pal );

    QString family = settings->fixedfontCombo->currentText();

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
    QString fn = QDir::homeDirPath() + "/.assistanttbrc";
    QFile f( fn );
    f.open( IO_WriteOnly );
    QTextStream ts( &f );
    ts << *this;
    f.close();
}
