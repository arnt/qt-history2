#include "../designerinterface.h"
#include "p4.h"

#include <qcleanuphandler.h>
#include <qaction.h>
#include <qapplication.h>
#include <qdict.h>
#include <qpainter.h>

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
static char *refresh_xpm[]={
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
".#b#............"
"................"};


class P4Interface : public QObject, public ActionInterface
{
    Q_OBJECT

public:
    P4Interface();
    ~P4Interface();

    QString name() { return "P4 Integration"; }
    QString description() { return "Integrates P4 Source Control into the Qt Designer"; }
    QString author() { return "Trolltech"; }

    bool connectNotify( QApplication* );

    QStringList featureList();
    QAction *create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname );

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

    void p4MightEdit( bool b, const QString &filename );
    void formChanged();
    void p4Info( const QString& filename, P4Info* );
    void statusMessage( const QString& );

private:
    bool aware;
    QAction *actionSync;
    QAction *actionEdit;
    QAction *actionSubmit;
    QAction *actionRevert;
    QAction *actionAdd;
    QAction *actionDelete;
    QAction *actionDiff;

    QGuardedCleanUpHandler<QAction> actions;
    QGuardedPtr<QApplicationInterface> appInterface;
};

P4Interface::P4Interface()
{
    aware = FALSE;
}

P4Interface::~P4Interface()
{
}

bool P4Interface::connectNotify( QApplication* theApp )
{
    if ( !theApp || ! ( appInterface = theApp->queryInterface() ) )
	return FALSE;

    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    
    if ( ( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) &&
	( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) ) {
	fwIface->requestConnect( SIGNAL( modificationChanged( bool, const QString & ) ), 
				 this, SLOT( p4MightEdit( bool, const QString & ) ) );
	flIface->requestConnect( SIGNAL( selectionChanged() ), this, SLOT(formChanged() ) );
    }
    P4Init* init = new P4Init;
    return init->execute();
}

QStringList P4Interface::featureList()
{
    QStringList list;
    list << "P4";
    return list;
}

QAction* P4Interface::create( const QString& actionname, QObject* parent )
{
    if ( actionname != "P4" )
	return 0;

    QActionGroup *ag = new QActionGroup( parent, 0, FALSE );

    QAction *a = new QAction( "P4 Aware", QIconSet((const char**)report_xpm), "A&ware", 0, ag, "P4 Aware", TRUE );
    connect( a, SIGNAL( toggled(bool) ), this, SLOT( p4Aware(bool) ) );

    ag->insertSeparator();

    actionSync = new QAction( "P4 Sync", QIconSet((const char**)sync_xpm), "&Sync", 0, ag, "P4 Sync" );
    connect( actionSync, SIGNAL( activated() ), this, SLOT( p4Sync() ) );

    actionEdit = new QAction( "P4 Edit", QIconSet((const char**)edit_xpm), "&Edit", 0, ag, "P4 Edit" );
    connect( actionEdit, SIGNAL( activated() ), this, SLOT( p4Edit() ) );

    actionSubmit = new QAction( "P4 Submit", QIconSet((const char**)submit_xpm), "&Submit", 0, ag, "P4 Submit" );
    connect( actionSubmit, SIGNAL( activated() ), this, SLOT( p4Submit() ) );

    actionRevert = new QAction( "P4 Revert", QIconSet((const char**)revert_xpm), "&Revert", 0, ag, "P4 Revert" );
    connect( actionRevert, SIGNAL( activated() ), this, SLOT( p4Revert() ) );

    ag->insertSeparator();

    actionAdd = new QAction( "P4 Add", QIconSet((const char**)add_xpm), "&Add", 0, ag, "P4 Add" );
    connect( actionAdd, SIGNAL( activated() ), this, SLOT( p4Add() ) );

    actionDelete = new QAction( "P4 Delete", QIconSet((const char**)delete_xpm), "&Delete", 0, ag, "P4 Delete" );
    connect( actionDelete, SIGNAL( activated() ), this, SLOT( p4Delete() ) );

    ag->insertSeparator();

    actionDiff = new QAction( "P4 Diff", QIconSet((const char**)diff_xpm), "Di&ff", 0, ag, "P4 Diff" );
    connect( actionDiff, SIGNAL( activated() ), this, SLOT( p4Diff() ) );

    a = new QAction( "P4 Refresh", QIconSet((const char**)refresh_xpm), "Refres&h", 0, ag, "P4 Refresh" );
    connect( a, SIGNAL( activated() ), this, SLOT( p4Refresh() ) );

    actions.addCleanUp( ag );
    actionSync->setEnabled( FALSE );
    actionEdit->setEnabled( FALSE );
    actionSubmit->setEnabled( FALSE );
    actionRevert->setEnabled( FALSE );
    actionAdd->setEnabled( FALSE );
    actionDelete->setEnabled( FALSE );
    actionDiff->setEnabled( FALSE );

    return ag;
}

QString P4Interface::group( const QString & )
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
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Sync *sync = new P4Sync( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( sync, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( sync, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    sync->execute();
}

void P4Interface::p4Edit()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Edit *edit = new P4Edit( fwIface->requestProperty( "fileName" ).toString().latin1(), TRUE );
    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    edit->execute();
}

void P4Interface::p4Submit()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Submit *submit = new P4Submit( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( submit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( submit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    submit->execute();
}

void P4Interface::p4Revert()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Revert *revert = new P4Revert( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( revert, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( revert, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    revert->execute();

}

void P4Interface::p4Add()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Add *add = new P4Add( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( add, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( add, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    add->execute();
}

void P4Interface::p4Delete()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Delete *del = new P4Delete( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( del, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( del, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    del->execute();
}

void P4Interface::p4Diff()
{
    if ( !appInterface )
	return;
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;

    P4Diff *diff = new P4Diff( fwIface->requestProperty( "fileName" ).toString().latin1() );
    connect( diff, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( diff, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    diff->execute();
}

void P4Interface::p4Refresh()
{
    P4Info::files.clear();

    DesignerFormListInterface *flIface = 0;
    if ( !( flIface = (DesignerFormListInterface*)appInterface->queryInterface( "DesignerFormListInterface" ) ) ) 
	return;

    QList<QComponentInterface> fwIfaces = flIface->queryFormInterfaceList();
    QListIterator<QComponentInterface> it( fwIfaces );
    while ( it.current() ) {
	QString filename = it.current()->requestProperty( "fileName" ).toString();
	if ( !!filename ) {
	    P4FStat* fs = new P4FStat( filename );
	    connect( fs, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
	    connect( fs, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
	    fs->execute();
	}
	++it;
    }
    formChanged();
}

void P4Interface::p4MightEdit( bool b, const QString &filename )
{
    if ( !aware || !b || !appInterface )
	return;
    QComponentInterface *mwIface = 0;
    if ( !( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) )
	return;
    P4Edit *edit = new P4Edit( filename, FALSE );
    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    edit->execute();
}

void P4Interface::formChanged()
{
    QComponentInterface *flIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( flIface = appInterface->queryInterface( "DesignerFormListInterface" ) ) ||
	 !( fwIface = flIface->queryInterface( "DesignerActiveFormWindowInterface" ) ) )
	return;
    QString filename = fwIface->requestProperty( "fileName" ).toString();
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

    P4Info* p4i = P4Info::files[filename];
    if ( !p4i ) {
	P4FStat* fs = new P4FStat( filename );
	connect( fs, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
	connect( fs, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
	fs->execute();
	return;
    }
    p4Info( filename, p4i );
}

void P4Interface::p4Info( const QString& filename, P4Info* p4i )
{
    if ( !p4i )
	return;

    DesignerFormListInterface *flIface = 0;
    
    if ( !( flIface = (DesignerFormListInterface*)appInterface->queryInterface( "DesignerFormListInterface" ) ) )
	return;
    QList<QComponentInterface> fwIfaces = flIface->queryFormInterfaceList();
    QListIterator<QComponentInterface> it( fwIfaces );
    QComponentInterface* fwIface = 0;
    while ( it.current() ) {
	fwIface = it.current();
	++it;
	if ( fwIface->requestProperty( "fileName" ).toString() == filename )
	    break;
    }
    if ( !fwIface )
	return;

    if ( p4i->controlled ) {
	QPixmap pix = fwIface->requestProperty( "icon" ).toPixmap();
	if ( pix.isNull() ) {
	    pix = QPixmap( 22,22,32 );
	    pix.fill( Qt::color0 );
	}
	QPainter paint( &pix );
	paint.setRasterOp( AndROP );
	paint.fillRect( 0, 0, pix.width() - 1 , pix.height() - 1, p4i->uptodate ? gray : red );
	paint.setRasterOp( CopyROP );
	actionAdd->setEnabled( FALSE );
	actionDelete->setEnabled( TRUE );
	if ( p4i->action != P4Info::None ) {
	    switch ( p4i->action ) {
	    case P4Info::Edit:
		{
		    QPixmap check( (const char**)editmark_xpm );
		    paint.drawPixmap( ( pix.width() - check.width() ) / 2, ( pix.height() - check.height() ) / 2, check );
		}
		break;
	    case P4Info::Add:
		{
		    QPixmap add( (const char**)addmark_xpm );
		    paint.drawPixmap( ( pix.width() - add.width() ) / 2, ( pix.height() - add.height() ) / 2, add );
		}
		break;
	    case P4Info::Delete:
		{
		    QPixmap del( (const char**)deletemark_xpm );
		    paint.drawPixmap( ( pix.width() - del.width() ) / 2, ( pix.height() - del.height() ) / 2, del );
		}
		break;
	    default:
		break;
	    }
	    
	    actionSync->setEnabled( FALSE );
	    actionEdit->setEnabled( FALSE );
	    actionSubmit->setEnabled( TRUE );
	    actionRevert->setEnabled( TRUE );
	    actionDiff->setEnabled( TRUE );
	} else {
	    actionSync->setEnabled( TRUE );
	    actionEdit->setEnabled( TRUE );
	    actionSubmit->setEnabled( FALSE );
	    actionRevert->setEnabled( FALSE );
	    actionDiff->setEnabled( FALSE );
	}
	flIface->setPixmap( fwIface, 0, pix );
    } else {
	QPixmap pix = fwIface->requestProperty( "icon" ).toPixmap();
	actionAdd->setEnabled( TRUE );
	actionDelete->setEnabled( FALSE );
	actionSubmit->setEnabled( FALSE );
	actionRevert->setEnabled( FALSE );
	actionDiff->setEnabled( FALSE );
	actionEdit->setEnabled( FALSE );
	flIface->setPixmap( fwIface, 0, pix );
    }
}

void P4Interface::statusMessage( const QString &text )
{
    QComponentInterface *mwIface = 0;
    QComponentInterface *sbIface = 0;
    if ( ( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) &&
	 ( sbIface = mwIface->queryInterface( "DesignerStatusBarInterface" ) ) )
	sbIface->requestSetProperty( "message", text );
}

#include "main.moc"

Q_EXPORT_INTERFACE(ActionInterface, P4Interface)
