#include "widgetaction.h"
#include <qwidget.h>
#include <qtoolbutton.h>

void WidgetAction::addedTo( QWidget *w, QWidget *container )
{
    if ( w->inherits( "QToolButton" ) && container->parent()->parent() &&
	 container->parent()->parent()->inherits( "QScrollView" ) )
	( (QToolButton*)w )->setUsesTextLabel( TRUE );
}

