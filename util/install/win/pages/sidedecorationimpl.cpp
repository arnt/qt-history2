#include "sidedecorationimpl.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qregexp.h>

/* XPM */
static char *check_data[] = {
/* width height num_colors chars_per_pixel */
"    11    12        4            1",
/* colors */
". c #939393",
"# c #dcdcdc",
"a c None",
"b c #191919",
/* pixels */
"aaaaaaaaaa#",
"aaaaaaaaabb",
"aaaaaaaabba",
"aaaaaaabbaa",
"aaaaaabbaaa",
"a#aaabbaaaa",
"ab.a.b.aaaa",
"a#bbbbaaaaa",
"aabbbaaaaaa",
"aa#b.aaaaaa",
"aaa.aaaaaaa",
"aaaaaaaaaaa"
};

/* XPM */
static char *arrow_data[] = {
/* width height num_colors chars_per_pixel */
"    11    11        4            1",
/* colors */
". c None",
"# c #b9b9b9",
"a c #8a8a8a",
"b c #0d0d0d",
/* pixels */
"...##......",
"...ab#.....",
"...abb#....",
"...abbb#...",
"...abbbb#..",
"...abbbba..",
"...abbba...",
"...abba....",
"...aba.....",
"...aa......",
"..........."
};

/* XPM */
static char *cross_data[] = {
/* width height num_colors chars_per_pixel */
"    11    11        3            1",
/* colors */
". c #cc0000",
"# c None",
"a c #fc3464",
/* pixels */
"###########",
"###########",
"########a.#",
"##a####a.##",
"##a.###.###",
"###a...a###",
"####...####",
"####...a###",
"###.a##..##",
"##a.####aa#",
"##.########"
};


SideDecorationImpl::SideDecorationImpl( QWidget* parent, const char* name, WFlags fl ) :
    SideDecoration( parent, name, fl ),
    checkPix( ( const char** ) check_data ),
    arrowPix( ( const char** ) arrow_data ),
    crossPix( ( const char** ) cross_data ),
    activeBullet( -1 )
{
    Q_ASSERT( layout() != 0 );
    if ( layout()->inherits("QBoxLayout") ) {
	((QBoxLayout*)layout())->setMargin( 0 );
    }
    setSizePolicy( QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding) );
    if ( globalInformation.reconfig() ) {
	versionLabel->setText( "Reconfigure Qt " + globalInformation.qtVersionStr() );
    } else {
#if defined(EVAL)
	QString versionStr = globalInformation.qtVersionStr();
	versionStr.replace( QRegExp(" Evaluation"), "" );
	versionLabel->setText( versionLabel->text() + " " + versionStr );
#else
	versionLabel->setText( versionLabel->text() + " " + globalInformation.qtVersionStr() );
#endif
    }
#if defined(EVAL)
    editionLabel->setText( "Evaluation Version" );
#else
    editionLabel->setText( "" );
#endif
}

SideDecorationImpl::~SideDecorationImpl()
{
}

void SideDecorationImpl::wizardPages( const QPtrList<Page>& li )
{
    QBoxLayout *lay = 0;
    Q_ASSERT( layout() != 0 );
    if ( layout()->inherits("QBoxLayout") ) {
	lay = (QBoxLayout*)layout();
    } else {
	return;
    }
    QPtrList<Page> list = li;
    Page *page;
    QGrid *grid = new QGrid( 2, this );
    grid->setSpacing( 2 );
    for ( page=list.first(); page; page=list.next() ) {
	QLabel *l = new QLabel( grid );
	l->setSizePolicy( QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed) );
	bullets.append( l );
	l = new QLabel( page->shortTitle(), grid );
    }
    lay->insertWidget( -1, grid );
    lay->insertStretch( -1 );
}

void SideDecorationImpl::wizardPageShowed( int a )
{
    if ( a == activeBullet )
	return;
    if ( activeBullet>=0 && (uint)activeBullet<bullets.count() ) {
	if ( a < activeBullet )
	    bullets.at(activeBullet)->clear();
	else
	    bullets.at(activeBullet)->setPixmap( checkPix );
    }
    bullets.at(a)->setPixmap( arrowPix );
    activeBullet = a;
}

void SideDecorationImpl::wizardPageFailed( int a )
{
    bullets.at(a)->setPixmap( crossPix );
}
