#include "qradiobutton.h"
#include "qsqleditor.h"

#ifndef QT_NO_SQL

/*!

  Constructs a custom SQL editor.
*/

QSqlCustomEd::QSqlCustomEd( QWidget * parent, const char * name, WFlags f )
    : QFrame( parent, name, f + QWidget::WStyle_Customize +
	      QWidget::WStyle_NoBorder )
{
    setFrameStyle( QFrame::Raised | QFrame::Box );

    s = new QRadioButton( this );
    s->resize( 150, s->height() );
    s->move( 20, 10 );

    resize( 150, 50 );
}

bool QSqlCustomEd::state() const
{
    return s->isChecked();
}

void QSqlCustomEd::setState( bool st )
{
    s->setChecked( st );
}

#endif
