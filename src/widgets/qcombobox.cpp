/**********************************************************************
** $Id: $
**
** Implementation of QComboBox widget class
**
** Created : 940426
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcombobox.h"
#ifndef QT_NO_COMBOBOX
#include "qpopupmenu.h"
#include "qlistbox.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qlineedit.h"
#include "qbitmap.h"
#include "qeffects_p.h"
#include "qstringlist.h"
#include "qcombobox.h"
#include <limits.h>

// NOT REVISED
/*!
  \class QComboBox qcombobox.h
  \brief The QComboBox widget is a combined button and popup list.

  \ingroup basic

  A combobox is a selection widget which displays the current item and
  can pop up a list of items.  A combobox may be editable in which case
  the user can enter arbitrary strings.

  Since comboboxes occupy little screen space and always display the
  current item, they are well suited to displaying items that the user
  will want to see, such as font family or size. Using a combobox the
  user can always see which item they've selected with the minimum amount
  of screen space being used.

  QComboBox supports three different display styles: Motif 1.x, Motif 2.0
  and Windows 95.  In Motif 1.x, a combobox was called XmOptionMenu.
  In Motif 2.0, OSF introduced an improved combobox and
  named that XmComboBox.  QComboBox provides both.

  QComboBox provides two different constructors.  The simplest
  constructor creates an old-style combobox in Motif style:
  \code
      QComboBox *c = new QComboBox( this, "read-only combobox" );
  \endcode

  The other constructor creates a new-style combobox in Motif style, and
  can create both read-only and read-write comboboxes:
  \code
      QComboBox *c1 = new QComboBox( FALSE, this, "read-only combobox" );
      QComboBox *c2 = new QComboBox( TRUE, this, "read-write combobox" );
  \endcode

  New-style comboboxes use a list box in both Motif and Windows
  styles, and both the content size and the on-screen size of the list
  box can be limited with sizeLimit() and setMaxCount() respectively.
  Old-style comboboxes use a popup in Motif style, and that popup will
  happily grow larger than the desktop if you put enough data into it.

  The two constructors create identical-looking comboboxes in Windows
  style.

  Comboboxes can contain pixmaps as well as strings; the
  insertItem() and changeItem() functions are suitably overloaded.  For
  read-write comboboxes, the function clearEdit()
  is provided, to clear the displayed string without changing the
  combobox's contents.

  A combobox emits two signals, activated() and highlighted(), when a
  new item has been activated (selected) or highlighted (made
  current).  Both signals exist in two versions, one with a \c QString
  argument and one with an \c int argument.  If the user highlights or
  activates a pixmap, only the \c int signals are emitted. Whenever the
  text of an editable combobox is changed the textChanged() signal is
  emitted.

  When the user enters a new string in a read-write combobox, the widget
  may or may not insert it, and it can insert it in several locations.
  The default policy is is \c AtBottom but you can change this using
  setInsertionPolicy().

  It is possible to constrain the input to an editable combobox using
  QValidator; see setValidator().  By default, all input is accepted.

  If the combo box is not editable then it has a default focusPolicy()
  of \c TabFocus, i.e. it will not grab focus if clicked.  This
  differs from both Windows and Motif.  If the combo box is editable then it
  has a default focusPolicy() of \c StrongFocus, i.e. it will grab focus if
  clicked.

  A combobox can be populated using the insert functions,
  insertStringList() and insertItem() for example. Items can be changed
  with changeItem(). An item can be removed with removeItem() and all
  items can be removed with clear(). The text of the current item is
  returned by currentText(), and the text of a numbered item is returned
  with text(). The current item can be set with setCurrentItem(). The
  number of items in the combobox is returned by count(); the maximum
  number of items can be set with setMaxCount(). You can allow editing
  using setEditable(). For editable comboboxes you can set
  auto-completion using setAutoCompletion() and whether or not the user
  can add duplicates is set with setDuplicatesEnabled().

  <img src="qcombo1-m.png">(Motif 1, read-only)<br clear=all>
  <img src="qcombo2-m.png">(Motif 2, read-write)<br clear=all>
  <img src="qcombo3-m.png">(Motif 2, read-only)<br clear=all>
  <img src="qcombo1-w.png">(Windows style)

  \sa QLineEdit QListBox QSpinBox QRadioButton QButtonGroup
  <a href="guibooks.html#fowler">GUI Design Handbook: Combo Box,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Drop-Down List Box.</a>
*/


/*! \enum QComboBox::Policy

  This enum specifies what the QComboBox should do when a new string is
  entered by the user.  The following policies are defined:

  \value NoInsertion the string will not be inserted into the combobox.

  \value AtTop insert the string as the first item in the combobox.

  \value AtCurrent replace the previously selected item with the string
  the user has entered.

  \value AtBottom insert the string as the last item in the
  combobox.

  \value AfterCurrent insert the string after the
  previously selected item.

  \value BeforeCurrent insert the string before the
  previously selected item.

  activated() is always emitted when the string is entered.

  If inserting the new string would cause the combobox to breach its
  content size limit, the item at the other end of the list is deleted.
  The definition of "other end" is implementation-dependent.
*/


/*! \fn void QComboBox::activated( int index )

  This signal is emitted when a new item has been activated (selected).
  The \e index is the position of the item in the combobox.
*/

/*! \fn void QComboBox::activated( const QString &string )

  This signal is emitted when a new item has been activated
  (selected). \a string is the selected string.

  You can also use the activated(int) signal, but be aware that its
  argument is meaningful only for selected strings, not for user entered
  strings.
*/

/*! \fn void QComboBox::highlighted( int index )

  This signal is emitted when a new item has been set to current.
  The \e index is the position of the item in the combobox.
*/

/*! \fn void QComboBox::highlighted( const QString &string )

  This signal is emitted when a new item has been highlighted. \a
  string is the highlighted string.

  You can also use highlighted(int) signal.
*/

/*! \fn void QComboBox::textChanged( const QString &string )

  This signal is used for editable comboboxes. It is emitted whenever
  the contents of the text entry field changes.
*/

/*! \property QComboBox::autoCompletion
    \brief whether auto-completion is enabled

  This property can only be set for editable comboboxes, for non-editable
  comboboxes it has no effect.
*/

/*! \property QComboBox::autoResize
    \brief whether auto resize is enabled
    \obsolete

  If this property is set to TRUE then the combobox will resize itself
  whenever its contents change.
*/

/*! \property QComboBox::count
    \brief the number of items in the combobox
*/

/*! \property QComboBox::currentItem
    \brief the index of the current item in the combobox
*/

/*! \property QComboBox::currentText
    \brief the text of the combobox's current item
*/

/*! \property QComboBox::duplicatesEnabled
    \brief whether duplicates are allowed

  This property only has an effect if the combobox is editable.
*/

/*! \property QComboBox::editable
    \brief whether the combobox is editable
*/

/*! \property QComboBox::insertionPolicy
    \brief the position of the items inserted by the user

  The default insertion policy is \c AtBottom.
*/

/*! \property QComboBox::maxCount
    \brief the maximum number of items allowed in the combobox
*/

/*! \property QComboBox::sizeLimit
    \brief the maximum on-screen size of the combobox.

  This is disregarded in Motif 1.x style.  The default limit is ten
  lines. If the number of items in the combobox is or grows larger than
  lines, a scrollbar is added.
*/

class QComboBoxPopup : public QPopupMenu
{
public:
    QComboBoxPopup( QWidget *parent = 0, const char *name = 0 )
	: QPopupMenu( parent, name )
    {
    }

    int itemHeight( int index )
    {
	return QPopupMenu::itemHeight( index );
    }

};


class QComboBoxData
{
public:
    QComboBoxData( QComboBox *cb ): usingLBox( FALSE ), pop( 0 ), lBox( 0 ), combo( cb )
    {
	duplicatesEnabled = TRUE;
	cb->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    }

    bool usingListBox()  { return usingLBox; }
    QListBox * listBox() { Q_ASSERT(usingLBox); return lBox; }
    QComboBoxPopup * popup() { Q_ASSERT(!usingLBox); return pop; }
    void updateLinedGeometry();

    void setListBox( QListBox *l ) { lBox = l ; usingLBox = TRUE;
				l->setMouseTracking( TRUE );}

    void setPopupMenu( QComboBoxPopup * pm ) { pop = pm; usingLBox = FALSE; }

    int		current;
    int		maxCount;
    int		sizeLimit;
    QComboBox::Policy p;
    bool	autoresize;
    bool	poppedUp;
    bool	mouseWasInsidePopup;
    bool	arrowPressed;
    bool	arrowDown;
    bool	discardNextMousePress;
    bool	shortClick;
    bool	useCompletion;
    bool	completeNow;
    int		completeAt;
    bool duplicatesEnabled;
    int fullHeight, currHeight;

    QLineEdit * ed;  // /bin/ed rules!

    QSize sizeHint;

private:
    bool	usingLBox;
    QComboBoxPopup *pop;
    QListBox   *lBox;
    QComboBox *combo;

};

void QComboBoxData::updateLinedGeometry()
{
    if ( !ed || !combo )
	return;
    QRect r = combo->style().querySubControlMetrics(QStyle::CC_ComboBox, combo,
						    QStyle::SC_ComboBoxEditField);

    const QPixmap *pix = current < combo->count() ? combo->pixmap( current ) : 0;
    if ( pix && pix->width() < r.width() )
	r.setLeft( r.left() + pix->width() + 4 );
    if ( r != ed->geometry() )
	ed->setGeometry( r );
}

bool QComboBox::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
    if ( d->usingListBox()  && style() == WindowsStyle ) {
	QRect r  = arrowRect();
	*buttonW = r.width();
	*buttonH = r.height();
	*dist    = 4;
    } else if ( d->usingListBox() ) {
	*dist = 6;
	*buttonW = 16;
	*buttonH = 18;
    } else {
	*dist     = 8;
	*buttonH  = 7;
	*buttonW  = 11;
    }
    return TRUE;
}


static inline bool checkInsertIndex( const char *method, const char * name,
				     int count, int *index)
{
    bool range_err = (*index > count);
#if defined(QT_CHECK_RANGE)
    if ( range_err )
	qWarning( "QComboBox::%s: (%s) Index %d out of range",
		 method, name ? name : "<no name>", *index );
#else
    Q_UNUSED( method )
    Q_UNUSED( name )
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}


static inline bool checkIndex( const char *method, const char * name,
			       int count, int index )
{
    bool range_err = (index >= count);
#if defined(QT_CHECK_RANGE)
    if ( range_err )
	qWarning( "QComboBox::%s: (%s) Index %i out of range",
		 method, name ? name : "<no name>", index );
#else
    Q_UNUSED( method )
    Q_UNUSED( name )
#endif
    return !range_err;
}



/*!
  Constructs a combobox widget with parent \a parent and name \a name.

  This constructor creates a popup list if the program uses Motif look
  and feel; this is compatible with Motif 1.x.
*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name, WResizeNoErase )
{
    d = new QComboBoxData( this );
    if ( style() == WindowsStyle ) {
	setUpListBox();
    } else {
	d->setPopupMenu( new QComboBoxPopup( this, "in-combo" ) );
	d->popup()->setFont( font() );
	connect( d->popup(), SIGNAL(activated(int)),
			     SLOT(internalActivate(int)) );
	connect( d->popup(), SIGNAL(highlighted(int)),
			     SLOT(internalHighlight(int)) );
    }
    d->ed                    = 0;
    d->current               = 0;
    d->maxCount              = INT_MAX;
    d->sizeLimit	     = 10;
    d->p                    =  AtBottom;
    d->autoresize            = FALSE;
    d->poppedUp              = FALSE;
    d->arrowDown             = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick            = FALSE;
    d->useCompletion = FALSE;

    setFocusPolicy( TabFocus );
    setBackgroundMode( PaletteButton );
}


/*!
  Constructs a combobox with a maximum size and either Motif 2.0 or
  Windows look and feel.

  The input field can be edited if \a rw is TRUE, otherwise the user
  may only choose one of the items in the combobox.
*/


QComboBox::QComboBox( bool rw, QWidget *parent, const char *name )
    : QWidget( parent, name, WResizeNoErase )
{
    d = new QComboBoxData( this );
    setUpListBox();

    d->current = 0;
    d->maxCount = INT_MAX;
    setSizeLimit(10);
    d->p = AtBottom;
    d->autoresize = FALSE;
    d->poppedUp = FALSE;
    d->arrowDown = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick = FALSE;
    d->useCompletion = FALSE;

    setFocusPolicy( StrongFocus );

    d->ed = 0;
    if ( rw )
	setUpLineEdit();
    setBackgroundMode( PaletteButton, PaletteBase );
}



/*!
  Destroys the combobox.
*/

QComboBox::~QComboBox()
{
    delete d;
}

void QComboBox::setDuplicatesEnabled( bool enable )
{
   d->duplicatesEnabled = enable;
}

bool QComboBox::duplicatesEnabled() const
{
    return d->duplicatesEnabled;
}

int QComboBox::count() const
{
    if ( d->usingListBox() )
	return d->listBox()->count();
    else
	return d->popup()->count();
}


/*!
  \overload
*/

void QComboBox::insertStrList( const QStrList &list, int index )
{
    insertStrList( &list, index );
}

/*!
  Inserts the \a list of strings at position \a index in the combobox.

  This is only for compatibility, as it does not support Unicode
  strings.  See insertStringList().
*/

void QComboBox::insertStrList( const QStrList *list, int index )
{
    if ( !list ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char* tmp;
    if ( index < 0 )
	index = count();
    while ( (tmp=it.current()) ) {
	++it;
	if ( d->usingListBox() )
	    d->listBox()->insertItem( QString::fromLatin1(tmp), index );
	else
	    d->popup()->insertItem( QString::fromLatin1(tmp), index, index );
	if ( index++ == d->current && d->current < count() ) {
	    if ( d->ed ) {
		d->ed->setText( text( d->current ) );
		d->updateLinedGeometry();
	    } else
		update();
	    currentChanged();
	}
    }
    if ( index != count() )
	reIndex();
}

/*!
  Inserts the \a list of strings at position \a index in the combobox.
*/

void QComboBox::insertStringList( const QStringList &list, int index )
{
    QStringList::ConstIterator it = list.begin();
    if ( index < 0 )
	index = count();
    while ( it != list.end() ) {
	if ( d->usingListBox() )
	    d->listBox()->insertItem( *it, index );
	else
	    d->popup()->insertItem( *it, index, index );
	if ( index++ == d->current && d->current < count() ) {
	    if ( d->ed ) {
		d->ed->setText( text( d->current ) );
		d->updateLinedGeometry();
	    } else
		update();
	    currentChanged();
	}
	++it;
    }
    if ( index != count() )
	reIndex();
}

/*!
  Inserts the array of char * \a strings at position \a index in the
  combobox.

  The \e numStrings argument is the number of strings.
  If \e numStrings is -1 (default), the \e strs array must be
  terminated with 0.

  Example:
  \code
    static const char* items[] = { "red", "green", "blue", 0 };
    combo->insertStrList( items );
  \endcode
*/

void QComboBox::insertStrList( const char **strings, int numStrings, int index)
{
    if ( !strings ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	if ( d->usingListBox() )
	    d->listBox()->insertItem( QString::fromLatin1(strings[i]), index );
	else
	    d->popup()->insertItem( QString::fromLatin1(strings[i]), index, index );
	i++;
	if ( index++ == d->current && d->current < count()  ) {
	    if ( d->ed ) {
		d->ed->setText( text( d->current ) );
		d->updateLinedGeometry();
	    } else
		update();
	    currentChanged();
	}
    }
    if ( index != count() )
	reIndex();
}


/*!
  Inserts a text item with text \a t, at position \a index. The item
  will be appended if \a index is negative.
*/

void QComboBox::insertItem( const QString &t, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", name(), cnt, &index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->insertItem( t, index );
    else
	d->popup()->insertItem( t, index, index );
    if ( index != cnt )
	reIndex();
    if ( index == d->current && d->current < count()  ) {
	if ( d->ed ) {
	    d->ed->setText( text( d->current ) );
	    d->updateLinedGeometry();
	} else
	    update();
    }
    if ( index == d->current )
	currentChanged();
}

/*!
  Inserts a \a pixmap item at position \a index. The item will be
  appended if \a index is negative.
*/

void QComboBox::insertItem( const QPixmap &pixmap, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", name(), cnt, &index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->insertItem( pixmap, index );
    else
	d->popup()->insertItem( pixmap, index, index );
    if ( index != cnt )
	reIndex();
    if ( index == d->current && d->current < count()  ) {
	if ( d->ed ) {
	    d->ed->setText( text( d->current ) );
	    d->updateLinedGeometry();
	} else
	    update();
    }
    if ( index == d->current )
	currentChanged();
}

/*!
  Inserts a \a pixmap item with additional text \a text at position \a
  index. The item will be appended if \a index is negative.
*/

void QComboBox::insertItem( const QPixmap &pixmap, const QString& text, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", name(), cnt, &index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->insertItem( pixmap, text, index );
    else
	d->popup()->insertItem( pixmap, text, index, index );
    if ( index != cnt )
	reIndex();
    if ( index == d->current && d->current < count()  ) {
	if ( d->ed ) {
	    d->ed->setText( this->text( d->current ) );
	    d->updateLinedGeometry();
	} else
	    update();
    }
    if ( index == d->current )
	currentChanged();
}


/*!
  Removes the item at position \a index.
*/

void QComboBox::removeItem( int index )
{
    int cnt = count();
    if ( !checkIndex( "removeItem", name(), cnt, index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->removeItem( index );
    else
	d->popup()->removeItemAt( index );
    if ( index != cnt-1 )
	reIndex();
    if ( index == d->current ) {
	if ( d->ed ) {
	    QString s = QString::fromLatin1("");
	    if (d->current < cnt - 1)
		s = text( d->current );
	    d->ed->setText( s );
	    d->updateLinedGeometry();
	}
	else {
	    if ( d->usingListBox() )
		d->current = d->listBox()->currentItem();
	    else {
		if (d->current > count()-1 && d->current > 0)
		    d->current--;
	    }
	    update();
	}
	currentChanged();
    }
    else {
	if ( !d->ed ) {
	    if (d->current < cnt - 1)
		setCurrentItem( d->current );
	    else
		setCurrentItem( d->current - 1 );
	}
    }

}


/*!
  Removes all combobox items.
*/

void QComboBox::clear()
{
    if ( d->usingListBox() ) {
	d->listBox()->resize( 0, 0 );
	d->listBox()->clear();
    } else {
	d->popup()->clear();
    }

    d->current = 0;
    if ( d->ed ) {
	d->ed->setText( QString::fromLatin1("") );
	d->updateLinedGeometry();
    }
    currentChanged();
}


QString QComboBox::currentText() const
{
    if ( d->ed )
	return d->ed->text();
    else if ( d->current < count() )
	return text( currentItem() );
    else
	return QString::null;
}


/*!
  Returns the text item at position \a index, or
  \link QString::operator!() null string\endlink
  if the item is not a string.
  \sa currentText()
*/

QString QComboBox::text( int index ) const
{
    if ( !checkIndex( "text", name(), count(), index ) )
	return QString::null;
    if ( d->usingListBox() )
	return d->listBox()->text( index );
    else
	return d->popup()->text( index );
}

/*!
  Returns the pixmap item at position \a index, or 0 if the item is not
  a pixmap.
*/

const QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", name(), count(), index ) )
	return 0;
    if ( d->usingListBox() )
	return d->listBox()->pixmap( index );
    else
	return d->popup()->pixmap( index );
}

/*!
  Replaces the item at position \a index with the text \a t.
*/

void QComboBox::changeItem( const QString &t, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->changeItem( t, index );
    else
	d->popup()->changeItem( t, index );
    if ( index == d->current ) {
	if ( d->ed ) {
	    d->ed->setText( text( d->current ) );
	    d->updateLinedGeometry();
	} else
	    update();
    }
}

/*!
  Replaces the item at position \a index with the pixmap \a im, unless
  the combobox is editable.

  \sa insertItem()
*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->changeItem( im, index );
    else
	d->popup()->changeItem( im, index );
    if ( index == d->current )
	update();
}

/*!
  Replaces the item at position \a index with the pixmap \a im and the
  text \a t.

  \sa insertItem()
*/

void QComboBox::changeItem( const QPixmap &im, const QString &t, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    if ( d->usingListBox() )
	d->listBox()->changeItem( im, t, index );
    else
	d->popup()->changeItem( im, t, index );
    if ( index == d->current )
	update();
}


int QComboBox::currentItem() const
{
    return d->current;
}

void QComboBox::setCurrentItem( int index )
{
    if ( index == d->current && !d->ed ) {
	return;
    }
    if ( !checkIndex( "setCurrentItem", name(), count(), index ) ) {
	return;
    }
    d->current = index;
    d->completeAt = 0;
    if ( d->ed ) {
	d->ed->setText( text( index ) );
	d->updateLinedGeometry();
    }
    if ( d->poppedUp && d->usingListBox() && d->listBox() )
	d->listBox()->setCurrentItem( index );
    else
	internalHighlight( index );

    currentChanged();
}

bool QComboBox::autoResize() const
{
    return d->autoresize;
}

void QComboBox::setAutoResize( bool enable )
{
    if ( (bool)d->autoresize != enable ) {
	d->autoresize = enable;
	if ( enable )
	    adjustSize();
    }
}


/*!\reimp
*/
QSize QComboBox::sizeHint() const
{
    if ( isVisibleTo(0) && d->sizeHint.isValid() )
	return d->sizeHint;

    constPolish();
    int i, w, h;
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = QMAX( fm.lineSpacing() + 2, 12 );

    for( i = 0; i < count(); i++ ) {
	if ( d->usingListBox() ) {
	    w = d->listBox()->item( i )->width( d->listBox() );
	    h = d->listBox()->item( i )->height( d->listBox() );
	}
	else {
	    h = d->popup()->itemHeight( i );
	    w = d->popup()->sizeHint().width() - 2* d->popup()->frameWidth();
	}
	if ( w > maxW )
	    maxW = w;
	if ( h > maxH )
	    maxH = h;
    }


    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
					    QSize(maxW, maxH)).
		   expandedTo(QApplication::globalStrut()));

//     if ( maxH <= 20 && style() == WindowsStyle || parentWidget() &&
// 	 ( parentWidget()->inherits( "QToolBar" ) ||
// 	   parentWidget()->inherits( "QDialog" ) && d->ed ) )
// 	maxH = 12;

//     int sw, sh;
//     if ( d->usingListBox() ) {
// 	sw = 4 + 4 + maxW;
// 	sh = 5 + 5 + maxH;
// 	QRect cr = style().comboButtonRect( 0, 0, sw, sh );
// 	sw += sw - cr.width();
// 	sw += sh - cr.height();
//     } else {
// 	//hardcoded values for motif 1.x style
// 	int extraW = 20+5;
// 	sw = 4 + 4 + maxW + extraW;
// 	sh = 5 + 5 + maxH;
//     }

//     d->sizeHint = QSize( sw, sh ).expandedTo( QApplication::globalStrut() );

    return d->sizeHint;
}


/*!
  \internal
  Receives activated signals from an internal popup list and emits
  the activated() signal.
*/

void QComboBox::internalActivate( int index )
{
    if ( d->current != index ) {
	d->current = index;
	currentChanged();
    }
    if ( d->usingListBox() )
	popDownListBox();
    else
	d->popup()->removeEventFilter( this );
    d->poppedUp = FALSE;

    QString t( text( index ) );
    if ( d->ed ) {
	d->ed->setText( t );
	d->updateLinedGeometry();
    }
    emit activated( index );
    emit activated( t );
}

/*!
  \internal
  Receives highlighted signals from an internal popup list and emits
  the highlighted() signal.
*/

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
    QString t = text( index );
    if ( !t.isNull() )
	emit highlighted( t );
}

/*!
  \internal
  Receives timeouts after a click. Used to decide if a Motif style
  popup should stay up or not after a click.
*/
void QComboBox::internalClickTimeout()
{
    d->shortClick = FALSE;
}

/*!
  Reimplements QWidget::setPalette().

  Sets the palette for both the combobox button and the
  combobox popup list to \a palette.
*/

void QComboBox::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    if ( d->usingListBox() )
	d->listBox()->setPalette( palette );
    else
	d->popup()->setPalette( palette );
}

/*!
  Reimplements QWidget::setFont().

  Sets the font for both the combobox button and the
  combobox popup list to \a font.
*/

void QComboBox::setFont( const QFont &font )
{
    QWidget::setFont( font );
    if ( d->usingListBox() )
	d->listBox()->setFont( font );
    else
	d->popup()->setFont( font );
    if (d->ed)
	d->ed->setFont( font );
    if ( d->autoresize )
	adjustSize();
}


/*!\reimp
*/

void QComboBox::resizeEvent( QResizeEvent * e )
{
    if ( d->ed ) {
	d->updateLinedGeometry();
    }
    QWidget::resizeEvent( e );
}

/*!\reimp
*/

void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    const QColorGroup & g = colorGroup();

    if ( width() < 5 || height() < 5 ) {
	qDrawShadePanel( &p, rect(), g, FALSE, 2,
			 &g.brush( QColorGroup::Button ) );
	return;
    }

    bool reverse = QApplication::reverseLayout();
    if ( !d->usingListBox() ) {			// motif 1.x style
	int dist, buttonH, buttonW;
	getMetrics( &dist, &buttonW, &buttonH );
	int xPos;
	int x0;
	int w = width() - dist - buttonW - 1;
	if ( reverse ) {
	    xPos = dist + 1;
	    x0 = xPos + 4;
	} else {
	    xPos = w;
	    x0 = 4;
	}
	qDrawShadePanel( &p, rect(), g, FALSE,
			 style().pixelMetric(QStyle::PM_DefaultFrameWidth, this),
			 &g.brush( QColorGroup::Button ) );
	qDrawShadePanel( &p, xPos, (height() - buttonH)/2,
			 buttonW, buttonH, g, FALSE,
			 style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) );
	QRect clip( x0, 2, w - 2 - 4 - 5, height() - 4 );
	QString str = d->popup()->text( this->d->current );
	if ( !str.isNull() ) {
	    p.drawText( clip, AlignCenter | SingleLine, str );
	} else {
	    QPixmap *pix = d->popup()->pixmap( this->d->current );
	    if ( pix ) {
		p.setClipRect( clip );
		p.drawPixmap( 4, (height()-pix->height())/2, *pix );
		p.setClipping( FALSE );
	    }
	}

	if ( hasFocus() )
	    p.drawRect( xPos - 5, 4, width() - xPos + 1 , height() - 8 );

    } else if ( style() == MotifStyle ) {	// motif 2.0 style
	QStyle::PFlags flags = QStyle::PStyle_Default;

	if ( d->arrowDown )
	    flags |= QStyle::PStyle_Sunken;
	style().drawComplexControl( QStyle::CC_ComboBox, &p, this, rect(), g,
				    QStyle::CStyle_Default, QStyle::SC_None,
				    flags );

	if ( !d->ed ) {
	    QRect clip = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
							QStyle::SC_ComboBoxEditField);
	    p.setPen( g.foreground() );
	    p.setClipRect( clip );
	    p.setPen( g.foreground() );
	    QListBoxItem * item = d->listBox()->item( d->current );
	    if ( item ) {
		int itemh = item->height( d->listBox() );
		p.translate( clip.x(), clip.y() + (clip.height() - itemh)/2  );
		item->paint( &p );
	    }
	} else if ( d->listBox() && d->listBox()->item( d->current ) ) {
	    QRect r = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
						     QStyle::SC_ComboBoxEditField);
	    QListBoxItem * item = d->listBox()->item( d->current );
	    const QPixmap *pix = item->pixmap();
	    if ( pix ) {
		p.fillRect( r.x(), r.y(), pix->width() + 4, r.height(), colorGroup().brush( QColorGroup::Base ) );
		p.drawPixmap( r.x() + 2, r.y() + ( r.height() - pix->height() ) / 2, *pix );
	    }
	}
	p.setClipping( FALSE );
    } else {					// windows 95 style
	QStyle::PFlags flags = QStyle::PStyle_Default;

	if ( d->arrowDown )
	    flags |= QStyle::PStyle_Sunken;
	style().drawComplexControl( QStyle::CC_ComboBox, &p, this, rect(), g,
				    QStyle::CStyle_Default, QStyle::SC_None,
				    flags );

	QRect re = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
						   QStyle::SC_ComboBoxEditField );
	QRect textR;
	textR.setRect( re.x()+2, re.y()+1, re.width()-4, re.height()-2 );
	p.setClipRect( textR );

	if ( !d->ed ) {
	    QListBoxItem * item = d->listBox()->item( d->current );
	    if ( item ) {
		int itemh = item->height( d->listBox() );
		p.translate( textR.x(), textR.y() +
			     (textR.height() - itemh)/2  );
		item->paint( &p );
	    }
	} else if ( d->listBox() && d->listBox()->item( d->current ) ) {
	    p.setClipping( FALSE );
	    QRect r = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
						     QStyle::SC_ComboBoxEditField);
	    QListBoxItem * item = d->listBox()->item( d->current );
	    const QPixmap *pix = item->pixmap();
	    if ( pix ) {
		p.fillRect( r.x(), r.y(), pix->width() + 4, r.height(),
			    colorGroup().brush( QColorGroup::Base ) );
		p.drawPixmap( r.x() + 2, r.y() +
			      ( r.height() - pix->height() ) / 2, *pix );
	    }
	}
	p.setClipping( FALSE );
    }
}


/*!
  \internal
  Returns the button arrow rectangle for windows style comboboxes.
*/
QRect QComboBox::arrowRect() const
{
    return QRect( width() - 2 - 16, 2, 16, height() - 4 );
}


/*!\reimp
*/

void QComboBox::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    if ( d->discardNextMousePress ) {
	d->discardNextMousePress = FALSE;
	return;
    }
    if ( count() ) {
	d->arrowPressed = FALSE;
	if ( style() == WindowsStyle ) {
	    popup();
	    if ( arrowRect().contains( e->pos() ) ) {
		d->arrowPressed = TRUE;
		d->arrowDown    = TRUE;
		repaint( FALSE );
	    }
	} else if ( d->usingListBox() ) {
	    popup();
	    QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	    d->shortClick = TRUE;
	} else {
	    popup();
	    QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	    d->shortClick = TRUE;
	}
    }
}

/*!\reimp
*/

void QComboBox::mouseMoveEvent( QMouseEvent * )
{
}

/*!\reimp
*/

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
}

/*!\reimp
*/

void QComboBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}


/*!\reimp
*/

void QComboBox::keyPressEvent( QKeyEvent *e )
{
    int c;
    if ( ( e->key() == Key_F4 && e->state() == 0 ) ||
	 ( e->key() == Key_Down && (e->state() & AltButton) ) ||
	 ( !d->ed && e->key() == Key_Space ) ) {
	if ( count() ) {
	    if ( !d->usingListBox() )
		d->popup()->setActiveItem( this->d->current );
	    popup();
	}
	return;
    } else if ( d->usingListBox() && e->key() == Key_Up ) {
	c = currentItem();
	if ( c > 0 )
	    setCurrentItem( c-1 );
    } else if ( d->usingListBox() && e->key() == Key_Down ) {
	c = currentItem();
	if ( ++c < count() )
	    setCurrentItem( c );
    } else if ( d->usingListBox() && e->key() == Key_Home && ( !d->ed || !d->ed->hasFocus() ) ) {
	setCurrentItem( 0 );
    } else if ( d->usingListBox() && e->key() == Key_End && ( !d->ed || !d->ed->hasFocus() ) ) {
	setCurrentItem( count()-1 );
    } else if ( /* d->useCompletion && */ d->ed == 0 &&
		e->text() != QString::null ) {
	// strictly speaking, we should test for useCompletion, but
	// the code is a pure win, so let's do it anyway.
	QString ct = currentText().left( d->completeAt ) + e->text();
	int i = completionIndex( ct, currentItem() );
	if ( i < 0 && d->completeAt > 0 )
	    i = completionIndex( e->text(), 0 );
	d->completeAt = 0;
	if ( i >= 0 ) {
	    setCurrentItem( i );
	    d->completeAt = ct.length();
	}
    } else {
	e->ignore();
	return;
    }

    c = currentItem();
    if ( !text( c ).isNull() )
	emit activated( text( c ) );
    emit highlighted( c );
    emit activated( c );
}


/*!\reimp
*/

void QComboBox::focusInEvent( QFocusEvent * e )
{
    QWidget::focusInEvent( e );
    d->completeNow = FALSE;
}

/*!
  \internal
   Calculates the listbox height needed to contain all items, or as
   many as the list box is supposed to contain.
*/
static int listHeight( QListBox *l, int sl )
{
    if ( l->count() > 0 )
	return QMIN( l->count(), (uint)sl) * l->item( 0 )->height(l);
    else
	return l->sizeHint().height();
}


/*!
  Pops up the combobox popup list.

  If the list is empty, no items appear.
*/

void QComboBox::popup()
{
    if ( !count() )
	return;

    if ( d->usingListBox() ) {
			// Send all listbox events to eventFilter():
	d->listBox()->installEventFilter( this );
	d->mouseWasInsidePopup = FALSE;
	d->listBox()->resize( width(),
			    listHeight( d->listBox(), d->sizeLimit ) + 2 );
	QRect screen = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( this ) );

	int sx = screen.x();				// screen pos
	int sy = screen.y();
	int sw = screen.width();			// screen width
	int sh = screen.height();			// screen height
	QPoint pos = mapToGlobal( QPoint(0,height()) );

	// ### Similar code is in QPopupMenu
	int x = pos.x();
	int y = pos.y();
	int w = d->listBox()->width();
	int h = d->listBox()->height();

	// the complete widget must be visible
	if ( x + w > sx + sw )
	    x = sx+sw - w;
	if ( x < sx )
	    x = sx;
	if (y + h > sy+sh && y - h - height() >= 0 )
	    y = y - h - height();

	d->listBox()->move( x,y );
	d->listBox()->raise();
	bool block = d->listBox()->signalsBlocked();
	d->listBox()->blockSignals( TRUE );
	d->listBox()->setCurrentItem( d->listBox()->item( d->current ) );
	d->listBox()->blockSignals( block );
	d->listBox()->setVScrollBarMode(QScrollView::Auto);

#ifndef QT_NO_EFFECTS
	if ( QApplication::isEffectEnabled( UI_AnimateCombo ) )
	    qScrollEffect( d->listBox() );
	else
#endif
	    d->listBox()->show();
    } else {
	d->popup()->installEventFilter( this );
	d->popup()->popup( mapToGlobal( QPoint(0,0) ), this->d->current );
    }
    d->poppedUp = TRUE;
}


/*!
  \reimp
*/
void QComboBox::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    {
	QPainter p( &bm, this );
	style().drawComplexControlMask(QStyle::CC_ComboBox, &p, this, rect());
    }

    setMask( bm );
}

/*!
  \internal
  Pops down (removes) the combobox popup list box.
*/
void QComboBox::popDownListBox()
{
    Q_ASSERT( d->usingListBox() );
    d->listBox()->removeEventFilter( this );
    d->listBox()->viewport()->removeEventFilter( this );
    d->listBox()->hide();
    d->listBox()->setCurrentItem( d->current );
    if ( d->arrowDown ) {
	d->arrowDown = FALSE;
	repaint( FALSE );
    }
    d->poppedUp = FALSE;
}


/*!
  \internal
  Re-indexes the identifiers in the popup list.
*/

void QComboBox::reIndex()
{
    if ( !d->usingListBox() ) {
	int cnt = count();
	while ( cnt-- )
	    d->popup()->setId( cnt, cnt );
    }
}

/*!
  \internal
  Repaints the combobox.
*/

void QComboBox::currentChanged()
{
    if ( d->autoresize )
	adjustSize();
    update();
}

/*! \reimp

  \internal

  The event filter steals events from the popup or listbox when they
  are popped up. It makes the popup stay up after a short click in
  motif style. In windows style it toggles the arrow button of the
  combobox field, and activates an item and takes down the listbox
  when the mouse button is released.
*/

bool QComboBox::eventFilter( QObject *object, QEvent *event )
{
    if ( !event )
	return TRUE;
    else if ( object == d->ed ) {
	if ( event->type() == QEvent::KeyPress ) {
	    bool isAccepted = ( (QKeyEvent*)event )->isAccepted();
	    keyPressEvent( (QKeyEvent *)event );
	    if ( ((QKeyEvent *)event)->isAccepted() ) {
		d->completeNow = FALSE;
		return TRUE;
	    } else if ( ((QKeyEvent *)event)->key() != Key_End ) {
		d->completeNow = TRUE;
		d->completeAt = d->ed->cursorPosition();
	    }
	    if ( isAccepted )
		( (QKeyEvent*)event )->accept();
	    else
		( (QKeyEvent*)event )->ignore();
	} else if ( event->type() == QEvent::KeyRelease ) {
	    d->completeNow = FALSE;
	    keyReleaseEvent( (QKeyEvent *)event );
	    return ((QKeyEvent *)event)->isAccepted();
	} else if ( (event->type() == QEvent::FocusIn ||
		     event->type() == QEvent::FocusOut ) ) {
	    d->completeNow = FALSE;
	    // to get the focus indication right
	    update();
	} else if ( d->useCompletion && d->completeNow ) {
	    if ( !d->ed->text().isNull() &&
		 d->ed->cursorPosition() > d->completeAt &&
		 d->ed->cursorPosition() == (int)d->ed->text().length() ) {
		d->completeNow = FALSE;
		QString ct( d->ed->text() );
		int i = completionIndex( ct, currentItem() );
		if ( i > -1 ) {
		    QString it = text( i );
		    d->ed->validateAndSet( it, ct.length(),
					   ct.length(), it.length() );
		}
	    }
	}
    } else if ( d->usingListBox() && ( object == d->listBox() ||
				       object == d->listBox()->viewport() )) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch( event->type() ) {
	case QEvent::MouseMove:
	    if ( !d->mouseWasInsidePopup  ) {
		QPoint pos = e->pos();
		if ( d->listBox()->rect().contains( pos ) )
		    d->mouseWasInsidePopup = TRUE;
		// Check if arrow button should toggle
		// this applies only to windows style
		if ( d->arrowPressed ) {
		    QPoint comboPos;
		    comboPos = mapFromGlobal( d->listBox()->mapToGlobal(pos) );
		    if ( arrowRect().contains( comboPos ) ) {
			if ( !d->arrowDown  ) {
			    d->arrowDown = TRUE;
			    repaint( FALSE );
			}
		    } else {
			if ( d->arrowDown  ) {
			    d->arrowDown = FALSE;
			    repaint( FALSE );
			}
		    }
		}
	    } else if ((e->state() & ( RightButton | LeftButton | MidButton ) )
		       == 0 && style() == WindowsStyle ){
		QWidget *mouseW = QApplication::widgetAt( e->globalPos(), TRUE );
		if ( mouseW == d->listBox()->viewport() ) { //###
		    QMouseEvent m( QEvent::MouseMove, e->pos(), e->globalPos(),
				   0, LeftButton );
		    QApplication::sendEvent( object, &m ); //### Evil
		    return TRUE;
		}
	    }

	    break;
	case QEvent::MouseButtonRelease:
	    if ( d->listBox()->rect().contains( e->pos() ) ) {
		QMouseEvent tmp( QEvent::MouseButtonDblClick,
				 e->pos(), e->button(), e->state() ) ;
		// will hide popup
		QApplication::sendEvent( object, &tmp );
		return TRUE;
	    } else {
		if ( d->mouseWasInsidePopup ) {
		    popDownListBox();
		} else {
		    d->arrowPressed = FALSE;
		    if ( d->arrowDown  ) {
			d->arrowDown = FALSE;
			repaint( FALSE );
		    }
		}
	    }
	    break;
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseButtonPress:
	    if ( !d->listBox()->rect().contains( e->pos() ) ) {
		QPoint globalPos = d->listBox()->mapToGlobal(e->pos());
		if ( QApplication::widgetAt( globalPos, TRUE ) == this ) {
		    d->discardNextMousePress = TRUE;
		    // avoid popping up again
		}
		popDownListBox();
		return TRUE;
	    }
	    break;
	case QEvent::KeyPress:
	    switch( ((QKeyEvent *)event)->key() ) {
	    case Key_Up:
	    case Key_Down:
		if ( !(((QKeyEvent *)event)->state() & AltButton) )
		    break;
	    case Key_F4:
	    case Key_Escape:
		if ( d->poppedUp ) {
		    popDownListBox();
		    return TRUE;
		}
		break;
	    case Key_Enter:
	    case Key_Return:
		// work around QDialog's enter handling
		return FALSE;
	    default:
		break;
	    }
	default:
	    break;
	}
    } else if ( !d->usingListBox() && object == d->popup() ) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch ( event->type() ) {
	case QEvent::MouseButtonRelease:
	    if ( d->shortClick ) {
		QMouseEvent tmp( QEvent::MouseMove,
				 e->pos(), e->button(), e->state() ) ;
		// highlight item, but don't pop down:
		QApplication::sendEvent( object, &tmp );
		return TRUE;
	    }
	    break;
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseButtonPress:
	    if ( !d->popup()->rect().contains( e->pos() ) ) {
		// remove filter, event will take down popup:
		d->popup()->removeEventFilter( this );
		// ### uglehack!
		// call internalHighlight so the highlighed signal
		// will be emitted at least as often as necessary.
		// it may be called more often than necessary
		internalHighlight( d->current );
	    }
	    break;
	default:
	    break;
	}
    }
    return QWidget::eventFilter( object, event );
}


/*! Returns the index of the first item \e after \a startingAt of
  which \a prefix is a case-insensitive prefix. Returns -1 if no items
  start with \a prefix.
*/

int QComboBox::completionIndex( const QString & prefix,
				int startingAt = 0 ) const
{
    int start = startingAt;
    if ( start < 0 || start >= count() )
	start = 0;
    if ( start >= count() )
	return -1;
    QString match = prefix.lower();
    if ( match.length() < 1 )
	return start;

    QString current;
    int i = start;
    do {
	current = text( i );
	if ( current.length() >= match.length() ) {
	    current.truncate( match.length() );
	    if ( current.lower() == match.lower() )
		return i;
	}
	i++;
	if ( i == count() )
	    i = 0;
    } while ( i != start );
    return -1;
}


int QComboBox::sizeLimit() const
{
    return d ? d->sizeLimit : INT_MAX;
}

void QComboBox::setSizeLimit( int lines )
{
    d->sizeLimit = lines;
}


int QComboBox::maxCount() const
{
    return d ? d->maxCount : INT_MAX;
}

void QComboBox::setMaxCount( int count )
{
    int l = this->count();
    while( --l > count )
	removeItem( l );
    d->maxCount = count;
}

QComboBox::Policy QComboBox::insertionPolicy() const
{
    return d->p;
}

void QComboBox::setInsertionPolicy( Policy policy )
{
    d->p = policy;
}



/*!
  Internal slot to keep the line editor up to date.
*/

void QComboBox::returnPressed()
{
    QString s( d->ed->text() );
    int c = 0;
    bool doInsert = TRUE;
    if ( !d->duplicatesEnabled ) {
	for ( int i = 0; i < count(); ++i ) {
	    if ( s == text( i ) ) {
		doInsert = FALSE;
		c = i;
		break;
	    }
	}
    }

    if ( doInsert ) {
	if ( insertionPolicy() != NoInsertion ) {
	    int cnt = count();
	    while ( cnt >= d->maxCount ) {
		removeItem( --cnt );
	    }
	}

	switch ( insertionPolicy() ) {
	case AtCurrent:
	    if ( s != text( currentItem() ) )
		changeItem( s, currentItem() );
	    emit activated( currentItem() );
	    emit activated( s );
	    return;
	case NoInsertion:
	    emit activated( s );
	    return;
	case AtTop:
	    c = 0;
	    break;
	case AtBottom:
	    c = count();
	    break;
	case BeforeCurrent:
	    c = currentItem();
	    break;
	case AfterCurrent:
	    c = currentItem() + 1;
	    break;
	}
	insertItem( s, c );
    }

    setCurrentItem( c );
    emit activated( c );
    emit activated( s );
}


/*! \reimp
*/

void QComboBox::setEnabled( bool enable )
{
    QWidget::setEnabled( enable );
}



/*!
    Applies the validator \a v to the combobox so that only text which
    is valid according to \a v is accepted.

  This function does nothing if the combo is not editable.

  \sa validator() clearValidator() QValidator
*/

void QComboBox::setValidator( const QValidator * v )
{
    if ( d && d->ed )
	d->ed->setValidator( v );
}


/*!  Returns the validator which constrains editing for this combobox if
 there is one, otherwise returns 0.

  \sa setValidator() clearValidator() QValidator
*/

const QValidator * QComboBox::validator() const
{
    return d && d->ed ? d->ed->validator() : 0;
}


/*!  This slot is equivalent to setValidator( 0 ). */

void QComboBox::clearValidator()
{
    if ( d && d->ed )
	d->ed->setValidator( 0 );
}


/*!  Sets the combobox to use \a newListBox instead of the current
  list box or popup.  As a side effect, it clears the combobox of its
  current contents.

  \warning QComboBox assumes that newListBox->text(n) returns
  non-null for 0 \<= n \< newListbox->count().  This assumption is
  necessary because of the line edit in QComboBox.
*/

void QComboBox::setListBox( QListBox * newListBox )
{
    clear();

    if ( d->usingListBox() )
	delete d->listBox();
    else
	delete d->popup();

    newListBox->reparent( this, WType_Popup, QPoint(0,0), FALSE );
    d->setListBox( newListBox );
    d->listBox()->setFont( font() );
    d->listBox()->setVScrollBarMode(QScrollView::AlwaysOff);
    d->listBox()->setHScrollBarMode(QScrollView::AlwaysOff);
    d->listBox()->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listBox()->setLineWidth( 1 );
    d->listBox()->resize( 100, 10 );

    connect( d->listBox(), SIGNAL(selected(int)),
	     SLOT(internalActivate(int)) );
    connect( d->listBox(), SIGNAL(highlighted(int)),
	     SLOT(internalHighlight(int)));
}


/*!  Returns the current list box, or 0 if there is no list box.
  (QComboBox can use QPopupMenu instead of QListBox.)
  Provided to match setListBox().

  \sa setListBox()
*/

QListBox * QComboBox::listBox() const
{
    return d && d->usingListBox() ? d->listBox() : 0;
}

/*!
  Returns the line editor, or 0 if there is no line editor.

  Only editable listboxes have a line editor.
 */
QLineEdit* QComboBox::lineEdit() const
{
    return d->ed;
}



/*!  Clears the line edit without changing the combobox's contents.
 Does nothing if the combobox isn't editable.

  This is particularly handy when using a combobox as a line edit
  with history.  For example you can connect the combobox's activated()
  signal to clearEdit() in order to present the user with a new, empty
  line as soon as Return is pressed.

  \sa setEditText()
*/

void QComboBox::clearEdit()
{
    if ( d && d->ed )
	d->ed->clear();
}


/*!  Sets the text in the line edit to \a newText without
  changing the combobox's contents.  Does nothing if the combobox isn't
  editable.

  This is useful e.g. for providing a good starting point for the
  user's editing and entering the change in the combobox only when the
  user presses Enter.

  \sa clearEdit() insertItem()
*/

void QComboBox::setEditText( const QString &newText )
{
    if ( d && d->ed ) {
	d->updateLinedGeometry();
	d->ed->setText( newText );
    }
}

void QComboBox::setAutoCompletion( bool enable )
{
    d->useCompletion = enable;
    d->completeNow = FALSE;
}


bool QComboBox::autoCompletion() const
{
    return d->useCompletion;
}

/*!\reimp
 */
void QComboBox::styleChange( QStyle& s )
{
    if ( d->ed )
	d->updateLinedGeometry();
    QWidget::styleChange( s );
}

bool QComboBox::editable() const
{
    return d->ed != 0;
}

void QComboBox::setEditable( bool y )
{
    if ( y == editable() )
	return;
    if ( y ) {
	if ( !d->usingListBox() )
	    setUpListBox();
	setUpLineEdit();
	d->ed->show();
    } else {
	delete d->ed;
	d->ed = 0;
    }
    updateGeometry();
    update();
}


void QComboBox::setUpListBox()
{
    d->setListBox( new QListBox( this, "in-combo", WType_Popup ) );
    d->listBox()->setFont( font() );
    d->listBox()->setAutoScrollBar( FALSE );
    d->listBox()->setBottomScrollBar( FALSE );
    d->listBox()->setAutoBottomScrollBar( FALSE );
    d->listBox()->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listBox()->setLineWidth( 1 );
    d->listBox()->resize( 100, 10 );

    connect( d->listBox(), SIGNAL(selected(int)),
	     SLOT(internalActivate(int)) );
    connect( d->listBox(), SIGNAL(highlighted(int)),
	     SLOT(internalHighlight(int)));
}


void QComboBox::setUpLineEdit()
{
    d->ed = new QLineEdit( this, "combo edit" );
    connect (d->ed, SIGNAL( textChanged( const QString& ) ),
	     this, SIGNAL( textChanged( const QString& ) ) );
    d->ed->setFrame( FALSE );
    d->updateLinedGeometry();
    d->ed->installEventFilter( this );
    setFocusProxy( d->ed );

    connect( d->ed, SIGNAL(returnPressed()), SLOT(returnPressed()) );
}

#endif // QT_NO_COMBOBOX
