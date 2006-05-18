#include "q3hgroupbox.h"

/*!
    \class Q3HGroupBox

    \brief The Q3HGroupBox widget organizes widgets in a group with one
    horizontal row.

    \compat

    Q3HGroupBox is a convenience class that offers a thin layer on top
    of Q3GroupBox. Think of it as a Q3HBox that offers a frame with a
    title.

    \sa QVGroupBox
*/

/*!
    Constructs a horizontal group box with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/
Q3HGroupBox::Q3HGroupBox( QWidget *parent, const char *name )
    : Q3GroupBox( 1, Qt::Vertical /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3HGroupBox::Q3HGroupBox( const QString &title, QWidget *parent,
			    const char *name )
    : Q3GroupBox( 1, Qt::Vertical /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal group box, deleting its child widgets.
*/
Q3HGroupBox::~Q3HGroupBox()
{
}
