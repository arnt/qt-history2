/****************************************************************************
**
** Implementation of QKeySequence class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkeysequence.h"

#ifndef QT_NO_ACCEL

#include "qaccel.h"
#include "qshared.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#ifndef QT_NO_DATASTREAM
#  include "qdatastream.h"
#endif

#ifdef Q_WS_MAC
#define QMAC_CTRL  (QString(QChar(0x2318)))
#define QMAC_META  (QString(QChar(0x2303)))
#define QMAC_ALT   (QString(QChar(0x2325)))
#define QMAC_SHIFT (QString(QChar(0x21E7)))
#endif

/*!
    \class QKeySequence qkeysequence.h
    \brief The QKeySequence class encapsulates a key sequence as used
    by accelerators.

    \ingroup misc

    A key sequence consists of up to four keyboard codes, each
    optionally combined with modifiers, e.g. \c SHIFT, \c CTRL, \c
    ALT or \c META. For example, \c{CTRL + Key_P}
    might be a sequence used as a shortcut for printing a document.
    The key codes are listed in \c{qnamespace.h}. As an alternative,
    use the unicode code point of the character.
    For example, \c{'A'} gives the same key sequence
    as \c Key_A.

    Key sequences can be constructed either from an integer key code,
    or from a human readable translatable string such as
    "Ctrl+X,Alt+Space". A key sequence can be cast to a QString to
    obtain a human readable translated version of the sequence.
    Translations are done in the "QAccel" context.

    \sa QAccel
*/

/*!
    \enum Qt::SequenceMatch

    \value NoMatch Sequences have nothing in common
    \value PartialMatch Sequences match partially, but are not complete
    \value Identical Sequences do not differ
*/

static struct {
    int key;
    const char* name;
} keyname[] = {
    { Qt::Key_Space,	    QT_TRANSLATE_NOOP("QAccel", "Space") },
    { Qt::Key_Escape,	    QT_TRANSLATE_NOOP("QAccel", "Esc") },
    { Qt::Key_Tab,	    QT_TRANSLATE_NOOP("QAccel", "Tab") },
    { Qt::Key_Backtab,	    QT_TRANSLATE_NOOP("QAccel", "Backtab") },
    { Qt::Key_Backspace,    QT_TRANSLATE_NOOP("QAccel", "Backspace") },
    { Qt::Key_Return,	    QT_TRANSLATE_NOOP("QAccel", "Return") },
    { Qt::Key_Enter,	    QT_TRANSLATE_NOOP("QAccel", "Enter") },
    { Qt::Key_Insert,	    QT_TRANSLATE_NOOP("QAccel", "Ins") },
    { Qt::Key_Delete,	    QT_TRANSLATE_NOOP("QAccel", "Del") },
    { Qt::Key_Pause,	    QT_TRANSLATE_NOOP("QAccel", "Pause") },
    { Qt::Key_Print,	    QT_TRANSLATE_NOOP("QAccel", "Print") },
    { Qt::Key_SysReq,	    QT_TRANSLATE_NOOP("QAccel", "SysReq") },
    { Qt::Key_Home,	    QT_TRANSLATE_NOOP("QAccel", "Home") },
    { Qt::Key_End,	    QT_TRANSLATE_NOOP("QAccel", "End") },
    { Qt::Key_Left,	    QT_TRANSLATE_NOOP("QAccel", "Left") },
    { Qt::Key_Up,	    QT_TRANSLATE_NOOP("QAccel", "Up") },
    { Qt::Key_Right,	    QT_TRANSLATE_NOOP("QAccel", "Right") },
    { Qt::Key_Down,	    QT_TRANSLATE_NOOP("QAccel", "Down") },
    { Qt::Key_Prior,	    QT_TRANSLATE_NOOP("QAccel", "PgUp") },
    { Qt::Key_Next,	    QT_TRANSLATE_NOOP("QAccel", "PgDown") },
    { Qt::Key_CapsLock,	    QT_TRANSLATE_NOOP("QAccel", "CapsLock") },
    { Qt::Key_NumLock,	    QT_TRANSLATE_NOOP("QAccel", "NumLock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QAccel", "ScrollLock") },
    { Qt::Key_Menu,	    QT_TRANSLATE_NOOP("QAccel", "Menu") },
    { Qt::Key_Help,	    QT_TRANSLATE_NOOP("QAccel", "Help") },

    // Multimedia keys
    { Qt::Key_Back,	    QT_TRANSLATE_NOOP("QAccel", "Back") },
    { Qt::Key_Forward,	    QT_TRANSLATE_NOOP("QAccel", "Forward") },
    { Qt::Key_Stop,	    QT_TRANSLATE_NOOP("QAccel", "Stop") },
    { Qt::Key_Refresh,	    QT_TRANSLATE_NOOP("QAccel", "Refresh") },
    { Qt::Key_VolumeDown,   QT_TRANSLATE_NOOP("QAccel", "Volume Down") },
    { Qt::Key_VolumeMute,   QT_TRANSLATE_NOOP("QAccel", "Volume Mute") },
    { Qt::Key_VolumeUp,	    QT_TRANSLATE_NOOP("QAccel", "Volume Up") },
    { Qt::Key_BassBoost,    QT_TRANSLATE_NOOP("QAccel", "Bass Boost") },
    { Qt::Key_BassUp,	    QT_TRANSLATE_NOOP("QAccel", "Bass Up") },
    { Qt::Key_BassDown,	    QT_TRANSLATE_NOOP("QAccel", "Bass Down") },
    { Qt::Key_TrebleUp,	    QT_TRANSLATE_NOOP("QAccel", "Treble Up") },
    { Qt::Key_TrebleDown,   QT_TRANSLATE_NOOP("QAccel", "Treble Down") },
    { Qt::Key_MediaPlay,    QT_TRANSLATE_NOOP("QAccel", "Media Play") },
    { Qt::Key_MediaStop,    QT_TRANSLATE_NOOP("QAccel", "Media Stop") },
    { Qt::Key_MediaPrev,    QT_TRANSLATE_NOOP("QAccel", "Media Previous") },
    { Qt::Key_MediaNext,    QT_TRANSLATE_NOOP("QAccel", "Media Next") },
    { Qt::Key_MediaRecord,  QT_TRANSLATE_NOOP("QAccel", "Media Record") },
    { Qt::Key_HomePage,	    QT_TRANSLATE_NOOP("QAccel", "Home") },
    { Qt::Key_Favorites,    QT_TRANSLATE_NOOP("QAccel", "Favorites") },
    { Qt::Key_Search,	    QT_TRANSLATE_NOOP("QAccel", "Search") },
    { Qt::Key_Standby,	    QT_TRANSLATE_NOOP("QAccel", "Standby") },
    { Qt::Key_OpenUrl,	    QT_TRANSLATE_NOOP("QAccel", "Open URL") },
    { Qt::Key_LaunchMail,   QT_TRANSLATE_NOOP("QAccel", "Launch Mail") },
    { Qt::Key_LaunchMedia,  QT_TRANSLATE_NOOP("QAccel", "Launch Media") },
    { Qt::Key_Launch0,	    QT_TRANSLATE_NOOP("QAccel", "Launch (0)") },
    { Qt::Key_Launch1,	    QT_TRANSLATE_NOOP("QAccel", "Launch (1)") },
    { Qt::Key_Launch2,	    QT_TRANSLATE_NOOP("QAccel", "Launch (2)") },
    { Qt::Key_Launch3,	    QT_TRANSLATE_NOOP("QAccel", "Launch (3)") },
    { Qt::Key_Launch4,	    QT_TRANSLATE_NOOP("QAccel", "Launch (4)") },
    { Qt::Key_Launch5,	    QT_TRANSLATE_NOOP("QAccel", "Launch (5)") },
    { Qt::Key_Launch6,	    QT_TRANSLATE_NOOP("QAccel", "Launch (6)") },
    { Qt::Key_Launch7,	    QT_TRANSLATE_NOOP("QAccel", "Launch (7)") },
    { Qt::Key_Launch8,	    QT_TRANSLATE_NOOP("QAccel", "Launch (8)") },
    { Qt::Key_Launch9,	    QT_TRANSLATE_NOOP("QAccel", "Launch (9)") },
    { Qt::Key_LaunchA,	    QT_TRANSLATE_NOOP("QAccel", "Launch (A)") },
    { Qt::Key_LaunchB,	    QT_TRANSLATE_NOOP("QAccel", "Launch (B)") },
    { Qt::Key_LaunchC,	    QT_TRANSLATE_NOOP("QAccel", "Launch (C)") },
    { Qt::Key_LaunchD,	    QT_TRANSLATE_NOOP("QAccel", "Launch (D)") },
    { Qt::Key_LaunchE,	    QT_TRANSLATE_NOOP("QAccel", "Launch (E)") },
    { Qt::Key_LaunchF,	    QT_TRANSLATE_NOOP("QAccel", "Launch (F)") },

    // --------------------------------------------------------------
    // More consistent namings
    { Qt::Key_Print,	    QT_TRANSLATE_NOOP("QAccel", "Print Screen") },
    { Qt::Key_Prior,	    QT_TRANSLATE_NOOP("QAccel", "Page Up") },
    { Qt::Key_Next,	    QT_TRANSLATE_NOOP("QAccel", "Page Down") },
    { Qt::Key_CapsLock,	    QT_TRANSLATE_NOOP("QAccel", "Caps Lock") },
    { Qt::Key_NumLock,	    QT_TRANSLATE_NOOP("QAccel", "Num Lock") },
    { Qt::Key_NumLock,	    QT_TRANSLATE_NOOP("QAccel", "Number Lock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QAccel", "Scroll Lock") },
    { Qt::Key_Insert,	    QT_TRANSLATE_NOOP("QAccel", "Insert") },
    { Qt::Key_Delete,	    QT_TRANSLATE_NOOP("QAccel", "Delete") },
    { Qt::Key_Escape,	    QT_TRANSLATE_NOOP("QAccel", "Escape") },
    { Qt::Key_SysReq,	    QT_TRANSLATE_NOOP("QAccel", "System Request") },

    { 0, 0 }
};


class QKeySequencePrivate : public QShared
{
public:
    inline QKeySequencePrivate()
    {
	key[0] = key[1] = key[2] = key[3] =  0;
    }
    inline QKeySequencePrivate(QKeySequencePrivate *copy)
    {
	key[0] = copy->key[0];
	key[1] = copy->key[1];
	key[2] = copy->key[2];
	key[3] = copy->key[3];
    }
    int key[4];
};


/*!
    Constructs an empty key sequence.
*/
QKeySequence::QKeySequence()
{
    d = new QKeySequencePrivate();
}

/*!
    Creates a key sequence from the string \a key. For example
    "Ctrl+O" gives CTRL+'O'. The strings "Ctrl",
    "Shift", "Alt" and "Meta" are recognized, as well as their
    translated equivalents in the "QAccel" context (using
    QObject::tr()).

    Multiple key codes (up to four) may be entered by separating them
    with commas, e.g. "Alt+X,Ctrl+S,Q".

    This contructor is typically used with \link QObject::tr() tr
    \endlink(), so that accelerator keys can be replaced in
    translations:

    \code
	QPopupMenu *file = new QPopupMenu(this);
	file->insertItem(tr("&Open..."), this, SLOT(open()),
			  QKeySequence(tr("Ctrl+O", "File|Open")));
    \endcode

    Note the \c "File|Open" translator comment. It is by no means
    necessary, but it provides some context for the human translator.
*/
QKeySequence::QKeySequence(const QString& key)
{
    d = new QKeySequencePrivate();
    assign(key);
}

/*!
    Constructs a key sequence that has a single \a key.

    The key codes are listed in \c{qnamespace.h} and can be
    combined with modifiers, e.g. with \c SHIFT, \c CTRL, \c
    ALT or \c META.
*/
QKeySequence::QKeySequence(int key)
{
    d = new QKeySequencePrivate();
    d->key[0] = key;
}

/*!
    Constructs a key sequence with up to 4 keys \a k1, \a k2,
    \a k3 and \a k4.

    The key codes are listed in \c{qnamespace.h} and can be
    combined with modifiers, e.g. with \c SHIFT, \c CTRL, \c
    ALT or \c META.
*/
QKeySequence::QKeySequence(int k1, int k2, int k3, int k4)
{
    d = new QKeySequencePrivate();
    d->key[0] = k1;
    d->key[1] = k2;
    d->key[2] = k3;
    d->key[3] = k4;
}

/*!
    Copy constructor. Makes a copy of \a keysequence.
 */
QKeySequence::QKeySequence(const QKeySequence& keysequence)
    : d(keysequence.d)
{
    d->ref();
}


/*!
    Destroys the key sequence.
 */
QKeySequence::~QKeySequence()
{
    if (d->deref())
	delete d;
}

/*!
    \internal
    KeySequences should never be modified, but rather just created.
    Internally though we do need to modify to keep pace in event
    delivery.
*/

void QKeySequence::setKey(int key, int index)
{
    if (0 > index && 4 < index) {
	qWarning("QKeySequence::setKey: index %u out of range", index);
	return;
    }

    if (1 < d->count) {
	QKeySequencePrivate *newd = new QKeySequencePrivate(d);
	d->deref();
	d = newd;
    }
    d->key[index] = key;
}

/*!
    Returns the number of keys in the key sequence.
    The maximum is 4.
 */
uint QKeySequence::count() const
{
    if (! d->key[0])
	return 0;
    if (! d->key[1])
	return 1;
    if (! d->key[2])
	return 2;
    if (! d->key[3])
	return 3;
    return 4;
}


/*!
    Returns true if the key sequence is empty; otherwise returns
    false.
*/
bool QKeySequence::isEmpty() const
{
    return !d->key[0];
}


/*!
    Adds the string \a keyseq to the key sequence. \a keyseq may
    contain up to four key codes, provided they are seperated by a
    comma, e.g. "Alt+X,Ctrl+S,Z"). Returns the number of key codes
    added.
*/
int QKeySequence::assign(QString keyseq)
{
    QString part;
    int n = 0;
    int p = 0, diff = 0;

    // Run through the whole string, but stop
    // if we have 4 keys before the end.
    while (keyseq.length() && n < 4) {
	// We MUST use something to seperate each sequence, and space
	// does not cut it, since some of the key names have space
	// in them.. (Let's hope no one translate with a comma in it:)
	p = keyseq.indexOf(',');
	if (-1 != p) {
	    if (',' == keyseq[p+1]) // e.g. 'Ctrl+,, Shift+,,'
		p++;
	    if (' ' == keyseq[p+1]) { // Space after comma
		diff = 1;
		p++;
	    } else {
		diff = 0;
	    }
	}
	part = keyseq.left(-1==p?keyseq.length():p-diff);
	keyseq = keyseq.right(-1==p?0:keyseq.length() - (p + 1));
	d->key[n] = decodeString(part);
	n++;
    }
    return n;
}

struct ModifKeyName {
    ModifKeyName() { }
    ModifKeyName(int q, QString n) : qt_key(q), name(n) { }
    int qt_key;
    QString name;
};

/*!
    Constructs a single key from the string \str.
 */
int QKeySequence::decodeString(const QString& str)
{
    int ret = 0;
    QString accel = str.toLower();

    // ############ FIXME: We should not need to construct a valuelist with 20 entries for every
    // key decode operation. There _is_ a way to do this more intelligent.
    static QList<ModifKeyName> modifs;
    modifs.ensure_constructed();
    if (modifs.isEmpty()) {
#ifdef QMAC_CTRL
	modifs << ModifKeyName(CTRL, QMAC_CTRL);
#endif
#ifdef QMAC_ALT
	modifs << ModifKeyName(ALT, QMAC_ALT);
#endif
#ifdef QMAC_META
	modifs << ModifKeyName(META, QMAC_META);
#endif
#ifdef QMAC_SHIFT
	modifs << ModifKeyName(SHIFT, QMAC_SHIFT);
#endif
	modifs << ModifKeyName(CTRL, "ctrl+")
	       << ModifKeyName(CTRL, QAccel::tr("Ctrl").toLower().append('+'))
	       << ModifKeyName(SHIFT, "shift+")
	       << ModifKeyName(SHIFT, QAccel::tr("Shift").toLower().append('+'))
	       << ModifKeyName(ALT, "alt+")
	       << ModifKeyName(ALT, QAccel::tr("Alt").toLower().append('+'))
	       << ModifKeyName(META, "meta+")
	       << ModifKeyName(ALT, QAccel::tr("Meta").toLower().append('+'));
    }
    QString sl = accel;
    for (int i = 0; i < modifs.size(); ++i) {
	const ModifKeyName &mkf = modifs.at(i);
	if (sl.contains(mkf.name)) {
	    ret |= mkf.qt_key;
	    accel.remove(mkf.name);
	    sl = accel;
	}
    }

    int p = accel.lastIndexOf('+', str.length() - 2); // -2 so that Ctrl++ works
    if(p > 0)
	accel = accel.mid(p + 1);

    int fnum = 0;
    if (accel.length() == 1) {
	ret |= accel[0].toUpper().unicode();
    } else if (accel[0] == 'f' && (fnum = accel.mid(1).toInt())) {
        ret |= Key_F1 + fnum - 1;
    } else {
	// Check through translation table for the correct key name
	// ...or fall back on english table.
	bool found = false;
	for (int tran = 0; tran < 2; tran++) {
	    for (int i = 0; keyname[i].name; i++) {
		if (tran ? accel == QAccel::tr(keyname[i].name)
			  : accel == keyname[i].name) {
		    ret |= keyname[i].key;
		    found = true;
		    break;
		}
	    }
	    if(found)
		break;
	}
    }
    return ret;
}


/*!
    Creates an accelerator string for \a key. For example,
    CTRL+Key_O gives "Ctrl+O". The strings, "Ctrl", "Shift", etc. are
    translated (using QObject::tr()) in the "QAccel" context.
 */
QString QKeySequence::encodeString(int key)
{
    QString s;
#if defined(Q_OS_MAC)
    // On MAC the order is Meta, Alt, Shift, Control.
    if ((key & META) == META)
	s += QMAC_META;
    if ((key & ALT) == ALT)
	s += QMAC_ALT;
    if ((key & SHIFT) == SHIFT)
	s += QMAC_SHIFT;
    if ((key & CTRL) == CTRL)
	s += QMAC_CTRL;
#else
    // On other systems the order is Meta, Control, Alt, Shift
    if ((key & META) == META)
	s += QAccel::tr("Meta");
    if ((key & CTRL) == CTRL ) {
	if (!s.isEmpty())
	    s += QAccel::tr("+");
	s += QAccel::tr("Ctrl");
    }
    if ((key & ALT) == ALT) {
	if (!s.isEmpty())
	    s += QAccel::tr("+");
	s += QAccel::tr("Alt");
    }
    if ((key & SHIFT) == SHIFT) {
	if (!s.isEmpty())
	    s += QAccel::tr("+");
	s += QAccel::tr("Shift");
    }
#endif


    key &= ~(SHIFT | CTRL | ALT | META);
    QString p;

    if (key && key < Key_Escape) {
	if (key < 0x10000) {
	    p = QChar(key & 0xffff).toUpper();
	} else {
	    p = QChar((key-0x10000)/0x400+0xd800);
	    p += QChar((key-0x10000)%400+0xdc00);
	}
    } else if (key >= Key_F1 && key <= Key_F35) {
	p = QAccel::tr("F%1").arg(key - Key_F1 + 1);
    } else if (key > Key_Space && key <= Key_AsciiTilde) {
	p.sprintf("%c", key);
    } else if (key) {
	int i=0;
	while (keyname[i].name) {
	    if (key == keyname[i].key) {
		p = QAccel::tr(keyname[i].name);
		break;
	    }
	    ++i;
	}
	// If we can't find the actual translatable keyname,
	// fall back on the unicode representation of it...
	// Or else characters like Key_aring may not get displayed
	// (Really depends on you locale)
	if (!keyname[i].name) {
	    if (key < 0x10000) {
		p = QChar(key & 0xffff).toUpper();
	    } else {
		p = QChar((key-0x10000)/0x400+0xd800);
		p += QChar((key-0x10000)%400+0xdc00);
	    }
	}
    }

#ifndef Q_OS_MAC
    if (!s.isEmpty())
	s += QAccel::tr("+");
#endif

    s += p;
    return s;
}

/*!
    Matches the sequence with \a seq. Returns \c Qt::Identical if
    successful, \c Qt::PartialMatch for matching but incomplete \a seq,
    and \c Qt::NoMatch if the sequences have nothing in common.
    Returns \c Qt::NoMatch if \a seq is shorter.
*/
Qt::SequenceMatch QKeySequence::matches(const QKeySequence& seq) const
{
    uint userN = count(),
	  seqN = seq.count();

    if (userN > seqN)
	return NoMatch;

    // If equal in length, we have a potential Identical sequence,
    // else we already know it can only be partial.
    SequenceMatch match = (userN == seqN ? Identical : PartialMatch);

    for (uint i = 0; i < userN; i++) {
	int userKey      = (*this)[i],
	    sequenceKey  = seq[i];

	if (userKey != sequenceKey)
	    return NoMatch;
    }
    return match;
}


/*!
    Creates an accelerator string for the key sequence.
    For instance CTRL+Key_O gives "Ctrl+O". If the key sequence has
    multiple key codes they are returned comma-separated, e.g.
    "Alt+X, Ctrl+Y, Z". The strings, "Ctrl", "Shift", etc. are
    translated (using QObject::tr()) in the "QAccel" scope. If the key
    sequence has no keys, QString::null is returned.

    On Mac OS X, the string returned resembles the sequence that is shown in
    the menubar.
*/
QKeySequence::operator QString() const
{
    int end = count();
    if (!end) return QString::null;

    QString complete;
    int i = 0;
    while (i < end) {
	complete += encodeString(d->key[i]);
	i++;
	if (i != end)
	    complete += ", ";
    }
    return complete;
}


/*!
    \obsolete
    For backward compatibility: returns the first keycode
    as integer. If the key sequence is empty, 0 is returned.
 */
QKeySequence::operator int () const
{
    if (1 <= count())
	return d->key[0];
    return 0;
}


/*!
    Returns a reference to the element at position \a index in the key
    sequence. This can only be used to read an element.
 */
int QKeySequence::operator[](uint index) const
{
    if (index > 4) {
	qWarning("QKeySequence::operator[]: index %u out of range", index);
	return 0;
    }
    return d->key[index];
}


/*!
    Assignment operator. Assigns \a keysequence to this
    object.
 */
QKeySequence &QKeySequence::operator=(const QKeySequence & keysequence)
{
    keysequence.d->ref();
    if (d->deref())
	delete d;
    d = keysequence.d;
    return *this;
}


/*!
    Returns true if \a keysequence is equal to this key
    sequence; otherwise returns false.
 */


bool QKeySequence::operator==(const QKeySequence& keysequence) const
{
    return (d->key[0] == keysequence.d->key[0] &&
	    d->key[1] == keysequence.d->key[1] &&
	    d->key[2] == keysequence.d->key[2] &&
	    d->key[3] == keysequence.d->key[3]);
}


/*!
    Returns true if \a keysequence is not equal to this key sequence;
    otherwise returns false.
*/
bool QKeySequence::operator!= (const QKeySequence& keysequence) const
{
    QKeySequence *that = (QKeySequence*)this;
    return !((*that) == keysequence);
}


/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
/*!
    \relates QKeySequence

    Writes the key sequence \a keysequence to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &s, const QKeySequence &keysequence)
{
    QList<int> list;
    list += keysequence.d->key[0];
    list += keysequence.d->key[1];
    list += keysequence.d->key[2];
    list += keysequence.d->key[3];
    s << list;

    return s;
}


/*!
    \relates QKeySequence

    Reads a key sequence from the stream \a s into the key sequence \a
    keysequence.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &s, QKeySequence &keysequence)
{
    QList<int> list;
    s >> list;

    if (1 != list.count() && 4 != list.count()) {
	qWarning("Invalid QKeySequence data in the datastream.");
	return s;
    }

    if (1 == list.size()) {
	keysequence.d->key[0] = list.at(0);
	keysequence.d->key[1] =
	    keysequence.d->key[2] =
	    keysequence.d->key[3] = 0;
    } else {
	keysequence.d->key[0] = list.at(0);
	keysequence.d->key[1] = list.at(1);
	keysequence.d->key[2] = list.at(2);
	keysequence.d->key[3] = list.at(3);
    }
    return s;
}

#endif //QT_NO_DATASTREAM

#endif //QT_NO_ACCEL
