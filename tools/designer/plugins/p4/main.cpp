#include <designerinterface.h>
#include <actioninterface.h>

#include "p4.h"

#include <qobjectcleanuphandler.h>
#include <qguardedptr.h>
#include <qaction.h>
#include <qdict.h>
#include <qpainter.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qvariant.h>

/* XPM */
static const char * const editmark_xpm[] ={
"12 8 2 1",
". c None",
"c c #ff0000",
".........ccc",
"........ccc.",
".......ccc..",
"ccc...ccc...",
".ccc.ccc....",
"..ccccc.....",
"...ccc......",
"....c.......",
};
/* XPM */
static const char * const addmark_xpm[]={
"8 8 3 1",
"b c #000000",
". c None",
"d c #ffff00",
"..bbbb..",
"..bddb..",
"bbbddbbb",
"bddddddb",
"bddddddb",
"bbbddbbb",
"..bddb..",
"..bbbb.."};
/* XPM */
static const char * const deletemark_xpm[]={
"8 7 3 1",
"d c #800000",
"# c #808080",
". c None",
".....dd#",
"ddd#dd#.",
"..ddd#..",
"...ddd..",
"..dd.dd.",
".dd#.#dd",
".d#...#d"};
/* XPM */
static const char * const sync_xpm[]={
"16 16 5 1",
"b c #000000",
"c c #000080",
"# c #808080",
". c None",
"a c #ffffff",
".......####.....",
".......#aa#b....",
".......#aa#ab.cc",
"..bb...#aa###cc.",
".......#accacc..",
"..bb...#aaccc#..",
".......#aaacab..",
".......bbbbbbb..",
"####............",
"#aa#b...........",
"#aa#ab..........",
"#aabbbb...bb....",
"#aaaaab.........",
"#aaaaab...bb....",
"#aaaaab.........",
"bbbbbbb........."};
/* XPM */
static const char * const edit_xpm[]={
"16 16 5 1",
"b c #000000",
"# c #808080",
". c None",
"c c #ff0000",
"a c #ffffff",
"........####....",
"........#aa#b...",
"........#aa#ab..",
"........#aabbbb.",
"........#aaaccc.",
"........#aacccb.",
"........#acccab.",
"...ccc..#cccbbb.",
"....ccc.ccc.....",
".....ccccc......",
"......ccc.......",
"..b....c...b....",
".bb........bb...",
"bbbbbb..bbbbbb..",
".bb........bb...",
"..b........b...."};
/* XPM */
static const char * const submit_xpm[]={
"16 16 5 1",
"b c #000000",
"c c #000080",
"# c #808080",
". c None",
"a c #ffffff",
"........####....",
"........#aa#b...",
"........#aa#ab..",
"........#aabbbb.",
"........#aaaccc.",
"........#aaccc#.",
"........#acccab.",
"...ccc..#cccbbb.",
"....ccc.ccc.....",
".....ccccc......",
"......ccc.......",
"...b...c..b.....",
"...bb....bb.....",
"bbbbbb..bbbbbb..",
"...bb....bb.....",
"...b......b....."};
/* XPM */
static const char * const revert_xpm[]={
"16 16 6 1",
"b c #000000",
"d c #0000ff",
"# c #808080",
". c None",
"c c #ff0000",
"a c #ffffff",
"......####......",
"......#aa#b.....",
"......#aa#ab.cc.",
"......#aa###cc..",
"......#accacc...",
"......#aaccc#...",
"......#aaacab...",
"......bbbbbbb...",
"................",
"................",
"....#dddd#......",
"d..dd....d#.....",
"ddd.......d.....",
"ddd.......d.....",
"dddd.....#d.....",
".........d......"};
/* XPM */
static const char * const add_xpm[]={
"16 16 6 1",
"b c #000000",
"c c #000080",
"# c #808080",
". c None",
"d c #ffff00",
"a c #ffffff",
".......####.....",
".......#aa#b....",
".......#aa#ab.cc",
".......#aa###cc.",
".......#accacc..",
".......#aaccc#..",
".......#aaacab..",
".......bbbbbbb..",
"..bbbb..........",
"..bddb..........",
"bbbddbbb........",
"bddddddb........",
"bddddddb........",
"bbbddbbb........",
"..bddb..........",
"..bbbb.........."};
/* XPM */
static const char * const delete_xpm[]={
"16 16 6 1",
"b c #000000",
"c c #000080",
"d c #800000",
"# c #808080",
". c None",
"a c #ffffff",
".......####.....",
".......#aa#b....",
".......#aa#ab.cc",
".......#aa###cc.",
".......#accacc..",
".......#aaccc#..",
".......#aaacab..",
".......bbbbbbb..",
"................",
".....dd#........",
"ddd#dd#.........",
"..ddd#..........",
"...ddd..........",
"..dd.dd.........",
".dd#.#dd........",
".d#...#d........"};
/* XPM */
static const char * const report_xpm[]={
"16 16 5 1",
"c c #000000",
"a c #000080",
"# c #808080",
". c None",
"b c #ffffff",
"......####....aa",
"......#bb##..aa.",
"......#bbaa#aa..",
"......#b##aaa...",
"...####bbbba#...",
"...#bb#b###bc...",
"...#bb#bbbbbc...",
"...#bbccccccc...",
"####bbbbbc......",
"#bb#b###bc......",
"#bb#bbbbbc......",
"#bbccccccc......",
"#bbbbbc.........",
"#bbbbbc.........",
"#bbbbbc.........",
"ccccccc........."};
/* XPM */
static const char * const diff_xpm[]={
"16 16 7 1",
"b c #000000",
"c c #000080",
"e c #0000ff",
"d c #00ffff",
"# c #808080",
". c None",
"a c #ffffff",
"......####......",
"......#aa#b.....",
"......#aa#ab.cc.",
"......#aa###cc..",
"......#accacc...",
"....bbbaaccc#...",
"...badabaacab...",
"..badad.bbbbb...",
"##bdad.db.......",
"#abad.d.b.......",
"#aab.d.b##......",
"#aaabbb#.ee.....",
"#aaaaab.cdee....",
"#aaaaab..cdee...",
"#aaaaab...cde...",
"bbbbbbb....bb..."};
/* XPM */
static const char * const refresh_xpm[]={
"16 16 5 1",
"b c #000000",
"c c #000080",
"# c #808080",
". c None",
"a c #ffffff",
"......####......",
"......#aa#b.....",
"......#aa#ab.cc.",
"......#aa###cc..",
"..b...#accacc...",
".bbb..#aaccc#...",
".bbb..#aaacab...",
".bbb..bbbbbbb...",
".bbb............",
".#b#............",
"..b.............",
"................",
".#b#............",
".bbb............",
".#b#............",
"................"};

class P4Interface : public QObject, public ActionInterface, public QLibraryInterface
{
    Q_OBJECT

public:
    P4Interface();
    ~P4Interface();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QAction *create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname ) const;
    void connectTo( QUnknownInterface *ai );
    bool location( const QString &actionname, Location l ) const;

    bool init();
    void cleanup();
    bool canUnload() const;

private slots:
    void p4Aware( bool );
    void p4Sync();
    void p4Edit();
    void p4Submit();
    void p4Revert();
    void p4Add();
    void p4Delete();
    void p4Diff();
    void p4Refresh();

    void p4MightEdit( bool b );
    void formChanged();
    void p4Info( const QString& filename, P4Info* );
    void statusMessage( const QString& );
    void reloadFile( const QString & );

private:
    bool aware;
    QAction *actionSync;
    QAction *actionEdit;
    QAction *actionSubmit;
    QAction *actionRevert;
    QAction *actionAdd;
    QAction *actionDelete;
    QAction *actionDiff;
    QActionGroup *p4Actions;

    QGuardedPtr<QWidget> outputPage;
    QTextView *outputView;
    QObjectCleanupHandler actions;
    DesignerInterface *appInterface;

    unsigned long ref;
};

P4Interface::P4Interface()
    : p4Actions( 0 ), outputPage( 0 ), outputView( 0 ), appInterface( 0 ), ref( 0 )
{
    aware = TRUE;
}

P4Interface::~P4Interface()
{
    if ( appInterface )
	appInterface->release();
}

QStringList P4Interface::featureList() const
{
    QStringList list;
    list << "P4";
    return list;
}

void P4Interface::connectTo( QUnknownInterface *ai )
{
    if ( !appInterface && ai ) {
	QUnknownInterface *temp = appInterface;

	ai->queryInterface( IID_Designer, &temp );
	appInterface = (DesignerInterface*)temp;
	if ( !appInterface )
	    return;

	outputPage = appInterface->outputDock() ? appInterface->outputDock()->addView( "P4 Source Control" ) : 0;
	QVBoxLayout *box = new QVBoxLayout( outputPage );
	outputView = new QTextView( outputPage );
	box->addWidget( outputView );

	outputView->show();
	appInterface->onFormChange( this, SLOT( formChanged() ) );

	P4Init* init = new P4Init;
	connect( init, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
	init->execute();
    }
}

bool P4Interface::location( const QString &/*actionname*/, Location /*l*/ ) const
{
    return TRUE;
}

QAction* P4Interface::create( const QString& actionname, QObject* parent )
{
    if ( actionname != "P4" )
	return 0;

    if ( !p4Actions ) {
	p4Actions = new QActionGroup( parent, 0, FALSE );

	QAction *a = new QAction( "P4 Aware", QIconSet((const char**)report_xpm), "A&ware", 0, p4Actions, "P4 Aware", TRUE );
	a->setToolTip( tr("Toggle Edit Awareness") );
	a->setStatusTip( tr("Toggles whether forms in source control should be checked out before edited") );
	a->setWhatsThis( tr("") );
	connect( a, SIGNAL( toggled(bool) ), this, SLOT( p4Aware(bool) ) );
	a->setOn( aware );

	p4Actions->addSeparator();

	actionSync = new QAction( "P4 Sync", QIconSet((const char**)sync_xpm), "&Sync", 0, p4Actions, "P4 Sync" );
	actionSync->setToolTip( tr("Sync to Head Revision") );
	actionSync->setStatusTip( tr("Synchronizes client file to depot head revision") );
	connect( actionSync, SIGNAL( activated() ), this, SLOT( p4Sync() ) );

	actionEdit = new QAction( "P4 Edit", QIconSet((const char**)edit_xpm), "&Edit", 0, p4Actions, "P4 Edit" );
	actionEdit->setToolTip( tr("Check Out for Edit") );
	actionEdit->setStatusTip( tr("Checks out file for edit") );
	connect( actionEdit, SIGNAL( activated() ), this, SLOT( p4Edit() ) );

	actionSubmit = new QAction( "P4 Submit", QIconSet((const char**)submit_xpm), "&Submit", 0, p4Actions, "P4 Submit" );
	actionSubmit->setToolTip( tr("Submit Changes") );
	actionSubmit->setStatusTip( tr("Submits changed form(s) to depot") );
	connect( actionSubmit, SIGNAL( activated() ), this, SLOT( p4Submit() ) );

	actionRevert = new QAction( "P4 Revert", QIconSet((const char**)revert_xpm), "&Revert", 0, p4Actions, "P4 Revert" );
	actionRevert->setToolTip( tr("Revert Changes") );
	actionRevert->setStatusTip( tr("Reverts changes to form(s)") );
	connect( actionRevert, SIGNAL( activated() ), this, SLOT( p4Revert() ) );

	p4Actions->addSeparator();

	actionAdd = new QAction( "P4 Add", QIconSet((const char**)add_xpm), "&Add", 0, p4Actions, "P4 Add" );
	actionAdd->setToolTip( tr("Add Form") );
	actionAdd->setStatusTip( tr("Adds form to source control") );
	connect( actionAdd, SIGNAL( activated() ), this, SLOT( p4Add() ) );

	actionDelete = new QAction( "P4 Delete", QIconSet((const char**)delete_xpm), "&Delete", 0, p4Actions, "P4 Delete" );
	actionDelete->setToolTip( tr("Check Out for Delete") );
	actionDelete->setStatusTip( tr("Checks out file for delete") );
	connect( actionDelete, SIGNAL( activated() ), this, SLOT( p4Delete() ) );

	p4Actions->addSeparator();

	actionDiff = new QAction( "P4 Diff", QIconSet((const char**)diff_xpm), "Di&ff", 0, p4Actions, "P4 Diff" );
	actionDiff->setToolTip( tr("Diff Against Depot") );
	actionDiff->setStatusTip( tr("Opens diff for client file against depot file") );
	connect( actionDiff, SIGNAL( activated() ), this, SLOT( p4Diff() ) );

	a = new QAction( "P4 Refresh", QIconSet((const char**)refresh_xpm), "Refres&h", 0, p4Actions, "P4 Refresh" );
	a->setToolTip( tr("Refresh") );
	a->setStatusTip( tr("Refreshes state of forms") );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Refresh() ) );

	actions.add( p4Actions );

	actionSync->setEnabled( FALSE );
	actionEdit->setEnabled( FALSE );
	actionSubmit->setEnabled( FALSE );
	actionRevert->setEnabled( FALSE );
	actionAdd->setEnabled( FALSE );
	actionDelete->setEnabled( FALSE );
	actionDiff->setEnabled( FALSE );
    }

    return p4Actions;
}

QString P4Interface::group( const QString & ) const
{
    return "P4";
}

void P4Interface::p4Aware( bool on )
{
    aware = on;
}

void P4Interface::p4Sync()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Sync *sync = new P4Sync( fwIface->fileName() );
    connect( sync, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( sync, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    connect( sync, SIGNAL( fileChanged( const QString & ) ), this, SLOT( reloadFile( const QString & ) ) );
    sync->execute();
}

void P4Interface::p4Edit()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Edit *edit = new P4Edit( fileName, TRUE );
    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    connect( edit, SIGNAL( fileChanged( const QString & ) ), this, SLOT( reloadFile( const QString & ) ) );
    edit->execute();
}

void P4Interface::p4Submit()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Submit *submit = new P4Submit( fileName );
    connect( submit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( submit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    submit->execute();
}

void P4Interface::p4Revert()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Revert *revert = new P4Revert( fileName );
    connect( revert, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( revert, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    connect( revert, SIGNAL( fileChanged( const QString & ) ), this, SLOT( reloadFile( const QString & ) ) );
    revert->execute();
}

void P4Interface::p4Add()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Add *add = new P4Add( fileName );
    connect( add, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( add, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    add->execute();
}

void P4Interface::p4Delete()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Delete *del = new P4Delete( fileName );
    connect( del, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( del, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    del->execute();
}

void P4Interface::p4Diff()
{
    if ( !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    P4Diff *diff = new P4Diff( fileName );
    connect( diff, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( diff, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    diff->execute();
}

void P4Interface::p4Refresh()
{
    if ( !appInterface )
	return;

    P4Info::files()->clear();

    QPtrList<DesignerProject> projects = appInterface->projectList();
    QPtrListIterator<DesignerProject> pit( projects );
    while ( pit.current() ) {
	DesignerProject *pIface = pit.current();
	++pit;

	QPtrList<DesignerFormWindow> forms = pIface->formList();
	QPtrListIterator<DesignerFormWindow> fit( forms );
	while ( fit.current() ) {
	    DesignerFormWindow *fwIface = fit.current();
	    ++fit;
	    QString filename = fwIface->fileName();
	    if ( !!filename ) {
		P4FStat* fs = new P4FStat( filename );
		connect( fs, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
		connect( fs, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
		fs->execute();
	    }
	}
    }
    formChanged();
}

void P4Interface::p4MightEdit( bool b )
{
    if ( !aware || !b || !appInterface )
	return;

    QString fileName;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    DesignerFormWindow *fwIface = appInterface->currentForm();
    if ( sfIface )
	fileName = sfIface->fileName();
    else if ( fwIface )
	fileName = fwIface->fileName();
    else
	return;

    if ( fileName.isEmpty() )
	return;

    P4Edit *edit = new P4Edit( fileName, FALSE );
    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    edit->execute();
}

void P4Interface::formChanged()
{
    DesignerFormWindow *fwIface = 0;

    if ( !appInterface )
	return;

    QString filename;
    DesignerSourceFile *sfIface = appInterface->currentSourceFile();
    if ( sfIface ) {
	filename = sfIface->fileName();
    } else if ( ( fwIface = appInterface->currentForm() ) ) {
	filename = fwIface->fileName();
	fwIface->onModificationChange( this, SLOT( p4MightEdit( bool ) ) );
    }

    if ( filename.isEmpty() ) {
	actionSync->setEnabled( FALSE );
	actionEdit->setEnabled( FALSE );
	actionSubmit->setEnabled( FALSE );
	actionRevert->setEnabled( FALSE );
	actionAdd->setEnabled( FALSE );
	actionDelete->setEnabled( FALSE );
	actionDiff->setEnabled( FALSE );
	return;
    }

    P4Info* p4i = P4Info::files()->find( filename );
    if ( !p4i ) {
	P4FStat* fs = new P4FStat( filename );
	connect( fs, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
	connect( fs, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
	fs->execute();
	return;
    }
    p4Info( filename, p4i );
}

void P4Interface::p4Info( const QString &filename, P4Info *p4i )
{
    if ( !p4i )
	return;

    QWidget *form = 0;
    DesignerFormWindow *fwIface = 0;

    QPtrList<DesignerProject> projects = appInterface->projectList();
    QPtrListIterator<DesignerProject> pit( projects );
    while ( pit.current() ) {
	DesignerProject *pIface = pit.current();
	++pit;

	QPtrList<DesignerFormWindow> forms = pIface->formList();
	QPtrListIterator<DesignerFormWindow> fit( forms );
	while ( fit.current() ) {
	    fwIface = fit.current();
	    ++fit;
	    if ( fwIface->fileName() == filename ) {
		form = fwIface->form();
		break;
	    }
	}
	if ( form )
	    break;
    }

    QPixmap pix;
    if ( form )
	pix = form->property( "icon" ).toPixmap();
    else
	statusMessage( QString( "No form for file %1" ).arg( filename ) );

    if ( p4i->controlled ) {
// 	if ( pix.isNull() ) {
// 	    pix = QPixmap( 22,22,32 );
// 	    pix.fill( Qt::color0 );
// 	}
// 	QPainter paint( &pix );
// 	paint.setRasterOp( AndROP );
// 	paint.fillRect( 0, 0, pix.width() - 1 , pix.height() - 1, p4i->uptodate ? gray : red );
// 	paint.setRasterOp( CopyROP );
	actionAdd->setEnabled( FALSE );
	actionDelete->setEnabled( TRUE );
	if ( p4i->action != P4Info::None ) {
// 	    switch ( p4i->action ) {
// 	    case P4Info::Edit:
// 		{
// 		    QPixmap check( (const char**)editmark_xpm );
// 		    paint.drawPixmap( ( pix.width() - check.width() ) / 2, ( pix.height() - check.height() ) / 2, check );
// 		}
// 		break;
// 	    case P4Info::Add:
// 		{
// 		    QPixmap add( (const char**)addmark_xpm );
// 		    paint.drawPixmap( ( pix.width() - add.width() ) / 2, ( pix.height() - add.height() ) / 2, add );
// 		}
// 		break;
// 	    case P4Info::Delete:
// 		{
// 		    QPixmap del( (const char**)deletemark_xpm );
// 		    paint.drawPixmap( ( pix.width() - del.width() ) / 2, ( pix.height() - del.height() ) / 2, del );
// 		}
// 		break;
// 	    default:
// 		break;
// 	    }

	    actionSync->setEnabled( FALSE );
	    actionEdit->setEnabled( FALSE );
	    actionSubmit->setEnabled( TRUE );
	    actionRevert->setEnabled( TRUE );
	    actionDiff->setEnabled( TRUE );
	} else {
	    actionSync->setEnabled( !p4i->uptodate );
	    actionEdit->setEnabled( TRUE );
	    actionSubmit->setEnabled( FALSE );
	    actionRevert->setEnabled( FALSE );
	    actionDiff->setEnabled( FALSE );
	}
    } else {
	actionAdd->setEnabled( TRUE );
	actionDelete->setEnabled( FALSE );
	actionSubmit->setEnabled( FALSE );
	actionRevert->setEnabled( FALSE );
	actionDiff->setEnabled( FALSE );
	actionEdit->setEnabled( FALSE );
    }

//     if ( fwIface )
// 	fwIface->setListViewIcon( pix );
}

void P4Interface::statusMessage( const QString &text )
{
    QString txt = text.left( text.length() );
    if ( !text.contains( '\n' ) )
	appInterface->showStatusMessage( txt );
    outputView->append( txt );
}

void P4Interface::reloadFile( const QString &filename )
{
    if ( !appInterface || filename.isEmpty() )
	return;

    qDebug( "P4 todo: reload file after change" );
}

QRESULT P4Interface::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ActionInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_Action )
	*iface = (ActionInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

unsigned long P4Interface::addRef()
{
    return ref++;
}

unsigned long P4Interface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

bool P4Interface::init()
{
    return TRUE;
}

void P4Interface::cleanup()
{
    delete P4Info::_files;
    P4Info::_files = 0;
    actions.clear();
    delete outputPage;
}

bool P4Interface::canUnload() const
{
    return actions.isEmpty() && outputPage.isNull();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( P4Interface )
}

#include "main.moc"
