#include "itemtextview.h"

ItemTextView::ItemTextView( QWidget * parent, const char * name )
    : QTextView( parent, name )
{
    index = 0;
}

ItemTextView::~ItemTextView()
{
}

QSize ItemTextView::sizeHint() const
{
    return QSize( 200, 300 );
}

void ItemTextView::setIndex( int i )
{
    index = i;
}

void ItemTextView::change( QListViewItem* lvi )
{
    setText( lvi->text( index ) );

}
