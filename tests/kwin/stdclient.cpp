#include "stdclient.h"
#include <qapplication.h>
#include <qcursor.h>
#include <qabstractlayout.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qdrawutil.h>
#include "workspace.h"


static const char * close_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};


static const char * maximize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};


static const char * minimize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * normalize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c #707070",
"X      c white",
/* pixels */
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};                      

static QPixmap* close_pix = 0;
static QPixmap* maximize_pix = 0;
static QPixmap* minimize_pix = 0;
static QPixmap* normalize_pix = 0;
static bool pixmaps_created = FALSE;

static void create_pixmaps()
{
    if ( pixmaps_created )
	return;
    close_pix = new QPixmap( close_xpm );
    maximize_pix = new QPixmap( maximize_xpm );
    minimize_pix = new QPixmap( minimize_xpm );
    normalize_pix = new QPixmap(  normalize_xpm );
}


StdClient::StdClient( Workspace *ws, WId w, QWidget *parent, const char *name )
    : Client( ws, w, parent, name, WResizeNoErase )
{
    create_pixmaps();

    QFont f = font();
    f.setBold( TRUE );
    setFont( f );

    QGridLayout* g = new QGridLayout( this, 0, 0, 2 );
    g->setRowStretch( 1, 10 );
    g->addWidget( windowWrapper(), 1, 1 );
    g->addColSpacing(0, 2);
    g->addColSpacing(2, 2);
    g->addRowSpacing(2, 2);


    button[0] = new QToolButton( this );
    button[1] = new QToolButton( this );
    button[2] = new QToolButton( this );
    button[3] = new QToolButton( this );
    button[4] = new QToolButton( this );
    button[5] = new QToolButton( this );

    QHBoxLayout* hb = new QHBoxLayout;
    g->addLayout( hb, 0, 1 );
    hb->addWidget( button[0] );
    hb->addWidget( button[1] );
    hb->addWidget( button[2] );

    int fh = fontMetrics().lineSpacing();

    titlebar = new QSpacerItem(10, fh, QSizePolicy::Expanding,
			       QSizePolicy::Minimum );
    hb->addItem( titlebar );

    hb->addWidget( button[3] );
    hb->addWidget( button[4] );
    hb->addWidget( button[5] );

    for ( int i = 0; i < 6; i++) {
	button[i]->setMouseTracking( TRUE );
	button[i]->setFixedSize( 20, 20 );
    }

    button[1]->hide();
    button[2]->hide();

    button[3]->setIconSet( *minimize_pix );
    connect( button[3], SIGNAL( clicked() ), this, ( SLOT( iconify() ) ) );
    button[4]->setIconSet( *maximize_pix  );
    connect( button[4], SIGNAL( clicked() ), this, ( SLOT( maximize() ) ) );
    button[5]->setIconSet( *close_pix );
    connect( button[5], SIGNAL( clicked() ), this, ( SLOT( closeWindow() ) ) );

}


StdClient::~StdClient()
{
}


void StdClient::resizeEvent( QResizeEvent* e)
{
    Client::resizeEvent( e );

    if ( isVisibleToTLW() ) {
	// manual clearing without the titlebar (we selected WResizeNoErase )
	QPainter p( this );
	QRect t = titlebar->geometry();
	t.setTop( 0 );
	QRegion r = rect();
	r = r.subtract( t );
	p.setClipRegion( r );
	p.eraseRect( rect() );
    }
}

/*!\reimp
 */
void StdClient::captionChange( const QString& )
{
    repaint( titlebar->geometry(), FALSE );
}


/*!\reimp
 */
void StdClient::maximizeChange( bool m )
{
    button[4]->setIconSet( m?*normalize_pix:*maximize_pix  );
}

void StdClient::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    QRect t = titlebar->geometry();
    t.setTop( 0 );
    QRegion r = rect();
    r = r.subtract( t );
    p.setClipRegion( r );
    qDrawWinPanel( &p, rect(), colorGroup() );
    p.setClipping( FALSE );
    p.fillRect( t, isActive()?darkBlue:gray );
    qDrawShadePanel( &p, t.x(), t.y(), t.width(), t.height(),
		     colorGroup(), TRUE );

    t.setTop( 2 );
    t.setLeft( t.left() + 4 );
    t.setRight( t.right() - 2 );

    p.setPen( colorGroup().light() );
    p.drawText( t, AlignLeft|AlignVCenter, caption() );
}


void StdClient::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( titlebar->geometry().contains( e->pos() ) )
	setShade( !isShade() );
    workspace()->requestFocus( this );
}
