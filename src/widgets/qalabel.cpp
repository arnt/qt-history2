#include "qalabel.h"

/*! \class QAccelLabel qalabel.h

  \brief The QAccelLabel class provides a label with an accelerator
  for moving the keyboard focus to a given widget.

  This is useful in many dialogs - for each data entry widget, there
  is often a label.  By using QAccelLabel, you can add an accelerator
  key to set the focus to each data entry key.

  In a dialog, you might create three data entry widgets and a label
  for each, and set up the geometry so each label is just to the left
  of its data entry widget (its \"buddy\"), somewhat like this:

  \code
    QLineEdit * name( this, "customer name" );
    QAccelLabel * alName( name, Ctrl+Key_N, "&Name:", this );
    QLineEdit * phone( this, "customer telephone number" );
    QAccelLabel * alPhone( phone, Ctrl+Key_P, "&Phone:", this );
    QMultiLineEdit * address( this, "customer address:" );
    QAccelLabel * alAddress( address, Ctrl+Key_A, "&Address", this );
    // geometry management setup not shown
  \\endcode

  With the code above, the focus jumps to the Address field when the
  user presses Ctrl-A, to the Name field when the user presses Ctrl-N,
  and to the Phone field when the user presses Ctrl-P.

*/


/*! Creates a QAccelLabel with text \a text which moves the focus to
  \a buddy when accelerator \a key is pressed.

  \a parent, \a name and \a f mean the same as for QWidget::QWidget().

*/

QAccelLabel::QAccelLabel( QWidget * buddy, int key, const char * text,
			   QWidget *parent, const char *name, WFlags f )
    : QLabel( text, parent, name, f )
{
    a = new QAccel( buddy, "accel label accel" );
    CHECK_PTR( a );
    a->connectItem( a->insertItem( key ), this, SLOT(setFocus()) );
    b = buddy;
    setAlignment( alignment() ); // will OR in ShowPrefix
}


/*!
  Used to communicate with the QAccel object.
*/

void QAccelLabel::focusSlot( int )
{
    if ( b && b->focusPolicy() != NoFocus )
	b->setFocus();
}


/*!  Reimplemented in order to make sure that \c ShowPrefix is always
  on.

  Note that QLabel::setAlignment is not virtual.
*/

void QAccelLabel::setAlignment( int a )
{
    QLabel::setAlignment( a | ShowPrefix );
}
