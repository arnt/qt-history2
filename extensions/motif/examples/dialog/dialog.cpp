#include "dialog.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>


CustomDialog::CustomDialog( Widget parent, const char *name, bool modal, WFlags flags )
    : QMotifDialog( parent, name, modal, flags )
{
    setWindowTitle(tr("Custom Dialog"));

    QLabel *label =
	new QLabel( tr("<p><h3>Custom Dialog</h3></p>"
		       "<p>This is a custom Qt-based dialog using "
		       "QMotifDialog with a Motif-based parent.</p>"),
		    this, "custom dialog label" );
    label->setAlignment( AlignCenter );

    QPushButton *button = new QPushButton( tr("OK"), this, "custom dialog buton" );
    // button->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    connect( button, SIGNAL(clicked()), this, SLOT(accept()) );

    QGridLayout *layout = new QGridLayout( this, 2, 3, 8, 4, "custom dialog layout" );
    layout->addWidget( label, 0, 0, 0, 2 );
    layout->addWidget( button, 1, 1 );

    setMinimumSize( minimumSizeHint() );
}
