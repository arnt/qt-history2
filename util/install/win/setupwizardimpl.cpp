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
#include <qlayout.h>

#include <keyinfo.h>

#if defined(Q_OS_WIN32)
#include <process.h>
#endif

#if defined(EVAL) || defined(EDU)
#include <check-and-patch.h>
#endif

#if defined(EVAL)
#  define LICENSE_DEST "LICENSE.EVAL"
#elif defined(EDU)
#  define LICENSE_DEST "LICENSE.EDU"
#else
#  define LICENSE_DEST "LICENSE"
#endif

#include "resource.h"
#include "pages/sidedecorationimpl.h"

#define FILESTOCOPY 4582

#define MSVCNET_BUTTON 0
#define MSVC_BUTTON    2
#define BORLAND_BUTTON 3

static const char* const logo_data[] = {
"32 32 238 2",
"Qt c None",
"#u c #000000",
".# c #020204",
"a. c #102322",
"af c #282500",
"as c #292e26",
"a8 c #2c686a",
"ae c #307072",
"#C c #322a0c",
"#s c #36320c",
"am c #3b3d3f",
"#3 c #3c8082",
"#f c #3e3a0c",
"## c #423e0c",
"#9 c #434341",
"ad c #438888",
"aU c #458d8e",
"#g c #46420c",
"aM c #46494a",
"ay c #474948",
"#D c #4a4328",
".W c #4a4611",
"az c #4a4641",
"a1 c #4a4a49",
"aH c #4b9e9e",
"au c #4d9a9f",
"aS c #4e9a9a",
"an c #4f4e4a",
".X c #504e0c",
"a7 c #51a4a9",
"#0 c #525250",
"aT c #55a6a3",
".Y c #56520c",
"#a c #5a5604",
".Z c #5e5a0c",
".V c #5e5e5c",
"a0 c #5e5e60",
"a6 c #5ea0a6",
".J c #625e0c",
"bB c #64aaa9",
"#m c #665e2c",
"aL c #686867",
"bw c #68acb2",
"bo c #696928",
"ba c #696967",
"aE c #69aeb2",
"#z c #6a5614",
".K c #6a660c",
"aZ c #6a6a65",
"bG c #6db4b4",
".9 c #6e5e24",
"#. c #6e6a5c",
"bv c #6fb6b9",
"bC c #706d28",
"br c #70bcc5",
"aQ c #71b7ba",
".I c #726234",
".L c #726e0c",
".0 c #72720c",
"#w c #746d44",
"be c #747028",
"bH c #747428",
".M c #76720a",
"aR c #78c1c2",
"#Z c #797977",
"a2 c #7a5d3d",
"#H c #7a6614",
"#I c #7a760a",
"#l c #7a7634",
".1 c #7a7a0c",
"#e c #7a7a5c",
"bL c #7bc0c2",
"b. c #7c7d82",
"#d c #7e6e34",
".N c #7e7a0a",
"bP c #816c20",
".8 c #82763c",
"#h c #827a3c",
".x c #827e0c",
"#t c #827f4b",
".O c #828204",
"#v c #828384",
".P c #868604",
"bq c #87d4d9",
"#k c #89864b",
"#c c #8a8244",
".y c #8a8604",
"#j c #8d8652",
"al c #8d8d8a",
"#b c #8e8644",
".z c #8e8e04",
"aW c #8f9094",
"#i c #908952",
"#Q c #909021",
"ag c #90d0d2",
"bO c #916f34",
"bQ c #91cdd3",
".7 c #928a44",
"#p c #928e6c",
"#P c #947f2f",
".A c #949204",
"bh c #949495",
".6 c #968e4c",
"aC c #999721",
".w c #9a8a44",
"#M c #9a9a99",
"ap c #9b9b21",
".5 c #9c924c",
"#R c #9c9a04",
"#7 c #9d9d9b",
"ao c #9e7641",
".4 c #9e964c",
"#J c #9e9b21",
".B c #9e9e04",
"ac c #9e9e9d",
"#S c #a09e21",
"ax c #a0a0a3",
"aK c #a1a1a2",
"aX c #a1a1a4",
".r c #a2a204",
"#1 c #a2a221",
"aF c #a2e1dd",
".3 c #a49a54",
".2 c #a69e54",
"bR c #a78446",
"#6 c #a9a9a8",
".T c #aaa254",
".s c #aaaa04",
"#W c #abaaa6",
"aN c #ac8861",
".S c #aea25c",
".R c #aea65c",
".t c #aeae04",
"#L c #b0b0b0",
"#o c #b2ae94",
".u c #b2b204",
"aI c #b2b2b4",
"b# c #b3b3b2",
"#X c #b4b4b6",
"#V c #b5b4b4",
".Q c #b6aa5c",
".n c #b6b604",
"aY c #b6b6b7",
"bN c #b79658",
"ah c #b7e5e3",
"aG c #b7ebe9",
"ar c #b9d9dc",
"#8 c #bcbcbe",
"ab c #bdbdbe",
".m c #beae5c",
".F c #beb264",
"aq c #bef6f6",
"aB c #c1a470",
"#F c #c1c1c3",
".E c #c2b664",
"at c #c2e9eb",
"bI c #c39c6a",
"bs c #c3a366",
"#U c #c3c3c0",
"aw c #c3c3c1",
"#G c #c3c3c7",
"aD c #c3f1f2",
"a# c #c6c6c3",
"#2 c #c7edf3",
".D c #c8ba6c",
"bM c #c9a470",
"#N c #c9c9c4",
".C c #cabe6c",
"ak c #cacaca",
"bx c #cbb076",
"aa c #cbcbc9",
"a3 c #ccac7f",
".H c #ceba54",
"#E c #ceced0",
"bi c #cfaf7e",
"#Y c #cfcfcb",
"bK c #d1ac80",
"#5 c #d1d1cf",
"bu c #d2ae83",
"bm c #d3b180",
"bD c #d3b384",
"bF c #d4b589",
"aJ c #d4d4d3",
".j c #d6c664",
".v c #d6c674",
"#K c #d6d6d5",
"bJ c #d7b588",
"bd c #d8b289",
"bz c #d8b78d",
".q c #d8ca74",
"aj c #d8d8d9",
"bb c #dabd97",
"a5 c #dcba91",
"bE c #dcc097",
"aA c #ddc292",
"aP c #dec491",
".p c #dece75",
"bk c #dfc79c",
"av c #e0e0e0",
"#A c #e2dabc",
"#O c #e2e2e4",
"aO c #e3c898",
"by c #e4c7a1",
".l c #e6da84",
"a4 c #e7c7a2",
"bt c #eacaa5",
".o c #eede84",
".G c #eee284",
".i c #eee294",
"bn c #efd7b4",
".k c #f2e69c",
".e c #f2eaa4",
"bc c #f3d8b8",
"bj c #f5e2c8",
"#r c #f6eea4",
".f c #f6eeb8",
".g c #f6f2cc",
".c c #faf6d4",
".d c #fafae4",
".U c #feee95",
"bA c #fef26c",
"#q c #fef2ac",
"#x c #fef2b8",
"bp c #fef684",
"bl c #fef690",
"bg c #fef69c",
"bf c #fef6a4",
".a c #fef6b4",
"#B c #fef6c4",
"#y c #fef6ce",
"a9 c #fefaac",
"aV c #fefab7",
"ai c #fefac4",
"#4 c #fefad1",
"#n c #fefadf",
".h c #fefaec",
"#T c #fefee6",
".b c #fefefa",
"QtQtQtQtQtQtQtQtQtQtQtQt.#QtQtQtQtQtQt.#QtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQt.#.#.a.#QtQtQtQt.#.b.#.#QtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.#.#.a.a.a.a.#QtQt.#.c.d.b.b.#.#QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQt.#.#.e.e.e.e.e.e.e.#.#.f.f.g.g.c.h.b.#.#QtQtQtQtQtQt",
"QtQtQtQt.#.#.i.i.i.i.i.i.i.i.j.j.b.b.k.e.f.f.g.g.c.d.#.#QtQtQtQt",
"QtQt.#.#.l.l.l.l.l.l.l.l.m.m.n.n.o.o.b.b.k.k.e.f.f.g.g.c.#.#QtQt",
"Qt.#.p.p.q.p.p.p.p.p.m.m.r.s.t.u.p.q.v.v.b.b.i.i.k.e.f.f.g.g.#Qt",
"QtQt.#.j.j.j.j.j.w.w.x.y.z.A.B.r.C.C.D.E.E.F.b.b.o.G.k.k.e.#QtQt",
"QtQtQt.#.H.H.I.I.J.K.L.M.N.O.P.z.Q.Q.Q.R.R.R.S.T.b.b.G.G.#QtQtQt",
"Qt.#.#.U.V.W.W.W.X.Y.Z.J.K.L.0.1.2.3.3.4.5.5.6.6.7.8.9#..b.#.#Qt",
"QtQtQt.#.U.U.U.W.W##.W.X.Y#a.J.K.7.7#b#b#b#c#c#d#d.h.h.b.#QtQtQt",
"QtQtQtQt.##e.U.U.U.W.W#f#g.W.X.Y#h#i#j#k#l#m#m#n#n.h#g.#QtQtQtQt",
"QtQtQtQt.##o#p#e#q#q#r#f#f#s#f#g#t#u#v#u#w#x#y#y#g#g#z.#QtQtQtQt",
"QtQtQtQt.#.b#A#o#p#e#B#B#B#C#C#D#u#E#F#G#u#y#g#g#H#I#J#uQtQtQtQt",
"QtQtQtQt.#.b.h.b#A#o#p#e.d.d.d#u#K#L#M#N#O#u#P#Q#R#S#u#u#uQtQtQt",
"QtQtQtQt.#.b#T.h#T#n#A#o#p#e#u#U#V#W#X#Y#Z#0#u#1#S#u#2#3#uQtQtQt",
"QtQtQtQt.##T.h#T#n#T#n#4#A#u#5#6#7#6#8#Z#0#9#u#S#u#2#3a.Qt#u#uQt",
"QtQtQtQt.##T#n#T#4#4#4#4#u#Oa#aaabac#Z#9#9#u#S#u#2adaeaf#uagah#u",
"QtQtQtQt.##n#n#4#4#4ai#u#Oajak#Yalamanao#u#Sap#uaqar#3asagatau#u",
"QtQtQtQt.##T#4#4#4ai#uav#O#OawaxayazaAaB#uapaC#uaDaEaFaGataH#uQt",
"QtQtQtQt.##4#4aiaiai#uaIaJ#OaKaLaMaNaOaPao#u#uaDaQaRaSaTaU#uQtQt",
"QtQtQtQt.##4aiaV.aaV#uaWaXaYaZa0a1a2a3a4a5ao#ua6a7a8#u#u#uQtQtQt",
"QtQtQtQt.#aiaiaiaV.aa9#ub.b#baa0#u#1#ubbbcbdao#ua8#ube.#.#.#.#Qt",
"QtQtQtQt.#aV.aaVaVbfbfbg#ubhba#u#1.AaC#ubibjbkao#ube#a.#.#.#.#.#",
"QtQtQtQt.#.aa9.abfa9bgbgbg#u#ubl.AaC#uaD#ubmbna5ao#ubo.#.#.#.#Qt",
"QtQtQtQt.#.aa9a9bgbgblblblblbpbpaC#uaDbqbr#ubsbtbuao#u.#.#QtQtQt",
"QtQtQtQtQt.#.#bgbgbgblblbpbpbpbp#uatbvbwa8#u#ubxbybzao#uQtQtQtQt",
"QtQtQtQtQtQtQt.#.#blblbpbpbAbp#uaDbBbwa8#ubebC#ubDbEbFao#uQtQtQt",
"QtQtQtQtQtQtQtQtQt.#.#bAbpbA#uaDbGbwa8#ubH.#.#Qt#ubIbJbKao#uQtQt",
"QtQtQtQtQtQtQtQtQtQtQt.#.##uaDbLbwa8#u.#.#QtQtQtQt#ubMbNbObP#uQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQt#ubQbwa8#u.#QtQtQtQtQtQtQt#ubRbO#uQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt#u#u#uQtQtQtQtQtQtQtQtQtQt#u#uQtQtQt"};

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

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags flag ) :
    QWizard( pParent, pName, modal, flag ),
    tmpPath( QEnvironment::getTempPath() ),
    filesCopied( false ),
    filesToCompile( 0 ),
    filesCompiled( 0 ),
    licensePage( 0 ),
    licenseAgreementPage( 0 ),
    optionsPage( 0 ),
    foldersPage( 0 ),
    configPage( 0 ),
    progressPage( 0 ),
    buildPage( 0 ),
    finishPage( 0 )
{
    // initialize
    if ( !pName )
	setName( "SetupWizard" );
    resize( 600, 390 );
    setCaption( trUtf8( "Qt Installation Wizard" ) );
    QPixmap logo( ( const char** ) logo_data );
    setIcon( logo );
    setIconText( trUtf8( "Qt Installation Wizard" ) );
    QFont f( font() );
    f.setFamily( "Arial" );
    f.setPointSize( 12 );
    f.setBold( TRUE );
    setTitleFont( f );

    totalFiles = 0;

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
	QString appDir = qApp->argv()[0];
	int truncpos = appDir.findRev( "/Contents/MacOS/" );
	if (truncpos != -1)
	    appDir.truncate( truncpos );
	archiveName  = appDir + "/Contents/Qt/qtmac.arq";
# endif
	ar.setPath( archiveName );
	if( ar.open( IO_ReadOnly ) )  {
	    archiveHeader = ar.readArchiveHeader();
	}
    }
#if defined(EVAL) || defined(EDU)
    int sysGroupButton = MSVC_BUTTON;
#else
    int sysGroupButton = BORLAND_BUTTON;
#endif

#if defined(Q_OS_WIN32)
    // First check for MSVC 6.0
    QString regValue = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++", "ProductDir", QEnvironment::LocalMachine );
    if ( regValue.length() )
	sysGroupButton = MSVC_BUTTON;

    // MSVC.NET 7.0 & 7.1 takes presedence over 6.0
    regValue = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\7.0", "InstallDir", QEnvironment::LocalMachine );
    QString regValue2 = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\7.1", "InstallDir", QEnvironment::LocalMachine );
    if ( QMAX( regValue.length(), regValue2.length() ) )
	sysGroupButton = MSVCNET_BUTTON;
#endif

    if ( archiveHeader ) {
	QString qt_version_str = archiveHeader->description();
	if ( !qt_version_str.isEmpty() )
	    globalInformation.setQtVersionStr( qt_version_str );

#if defined(EVAL) || defined(EDU)
	if ( archiveHeader->findExtraData( "compiler" ) == "borland" ) {
	    sysGroupButton = BORLAND_BUTTON;
	}
#endif
	delete archiveHeader;
    }

    initPages();
    initConnections();
    if ( optionsPage ) {
	optionsPage->sysGroup->setButton( sysGroupButton );
	clickedSystem( sysGroupButton );
    }
    readLicense( QDir::homeDirPath() + "/.qt-license" );
}

static bool copyFile( const QString& src, const QString& dest )
{
#ifdef Q_WS_WIN
    if ( qWinVersion() & Qt::WV_NT_based ) {
	bool res;
	TCHAR *tsrc = (TCHAR*)qt_winTchar_new( src );
	res = CopyFileW( tsrc, (TCHAR*)qt_winTchar( dest, TRUE ), false );
	delete tsrc;
	return res;
    } else {
	return CopyFileA( src.local8Bit(), dest.local8Bit(), false );
    }
#else
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
#endif
}

void SetupWizardImpl::initPages()
{
#define ADD_PAGE( var, Class ) \
    { \
    var = new Class( this, #var ); \
    SideDecorationImpl *sideDeco = new SideDecorationImpl( var ); \
    \
    Q_ASSERT( var->layout() != 0 ); \
    if ( var->layout()->inherits("QBoxLayout") ) { \
	((QBoxLayout*)var->layout())->insertWidget( 0, sideDeco ); \
	((QBoxLayout*)var->layout())->insertSpacing( 1, 10 ); \
    } \
    \
    pages.append( var ); \
    addPage( var, var->title() ); \
    setHelpEnabled( var, false ); \
    \
    connect( this, SIGNAL(wizardPages(const QPtrList<Page>&)), \
	    sideDeco, SLOT(wizardPages(const QPtrList<Page>&)) ); \
    connect( this, SIGNAL(wizardPageShowed(int)), \
	    sideDeco, SLOT(wizardPageShowed(int)) ); \
    connect( this, SIGNAL(wizardPageFailed(int)), \
	    sideDeco, SLOT(wizardPageFailed(int)) ); \
    connect( this, SIGNAL(editionString(const QString&)), \
	    sideDeco->editionLabel, SLOT(setText(const QString&)) ); \
    }

    QPtrList<Page> pages;
    if( globalInformation.reconfig() ) {
	ADD_PAGE( configPage,	ConfigPageImpl  )
	ADD_PAGE( buildPage,	BuildPageImpl   )
	ADD_PAGE( finishPage,	FinishPageImpl  )
    } else {
#if defined(Q_OS_WIN32)
	ADD_PAGE( winIntroPage,		WinIntroPageImpl	)
#endif
#if !defined(EVAL_CD)
	ADD_PAGE( licensePage,		LicensePageImpl		)
#endif
	ADD_PAGE( licenseAgreementPage, LicenseAgreementPageImpl)
	ADD_PAGE( optionsPage,		OptionsPageImpl		)
#if !defined(Q_OS_UNIX)
	ADD_PAGE( foldersPage,		FoldersPageImpl		)
#endif
	ADD_PAGE( configPage,		ConfigPageImpl		)
	ADD_PAGE( progressPage,		ProgressPageImpl		)
	ADD_PAGE( buildPage,		BuildPageImpl		)
	ADD_PAGE( finishPage,		FinishPageImpl		)
    }
#undef ADD_PAGE

    if ( licensePage ) {
	setNextEnabled( licensePage, FALSE );
    }
    if ( licenseAgreementPage ) {
	setNextEnabled( licenseAgreementPage, FALSE );
    }
    if ( progressPage ) {
	setBackEnabled( progressPage, FALSE );
	setNextEnabled( progressPage, FALSE );
    }
    if ( buildPage ) {
	setBackEnabled( buildPage, FALSE );
	setNextEnabled( buildPage, FALSE );
    }
    if ( finishPage ) {
	setBackEnabled( finishPage, FALSE );
	setFinishEnabled( finishPage, TRUE );
    }
    emit wizardPages( pages );
}

void SetupWizardImpl::initConnections()
{
    connect( &autoContTimer, SIGNAL( timeout() ), this, SLOT( timerFired() ) );

    if ( optionsPage ) {
	connect( optionsPage->sysGroup, SIGNAL(clicked(int)), SLOT(clickedSystem(int)));
	connect( optionsPage->installPathButton, SIGNAL(clicked()), SLOT(clickedPath()));
	connect( optionsPage->skipBuild, SIGNAL(clicked()), SLOT(clickedSkipBuild()));
    }
    if ( foldersPage ) {
	connect( foldersPage->folderPathButton, SIGNAL(clicked()), SLOT(clickedFolderPath()));
	connect( foldersPage->devSysPathButton, SIGNAL(clicked()), SLOT(clickedDevSysPath()));
    }
    if ( licenseAgreementPage ) {
	connect( licenseAgreementPage->licenceButtons, SIGNAL(clicked(int)), SLOT(licenseAction(int)));
    }
    if ( licensePage ) {
	connect( licensePage->readLicenseButton, SIGNAL(clicked()), SLOT(clickedLicenseFile()));
	connect( licensePage->customerID, SIGNAL(textChanged(const QString&)), SLOT(licenseChanged()));
	connect( licensePage->licenseID, SIGNAL(textChanged(const QString&)), SLOT(licenseChanged()));
	connect( licensePage->licenseeName, SIGNAL(textChanged(const QString&)), SLOT(licenseChanged()));
	connect( licensePage->expiryDate, SIGNAL(textChanged(const QString&)), SLOT(licenseChanged()));
	connect( licensePage->productsString, SIGNAL(activated(int)), SLOT(licenseChanged()));
	connect( licensePage->key, SIGNAL(textChanged(const QString&)), SLOT(licenseChanged()));
    }
    if ( configPage ) {
	connect( configPage->configList, SIGNAL(clicked(QListViewItem*)), SLOT(optionClicked(QListViewItem*)));
	connect( configPage->configList, SIGNAL(spacePressed(QListViewItem*)), SLOT(optionClicked(QListViewItem*)));
	connect( configPage->configList, SIGNAL(selectionChanged(QListViewItem*)), SLOT(optionSelected(QListViewItem*)));

	connect( configPage->advancedList, SIGNAL(clicked(QListViewItem*)), SLOT(optionClicked(QListViewItem*)));
	connect( configPage->advancedList, SIGNAL(spacePressed(QListViewItem*)), SLOT(optionClicked(QListViewItem*)));
	connect( configPage->advancedList, SIGNAL(selectionChanged(QListViewItem*)), SLOT(optionSelected(QListViewItem*)));

#if defined(EVAL) || defined(EDU)
	connect( configPage->installList, SIGNAL(selectionChanged(QListViewItem*)), SLOT(optionSelected(QListViewItem*)));
#endif

	connect( configPage->configTabs, SIGNAL(currentChanged(QWidget*)), SLOT(configPageChanged()));
    }
    if ( buildPage ) {
	connect( buildPage->restartBuild, SIGNAL(clicked()), this, SLOT(restartBuild()) );
    }
}

void SetupWizardImpl::stopProcesses()
{
    if( cleaner.isRunning() )
	cleaner.kill();
    if( configure.isRunning() )
	configure.kill();
    if( make.isRunning() )
	make.kill();
}

void SetupWizardImpl::clickedPath()
{
    QDir dir( optionsPage->installPath->text() );

#if defined(Q_OS_WIN32)
    if( !dir.exists() )
	dir.setPath( "C:\\Qt" );

    QString dest = QFileDialog::getExistingDirectory( optionsPage->installPath->text(), this, NULL, "Select installation directory" );
    if ( dest.isNull() )
	dest = "C:\\Qt";
    if ( dest.right(1) == "\\" )
	dest += "Qt";
    if ( dest.contains( QRegExp( "\\s" ) ) && !optionsPage->sysBorland->isChecked() )
	QMessageBox::warning( 0, "Invalid directory", "No whitespace is allowed in the directory name due to a limitation with MSVC" );
    else if ( dest.contains( "-" ) && optionsPage->sysBorland->isChecked() )
	QMessageBox::warning( 0, "Invalid directory", "No '-' characters are allowed in the directory name due to a limitation with the Borland linker" );
    else {
	dir.setPath( dest );
	optionsPage->installPath->setText( QDir::convertSeparators(dir.absPath()) );
    }
#elif defined(Q_OS_MACX)
    if( !dir.exists() )
	dir.setPath( "/" );

    QString dest = QFileDialog::getExistingDirectory( optionsPage->installPath->text(), this, NULL, "Select installation directory" );
    if (!dest.isNull())
	optionsPage->installPath->setText( dest );
#endif
}

void SetupWizardImpl::clickedFolderPath()
{
    foldersPage->folderPath->setText( shell.selectFolder( foldersPage->folderPath->text(), ( foldersPage->folderGroups->currentItem() == 0 ) ) );
}

void SetupWizardImpl::clickedDevSysPath()
{
    QDir dir( foldersPage->devSysPath->text() );
    if( !dir.exists() )
	dir.setPath( devSysFolder );

    QString dest = QFileDialog::getExistingDirectory( dir.absPath(), this, 0, "Select the path to Microsoft Visual Studio" );
    if (!dest.isNull())
	foldersPage->devSysPath->setText( dest );
}

void SetupWizardImpl::clickedSystem( int sys )
{
#ifndef Q_OS_MACX
    switch ( sys ) {
	case MSVCNET_BUTTON:
	    globalInformation.setSysId( GlobalInformation::MSVCNET );
	    break;
	case MSVC_BUTTON:
	    globalInformation.setSysId( GlobalInformation::MSVC );
	    break;
	case BORLAND_BUTTON:
	    globalInformation.setSysId( GlobalInformation::Borland );
	    break;
	default:
	    break;
    }
#endif
}

void SetupWizardImpl::licenseAction( int act )
{
    if( act )
	setNextEnabled( licenseAgreementPage, false );
    else
	setNextEnabled( licenseAgreementPage, true );
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
		if ( !globalInformation.reconfig() ) {
		    if ( currentOLine.right( 4 ) == ".cpp" ||
			 currentOLine.right( 2 ) == ".c" ||
			 currentOLine.right( 4 ) == ".pro" ||
			 currentOLine.right( 3 ) == ".ui" ) {
			buildPage->compileProgress->setProgress( ++filesCompiled );
		    }
		}
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
		    buildPage->compileProgress->setProgress( ++filesCompiled );

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
    if ( !filist )
	return;
    QFileInfoListIterator it( *filist );
    QFileInfo* fi;
    while( ( fi = it.current() ) ) {
	if( fi->fileName()[0] != '.' && // Exclude dot-dirs
	    fi->fileName() != "sql" ) { // Exclude SQL-dir
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
#if defined(Q_OS_WIN32)
    // install the precompiled MS integration
    QDir installDir( optionsPage->installPath->text() );
    if ( globalInformation.sysId() == GlobalInformation::MSVC ) {
	QDir addinsDir( foldersPage->devSysPath->text() );
	addinsDir.cd( "Common/MSDev98/Addins" );
	if ( copyFile( installDir.filePath("qmsdev.dll"), addinsDir.filePath("qmsdev.dll") ) ) {
	    installDir.remove( "qmsdev.dll" );
	}
	installDir.remove( "QMsNetSetup.msi" );
    } else if ( globalInformation.sysId() == GlobalInformation::MSVCNET ){
	if ( optionsPage->installNETIntegration->isChecked() ) {
	    QString filepath = installDir.filePath("QMsNetSetup.msi");
	    filepath = filepath.replace( '/', '\\' );

	    int res = _spawnlp( _P_NOWAIT, "msiexec.exe", "msiexec.exe", "-i", filepath.latin1(), NULL );
	    if ( res == -1 ) {
		//MSIExec is not in path, look up in registry (only works for NT machines)
		QString msiexec = QEnvironment::getRegistryString( "SYSTEM\\CurrentControlSet\\Services\\MSIServer", 
								   "ImagePath", 
								   QEnvironment::LocalMachine );
		if ( !msiexec.isEmpty() )
		    msiexec.remove( " /V" );
		res = _spawnl( _P_NOWAIT, msiexec.latin1(), msiexec.latin1(), "-i", filepath.latin1(), NULL );
	    }

	    if ( res == -1 ) {
		QMessageBox::warning( this, "Couldn't execute .NET addin installer script", 
				      "Microsoft Installer (MSI) was not found on your system.\n"
				      "Please install MSI, then execute the .NET addin installer "
				      "script,\nlocated at " + filepath );
	    }
	}
	installDir.remove( "qmsdev.dll" );
    } else {
	installDir.remove( "qmsdev.dll" );
	installDir.remove( "QMsNetSetup.msi" );
    }

    QString dirName, examplesName, tutorialsName;
    bool common( foldersPage->folderGroups->currentItem() == 0 );
    QString qtDir = QEnvironment::getEnv( "QTDIR" );

    QFile *autoexp  = 0;
    QFile *usertype = 0;
    switch( globalInformation.sysId() ) {
	case GlobalInformation::MSVC:
	    autoexp = new QFile( foldersPage->devSysPath->text() + "\\Common\\MsDev98\\bin\\autoexp.dat" );
	    usertype = new QFile( foldersPage->devSysPath->text() + "\\Common\\MsDev98\\bin\\usertype.dat" );
	    break;
	case GlobalInformation::MSVCNET:
	    autoexp = new QFile( foldersPage->devSysPath->text() + "\\Common7\\Packages\\Debugger\\autoexp.dat" );
	    usertype = new QFile( foldersPage->devSysPath->text() + "\\Common7\\Packages\\Debugger\\usertype.dat" );
	    break;
    }

    if ( autoexp ) {
	if ( !autoexp->exists() ) {
	    autoexp->open( IO_WriteOnly );
	} else {
	    autoexp->open( IO_ReadOnly );
	    QString existingUserType = autoexp->readAll();
	    autoexp->close();
	    if ( existingUserType.find( "; Trolltech Qt" ) == -1 )
		autoexp->open( IO_WriteOnly | IO_Append );
	}
	if( autoexp->isOpen() ) { // First try to open the file to search for existing installations
	    QTextStream outstream( autoexp );
	    outstream << endl;
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
	    autoexp->close();
	}
        delete autoexp;
    }

    if ( usertype ) {
	if ( !usertype->exists() ) {
	    usertype->open( IO_WriteOnly );
	} else {
	    usertype->open( IO_ReadOnly );
	    QString existingUserType = usertype->readAll();
	    usertype->close();
	    if ( existingUserType.find( "Q_OBJECT" ) == -1 )
		usertype->open( IO_WriteOnly | IO_Append );
	}
	if ( usertype->isOpen() ) {
	    QTextStream outstream( usertype );
	    outstream << endl;
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
	    usertype->close();
	}
        delete usertype;
    }


    /*
    ** Set up our icon folder and populate it with shortcuts.
    ** Then move to the next page.
    */
    dirName = shell.createFolder( foldersPage->folderPath->text(), common );
    shell.createShortcut( dirName, common, "Qt Designer", qtDir + "\\bin\\designer.exe", "GUI designer", "", qtDir );
#if !defined(EVAL) && !defined(EDU)
    shell.createShortcut( dirName, common, "Reconfigure Qt",
	    qtDir + "\\bin\\install.exe",
	    "Reconfigure the Qt library",
	    QString("-reconfig \"%1\"").arg(globalInformation.qtVersionStr()),
	    qtDir );
#endif
    shell.createShortcut( dirName, common, "License agreement", "notepad.exe", "Review the license agreement", "\"" + qtDir + "\\" LICENSE_DEST "\"" );
    shell.createShortcut( dirName, common, "Readme", "notepad.exe", "Important information", QString( "\"" ) + qtDir + "\\README\"" );
    shell.createShortcut( dirName, common, "Qt Assistant", qtDir + "\\bin\\assistant.exe", "Browse the On-line documentation", "", qtDir );
    shell.createShortcut( dirName, common, "Qt Linguist", qtDir + "\\bin\\linguist.exe", "Qt translation utility", "", qtDir );
    shell.createInternetShortcut( dirName, common, "Trolltech.com", "http://www.trolltech.com/" );
#if defined(EVAL_CD)
    shell.createInternetShortcut( dirName, common, "Register for Support", "http://www.trolltech.com/products/qt/evaluate.html" );
#endif

    if ( ( ( !globalInformation.reconfig() && optionsPage->skipBuild->isChecked() )
	 || ( globalInformation.reconfig() && !configPage->rebuildInstallation->isChecked() ) )
	|| qWinVersion() & WV_DOS_based ) {
	QString description;
#if defined(EVAL) || defined(EDU)
	buildQtShortcutText = "Build Qt Examples and Tutorials";
	description = "Build the Qt Examples and Tutorials";
#else
	buildQtShortcutText = "Build Qt " + globalInformation.qtVersionStr();
	description = "Build the Qt library";
#endif
	shell.createShortcut( dirName, common,
	    buildQtShortcutText,
	    QEnvironment::getEnv( "QTDIR" ) + "\\build.bat",
	    description );
    }

    if( optionsPage->installTutorials->isChecked() ) {
	if( qWinVersion() & WV_DOS_based ) {
	    shell.createShortcut( dirName, common,
		"Tutorials",
		QEnvironment::getEnv( "QTDIR" ) + "\\tutorial" );
	} else {
	    tutorialsName = shell.createFolder( foldersPage->folderPath->text() + "\\Tutorials", common );
	    installIcons( tutorialsName, QEnvironment::getEnv( "QTDIR" ) + "\\tutorial", common );
	}
    }
    if( optionsPage->installExamples->isChecked() ) {
	if( qWinVersion() & WV_DOS_based ) {
	    shell.createShortcut( dirName, common,
		"Examples",
		QEnvironment::getEnv( "QTDIR" ) + "\\examples" );
	} else {
	    examplesName = shell.createFolder( foldersPage->folderPath->text() + "\\Examples", common );
	    installIcons( examplesName, QEnvironment::getEnv( "QTDIR" ) + "\\examples", common );
	}
    }
#endif
    buildPage->compileProgress->setProgress( buildPage->compileProgress->totalSteps() );
}

void SetupWizardImpl::makeDone()
{
    if( !make.normalExit() || ( make.normalExit() && make.exitStatus() ) ) {
	logOutput( "The build process failed!\n" );
	emit wizardPageFailed( indexOf(currentPage()) );
	QMessageBox::critical( this, "Error", "The build process failed!\nSee the log for details." );
	buildPage->restartBuild->setText( "Restart compile" );
	setAppropriate( progressPage, false );
    } else {
	// We still have some more items to do in order to finish all the
	// integration stuff.
	if ( !globalInformation.reconfig() ) {
	    logOutput( "Doing the final integration steps..." );
	    doFinalIntegration();
	}
	buildPage->restartBuild->setText( "Success" );
	buildPage->restartBuild->setEnabled( FALSE );
	setNextEnabled( buildPage, true );
	logOutput( "The build was successful", true );
    }
}

void SetupWizardImpl::configDone()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake make gmake make nmake" );
    QStringList args;

    if( globalInformation.reconfig() && !configPage->rebuildInstallation->isChecked() )
	showPage( finishPage );

#if !defined(EVAL) && !defined(EDU)
    if( !configure.normalExit() || ( configure.normalExit() && configure.exitStatus() ) ) {
	logOutput( "The configure process failed.\n" );
	emit wizardPageFailed( indexOf(currentPage()) );
	buildPage->restartBuild->setText( "Restart configure" );
    } else
#endif
    {
	connect( &make, SIGNAL( processExited() ), this, SLOT( makeDone() ) );
	connect( &make, SIGNAL( readyReadStdout() ), this, SLOT( readMakeOutput() ) );
	connect( &make, SIGNAL( readyReadStderr() ), this, SLOT( readMakeError() ) );

	args << makeCmds[ globalInformation.sysId() ];
#if !defined(EVAL) && !defined(EDU)
	args << "sub-src";
	args << "sub-plugins";
	if ( optionsPage ) {
	    if ( optionsPage->installTools->isChecked() )
		args << "sub-tools";
	    if ( optionsPage->installTutorials->isChecked() )
		args << "sub-tutorial";
	    if ( optionsPage->installExamples->isChecked() )
		args << "sub-examples";
	    if ( optionsPage->installExtensions->isChecked() )
		args << "sub-extensions";
	}
#elif defined(Q_OS_WIN32)
	if ( optionsPage ) {
	    if ( optionsPage->installTutorials->isChecked() )
		args << "sub-tutorial";
	    if ( optionsPage->installExamples->isChecked() )
		args << "sub-examples";
	    if ( optionsPage->installExtensions->isChecked() )
		args << "sub-extensions";
	}
#endif

	make.setWorkingDirectory( QEnvironment::getEnv( "QTDIR" ) );
	make.setArguments( args );

	if( !make.start() ) {
	    logOutput( "Could not start make process.\n"
		       "Make sure that your compiler tools are installed\n"
		       "and registered correctly in your PATH environment." );
	    emit wizardPageFailed( indexOf(currentPage()) );
	    backButton()->setEnabled( TRUE );
	} else {
	    buildPage->restartBuild->setText( "Stop compilation" );
	}
    }
}

void SetupWizardImpl::restartBuild()
{
    if ( configure.isRunning() || 
       (!configure.isRunning() && (!configure.normalExit() || (configure.normalExit() && configure.exitStatus()))) ) {
	if ( configure.isRunning() ) {	// Stop configure
	    configure.kill();
	    buildPage->restartBuild->setText( "Restart configure" );
	    logOutput( "\n*** Configure stopped by user...\n" );
	    backButton()->setEnabled( TRUE );
	} else {			// Restart configure
	    wizardPageShowed( indexOf(currentPage()) );
	    backButton()->setEnabled( FALSE );
	    cleanDone();
	    buildPage->restartBuild->setText( "Stop configure" );
	    logOutput( "\n*** Configure restarted by user...\n" );
	}
    } else if ( make.isRunning() || 
	      (!make.isRunning() && (!make.normalExit() || (make.normalExit() && make.exitStatus()))) ) {
	if ( make.isRunning() ) {	// Stop compile
	    make.kill();
	    buildPage->restartBuild->setText( "Restart compile" );
	    logOutput( "\n*** Compilation stopped by user...\n" );
	    backButton()->setEnabled( TRUE );
	} else {			// Restart compile
	    wizardPageShowed( indexOf(currentPage()) );
	    backButton()->setEnabled( FALSE );
	    configDone();
	    buildPage->restartBuild->setText( "Stop compile" );
	    logOutput( "\n*** Compilation restarted by user...\n" );
	}
    }
}

void SetupWizardImpl::saveSettings()
{
#if !defined(EVAL) && !defined(EDU)
    QApplication::setOverrideCursor( Qt::waitCursor );
    saveSet( configPage->configList );
    saveSet( configPage->advancedList );
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
	    else
		settings.writeEntry( "/Trolltech/Qt/" + p->text(0), "Nothing selected" );
	}
    }
}

void SetupWizardImpl::showPage( QWidget* newPage )
{
    if ( currentPage() == configPage
	 && newPage == progressPage
	 && !verifyConfig() ) {
	QMessageBox::warning( this, "Invalid Configuration", 
			      "One or more of your selected options do not fullfill given\n"
			      "prerequisites. These options are marked with \"<--\"." );
	return;
    }

    QWizard::showPage( newPage );
    setInstallStep( indexOf(newPage) + 1 );

    if( newPage == licensePage ) {
	showPageLicense();
    } else if( newPage == licenseAgreementPage ) {
	readLicenseAgreement();
    } else if( newPage == optionsPage ) {
	showPageOptions();
    } else if( newPage == foldersPage ) {
	showPageFolders();
    } else if( newPage == configPage ) {
	showPageConfig();
    } else if( newPage == progressPage ) {
	showPageProgress();
    } else if( newPage == buildPage ) {
	showPageBuild();
    } else if( newPage == finishPage ) {
	showPageFinish();
    }
}

void SetupWizardImpl::showPageLicense()
{
    licenseChanged();
}

void SetupWizardImpl::showPageOptions()
{
    // First make sure that the current license information is saved
    if( !globalInformation.reconfig() )
	writeLicense( QDir::homeDirPath() + "/.qt-license" );

    // ### unsupported
    optionsPage->installDocs->hide();

    bool enterprise = licenseInfo[ "PRODUCTS" ] == "qt-enterprise";
    optionsPage->installExtensions->setChecked( enterprise );
    optionsPage->installExtensions->setEnabled( enterprise );

#if defined(EVAL) || defined(EDU)
    optionsPage->installDocs->setEnabled( FALSE );
    optionsPage->skipBuild->setEnabled( FALSE );
#  if defined(Q_OS_WIN32)
    optionsPage->installExamples->setEnabled( TRUE );
    optionsPage->installTutorials->setEnabled( TRUE );
    optionsPage->installExtensions->setChecked( TRUE );
    optionsPage->installExtensions->setEnabled( TRUE );
    optionsPage->installTools->setEnabled( FALSE );
#  else
    optionsPage->installExamples->setEnabled( FALSE );
    optionsPage->installTutorials->setEnabled( FALSE );
    optionsPage->installExtensions->setChecked( FALSE );
    optionsPage->installExtensions->setEnabled( FALSE );
#  endif
    if ( globalInformation.sysId()==GlobalInformation::Borland ) {
	optionsPage->sysMsvcNet->setEnabled( FALSE );
	optionsPage->sysMsvc->setEnabled( FALSE );
	optionsPage->sysBorland->setEnabled( FALSE );
    } else {
	optionsPage->sysMsvcNet->setEnabled( TRUE );
	optionsPage->sysMsvc->setEnabled( TRUE );
	optionsPage->sysBorland->setEnabled( FALSE );
    }
#else
#  if defined(Q_OS_WIN32)
    // No need to offer the option of skipping the build on 9x, it's skipped anyway
    if ( qWinVersion() & WV_DOS_based )
	optionsPage->skipBuild->setEnabled( FALSE ); 
#  endif
#endif
}

void SetupWizardImpl::showPageFolders()
{
    QStringList makeCmds = QStringList::split( ' ', "nmake.exe make.exe gmake.exe make nmake.exe" );
    QString makeCmd = makeCmds[ globalInformation.sysId() ];
    if( !findFile( makeCmd ) ) {
	QMessageBox::critical( this, "Environment problems",
				     "The installation program can't find the make tool '" + makeCmd + "'.\n"
				     "Make sure the path to it is present in the PATH environment\n"
				     "variable and restart the installation.\n"
				     "\n"
				     "You can find the path to the tool using the 'Find' tool\n"
				     "and add the location to the environment settings of your\n"
				     "system. Please contact your local system administration if\n"
				     "you have difficulties finding the files, or if you don't\n"
				     "know how to modifiy the environment settings of your system." );
	qApp->quit();
    }
    if ( globalInformation.sysId() != GlobalInformation::Borland && !findFile( "string.h" ) ) {
	QMessageBox::critical( this, "Environment problems",
			      "The file 'string.h' could not be located in any directory\n"
			      "listed in the 'INCLUDE' environment variable.\n"
			      "You might have to install the platform headers, or adjust\n"
			      "the environment variables of your system, and restart the\n"
			      "installation.\n\n"
			      "Please contact your local system administration if you have\n"
			      "difficulties finding the file, or if you don't know how to\n"
			      "modify the environment settings on your system." );
	qApp->quit();
    }

    QStringList devSys = QStringList::split( ';',"Microsoft Visual Studio 6.0 path;Borland C++ Builder path;GNU C++ path;MAC X buildtool path;Microsoft Visual Studio .NET path" );

    foldersPage->devSysLabel->setText( devSys[ globalInformation.sysId() ] );
    foldersPage->devSysPath->setEnabled( (globalInformation.sysId() == GlobalInformation::MSVC) || (globalInformation.sysId() == GlobalInformation::MSVCNET) );
    foldersPage->devSysPathButton->setEnabled( (globalInformation.sysId() == GlobalInformation::MSVC) || (globalInformation.sysId() == GlobalInformation::MSVCNET) );
#if defined(Q_OS_WIN32)
    if( globalInformation.sysId() == GlobalInformation::MSVC ) {
	QString devPath = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual Studio", "ProductDir", QEnvironment::LocalMachine );
	if ( devPath.isEmpty() ) {
	    // fallback for Windows 9x
	    QDir msdevDir( QEnvironment::getEnv("MSDEVDIR") );
	    msdevDir.cdUp();
	    msdevDir.cdUp();
	    devPath = QDir::convertSeparators( msdevDir.absPath() );
	}
	foldersPage->devSysPath->setText( devPath );
    } else if ( globalInformation.sysId() == GlobalInformation::MSVCNET ) {
	QString devPath = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\7.1\\Setup\\VS", "ProductDir", QEnvironment::LocalMachine );
	if ( !devPath.length() )
	    devPath = QEnvironment::getRegistryString( "Software\\Microsoft\\VisualStudio\\7.0\\Setup\\VS", "ProductDir", QEnvironment::LocalMachine );
	foldersPage->devSysPath->setText( devPath );
    }
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
	createDir( optionsPage->installPath->text() );
	progressPage->filesDisplay->append( "Installing files...\n" );

	// install the right LICENSE file
	QDir installDir( optionsPage->installPath->text() );
	QFile licenseFile( installDir.filePath( LICENSE_DEST ) );
	if ( licenseFile.open( IO_WriteOnly ) ) {
	    ResourceLoader *rcLoader;
#if defined(EVAL) || defined(EDU)
	    rcLoader = new ResourceLoader( "LICENSE" );
#else
	    if ( usLicense ) {
		rcLoader = new ResourceLoader( "LICENSE-US" );
	    } else {
		rcLoader = new ResourceLoader( "LICENSE" );
	    }
#endif
	    if ( rcLoader->isValid() ) {
		licenseFile.writeBlock( rcLoader->data() );
	    } else {
		emit wizardPageFailed( indexOf(currentPage()) );
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
	QString licenseKey;
#if !defined(EVAL_CD)
	licenseKey = licensePage->key->text();
#endif
	ar.setVerbosity( QArchive::Destination | QArchive::Verbose | QArchive::Progress );
	connect( &ar, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( archiveMsg( const QString& ) ) );
	connect( &ar, SIGNAL( operationFeedback( int ) ), progressPage->operationProgress, SLOT( setProgress( int ) ) );
	// First, try to find qt.arq as a binary resource to the file.
	ResourceLoader rcLoader( "QT_ARQ", 500 );
	if ( rcLoader.isValid() ) {
	    progressPage->operationProgress->setTotalSteps( rcLoader.data().count() );
	    QDataStream ds( rcLoader.data(), IO_ReadOnly );
	    ar.readArchive( &ds, optionsPage->installPath->text(), licenseKey );
	} else {
	    // If the resource could not be loaded or is smaller than 500
	    // bytes, we have the dummy qt.arq: try to find and install
	    // from qt.arq in the current directory instead.
	    QString archiveName = "qt.arq";
#if defined(Q_OS_MACX)
	    QString appDir = qApp->argv()[0];
	    int truncpos = appDir.findRev( "/Contents/MacOS/" );
	    if (truncpos != -1)
	    appDir.truncate( truncpos );
	    archiveName  = appDir + "/Contents/Qt/qtmac.arq";
#endif
	    fi.setFile( archiveName );
	    if( fi.exists() )
		totalSize = fi.size();
	    progressPage->operationProgress->setTotalSteps( totalSize );

	    ar.setPath( archiveName );
	    if( ar.open( IO_ReadOnly ) )  {
		ar.readArchive( optionsPage->installPath->text(), licenseKey );
	    } else {
		// We were not able to find any qt.arq -- so assume we have
		// the old fashioned zip archive and simply copy the files
		// instead.
		progressPage->operationProgress->setTotalSteps( FILESTOCOPY );
		copySuccessful = copyFiles( QDir::currentDirPath(), optionsPage->installPath->text(), true );

		/*These lines are only to be used when changing the filecount estimate
		  QString tmp( "%1" );
		  tmp = tmp.arg( totalFiles );
		  QMessageBox::information( this, tmp, tmp );
		 */
		progressPage->operationProgress->setProgress( FILESTOCOPY );
	    }
	}
	filesCopied = copySuccessful;

	timeCounter = 30;
	if( copySuccessful ) {
	    QDir installDir( optionsPage->installPath->text() );
#if defined(Q_OS_WIN32)
	    QDir windowsFolderDir( shell.windowsFolderName );
#  if !defined(EVAL) && !defined(EDU)
	    {
		// move $QTDIR/install.exe to $QTDIR/bin/install.exe
		// This is done because install.exe is also used to reconfigure Qt
		// (and this expects install.exe in bin). We can't move install.exe
		// to bin in first place, since for the snapshots, we don't have
		// the .arq archives.
		QFile inFile( installDir.filePath("install.exe") );
		QFile outFile( installDir.filePath("bin/install.exe") );

		if( inFile.exists() ) {
		    if( inFile.open( IO_ReadOnly ) ) {
			if( outFile.open( IO_WriteOnly ) ) {
			    outFile.writeBlock( inFile.readAll() );
			    outFile.close();
			    inFile.close();
			    inFile.remove();
			}
		    }
		}
	    }
#  endif
	    {
		// move the uninstaller to the Windows directory
		// This is necessary since the uninstaller deletes all files in
		// the installation directory (and therefore can't delete
		// itself).
		QFile inFile( installDir.filePath("bin/quninstall.exe") );
		QFile outFile( windowsFolderDir.filePath("quninstall.exe") );

		if( inFile.exists() ) {
		    if( inFile.open( IO_ReadOnly ) ) {
			if( outFile.open( IO_WriteOnly ) ) {
			    outFile.writeBlock( inFile.readAll() );
			    outFile.close();
			    inFile.close();
			    inFile.remove();
			}
		    }
		}
	    }
#endif
#if defined(EVAL) || defined(EDU)
	    QStringList::Iterator it;
	    QDir lib( optionsPage->installPath->text() );
	    lib.cd( "lib" );
#  if !defined(EVAL_CD)
	    // patch qt*.dll
#	if !defined(Q_OS_MAC)
	    QStringList qtDlls = lib.entryList( "qt*.dll" );
#	else
	    QStringList qtDlls = lib.entryList( "libqt-mt-eval.dylib" );
#	endif
	    if ( qtDlls.count() == 0 ) {
		copySuccessful = FALSE;
		QMessageBox::critical( this,
			tr( "Error patching Qt library" ),
#    if defined(EVAL)
			tr( "Could not patch the Qt library with the evaluation\n"
			    "license information - no Qt DLL was found." )
#    else
			tr( "Could not patch the Qt library with the educational\n"
			    "edition license information - no Qt DLL was found." )
#    endif
			);
	    }
	    for ( it=qtDlls.begin(); it!=qtDlls.end(); ++it ) {
		logFiles( tr("Patching the Qt library %1.").arg(*it) );
		int ret = trDoIt( lib.absFilePath(*it),
#    if defined(EVAL)
			licensePage->evalName->text().latin1(),
			licensePage->evalCompany->text().latin1(),
			licensePage->serialNumber->text().latin1(),
#    else
			"",
			licensePage->university->text().latin1(),
			licensePage->serialNumber->text().latin1(),
#    endif
			installDir.absPath()
			);
		if ( ret != 0 ) {
		    copySuccessful = FALSE;
		    QMessageBox::critical( this,
			    tr( "Error patching Qt library" ),
#    if defined(EVAL)
			    tr( "Could not patch the Qt library with the evaluation\n"
				"license information. You will not be able to execute\n"
				"any program linked against %1." ).arg( *it )
#    else
			    tr( "Could not patch the Qt library with the educational\n"
				"edition license information. You will not be able to\n"
				"execute any program linked against %1." ).arg( *it )
#    endif
			    );
		}
	    }
#  endif
#  if !defined(Q_OS_MACX)
	    // copy lib/*.dll bin/
	    QStringList dlls = lib.entryList( "*.dll" );
	    for ( it=dlls.begin(); it!=dlls.end(); ++it ) {
		copyFile( lib.absFilePath(*it), QDir::cleanDirPath(lib.absFilePath("../bin/"+*it)) );
	    }
	    // delete the non-wanted database drivers
	    QDir plugins( optionsPage->installPath->text() );
	    plugins.cd( "plugins" );
	    plugins.cd( "sqldrivers" );
	    QDir bin( optionsPage->installPath->text() );
	    bin.cd( "bin" );
	    if ( mysqlPluginInstall && !mysqlPluginInstall->isOn() ) {
		plugins.remove( "qsqlmysql.dll" );
		bin.remove( "libmySQL.dll" );
	    }
	    if ( ociPluginInstall && !ociPluginInstall->isOn() ) {
		plugins.remove( "qsqloci.dll" );
	    }
	    if ( odbcPluginInstall && !odbcPluginInstall->isOn() ) {
		plugins.remove( "qsqlodbc.dll" );
	    }
	    if ( psqlPluginInstall && !psqlPluginInstall->isOn() ) {
		plugins.remove( "qsqlpsql.dll" );
		bin.remove( "libpq.dll" );
	    }
	    if ( tdsPluginInstall && !tdsPluginInstall->isOn() ) {
		plugins.remove( "qsqltds.dll" );
	    }
	    // patch the .qmake.cache with the correct paths
	    QFile cacheFile( installDir.filePath(".qmake.cache") );
	    if ( cacheFile.open( IO_ReadOnly | IO_Translate ) ) {
		QTextStream tsIn( &cacheFile );
		QString cache = tsIn.read();
		cacheFile.close();
		if ( cacheFile.open( IO_WriteOnly | IO_Translate ) ) {
		    QTextStream tsOut( &cacheFile );
		    if ( globalInformation.sysId() == GlobalInformation::Borland )
			tsOut << cache.replace( "C:/QtEvaluation/qtborland", installDir.absPath() );
		    else
			tsOut << cache.replace( "C:/QtEvaluation/qtmsvc", installDir.absPath() );
		    cacheFile.close();
		}
	    }
#  endif
#endif
	    logFiles( tr("All files have been installed.\n"
			 "This log has been saved to the installation directory.\n"
			 "The build will start automatically in 30 seconds."), true );
	} else {
	    logFiles( tr("One or more errors occurred during file installation.\n"
			 "Please review the log and try to amend the situation.\n"), true );
	}
    }
    if ( copySuccessful ) {
#if defined(Q_OS_WIN32)
    /*
    ** Then record the installation in the registry, and set up the uninstallation
    */
    QStringList uninstaller;
    uninstaller << ( QString("\"") + shell.windowsFolderName + "\\quninstall.exe" + QString("\"") );
    uninstaller << optionsPage->installPath->text();

    if( foldersPage->folderGroups->currentItem() == 0 )
	uninstaller << ( QString("\"") + shell.commonProgramsFolderName + QString("\\") + foldersPage->folderPath->text() + QString("\"") );
    else
	uninstaller << ( QString("\"") + shell.localProgramsFolderName + QString("\\") + foldersPage->folderPath->text() + QString("\"") );

    uninstaller << ( QString("\"") + globalInformation.qtVersionStr() + QString("\"") );

    QEnvironment::recordUninstall( QString( "Qt " ) + globalInformation.qtVersionStr(), uninstaller.join( " " ) );
#endif
	autoContTimer.start( 1000 );
    }
    else
	emit wizardPageFailed( indexOf(currentPage()) );
    setNextEnabled( progressPage, copySuccessful );
}

void SetupWizardImpl::showPageFinish()
{
    autoContTimer.stop();
    nextButton()->setText( "Next >" );
    QString finishMsg;
    if ( ( ( !globalInformation.reconfig() && !optionsPage->skipBuild->isChecked() )
	    || ( globalInformation.reconfig() && configPage->rebuildInstallation->isChecked() ) )
#if defined(Q_OS_WIN32)
    && qWinVersion() & WV_NT_based ) {
#else
    ) {
#endif
	if( globalInformation.reconfig() ) {
	    finishMsg = "Qt has been reconfigured and rebuilt, and is ready for use.";
	} else {
#if defined(Q_OS_MACX)
            finishMsg = QString( "Qt has been installed to " ) + optionsPage->installPath->text() +
                        " and is ready to use.\n\nPlease try out the developer tools in the bin folder and example "
                        "programs in the examples folder.\n\nFor further information please consult the "
			"README.txt file included in the installation folder.";
#else
	    finishMsg = QString( "Qt has been installed to " ) + optionsPage->installPath->text() + " and is ready to use.";
#endif
	}
    } else {
	if( globalInformation.reconfig() ) {
		finishMsg = "The new configuration has been written.\nThe library needs to be rebuilt to activate the ";
		finishMsg += "new configuration.";
#if defined(Q_OS_WIN32)
                finishMsg += "To rebuild it, use the \"Build Qt ";
		finishMsg += globalInformation.qtVersionStr();
		finishMsg += "\" icon in the Qt program group in the start menu.";
#endif
	} else {
	    finishMsg = QString( "The Qt files have been installed to " ) + optionsPage->installPath->text() + " and is ready to be compiled.\n";
#if defined(Q_OS_WIN32)
	    if( persistentEnv && qWinVersion() & WV_DOS_based ) {
		finishMsg += "The environment variables needed to use Qt have been recorded into your AUTOEXEC.BAT file.\n";
		finishMsg += "Please review this file, and take action as appropriate depending on your operating system to get them into the persistent environment. (Windows Me users, run MsConfig)\n\n";
	    }
#  if defined(EVAL) || defined(EDU)
	    finishMsg += QString( "To build the examples and tutorials, use the "
				  "\"Build the Qt Examples and Tutorials\""
				  " icon which has been installed into your Start-Menu." );
#  else
	    finishMsg += QString( "To build Qt, use the "
				  "\"Build Qt " ) + globalInformation.qtVersionStr() + "\""
				  " icon which has been installed into your Start-Menu.";
#  endif
#endif
	}
    }
#if defined(EVAL_CD) && !defined(QSA)
    finishMsg += "\n\n"
		 "The Trolltech technical support service is available to "
		 "Qt Professional and Enterprise Edition licensees. As an "
		 "evaluation user, you can register for 30 days of evaluation "
		 "support at\n"
		 "http://www.trolltech.com/products/qt/evaluate.html";
#endif
    finishPage->finishText->setText( finishMsg );
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
	if ( xpPlugin->isOn() ) {
	    xpPlugin->setOn( false );
	    xpOff->setOn( true );
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
	xpPlugin->setEnabled( false );
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
	xpPlugin->setEnabled( true );
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
	if ( QMessageBox::information( this, "Are you sure?", "It will not be possible to build components "
				  "or plugins if you select the static build of the Qt library.\n"
				  "New features, e.g souce code editing in Qt Designer, will not "
				  "be available, "
				  "\nand you or users of your software might not be able "
				  "to use all or new features, e.g. new styles.\n\n"
				  "Are you sure you want to build a static Qt library?",
				  "OK", "Revert" ) ) {
		item->setOn( FALSE );
		if ( ( item = (QCheckListItem*)configPage->configList->findItem( "Shared", 0, 0 ) ) ) {
		item->setOn( TRUE );
		configPage->configList->setCurrentItem( item );
		setStaticEnabled( FALSE );
	    }
	}
	return;
    } else if ( item->text( 0 ) == "Shared" && item->isOn() ) {
	setStaticEnabled( FALSE );
	if( ( (QCheckListItem*)configPage->configList->findItem( "Non-threaded", 0, 0 ) )->isOn() ) {
	    if( QMessageBox::information( this, "Are you sure?", "Single-threaded, shared configurations "
								 "may cause instabilities because of runtime "
								 "library conflicts.", "OK", "Revert" ) ) {
		item->setOn( FALSE );
		if( ( item = (QCheckListItem*)configPage->configList->findItem( "Static", 0, 0 ) ) ) {
		    item->setOn( TRUE );
		    configPage->configList->setCurrentItem( item );
		    setStaticEnabled( TRUE );
		}
	    }
	}
    }
    else if( item->text( 0 ) == "Non-threaded" && item->isOn() ) {
	if( ( (QCheckListItem*)configPage->configList->findItem( "Shared", 0, 0 ) )->isOn() ) {
	    if( QMessageBox::information( this, "Are you sure?", "Single-threaded, shared configurations "
								 "may cause instabilities because of runtime "
								 "library conflicts.", "OK", "Revert" ) ) {
		item->setOn( FALSE );
		if( (item = (QCheckListItem*)configPage->configList->findItem( "Threaded", 0, 0 ) ) ) {
		    item->setOn( TRUE );
		    configPage->configList->setCurrentItem( item );
		}
	    }
	}
    } else if ( item==mngDirect || item==mngPlugin || item==mngOff ) {
	setJpegDirect( mngDirect->isOn() );
    }
}

void SetupWizardImpl::configPageChanged()
{
    if ( configPage->configList->isVisible() ) {
	configPage->configList->setSelected( configPage->configList->currentItem(), true );
	optionSelected( configPage->configList->currentItem() );
    } else if ( configPage->advancedList->isVisible() ) {
	configPage->advancedList->setSelected( configPage->advancedList->currentItem(), true );
	optionSelected( configPage->advancedList->currentItem() );
    }
#if defined(EVAL) || defined(EDU)
    else if ( configPage->installList->isVisible() ) {
	configPage->installList->setSelected( configPage->installList->currentItem(), true );
	optionSelected( configPage->installList->currentItem() );
    }
#endif
}

void SetupWizardImpl::licenseChanged()
{
#if defined(EVAL) || defined(EDU)
    int ret = trCheckIt(
#  if defined(EVAL)
	    licensePage->evalName->text().latin1(),
	    licensePage->evalCompany->text().latin1(),
	    licensePage->serialNumber->text().latin1()
#  else
	    "",
	    licensePage->university->text().latin1(),
	    licensePage->serialNumber->text().latin1()
#  endif
	    );

    if ( ret == 0 )
	setNextEnabled( licensePage, TRUE );
    else
	setNextEnabled( licensePage, FALSE );
    return;
#else
    QDate date;
    uint features;
    uint testFeature;
    QString platformString;
    QString licenseKey = licensePage->key->text().stripWhiteSpace();
    if ( licenseKey.length() != 14 ) {
	goto rejectLicense;
    }
    features = featuresForKey( licenseKey.upper() );
    date = decodedExpiryDate( licenseKey.mid(9) );
    if ( !date.isValid() ) {
	goto rejectLicense;
    }
#  if defined(Q_OS_MACX)
    testFeature = Feature_Mac;
    platformString = "Mac OS X";
#  elif defined(Q_OS_WIN32)
    testFeature = Feature_Windows;
    platformString = "Windows";
#  else
    testFeature = Feature_Unix;
    platformString = "UNIX";
#    ifdef Q_CC_GNU
#    warning "What about Qt/Embedded?"
#    endif
#  endif
    if ( !(features&testFeature) && currentPage() == licensePage ) {
	if ( features & (Feature_Unix|Feature_Windows|Feature_Mac|Feature_Embedded) ) {
	    int ret = QMessageBox::information( this,
		    tr("No %1 license").arg(platformString),
		    tr("You are not licensed for the %1 platform.\n"
		       "Please contact sales@trolltech.com to upgrade\n"
		       "your license to include the Windows platform.").arg(platformString),
		    tr("Try again"),
		    tr("Abort installation")
		    );
	    if ( ret == 1 ) {
		QApplication::exit();
	    } else {
		licensePage->key->setText( "" );
	    }
	}
	goto rejectLicense;
    }
    if ( date < QDate::currentDate() && currentPage() == licensePage ) {
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

    licensePage->expiryDate->setText( date.toString( Qt::ISODate ) );
    if( features & Feature_Enterprise ) {
	licensePage->productsString->setCurrentItem( 1 );
	emit editionString( "Enterprise Edition" );
    } else {
	licensePage->productsString->setCurrentItem( 0 );
	emit editionString( "Professional Edition" );
    }
    setNextEnabled( licensePage, TRUE );
    return;

rejectLicense:
    licensePage->expiryDate->setText( "" );
#  if defined(Q_OS_WIN32)
    //TODO: Is this a bug? It bus errors on MACX, ask rms.
    // it should work -- if it doesn't this seems to be a bug in the MACX code,
    // I guess (rms)
    licensePage->productsString->setCurrentItem( -1 );
#  endif
    emit editionString( "" );
    setNextEnabled( licensePage, FALSE );
    return;
#endif
}

void SetupWizardImpl::logFiles( const QString& entry, bool close )
{
    if( !fileLog.isOpen() ) {
	fileLog.setName( optionsPage->installPath->text() + QDir::separator() + "install.log" );
	if( !fileLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    QTextStream outstream( &fileLog );

    // ######### At the moment, there is a bug in QTextEdit. The log output
    // will cause the application to grow that much that we can't install the
    // evaluation packages on lusa -- for now, disable the output that we get
    // the packages out.
#if 0
    progressPage->filesDisplay->append( entry + "\n" );
#else
    progressPage->filesDisplay->setText( "Installing files...\n" + entry + "\n" );
#endif
    outstream << ( entry + "\n" );

    if( close )
	fileLog.close();
}

void SetupWizardImpl::logOutput( const QString& entry, bool close )
{
    static QTextStream outstream;
    if( !outputLog.isOpen() ) {
	QDir installDir;
	if ( optionsPage )
	    installDir.setPath( optionsPage->installPath->text() );
	else
	    installDir.setPath( QEnvironment::getEnv( "QTDIR" ) );
	outputLog.setName( installDir.filePath("build.log") );
	if( !outputLog.open( IO_WriteOnly | IO_Translate ) )
	    return;
    }
    outstream.setDevice( &outputLog );

    buildPage->outputDisplay->append( entry + "\n" );
    outstream << ( entry + "\n" );

    if( close )
	outputLog.close();
}

void SetupWizardImpl::archiveMsg( const QString& msg )
{
    if( msg.right( 7 ) == ".cpp..." || msg.right( 5 ) == ".c..." || msg.right( 7 ) == ".pro..." || msg.right( 6 ) == ".ui..." )
	filesToCompile++;
    qApp->processEvents();
    if ( msg.startsWith("Expanding") )
	// only show the "Expanding" entries to avoid flickering
	logFiles( msg );
}

#ifdef Q_WS_WIN
static HANDLE createFile( const QString &entryName, DWORD attr1, DWORD attr2 )
{
    if ( qWinVersion() & Qt::WV_NT_based ) {
	return ::CreateFileW( (WCHAR*)qt_winTchar( entryName, TRUE ), attr1, attr2, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    } else {
	return ::CreateFileA( entryName.local8Bit(), attr1, attr2, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    }
}
#endif

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
		    if ( fi->fileName() == "doc" )
			doCopy = optionsPage->installDocs->isChecked();
		    else if ( fi->fileName() == "tools" )
			doCopy = optionsPage->installTools->isChecked();
		    else if ( fi->fileName() == "tutorial" )
			doCopy = optionsPage->installTutorials->isChecked();
		    else if ( fi->fileName() == "examples" )
			doCopy = optionsPage->installExamples->isChecked();
		}
		if( doCopy )
		    if( !copyFiles( entryName, targetName, false ) )
			return false;
	    } else {
		if( qApp && !isHidden() ) {
		    qApp->processEvents();
		    progressPage->operationProgress->setProgress( totalFiles );
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
		if ( !QFile::exists( targetName ) )
		    res = copyFile( entryName, targetName );
#if defined(Q_OS_WIN32)
		if ( res ) {
		    totalFiles++;
		    HANDLE inFile, outFile;
		    if( inFile = createFile( entryName, GENERIC_READ, FILE_SHARE_READ ) ){
			if( outFile = createFile( targetName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE ) ){
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
#elif defined( Q_OS_UNIX )
		// ### TODO: keep file date the same, handle errors
#endif
	    }
	}
	++it;
    }
    return true;
}

void SetupWizardImpl::setInstallStep( int step )
{
    QString captionTxt;
#if defined(EVAL)
    captionTxt = tr("Qt Evaluation Version Installation Wizard");
#elif defined(EDU)
    captionTxt = tr("Qt Educational Edition Installation Wizard");
#else
    if( globalInformation.reconfig() )
	captionTxt = tr("Qt Configuration Wizard");
    else
	captionTxt = tr("Qt Installation Wizard");
#endif
    setCaption( tr("%1 - Step %2 of %3").arg( captionTxt ).arg( step ).arg( pageCount() ) );
    emit wizardPageShowed( step-1 );
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
#if !defined(EVAL) && !defined(EDU)
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

	if ( licensePage ) {
	    licensePage->customerID->setText( licenseInfo[ "CUSTOMERID" ] );
	    licensePage->licenseID->setText( licenseInfo[ "LICENSEID" ] );
	    licensePage->licenseeName->setText( licenseInfo[ "LICENSEE" ] );
	    if( licenseInfo[ "PRODUCTS" ] == "qt-enterprise" ) {
		licensePage->productsString->setCurrentItem( 1 );
		emit editionString( "Enterprise Edition" );
	    } else {
		licensePage->productsString->setCurrentItem( 0 );
		emit editionString( "Professional Edition" );
	    }
	    licensePage->expiryDate->setText( licenseInfo[ "EXPIRYDATE" ] );
	    licensePage->key->setText( licenseInfo[ "LICENSEKEY" ] );
	}
    }
#endif
}

void SetupWizardImpl::writeLicense( QString filePath )
{
#if !defined(EVAL) && !defined(EDU)
    QFile licenseFile( filePath );

    if( licenseFile.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream licStream( &licenseFile );

	licenseInfo[ "CUSTOMERID" ] = licensePage->customerID->text();
	licenseInfo[ "LICENSEID" ] = licensePage->licenseID->text();
	licenseInfo[ "LICENSEE" ] = licensePage->licenseeName->text();
	if( licensePage->productsString->currentItem() == 0 ) {
	    licenseInfo[ "PRODUCTS" ] = "qt-professional";
	    emit editionString( "Professional Edition" );
	} else {
	    licenseInfo[ "PRODUCTS" ] = "qt-enterprise";
	    emit editionString( "Enterprise Edition" );
	}

	licenseInfo[ "EXPIRYDATE" ] = licensePage->expiryDate->text();
	licenseInfo[ "LICENSEKEY" ] = licensePage->key->text();

	licStream << "# Toolkit license file" << endl;
	licStream << "CustomerID=\"" << licenseInfo[ "CUSTOMERID" ].latin1() << "\"" << endl;
	licStream << "LicenseID=\"" << licenseInfo[ "LICENSEID" ].latin1() << "\"" << endl;
	licStream << "Licensee=\"" << licenseInfo[ "LICENSEE" ].latin1() << "\"" << endl;
	licStream << "Products=\"" << licenseInfo[ "PRODUCTS" ].latin1() << "\"" << endl;
	licStream << "ExpiryDate=" << licenseInfo[ "EXPIRYDATE" ].latin1() << endl;
	licStream << "LicenseKey=" << licenseInfo[ "LICENSEKEY" ].latin1() << endl;

	licenseFile.close();
    }
#endif
}

void SetupWizardImpl::clickedLicenseFile()
{
    QString licensePath = QFileDialog::getOpenFileName( optionsPage->installPath->text(), QString::null, this, NULL, "Browse for license file" );

    if( !licensePath.isEmpty() )
	readLicense( licensePath );

}

bool SetupWizardImpl::findFile( const QString &fileName )
{
    QString file = fileName.lower();
    QStringList paths;
#if defined(Q_OS_WIN32)
    QRegExp split( "[;,]" );
#else
    QRegExp split( "[:]" );
#endif
    if ( file.endsWith( ".h" ) ) {
	if ( globalInformation.sysId() == GlobalInformation::Borland )
	    return TRUE;
	paths = QStringList::split( split, QEnvironment::getEnv( "INCLUDE" ) );
    } else if ( file.endsWith( ".lib" ) ) {
	if ( globalInformation.sysId() == GlobalInformation::Borland )
	    return TRUE;
	paths = QStringList::split( split, QEnvironment::getEnv( "LIB" ) );
    } else {
	paths = QStringList::split( split, QEnvironment::getEnv( "PATH" ) );
    }
    return findFileInPaths( file, paths );
}

bool SetupWizardImpl::findFileInPaths( const QString &fileName, const QStringList &paths )
{
	QDir d;
	for( QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it ) {
	    // Remove any leading or trailing ", this is commonly used in the environment
	    // variables
	    QString path = (*it);
	    if ( path.startsWith( "\"" ) )
		path = path.right( path.length() - 1 );
	    if ( path.endsWith( "\"" ) )
		path = path.left( path.length() - 1 );
	    if( d.exists( path + QDir::separator() + fileName ) )
		return true;
	}
	return false;
}

void SetupWizardImpl::readLicenseAgreement()
{
    // Intropage
    ResourceLoader *rcLoader;
#if defined(EVAL) || defined(EDU)
    rcLoader = new ResourceLoader( "LICENSE" );
#else
    if ( usLicense ) {
	rcLoader = new ResourceLoader( "LICENSE-US" );
    } else {
	rcLoader = new ResourceLoader( "LICENSE" );
    }
#endif
    if ( rcLoader->isValid() ) {
	licenseAgreementPage->introText->setText( rcLoader->data() );
	licenseAgreementPage->acceptLicense->setEnabled( TRUE );
    } else {
	emit wizardPageFailed( indexOf(currentPage()) );
	QMessageBox::critical( this, tr("Package corrupted"),
		tr("Could not find the LICENSE file in the package.\nThe package might be corrupted.") );
	licenseAgreementPage->acceptLicense->setEnabled( FALSE );
    }
    delete rcLoader;
}

bool SetupWizardImpl::findXPSupport()
{
    return findFile( "uxtheme.h" );
}

void SetupWizardImpl::accept()
{
#if defined(Q_OS_WIN32)
    if ( finishPage->showReadmeCheck->isChecked() ) {
	QProcess proc( QString("notepad.exe") );
	QString qtDir = QEnvironment::getEnv( "QTDIR" );
	proc.addArgument( qtDir + "\\README" );
	proc.start();
    }
#endif
    QDialog::accept();
}

void SetupWizardImpl::clickedSkipBuild()
{
    bool enable = !optionsPage->skipBuild->isChecked();
    optionsPage->installTools->setEnabled( enable );
    optionsPage->installExtensions->setEnabled( enable );
    optionsPage->installExamples->setEnabled( enable );
    optionsPage->installTutorials->setEnabled( enable );
}
