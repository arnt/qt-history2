/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.cpp#20 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qhbox.h"
#include "qlayout.h"
#include "qapplication.h"
#include "qobjectlist.h"

// NOT REVISED
/*!
  \class QHBox qhbox.h
  \brief The QHBox widget performs geometry management on its children

  \ingroup geomanagement

  All its children will be placed beside each other and sized
  according to their sizeHint()s.

  Use setMargin() to add space around the edge, and use setSpacing() to
  add space between the widgets.

  <img src=qhbox-m.png>

  \sa QVBox and QGrid
*/


/*!
  Constructs an hbox widget with parent \a parent and name \a name
 */
QHBox::QHBox( QWidget *parent, const char *name, WFlags f,  bool allowLines  )
    :QFrame( parent, name, f, allowLines )
{
    lay = new QHBoxLayout( this, frameWidth(), frameWidth(), name );
    lay->setAutoAdd( TRUE );
}


/*!
  Constructs a horizontal hbox if \a horizontal is TRUE, otherwise
  constructs a vertical hbox (also known as a vbox).

  This constructor is provided for the QVBox class. You should never need
  to use it directly.
*/

QHBox::QHBox( bool horizontal, QWidget *parent , const char *name, WFlags f, bool allowLines )
    :QFrame( parent, name, f, allowLines )
{
    lay = new QBoxLayout( this,
		       horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
			  frameWidth(), frameWidth(), name );
    lay->setAutoAdd( TRUE );
}

/*!\reimp
 */
void QHBox::frameChanged()
{
    if ( !layout() )
	return;
    layout()->setMargin( frameWidth() );
}


/*!
  Sets the spacing between children to \a space.
*/

void QHBox::setSpacing( int space )
{
    if ( layout() ) // ### why not use this->lay?
	layout()->setSpacing( space );
}


/*!
  \reimp
*/

QSize QHBox::sizeHint() const
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents( mThis, QEvent::ChildInserted );
    return QFrame::sizeHint();
}

/*!
  Sets the stretch factor of \a w to \a stretch.
  \sa QBoxLayout::setStretchFactor()
*/
bool QHBox::setStretchFactor( QWidget* w, int stretch )
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents( mThis, QEvent::ChildInserted );
    return lay->setStretchFactor( w, stretch );
}
