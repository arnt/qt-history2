/****************************************************************************
** $Id$
**
** Implementation of QKeySequence class
**
** Created : 0108007
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qkeysequence.h"

#ifndef QT_NO_ACCEL

#include "qaccel.h"
#include "qshared.h"
#include "qvaluelist.h"

#ifdef Q_WS_MAC
#define QMAC_CTRL (QString(QChar(0x2318)))
#define QMAC_ALT  (QString(QChar(0x2325)))
#endif

/*!
  \class QKeySequence qkeysequence.h
  \brief The QKeySequence class encapsulates a key sequence as used by accelerators.

  \ingroup misc
  
  A key sequence consists of a keyboard code, optionally combined with
  modifiers, e.g. \c SHIFT, \c CTRL, \c ALT or \c UNICODE_ACCEL.  For
  example, \c{CTRL + Key_P} might be a sequence used as a shortcut for
  printing a document. The key codes are listed in \c{qnamespace.h}.
  As an alternative, use \c UNICODE_ACCEL with the unicode code point
  of the character. For example, \c{UNICODE_ACCEL + 'A'} gives the
  same key sequence as \c Key_A.

   Key sequences can be constructed either from an integer key code,
   or from a human readable translatable string.  A key sequence can
   be cast to a QString to obtain a human readable translated
   version of the sequence. Translations are done in the "QAccel"
   scope.

   \sa QAccel
*/

static struct {
    int key;
    const char* name;
} keyname[] = {
    { Qt::Key_Space,	QT_TRANSLATE_NOOP( "QAccel", "Space" ) },
    { Qt::Key_Escape,	QT_TRANSLATE_NOOP( "QAccel", "Esc" ) },
    { Qt::Key_Tab,	QT_TRANSLATE_NOOP( "QAccel", "Tab" ) },
    { Qt::Key_Backtab,	QT_TRANSLATE_NOOP( "QAccel", "Backtab" ) },
    { Qt::Key_Backspace,	QT_TRANSLATE_NOOP( "QAccel", "Backspace" ) },
    { Qt::Key_Return,	QT_TRANSLATE_NOOP( "QAccel", "Return" ) },
    { Qt::Key_Enter,	QT_TRANSLATE_NOOP( "QAccel", "Enter" ) },
    { Qt::Key_Insert,	QT_TRANSLATE_NOOP( "QAccel", "Ins" ) },
    { Qt::Key_Delete,	QT_TRANSLATE_NOOP( "QAccel", "Del" ) },
    { Qt::Key_Pause,	QT_TRANSLATE_NOOP( "QAccel", "Pause" ) },
    { Qt::Key_Print,	QT_TRANSLATE_NOOP( "QAccel", "Print" ) },
    { Qt::Key_SysReq,	QT_TRANSLATE_NOOP( "QAccel", "SysReq" ) },
    { Qt::Key_Home,	QT_TRANSLATE_NOOP( "QAccel", "Home" ) },
    { Qt::Key_End,	QT_TRANSLATE_NOOP( "QAccel", "End" ) },
    { Qt::Key_Left,	QT_TRANSLATE_NOOP( "QAccel", "Left" ) },
    { Qt::Key_Up,		QT_TRANSLATE_NOOP( "QAccel", "Up" ) },
    { Qt::Key_Right,	QT_TRANSLATE_NOOP( "QAccel", "Right" ) },
    { Qt::Key_Down,	QT_TRANSLATE_NOOP( "QAccel", "Down" ) },
    { Qt::Key_Prior,	QT_TRANSLATE_NOOP( "QAccel", "PgUp" ) },
    { Qt::Key_Next,	QT_TRANSLATE_NOOP( "QAccel", "PgDown" ) },
    { Qt::Key_CapsLock,	QT_TRANSLATE_NOOP( "QAccel", "CapsLock" ) },
    { Qt::Key_NumLock,	QT_TRANSLATE_NOOP( "QAccel", "NumLock" ) },
    { Qt::Key_ScrollLock,	QT_TRANSLATE_NOOP( "QAccel", "ScrollLock" ) },
    { 0, 0 }
};

class QKeySequencePrivate : public QShared
{
public:
    QKeySequencePrivate( int k = 0 ): key(k) {}
    int key;
};


/*!
  Constructs an empty key sequence.
 */
QKeySequence::QKeySequence()
{
    d = new QKeySequencePrivate;
}

/*!
  Constructs a key sequence from the keycode \a key.

  The key codes are listed in \c{qnamespace.h} and can be combined with
  modifiers, e.g. with \c SHIFT, \c CTRL, \c ALT or \c UNICODE_ACCEL.
 */
QKeySequence::QKeySequence( int key )
{
    d = new QKeySequencePrivate( key  );
}

/*!
  Creates a key sequence from the string \a key.  For example "Ctrl+O"
  gives CTRL+UNICODE_ACCEL+'O'.  The strings "Ctrl", "Shift" and "Alt"
  are recognized, as well as their translated equivalents in the
  "QAccel" scope (using QObject::tr()).

  This contructor is typically used with \link QObject::tr() tr
  \endlink(), so that accelerator keys can be replaced in
  translations:

  \code
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( tr("&Open..."), this, SLOT(open()),
		      QKeySequence( tr("Ctrl+O", "File|Open") ) );
  \endcode

  Note the \c "File|Open" translator comment. It is by no means
  necessary, but it provides some context for the human translator.
*/

QKeySequence::QKeySequence( const QString& key )
{
    int k = 0;
    int p = key.findRev( '+', key.length() - 2 ); // -2 so that Ctrl++ works
    QString name;
    if ( p > 0 ) {
	name = key.mid( p + 1 );
    } else {
	name = key;
    }
    int fnum;
    if ( name.length() == 1 ) {
	if ( name.at(0).isLetterOrNumber() ) {
	    QString uppname = name.upper();
	    k = uppname[0].unicode();
	} else {
	    k = name[0].unicode() | UNICODE_ACCEL;
	}
    } else if ( name[0] == 'F' && (fnum = name.mid(1).toInt()) ) {
	k = Key_F1 + fnum - 1;
    } else {
	for ( int tran = 0; tran < 2; tran++ ) {
	    for ( int i = 0; keyname[i].name; i++ ) {
		if ( tran ? name == QAccel::tr(keyname[i].name)
			  : name == keyname[i].name )
		{
		    k = keyname[i].key;
		    goto done;
		}
	    }
	}
    }
done:
    if ( p > 0 ) {
	QString sl = key.lower();

#ifdef QMAC_CTRL
	if ( sl.contains(QMAC_CTRL+"+") )
	    k |= CTRL;
#endif
	if ( sl.contains("ctrl+") || sl.contains(QAccel::tr("Ctrl").lower() + "+") )
	    k |= CTRL;

#ifdef QMAC_ALT
	if ( sl.contains(QMAC_ALT+"+") )
	    k |= ALT;
#endif
	if ( sl.contains("alt+") || sl.contains(QAccel::tr("Alt").lower() + "+") )
	    k |= ALT;

	if ( sl.contains("shift+") || sl.contains(QAccel::tr("Shift").lower() + "+") )
	    k |= SHIFT;
    }
    d = new QKeySequencePrivate( k );
}

/*!
  Copy constructor. Makes a copy of \a keysequence.
 */
QKeySequence::QKeySequence( const QKeySequence& keysequence )
{
    d = keysequence.d;
    d->ref();
}

/*!  Destroys the key sequence.
 */
QKeySequence::~QKeySequence()
{
    if ( d->deref() ) {
	delete d;
    }
}

/*!
  Assignment operator. Assigns \a keysequence to this object.
 */
QKeySequence &QKeySequence::operator=( const QKeySequence & keysequence )
{
    keysequence.d->ref();
    if ( d->deref() )
	delete d;
    d = keysequence.d;
    return *this;
}


/*!
  For backward compatibility: returns the keycode as integer.

  If QKeySequence ever supports more than one keycode, this function
  will return the first one.
 */
QKeySequence::operator int () const
{
    return d->key;
}

/*!
   Creates an accelerator string for the keysequence.
   For instance CTRL+Key_O gives "Ctrl+O".  The strings, "Ctrl",
   "Shift", etc. are translated (using QObject::tr()) in the "QAccel"
   scope.
*/
QKeySequence::operator QString() const
{
    QString s;
    int k = d->key;
    if ( (k & CTRL) == CTRL ) {
	QString ctrl = QAccel::tr( "Ctrl" );
#ifdef QMAC_CTRL
	if(ctrl == "Ctrl")
	    ctrl = QMAC_CTRL;
#endif
	s += ctrl;
    }
    if ( (k & ALT) == ALT ) {
	if ( !s.isEmpty() )
	    s += QAccel::tr( "+" );
	QString alt = QAccel::tr( "Alt" );
#ifdef QMAC_ALT
	if(alt == "Alt")
	    alt = QMAC_ALT;
#endif
	s += alt;
    }
    if ( (k & SHIFT) == SHIFT ) {
	if ( !s.isEmpty() )
	    s += QAccel::tr( "+" );
	s += QAccel::tr( "Shift" );
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( (k & UNICODE_ACCEL) == UNICODE_ACCEL ) {
	p = QChar(k & 0xffff);
    } else if ( k >= Key_F1 && k <= Key_F24 ) {
	p = QAccel::tr( "F%1" ).arg(k - Key_F1 + 1);
    } else if ( k > Key_Space && k <= Key_AsciiTilde ) {
	p.sprintf( "%c", k );
    } else {
	int i=0;
	while (keyname[i].name) {
	    if ( k == keyname[i].key ) {
		p = QAccel::tr(keyname[i].name);
		break;
	    }
	    ++i;
	}
	if ( !keyname[i].name )
	    p.sprintf( "<%d?>", k );

    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += QAccel::tr( "+" );
	s += p;
    }
    return s;
}


/*!
  Returns TRUE if \a keysequence is equal to this keysequence;
  otherwise returns FALSE.
 */
bool QKeySequence::operator==( const QKeySequence& keysequence ) const
{
    return d->key == keysequence.d->key;
}

/*!
  Returns TRUE if \a keysequence is not equal to this keysequence;
  otherwise returns FALSE.
 */
bool QKeySequence::operator!= ( const QKeySequence& keysequence ) const
{
    return d->key != keysequence.d->key;
}



/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
/*!
  \relates QKeySequence
  Writes the key sequence \a keysequence to the stream \a s.

  \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QKeySequence &keysequence )
{
    QValueList<int> list;
    list += (int) keysequence;
    s << list;
    return s;
}

/*!
  \relates QKeySequence
  Reads a key sequence from the stream \a s into the keysequence \a keysequence.
*/

QDataStream &operator>>( QDataStream &s, QKeySequence &keysequence )
{
    QValueList<int> list;
    s >> list;
    keysequence = QKeySequence( list.first() );
    return s;
}

#endif //QT_NO_DATASTREAM

#endif //QT_NO_ACCEL
