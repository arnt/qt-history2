#include <qiconview.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qmultilineedit.h>

#include "dnd.h"
#include "styledbutton.h"
#include "listview.h"
#include "iconview.h"
#include "images.h"

DnDDemo::DnDDemo( QWidget* parent, const char* name )
    : DnDDemoBase( parent, name )
{
    buttonPixmap1->setEditor( StyledButton::PixmapEditor );
    buttonPixmap2->setEditor( StyledButton::PixmapEditor );
    buttonPixmap3->setEditor( StyledButton::PixmapEditor );
    buttonPixmap4->setEditor( StyledButton::PixmapEditor );

    multiLine1->setTextFormat( RichText );
    multiLine1->setMinimumHeight( 280 );
    multiLine1->setText( QString( "<p><b>Faust</b> - <i>Goethe</i></p>"  )
                         + "Habe nun, ach! Philosophie,<br>"
                         + "Juristerei und Medizin,<br>"
                         + "Und leider auch Theologie<br>"
                         + "Durchaus studiert, mit heißem Bemühn.<br>"
                         + "Da steh ich nun, ich armer Tor!<br>"
                         + "Und bin so klug als wie zuvor;<br>"
                         + "Heiße Magister, heiße Doktor gar<br>"
                         + "Und ziehe schon an die zehen Jahr<br>"
                         + "Herauf, herab und quer und krumm<br>"
                         + "Meine Schüler an der Nase herum-<br>"
                         + "Und sehe, daß wir nichts wissen können!<br>"
                         + "Das will mir schier das Herz verbrennen.<br>"
                         + "Zwar bin ich gescheiter als all die Laffen,<br>"
                         + "Doktoren, Magister, Schreiber und Pfaffen;<br>"
                         + "Mich plagen keine Skrupel noch Zweifel,<br>"
                         + "Fürchte mich weder vor Hölle noch Teufel-<br>"
                         + "Dafür ist mir auch alle Freud entrissen,<br>"
                         + "Bilde mir nicht ein, was Rechts zu wissen,<br>"
                         + "Bilde mir nicht ein, ich könnte was lehren,<br>"
                         + "Die Menschen zu bessern und zu bekehren.<br>"
                         + "Auch hab ich weder Gut noch Geld,<br>"
                         + "Noch Ehr und Herrlichkeit der Welt;<br>"
                         + "Es möchte kein Hund so länger leben!<br>"
                         + "Drum hab ich mich der Magie ergeben,<br>"
                         + "Ob mir durch Geistes Kraft und Mund<br>"
                         + "Nicht manch Geheimnis würde kund;<br>"
                         + "Daß ich nicht mehr mit saurem Schweiß<br>"
                         + "Zu sagen brauche, was ich nicht weiß;<br>"
                         + "Daß ich erkenne, was die Welt<br>"
                         + "Im Innersten zusammenhält,<br>"
                         + "Schau alle Wirkenskraft und Samen,<br>"
                         + "Und tu nicht mehr in Worten kramen. <br>" );

    multiLine2->setTextFormat( RichText );
    multiLine2->setMinimumHeight( 280 );
    multiLine2->setText( QString( "<p><b>To Milton</b> - <i>Oscar Wilde</i></p>" )
                         + "Milton!  I think thy spirit hath passed away"
                         + "From these white cliffs and high-embattled towers;"
                         + "This gorgeous fiery-coloured world of ours"
                         + "Seems fallen into ashes dull and grey,"
                         + "And the age changed unto a mimic play"
                         + "Wherein we waste our else too-crowded hours:"
                         + "For all our pomp and pageantry and powers"
                         + "We are but fit to delve the common clay,"
                         + "Seeing this little isle on which we stand,"
                         + "This England, this sea-lion of the sea,"
                         + "By ignorant demagogues is held in fee,"
                         + "Who love her not:  Dear God! is this the land"
                         + "Which bare a triple empire in her hand"
                         + "When Cromwell spake the word Democracy!" );

    QIconViewItem *iitem;
    iitem = new QIconViewItem( iconView, "Copy", loadPixmap( "editcopy.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Cut", loadPixmap( "editcut.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Paste", loadPixmap( "editpaste.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Raise", loadPixmap( "editraise.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Lower", loadPixmap( "editlower.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "New", loadPixmap( "filenew.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Load", loadPixmap( "fileopen.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Save", loadPixmap( "filesave.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Undo", loadPixmap( "undo.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Redo", loadPixmap( "redo.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Delete", loadPixmap( "editdelete.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Help", loadPixmap( "help.xpm" ) );
    iitem->setRenameEnabled( TRUE );
    iitem = new QIconViewItem( iconView, "Home", loadPixmap( "home.xpm" ) );
    iitem->setRenameEnabled( TRUE );

    listView->addColumn( "Actions", 240 );
    listView->setColumnWidthMode( 0, QListView::Maximum );
    QListViewItem *litem;
    litem = new QListViewItem( listView, "Copy" );
    litem->setPixmap( 0, loadPixmap ("editcopy.xpm" ) );
    litem = new QListViewItem( listView, "Cut" );
    litem->setPixmap( 0, loadPixmap ("editcut.xpm" ) );
    litem = new QListViewItem( listView, "Paste" );
    litem->setPixmap( 0, loadPixmap ("editpaste.xpm" ) );
    litem = new QListViewItem( listView, "Raise" );
    litem->setPixmap( 0, loadPixmap ("editraise.xpm" ) );
    litem = new QListViewItem( listView, "Lower" );
    litem->setPixmap( 0, loadPixmap ("editlower.xpm" ) );
    litem = new QListViewItem( listView, "New" );
    litem->setPixmap( 0, loadPixmap ("filenew.xpm" ) );
    litem = new QListViewItem( listView, "Load" );
    litem->setPixmap( 0, loadPixmap ("fileopen.xpm" ) );
    litem = new QListViewItem( listView, "Save" );
    litem->setPixmap( 0, loadPixmap ("filesave.xpm" ) );
    litem = new QListViewItem( listView, "Undo" );
    litem->setPixmap( 0, loadPixmap ("undo.xpm" ) );
    litem = new QListViewItem( listView, "Redo" );
    litem->setPixmap( 0, loadPixmap ("redo.xpm" ) );
    litem = new QListViewItem( listView, "Delete" );
    litem->setPixmap( 0, loadPixmap ("editdelete.xpm" ) );
    litem = new QListViewItem( listView, "Help" );
    litem->setPixmap( 0, loadPixmap ("help.xpm" ) );
    litem = new QListViewItem( listView, "Home" );
    litem->setPixmap( 0, loadPixmap ("home.xpm" ) );
}

DnDDemo::~DnDDemo()
{

}

QPixmap DnDDemo::loadPixmap( const QString& name )
{
    Embed *e = &embed_vec[ 0 ];
    while ( e->name ) {
	if ( QString( e->name ) == name ) {
	    QImage img;
	    img.loadFromData( (const uchar*)e->data, e->size );
	    QPixmap pix;
	    pix.convertFromImage( img );
	    return pix;
	}
	e++;
    }
    return QPixmap();
}
