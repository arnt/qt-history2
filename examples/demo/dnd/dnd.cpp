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
                         + "Milton!  I think thy spirit hath passed away<br>"
                         + "From these white cliffs and high-embattled towers;<br>"
                         + "This gorgeous fiery-coloured world of ours<br>"
                         + "Seems fallen into ashes dull and grey,<br>"
                         + "And the age changed unto a mimic play<br>"
                         + "Wherein we waste our else too-crowded hours:<br>"
                         + "For all our pomp and pageantry and powers<br>"
                         + "We are but fit to delve the common clay,<br>"
                         + "Seeing this little isle on which we stand,<br>"
                         + "This England, this sea-lion of the sea,<br>"
                         + "By ignorant demagogues is held in fee,<br>"
                         + "Who love her not:  Dear God! is this the land<br>"
                         + "Which bare a triple empire in her hand<br>"
                         + "When Cromwell spake the word Democracy!<br>" );


    items.insert( tr("copy"), IconItem( tr("Copy"), "editcopy.xpm" ) );
    items.insert( tr("cut"), IconItem( tr("Cut"), "editcut.xpm" ));
    items.insert( tr("paste"), IconItem( tr("Paste"), "editpaste.xpm" ));
    items.insert( tr("raise"), IconItem( tr("Raise"), "editraise.xpm" ));
    items.insert( tr("lower"), IconItem( tr("Lower"), "editlower.xpm" ));
    items.insert( tr("new"), IconItem( tr("New"), "filenew.xpm" ));
    items.insert( tr("load"), IconItem( tr("Load"), "fileopen.xpm" ));
    items.insert( tr("save"), IconItem( tr("Save"), "filesave.xpm" ));
    items.insert( tr("undo"), IconItem( tr("Undo"), "undo.xpm" ));
    items.insert( tr("redo"), IconItem( tr("Redo"), "redo.xpm" ));
    items.insert( tr("delete"), IconItem( tr("Delete"), "editdelete.xpm" ));
    items.insert( tr("help"), IconItem( tr("Help"), "help.xpm" ));
    items.insert( tr("home"), IconItem( tr("Home"), "home.xpm" ));

    listView->addColumn( tr("Actions"), 240 );
    listView->setColumnWidthMode( 0, QListView::Maximum );

    QMap<QString,IconItem>::Iterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
        IconItem item = it.data();

        QIconViewItem *iitem = new IconViewItem( iconView, item.name(), *item.pixmap(), it.key() );
        iitem->setRenameEnabled( TRUE );
        QListViewItem *litem = new ListViewItem( listView, item.name(), it.key() );
        litem->setPixmap( 0, *item.pixmap() );
    }
}

DnDDemo::~DnDDemo()
{

}

IconItem::IconItem( const QString& name, const QString& icon )
{
    _name = name;
    _pixmap = loadPixmap( icon );
}

QPixmap IconItem::loadPixmap( const QString& name )
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

IconItem DnDDemo::findItem( const QString& tag )
{
    return items[ tag ];
}
