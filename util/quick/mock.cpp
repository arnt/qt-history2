// Qt Quick design
//
// /*this is just arbitrary stored text*/
//   Author: warwick@troll.no
//  Created: ...
//   ....
// /*end of the stored stuff*/
//
// EDIT THIS FILE AT YOUR OWN RISK - PARSING IS VERY SENSITIVE TO CHANGES

#include "mock.h"

#include <qpushbt.h>
#include <qslider.h>
#include "qdesigner.h"
/* etc. */

class MyDialogPrivate : QDesignerPrivate {
public:
    MyDialogPrivate( MyDialog* owner ) :
	QDesignerPrivate( owner ),
	button0( owner, "button0" ),
	slider0( owner, "slider0" )
    {
	layout();
    }

    void layout()
    {
	button0.setGeometry(dluToRect(0,0,50,10));
	slider0.setGeometry(dluToRect(200,100,70,10));
    }

    QPushButton button0;
    QSlider slider0;
};

MyDialog::MyDialog(QWidget* parent, const char* name, WFlags f) :
    QWidget( parent, name, f )
{
    d = new MyDialogPrivate(this);

    /* These define the signals slots (not the .h file occurences) */
    /* Slots are abstract virtual, unless they are setters (see below) */
    connect( &d->button0, SIGNAL(clicked()), this, SLOT(button0Clicked()));
    connect( &d->slider0, SIGNAL(valueChanged(int)), this, SLOT(slider0Changed(int)));
    connect( &d->slider0, SIGNAL(valueChanged(int)), this, SIGNAL(slider0HasChanged(int)));
}

/* The following sections fully define all the member functions */
/* The functions are either:
**    - accessors for subwidget accessors
**    - accessors for subwidgets
**    - setters for subwidget setters
*/

// PUBLIC MEMBER FUNCTIONS

int MyDialog::slider0() const
{
    return d->slider0.value();
}

QPushButton* MyDialog::button0() const
{
    return & d->button0;
}

QPushButton& MyDialog::buttonblah() const
{
    return d->button0;
}

void MyDialog::setSliderValue(int a)
{
    d->slider0.setValue(a);
}

void MyDialog::setFoo(int a) // virtual
{
    d->slider0.setValue(a);
}

// PUBLIC SLOTS

/* Not all are defined here - abstract virtual are defined only by connects */

void MyDialog::setSliderValue2(int a)
{
    d->slider0.setValue(a);
}

void MyDialog::setFoo2(int a) // virtual
{
    d->slider0.setValue(a);
}

// PROTECTED MEMBER FUNCTIONS

/* ... */

// PROTECTED SLOTS

/* ... */


// The remainder of this file is Meta Object Compiler output
/* ... include it here, since it is generated anyway */
