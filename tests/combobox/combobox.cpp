#include <qapplication.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qpainter.h>

class MyListBoxItem : public QListBoxPixmap
{
public:
    MyListBoxItem( int indentation, const QPixmap& pix, const QString& text )
	: QListBoxPixmap( pix, text ), indent( indentation )
    {
	setCustomHighlighting( TRUE );
    }

protected:
    virtual void paint( QPainter * );
    virtual int width( const QListBox* ) const;

private:
    int indent;
};

void MyListBoxItem::paint( QPainter *p )
{
    // evil trick: find out whether we are painted onto our listbox
    bool in_list_box = listBox() && listBox()->viewport() == p->device();
    // ignore the indentation if we are drawn somewhere else
    int indentation = in_list_box?indent:0;
    
    p->translate( indentation, 0 );
    QRect r ( 0, 0, width( listBox() ), height( listBox() ) );
    if ( in_list_box && selected() )
	p->eraseRect( r );
    QListBoxPixmap::paint( p );
    if ( in_list_box && current() )
	listBox()->style().drawFocusRect( p, r, listBox()->colorGroup(), &p->backgroundColor(), TRUE );

    p->translate( -indentation, 0 );
}

int MyListBoxItem::width( const QListBox* lb ) const
{
    return QListBoxPixmap::width( lb ) + indent;
}

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QComboBox combo;
    combo.insertItem( "First line" );
    combo.insertItem( "Second line" );
    combo.insertItem( "Third Line" );
    combo.insertItem( QPixmap("fileopen.xpm"), "Normal item with pixmap" );
    //combo.listBox()->insertItem( new MyListBoxItem( 20, QPixmap("fileopen.xpm"), "Indented custom item" ) );
    combo.adjustSize();
    a.setMainWidget( &combo );
    combo.show();
    return a.exec();
}
