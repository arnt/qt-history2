#include "quickbutton.h"

QuickButton::QuickButton( QWidget *parent, const char *name=0 )
    : QPushButton( parent, name )
{
}

QuickButton::QuickButton( const QString &text, QWidget *parent, const char* name=0 )
    : QPushButton( text, parent, name )
{
}

QuickButton::QuickButton( const QIconSet& icon, const QString &text, QWidget *parent, const char* name=0 )
    : QPushButton( icon, text, parent, name )
{
}

void QuickButton::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != RightButton ) {
	QPushButton::mousePressEvent( e );
    } else {
	emit rightClick();
    }
}
