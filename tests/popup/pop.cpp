#include <qapp.h>

#include <qfiledlg.h>
#include <qpushbt.h>
#include <qpopmenu.h>
#include <qmlined.h>
#include <qpainter.h>
#include <qdrawutl.h>

#include <qlayout.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qpixmap.h>

/* XPM */
static const char *unix_pix[] = {
/* width height num_colors chars_per_pixel */
"    48    48       32            1",
/* colors */
". c None",
"# c #dadada",
"a c #aaaaaa",
"b c #bbbbbb",
"c c #b3b3b3",
"d c #c7c7c7",
"e c #8b8b8b",
"f c #6f6f6f",
"g c #dfdfdf",
"h c #959595",
"i c #424242",
"j c #313131",
"k c #606060",
"l c #fffcff",
"m c #4f4f4f",
"n c #101010",
"o c #8a8a8a",
"p c #727272",
"q c #383838",
"r c #a2a2a2",
"s c #efefef",
"t c #d8d8d8",
"u c #5a5a5a",
"v c #6d6d6d",
"w c #414141",
"x c #939393",
"y c #2c2c2c",
"z c #48f448",
"A c #5a5a5a",
"B c #4c4c4c",
"C c #000000",
"D c #747474",
/* pixels */
"................................................",
"................................................",
"................................................",
"................................................",
"...................d##cb#a#aba#.................",
"........g#dbbbbbbbbeeeeefefeeeea##e.............",
"......ccbfda#ddaaeeeeffkiiijjieb#babh...........",
"......oedahahaeeeffkiBBBjnnjjmca#l##a...........",
".....celamiBBBBBBBBBBBBBBBBqnpaa#ll#c...........",
".....aaldnjBBBBBBBBBBBBBBBBijfa#lsssea..........",
".....aalemBBBBBBBBBBBBBBBBBBkua#lls#af..........",
".....a#lahBBBBBBBBBBBBBBBBBBmeal#sl#aib.........",
".....a#leBBBBzzzBzzBBBBBBBBBfealls##enc.........",
".....e#laBBBBBBBBBBBBBBBBBBBuea#ls##eio.........",
".....u##aBBBBBBBBBBBBBBBBBBBua#lss#aeeec........",
".....ul#aBBBzzzzBzzBBzzzzzBBeaalss##ffbcc.......",
"....ael##BBBBBBBBBBBBBBBBBBBea#ls##avf##cb......",
"....ae###BBBzzBBBzzzBzzBBBBBae#l###aje##gcg.....",
"....aes#aBBBBBBBBBBBBBBBBBBBaa#l###cia#l##x.....",
"....eu##aBBzzzzBBzBBBzzzzBBBaa#llaaeib#l#ar.....",
"....ee#aaBBBBBBBBBBBBBBBBBmBaa#l#aaeiallgee.....",
"....felaaBBzzzzBzzzBzzzzzzBBebl##abfk##gcfp.....",
"....iuleaBBBBBBBBBBBBBBBBBBBab#l#aakf##lcix.....",
"....ju#eeBBzzzBBzzzBBBBBBBBBabl##acifaaaaio.....",
"...gyf#eaBBBBBBBBBBBBBBBBBBBa##laaajfa#aejr.....",
"...gje#uBBBBBBBBBBBBBBBBBBBBe##laaenfaaafnx.....",
"...gne#a#BBzzzzzBzzBBBBBBBBBa#l#aabjeaaaino.....",
"...gje#a#BBBBBBBBBBBBBBBBBBBc###aaeBcaaejnc.....",
"...cjuaa#BBBBBBBBBBBBBBBBueaa#laaaeAeaeknqc.....",
"...rje##aadbbaaaabbbbbbBafbab##adaeBbafnny......",
"...cjuaa#aaaaaaaaaaaaaaaaeaha##deaemffynnA......",
"...rikkueeaaaababbabababaabaa#aebbfjmmjnCx......",
"....cnjjjjjjjiBikrfeefefefimDfeaeafjiiynnb......",
".....comijjjnnnnnCjnnnnnnnnyninifefnnCnxc.......",
"........gc#gcxvpvppiinnnnnniwnjnjjjnnjc.........",
"........cxvxagggccbabrkBqnjnnnjnnmmnmear........",
".......cyxrxrdta#ca#harovABBAeehahbcceeeb.......",
".......cncl#lsssccccbcacccrabd#tlsg#deeno.......",
".......cnha#aa#cacccbccccccrgsllsdadeffnp.......",
".......rnffkeeeahaaaaabacrcpgsssddakkiqne.......",
"........kfkiijiBfrkkeeeaaacvgsdaaeDjnqfa........",
"...........ccrrvjjnnnjjiiffkcgrrBnnyrr..........",
".................cccvinnnnjqxvAqBpc.............",
"......................xoiqqqpx..................",
"................................................",
"................................................",
"................................................",
"................................................"
};



class Urk : public QWidget
{
    Q_OBJECT
public:
    Urk();
public slots:
    void popup();
    void setMenu( int );
protected:
    void paintEvent( QPaintEvent * );    
    //    void timerEvent( QTimerEvent * );
private:
    QPopupMenu *pop;
    QPixmap *pix;
    int menuId;
    //    int pixId, textID, bothId;
    enum { pixId = 1, textId, bothId };
};

Urk::Urk()  : QWidget(0,0) 
{

    pix = new QPixmap( unix_pix );

    QVBoxLayout *gm = new QVBoxLayout( this, 10 );

    QPushButton *b = new QPushButton("Pop&up!", this);
    b->setFixedSize( b->sizeHint() );
    gm->addWidget( b, 0, AlignLeft );
    connect( b, SIGNAL(clicked()), SLOT(popup()) );


    QButtonGroup *bg = new QButtonGroup;

    QRadioButton *r = new QRadioButton( "&Text", this );
    r->setFixedSize( r->sizeHint() );
    gm->addWidget( r, 0, AlignLeft );
    bg->insert( r, textId );

    r = new QRadioButton( "&Pixmap", this );
    r->setFixedSize( r->sizeHint() );
    gm->addWidget( r, 0, AlignLeft );
    bg->insert( r, pixId );

    r = new QRadioButton( "&Both Text and Pixmap", this );
    r->setFixedSize( r->sizeHint() );
    gm->addWidget( r, 0, AlignLeft );
    bg->insert( r, bothId );


    connect( bg, SIGNAL(clicked(int)), SLOT(setMenu(int)) );

    gm->addStretch( 10 );

    gm->activate();

    pop = new QPopupMenu;
    CHECK_PTR( pop );

    menuId = pop->insertItem( "&New" );
    pop->insertItem( "&Open" );
    pop->insertItem( "&Clear" );
    pop->insertSeparator();
    pop->insertItem( "&Save" );
    pop->insertItem( "Save &As" );
    pop->insertSeparator();
    pop->insertItem( *pix );

}

void Urk::setMenu( int id ) {
    switch ( id ) {
    case textId:
	pop->changeItem( "one", menuId );
	break;
    case pixId:
	pop->changeItem( *pix, menuId );
	break;
    case bothId:
	pop->changeItem( *pix, "text", menuId );
	break;
    }
}

void Urk::popup()
{
    debug( "Urk::popup()");
    int i = pop->exec();

    debug( "Menu returned %d", i );
}

void Urk::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin(this);
    qDrawShadeLine( &p, 0,1,width(),1,colorGroup() );

    p.drawPixmap( QPoint(200,100), *pix );
    p.end();
}


int main( int argc, char ** argv )
{

    //    const int fa = 0;

    QApplication a( argc, argv );

    Urk u;
    u.show();

    a.setMainWidget( &u );
    return a.exec();
}
 
#include "pop.moc"
