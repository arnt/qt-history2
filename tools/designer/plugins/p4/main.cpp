#include <qinitguid.h>
#include "../designerinterface.h"
#include "p4.h"

#include <qcleanuphandler.h>
#include <qaction.h>
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

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QAction *create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname ) const;
    void connectTo( QUnknownInterface *ai );

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

    QGuardedCleanupHandler<QAction> actions;
    QUnknownInterface *appInterface;

    unsigned long ref;
};

P4Interface::P4Interface()
: appInterface( 0 ), ref( 0 )
{
    aware = TRUE;
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
	DesignerFormListInterface *flIface = 0;

	if ( !( flIface = (DesignerFormListInterface*)ai->queryInterface( IID_DesignerFormListInterface ) ) )
	    return;

	appInterface = ai;
	appInterface->addRef();

	flIface->connect( SIGNAL( selectionChanged() ), this, SLOT( formChanged() ) );
	flIface->release();

	P4Init* init = new P4Init;
	init->execute();
    }
}

QAction* P4Interface::create( const QString& actionname, QObject* parent )
{
    if ( actionname != "P4" )
	return 0;

    QActionGroup *ag = new QActionGroup( parent, 0, FALSE );

    QAction *a = new QAction( "P4 Aware", QIconSet((const char**)report_xpm), "A&ware", 0, ag, "P4 Aware", TRUE );
    a->setToolTip( tr("Toggle edit awareness") );
    a->setStatusTip( tr("Toggles whether forms in source control should be checked out before edited") );
    a->setWhatsThis( tr("") );
    connect( a, SIGNAL( toggled(bool) ), this, SLOT( p4Aware(bool) ) );
    a->setOn( aware );

    ag->insertSeparator();

    actionSync = new QAction( "P4 Sync", QIconSet((const char**)sync_xpm), "&Sync", 0, ag, "P4 Sync" );
    actionSync->setToolTip( tr("Sync to head revision") );
    actionSync->setStatusTip( tr("Synchronizes client file to depot head revision") );
    connect( actionSync, SIGNAL( activated() ), this, SLOT( p4Sync() ) );

    actionEdit = new QAction( "P4 Edit", QIconSet((const char**)edit_xpm), "&Edit", 0, ag, "P4 Edit" );
    actionEdit->setToolTip( tr("Check out for edit") );
    actionEdit->setStatusTip( tr("Checks out file for edit") );
    connect( actionEdit, SIGNAL( activated() ), this, SLOT( p4Edit() ) );

    actionSubmit = new QAction( "P4 Submit", QIconSet((const char**)submit_xpm), "&Submit", 0, ag, "P4 Submit" );
    actionSubmit->setToolTip( tr("Submit changes") );
    actionSubmit->setStatusTip( tr("Submits changed form(s) to depot") );
    connect( actionSubmit, SIGNAL( activated() ), this, SLOT( p4Submit() ) );

    actionRevert = new QAction( "P4 Revert", QIconSet((const char**)revert_xpm), "&Revert", 0, ag, "P4 Revert" );
    actionRevert->setToolTip( tr("Revert changes") );
    actionRevert->setStatusTip( tr("Reverts changes to form(s)") );
    connect( actionRevert, SIGNAL( activated() ), this, SLOT( p4Revert() ) );

    ag->insertSeparator();

    actionAdd = new QAction( "P4 Add", QIconSet((const char**)add_xpm), "&Add", 0, ag, "P4 Add" );
    actionAdd->setToolTip( tr("Add form") );
    actionAdd->setStatusTip( tr("Adds form to source control") );
    connect( actionAdd, SIGNAL( activated() ), this, SLOT( p4Add() ) );

    actionDelete = new QAction( "P4 Delete", QIconSet((const char**)delete_xpm), "&Delete", 0, ag, "P4 Delete" );
    actionDelete->setToolTip( tr("Check out for delete") );
    actionDelete->setStatusTip( tr("Checks out file for delete") );
    connect( actionDelete, SIGNAL( activated() ), this, SLOT( p4Delete() ) );

    ag->insertSeparator();

    actionDiff = new QAction( "P4 Diff", QIconSet((const char**)diff_xpm), "Di&ff", 0, ag, "P4 Diff" );
    actionDiff->setToolTip( tr("Diff against depot") );
    actionDiff->setStatusTip( tr("Opens diff for client file against depot file") );
    connect( actionDiff, SIGNAL( activated() ), this, SLOT( p4Diff() ) );

    a = new QAction( "P4 Refresh", QIconSet((const char**)refresh_xpm), "Refres&h", 0, ag, "P4 Refresh" );
    a->setToolTip( tr("Refresh") );
    a->setStatusTip( tr("Refreshes state of forms") );
    connect( a, SIGNAL( activated() ), this, SLOT( p4Refresh() ) );

    actions.add( ag );
    
    actionSync->setEnabled( FALSE );
    actionEdit->setEnabled( FALSE );
    actionSubmit->setEnabled( FALSE );
    actionRevert->setEnabled( FALSE );
    actionAdd->setEnabled( FALSE );
    actionDelete->setEnabled( FALSE );
    actionDiff->setEnabled( FALSE );

    return ag;
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
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Sync *sync = new P4Sync( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( sync, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( sync, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    sync->execute();
}

void P4Interface::p4Edit()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;
    P4Edit *edit = new P4Edit( fwIface->property( "fileName" ).toString(), TRUE );
    fwIface->release();

    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    edit->execute();
}

void P4Interface::p4Submit()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Submit *submit = new P4Submit( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( submit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( submit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    submit->execute();    
}

void P4Interface::p4Revert()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Revert *revert = new P4Revert( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( revert, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( revert, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    revert->execute();
}

void P4Interface::p4Add()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Add *add = new P4Add( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( add, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( add, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    add->execute();
}

void P4Interface::p4Delete()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Delete *del = new P4Delete( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( del, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( del, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    del->execute();    
}

void P4Interface::p4Diff()
{
    if ( !appInterface )
	return;
    DesignerFormInterface *fwIface = 0;
    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    P4Diff *diff = new P4Diff( fwIface->property( "fileName" ).toString() );
    fwIface->release();

    connect( diff, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( diff, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    diff->execute();    
}

void P4Interface::p4Refresh()
{
    P4Info::files.clear();

    DesignerFormListInterface *flIface = 0;
    if ( !( flIface = (DesignerFormListInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) ) 
	return;

    DesignerFormInterface *fw = flIface->current();
    while ( fw ) {
	QString filename = fw->property( "fileName" ).toString();
	qDebug( "fstat for %s", filename.latin1() );
	if ( !!filename ) {
	    P4FStat* fs = new P4FStat( filename );
	    connect( fs, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
	    connect( fs, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
	    fs->execute();
	}
	delete fw;
	fw = flIface->next();
    }

    formChanged();
    flIface->release();
}

void P4Interface::p4MightEdit( bool b, const QString &filename )
{
    if ( !aware || !b || !appInterface )
	return;

    P4Edit *edit = new P4Edit( filename, FALSE );
    connect( edit, SIGNAL(finished(const QString&, P4Info*)), this, SLOT(p4Info(const QString&,P4Info*)) );
    connect( edit, SIGNAL( showStatusBarMessage( const QString & ) ), this, SLOT( statusMessage( const QString & ) ) );
    edit->execute();
}

void P4Interface::formChanged()
{
    DesignerFormInterface *fwIface = 0;

    if ( !( fwIface = (DesignerFormInterface*)appInterface->queryInterface( IID_DesignerFormInterface ) ) )
	return;

    QString filename = fwIface->property( "fileName" ).toString();

    qDebug( "P4: formChanged to %s", filename.latin1() );

    if ( ! fwIface->connect( SIGNAL( modificationChanged( bool, const QString & ) ), 
			     this, SLOT( p4MightEdit( bool, const QString & ) ) ) )
			     qDebug( "P4: formChanged: No connection to modificationChanged!" );

    fwIface->release();
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
    
    if ( !( flIface = (DesignerFormListInterface*)appInterface->queryInterface( IID_DesignerFormListInterface ) ) )
	return;

    DesignerFormInterface *fwIface = flIface->current();
    while ( fwIface ) {
	if ( fwIface->property( "fileName" ).toString() == filename )
	    break;

	fwIface->release();
	fwIface = flIface->next();
    }

    if ( !fwIface )
	return;

    if ( p4i->controlled ) {
	QPixmap pix = fwIface->property( "icon" ).toPixmap();
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
	    actionSync->setEnabled( !p4i->uptodate );
	    actionEdit->setEnabled( TRUE );
	    actionSubmit->setEnabled( FALSE );
	    actionRevert->setEnabled( FALSE );
	    actionDiff->setEnabled( FALSE );
	}
	flIface->setPixmap( fwIface, 0, pix );
    } else {
	QPixmap pix = fwIface->property( "icon" ).toPixmap();
	actionAdd->setEnabled( TRUE );
	actionDelete->setEnabled( FALSE );
	actionSubmit->setEnabled( FALSE );
	actionRevert->setEnabled( FALSE );
	actionDiff->setEnabled( FALSE );
	actionEdit->setEnabled( FALSE );
	flIface->setPixmap( fwIface, 0, pix );
    }
    fwIface->release();
    flIface->release();
}

void P4Interface::statusMessage( const QString &text )
{
    DesignerStatusBarInterface *sbIface = 0;
    if ( !( sbIface = (DesignerStatusBarInterface*)appInterface->queryInterface( IID_DesignerStatusBarInterface ) ) )
	return;

    sbIface->setMessage( text );
    sbIface->release();
}

QUnknownInterface *P4Interface::queryInterface( const QGuid &guid )
{
    QUnknownInterface *iface = 0;

    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( guid == IID_ActionInterface )
	iface = (ActionInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
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

Q_EXPORT_INTERFACE( P4Interface )

#include "main.moc"
