#include "quickbutton.h"

QuickButton::QuickButton( QWidget *parent, const char *name )
    : QPushButton( parent, name )
{
}

QuickButton::QuickButton( const QString &text, QWidget *parent, const char* name )
    : QPushButton( text, parent, name )
{
}

QuickButton::QuickButton( const QIconSet& icon, const QString &text, QWidget *parent, const char* name )
    : QPushButton( icon, text, parent, name )
{
}

void QuickButton::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != RightButton ) {
	QPushButton::mouseReleaseEvent( e );
    } else {
	emit rightClick();
    }
}
