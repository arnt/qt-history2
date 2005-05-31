/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkeysequence.h"

#include "qshortcut.h"
#include "qdebug.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#ifndef QT_NO_DATASTREAM
# include "qdatastream.h"
#endif
#include "qvariant.h"

#ifdef Q_WS_MAC
# include <private/qt_mac_p.h>
# define QMAC_CTRL QChar(kCommandUnicode)
# define QMAC_META QChar(kControlUnicode)
# define QMAC_ALT  QChar(kOptionUnicode)
# define QMAC_SHIFT QChar(kShiftUnicode)
#endif

#ifdef Q_WS_MAC
static bool qt_sequence_no_mnemonics = true;
#else
static bool qt_sequence_no_mnemonics = false;
#endif
void Q_GUI_EXPORT qt_set_sequence_auto_mnemonic(bool b) { qt_sequence_no_mnemonics = !b; }

/*!
    \class QKeySequence
    \brief The QKeySequence class encapsulates a key sequence as used
    by shortcuts.

    \ingroup misc
    \mainclass

    A key sequence consists of up to four keyboard codes, each
    optionally combined with modifiers, such as \c Qt::SHIFT, \c Qt::CTRL, \c
    Qt::ALT or \c Qt::META. For example, \c{Qt::CTRL + Qt::Key_P}
    might be a sequence used as a shortcut for printing a document.
    Valid codes for keys and modifiers are listed in Qt::Key and
    Qt::Modifier. As an alternative, use the unicode code point of the
    character; for example, \c{'A'} gives the same key sequence
    as \c Qt::Key_A.

    Key sequences can be constructed either from an integer key code,
    or from a human readable translatable string such as
    "Ctrl+X,Alt+Space". A key sequence can be cast to a QString to
    obtain a human readable translated version of the sequence.
    Translations are done in the "QShortcut" context.

    In code that does not inherit the Qt namespace class, you must
    include the namespace when writing keycodes; for example,
    instead of ALT+Key_Q you would write Qt::ALT+Qt::Key_Q.

    \sa QShortcut
*/

/*!
    \enum QKeySequence::SequenceMatch

    \value NoMatch The key sequences are different; not even partially
    matching.
    \value PartialMatch The key sequences match partially, but are not
    the same.
    \value ExactMatch The key sequences are the same.
    \omitvalue Identical
*/

static struct {
    int key;
    const char* name;
} keyname[] = {
    { Qt::Key_Space,        QT_TRANSLATE_NOOP("QShortcut", "Space") },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Esc") },
    { Qt::Key_Tab,          QT_TRANSLATE_NOOP("QShortcut", "Tab") },
    { Qt::Key_Backtab,      QT_TRANSLATE_NOOP("QShortcut", "Backtab") },
    { Qt::Key_Backspace,    QT_TRANSLATE_NOOP("QShortcut", "Backspace") },
    { Qt::Key_Return,       QT_TRANSLATE_NOOP("QShortcut", "Return") },
    { Qt::Key_Enter,        QT_TRANSLATE_NOOP("QShortcut", "Enter") },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Ins") },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Del") },
    { Qt::Key_Pause,        QT_TRANSLATE_NOOP("QShortcut", "Pause") },
    { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print") },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "SysReq") },
    { Qt::Key_Home,         QT_TRANSLATE_NOOP("QShortcut", "Home") },
    { Qt::Key_End,          QT_TRANSLATE_NOOP("QShortcut", "End") },
    { Qt::Key_Left,         QT_TRANSLATE_NOOP("QShortcut", "Left") },
    { Qt::Key_Up,           QT_TRANSLATE_NOOP("QShortcut", "Up") },
    { Qt::Key_Right,        QT_TRANSLATE_NOOP("QShortcut", "Right") },
    { Qt::Key_Down,         QT_TRANSLATE_NOOP("QShortcut", "Down") },
    { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "PgUp") },
    { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "PgDown") },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "CapsLock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "NumLock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "ScrollLock") },
    { Qt::Key_Menu,         QT_TRANSLATE_NOOP("QShortcut", "Menu") },
    { Qt::Key_Help,         QT_TRANSLATE_NOOP("QShortcut", "Help") },

    // Multimedia keys
    { Qt::Key_Back,         QT_TRANSLATE_NOOP("QShortcut", "Back") },
    { Qt::Key_Forward,      QT_TRANSLATE_NOOP("QShortcut", "Forward") },
    { Qt::Key_Stop,         QT_TRANSLATE_NOOP("QShortcut", "Stop") },
    { Qt::Key_Refresh,      QT_TRANSLATE_NOOP("QShortcut", "Refresh") },
    { Qt::Key_VolumeDown,   QT_TRANSLATE_NOOP("QShortcut", "Volume Down") },
    { Qt::Key_VolumeMute,   QT_TRANSLATE_NOOP("QShortcut", "Volume Mute") },
    { Qt::Key_VolumeUp,     QT_TRANSLATE_NOOP("QShortcut", "Volume Up") },
    { Qt::Key_BassBoost,    QT_TRANSLATE_NOOP("QShortcut", "Bass Boost") },
    { Qt::Key_BassUp,       QT_TRANSLATE_NOOP("QShortcut", "Bass Up") },
    { Qt::Key_BassDown,     QT_TRANSLATE_NOOP("QShortcut", "Bass Down") },
    { Qt::Key_TrebleUp,     QT_TRANSLATE_NOOP("QShortcut", "Treble Up") },
    { Qt::Key_TrebleDown,   QT_TRANSLATE_NOOP("QShortcut", "Treble Down") },
    { Qt::Key_MediaPlay,    QT_TRANSLATE_NOOP("QShortcut", "Media Play") },
    { Qt::Key_MediaStop,    QT_TRANSLATE_NOOP("QShortcut", "Media Stop") },
    { Qt::Key_MediaPrevious,QT_TRANSLATE_NOOP("QShortcut", "Media Previous") },
    { Qt::Key_MediaNext,    QT_TRANSLATE_NOOP("QShortcut", "Media Next") },
    { Qt::Key_MediaRecord,  QT_TRANSLATE_NOOP("QShortcut", "Media Record") },
    { Qt::Key_HomePage,     QT_TRANSLATE_NOOP("QShortcut", "Home") },
    { Qt::Key_Favorites,    QT_TRANSLATE_NOOP("QShortcut", "Favorites") },
    { Qt::Key_Search,       QT_TRANSLATE_NOOP("QShortcut", "Search") },
    { Qt::Key_Standby,      QT_TRANSLATE_NOOP("QShortcut", "Standby") },
    { Qt::Key_OpenUrl,      QT_TRANSLATE_NOOP("QShortcut", "Open URL") },
    { Qt::Key_LaunchMail,   QT_TRANSLATE_NOOP("QShortcut", "Launch Mail") },
    { Qt::Key_LaunchMedia,  QT_TRANSLATE_NOOP("QShortcut", "Launch Media") },
    { Qt::Key_Launch0,      QT_TRANSLATE_NOOP("QShortcut", "Launch (0)") },
    { Qt::Key_Launch1,      QT_TRANSLATE_NOOP("QShortcut", "Launch (1)") },
    { Qt::Key_Launch2,      QT_TRANSLATE_NOOP("QShortcut", "Launch (2)") },
    { Qt::Key_Launch3,      QT_TRANSLATE_NOOP("QShortcut", "Launch (3)") },
    { Qt::Key_Launch4,      QT_TRANSLATE_NOOP("QShortcut", "Launch (4)") },
    { Qt::Key_Launch5,      QT_TRANSLATE_NOOP("QShortcut", "Launch (5)") },
    { Qt::Key_Launch6,      QT_TRANSLATE_NOOP("QShortcut", "Launch (6)") },
    { Qt::Key_Launch7,      QT_TRANSLATE_NOOP("QShortcut", "Launch (7)") },
    { Qt::Key_Launch8,      QT_TRANSLATE_NOOP("QShortcut", "Launch (8)") },
    { Qt::Key_Launch9,      QT_TRANSLATE_NOOP("QShortcut", "Launch (9)") },
    { Qt::Key_LaunchA,      QT_TRANSLATE_NOOP("QShortcut", "Launch (A)") },
    { Qt::Key_LaunchB,      QT_TRANSLATE_NOOP("QShortcut", "Launch (B)") },
    { Qt::Key_LaunchC,      QT_TRANSLATE_NOOP("QShortcut", "Launch (C)") },
    { Qt::Key_LaunchD,      QT_TRANSLATE_NOOP("QShortcut", "Launch (D)") },
    { Qt::Key_LaunchE,      QT_TRANSLATE_NOOP("QShortcut", "Launch (E)") },
    { Qt::Key_LaunchF,      QT_TRANSLATE_NOOP("QShortcut", "Launch (F)") },

    // --------------------------------------------------------------
    // More consistent namings
    { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print Screen") },
    { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "Page Up") },
    { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "Page Down") },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "Caps Lock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Num Lock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Number Lock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "Scroll Lock") },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Insert") },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Delete") },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Escape") },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "System Request") },

    { 0, 0 }
};


class QKeySequencePrivate
{
public:
    inline QKeySequencePrivate()
    {
        ref = 1;
        key[0] = key[1] = key[2] = key[3] =  0;
    }
    inline QKeySequencePrivate(const QKeySequencePrivate &copy)
    {
        ref = 1;
        key[0] = copy.key[0];
        key[1] = copy.key[1];
        key[2] = copy.key[2];
        key[3] = copy.key[3];
    }
    QAtomic ref;
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
    Creates a key sequence from the \a key string. For example
    "Ctrl+O" gives CTRL+'O'. The strings "Ctrl",
    "Shift", "Alt" and "Meta" are recognized, as well as their
    translated equivalents in the "QShortcut" context (using
    QObject::tr()).

    Up to four key codes may be entered by separating them with
    commas, e.g. "Alt+X,Ctrl+S,Q".

    This contructor is typically used with \link QObject::tr() tr
    \endlink(), so that shortcut keys can be replaced in
    translations:

    \code
        QMenu *file = new QMenu(this);
        file->addAction(tr("&Open..."), this, SLOT(open()),
                          QKeySequence(tr("Ctrl+O", "File|Open")));
    \endcode

    Note the \c "File|Open" translator comment. It is by no means
    necessary, but it provides some context for the human translator.
*/
QKeySequence::QKeySequence(const QString &key)
{
    d = new QKeySequencePrivate();
    assign(key);
}

/*!
    Constructs a key sequence with up to 4 keys \a k1, \a k2,
    \a k3 and \a k4.

    The key codes are listed in Qt::Key and can be combined with
    modifiers (see Qt::Modifier) such as \c Qt::SHIFT, \c Qt::CTRL, \c Qt::ALT
    or \c Qt::META.
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
    d->ref.ref();
}


/*!
    Destroys the key sequence.
 */
QKeySequence::~QKeySequence()
{
    if (!d->ref.deref())
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
    Q_ASSERT_X(index >= 0 && index < 4, "QKeySequence::setKey", "index out of range");
    qAtomicDetach(d);
    d->key[index] = key;
}

/*!
    Returns the number of keys in the key sequence.
    The maximum is 4.
 */
uint QKeySequence::count() const
{
    if (!d->key[0])
        return 0;
    if (!d->key[1])
        return 1;
    if (!d->key[2])
        return 2;
    if (!d->key[3])
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
    Returns the shortcut key sequence for the mnemonic in \a text,
    or an empty key sequence if no mnemonics are found.

    For example, mnemonic("E&amp;xit") returns Qt::ALT+Qt::Key_X,
    mnemonic("&amp;Quit") returns ALT+Key_Q, and mnemonic("Quit")
    returns an empty QKeySequence.

    We provide a \link accelerators.html list of common mnemonics
    \endlink in English. At the time of writing, Microsoft and Open
    Group do not appear to have issued equivalent recommendations for
    other languages.
*/
QKeySequence QKeySequence::mnemonic(const QString &text)
{
    if(qt_sequence_no_mnemonics)
	return QKeySequence();

    int p = 0;
    while (p >= 0) {
        p = text.indexOf('&', p) + 1;
        if (p <= 0 || p >= (int)text.length())
            break;
        if (text.at(p) != '&') {
            QChar c = text.at(p);
            if (c.isPrint()) {
                c = c.toUpper();
                return QKeySequence(c.unicode() + Qt::ALT);
            }
        }
        p++;
    }
    return QKeySequence();
}

/*!
    \fn int QKeySequence::assign(const QString &keys)

    Adds the given \a keys to the key sequence. \a keys may
    contain up to four key codes, provided they are separated by a
    comma; for example, "Alt+X,Ctrl+S,Z". The return value is the
    number of key codes added.
*/
int QKeySequence::assign(const QString &ks)
{
    QString keyseq = ks;
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
        part = keyseq.left(-1 == p ? keyseq.length() : p - diff);
        keyseq = keyseq.right(-1 == p ? 0 : keyseq.length() - (p + 1));
        d->key[n] = decodeString(part);
        ++n;
    }
    return n;
}

struct ModifKeyName {
    ModifKeyName() { }
    ModifKeyName(int q, QChar n) : qt_key(q), name(n) { }
    ModifKeyName(int q, const QString &n) : qt_key(q), name(n) { }
    int qt_key;
    QString name;
};

Q_GLOBAL_STATIC(QList<ModifKeyName>, globalModifs)

/*!
  Constructs a single key from the string \a str.
*/
int QKeySequence::decodeString(const QString &str)
{
    int ret = 0;
    QString accel = str.toLower();

    QList<ModifKeyName> *gmodifs = globalModifs();
    if (!gmodifs) return ret;

    if (gmodifs->isEmpty()) {
#ifdef QMAC_CTRL
        *gmodifs << ModifKeyName(Qt::CTRL, QMAC_CTRL);
#endif
#ifdef QMAC_ALT
        *gmodifs << ModifKeyName(Qt::ALT, QMAC_ALT);
#endif
#ifdef QMAC_META
        *gmodifs << ModifKeyName(Qt::META, QMAC_META);
#endif
#ifdef QMAC_SHIFT
        *gmodifs << ModifKeyName(Qt::SHIFT, QMAC_SHIFT);
#endif
        *gmodifs << ModifKeyName(Qt::CTRL, "ctrl+")
                 << ModifKeyName(Qt::SHIFT, "shift+")
                 << ModifKeyName(Qt::ALT, "alt+")
                 << ModifKeyName(Qt::META, "meta+");
    }

    QList<ModifKeyName> modifs = *gmodifs;
    modifs << ModifKeyName(Qt::CTRL, QShortcut::tr("Ctrl").toLower().append(QLatin1Char('+')))
           << ModifKeyName(Qt::SHIFT, QShortcut::tr("Shift").toLower().append(QLatin1Char('+')))
           << ModifKeyName(Qt::ALT, QShortcut::tr("Alt").toLower().append(QLatin1Char('+')))
           << ModifKeyName(Qt::ALT, QShortcut::tr("Meta").toLower().append(QLatin1Char('+')));



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
        ret |= Qt::Key_F1 + fnum - 1;
    } else {
        // Check through translation table for the correct key name
        // ...or fall back on english table.
        bool found = false;
        for (int tran = 0; tran < 2; ++tran) {
            for (int i = 0; keyname[i].name; ++i) {
                QString keyName(tran
                                ? QShortcut::tr(keyname[i].name)
                                : QString::fromLatin1(keyname[i].name));
                if (accel == keyName.toLower()) {
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
    Creates a shortcut string for \a key. For example,
    Qt::CTRL+Qt::Key_O gives "Ctrl+O". The strings, "Ctrl", "Shift", etc. are
    translated (using QObject::tr()) in the "QShortcut" context.
 */
QString QKeySequence::encodeString(int key)
{
    QString s;
#if defined(Q_OS_MAC) && !defined(QWS)
    // On MAC the order is Meta, Alt, Shift, Control.
    if ((key & Qt::META) == Qt::META)
        s += QMAC_META;
    if ((key & Qt::ALT) == Qt::ALT)
        s += QMAC_ALT;
    if ((key & Qt::SHIFT) == Qt::SHIFT)
        s += QMAC_SHIFT;
    if ((key & Qt::CTRL) == Qt::CTRL)
        s += QMAC_CTRL;
#else
    // On other systems the order is Meta, Control, Alt, Shift
    if ((key & Qt::META) == Qt::META)
        s += QShortcut::tr("Meta");
    if ((key & Qt::CTRL) == Qt::CTRL) {
        if (!s.isEmpty())
            s += QShortcut::tr("+");
        s += QShortcut::tr("Ctrl");
    }
    if ((key & Qt::ALT) == Qt::ALT) {
        if (!s.isEmpty())
            s += QShortcut::tr("+");
        s += QShortcut::tr("Alt");
    }
    if ((key & Qt::SHIFT) == Qt::SHIFT) {
        if (!s.isEmpty())
            s += QShortcut::tr("+");
        s += QShortcut::tr("Shift");
    }
#endif


    key &= ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
    QString p;

    if (key && key < Qt::Key_Escape) {
        if (key < 0x10000) {
            p = QChar(key & 0xffff).toUpper();
        } else {
            p = QChar((key-0x10000)/0x400+0xd800);
            p += QChar((key-0x10000)%400+0xdc00);
        }
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
        p = QShortcut::tr("F%1").arg(key - Qt::Key_F1 + 1);
    } else if (key > Qt::Key_Space && key <= Qt::Key_AsciiTilde) {
        p.sprintf("%c", key);
    } else if (key) {
        int i=0;
        while (keyname[i].name) {
            if (key == keyname[i].key) {
                p = QShortcut::tr(keyname[i].name);
                break;
            }
            ++i;
        }
        // If we can't find the actual translatable keyname,
        // fall back on the unicode representation of it...
        // Or else characters like Qt::Key_aring may not get displayed
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
        s += QShortcut::tr("+");
#endif

    s += p;
    return s;
}

/*!
    Matches the sequence with \a seq. Returns \c ExactMatch if
    successful, \c PartialMatch if \a seq matches incompletely,
    and \c NoMatch if the sequences have nothing in common.
    Returns \c NoMatch if \a seq is shorter.
*/
QKeySequence::SequenceMatch QKeySequence::matches(const QKeySequence &seq) const
{
    uint userN = count(),
          seqN = seq.count();

    if (userN > seqN)
        return NoMatch;

    // If equal in length, we have a potential ExactMatch sequence,
    // else we already know it can only be partial.
    SequenceMatch match = (userN == seqN ? ExactMatch : PartialMatch);

    for (uint i = 0; i < userN; ++i) {
        int userKey = (*this)[i],
            sequenceKey = seq[i];
        if (userKey != sequenceKey)
            return NoMatch;
    }
    return match;
}


/*!
    Creates a shortcut string for the key sequence.

    For example, the value Qt::CTRL+Qt::Key_O results in "Ctrl+O".
    If the key sequence has multiple key codes, each is separated
    by commas in the string returned, such as "Alt+X, Ctrl+Y, Z".
    The strings, "Ctrl", "Shift", etc. are translated using
    QObject::tr() in the "QShortcut" scope.

    If the key sequence has no keys, an empty string is returned.

    On Mac OS X, the string returned resembles the sequence that is
    shown in the menubar.
*/
QKeySequence::operator QString() const
{
    QString complete;
    int end = count();
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
   Returns the key sequence as a QVariant
*/
QKeySequence::operator QVariant() const
{
    return QVariant(QVariant::KeySequence, this);
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
    Q_ASSERT_X(index < 4, "QKeySequence::operator[]", "index out of range");
    return d->key[index];
}


/*!
    Assignment operator. Assigns the \a other key sequence to this
    object.
 */
QKeySequence &QKeySequence::operator=(const QKeySequence &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    \fn bool QKeySequence::operator!=(const QKeySequence &other) const

    Returns true if this key sequence is not equal to the \a other
    key sequence; otherwise returns false.
*/


/*!
    Returns true if this key sequence is equal to the \a other
    key sequence; otherwise returns false.
 */
bool QKeySequence::operator==(const QKeySequence &other) const
{
    return (d->key[0] == other.d->key[0] &&
            d->key[1] == other.d->key[1] &&
            d->key[2] == other.d->key[2] &&
            d->key[3] == other.d->key[3]);
}


/*!
    Provides an arbitrary comparison of this key sequence and
    \a other key sequence. All that is guaranteed is that the
    operator returns false if both key sequences are equal and
    that (ks1 \< ks2) == !( ks2 \< ks1) if the key sequences
    are not equal.

    This function is useful in some circumstances, for example
    if you want to use QKeySequence objects as keys in a QMap.

    \sa operator==() operator!=() operator>() operator<=() operator>=()
*/
bool QKeySequence::operator< (const QKeySequence &other) const
{
    for (int i = 0; i < 4; ++i)
        if (d->key[i] != other.d->key[i])
            return d->key[i] < other.d->key[i];
    return false;
}

/*!
    \fn bool QKeySequence::operator> (const QKeySequence &other) const

    Returns true if this key sequence is larger than the \a other key
    sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator<=() operator>=()
*/

/*!
    \fn bool QKeySequence::operator<= (const QKeySequence &other) const

    Returns true if this key sequence is smaller or equal to the
    \a other key sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator>() operator>=()
*/

/*!
    \fn bool QKeySequence::operator>= (const QKeySequence &other) const

    Returns true if this key sequence is larger or equal to the
    \a other key sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator>() operator<=()
*/

/*!
    \internal
*/
bool QKeySequence::isDetached() const
{
    return d->ref == 1;
}

/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QKeySequence &sequence)
    \relates QKeySequence

    Writes the key \a sequence to the \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &s, const QKeySequence &keysequence)
{
    QList<quint32> list;
    list << keysequence.d->key[0];

    if (s.version() >= 5 && keysequence.count() > 1) {
        list << keysequence.d->key[1];
        list << keysequence.d->key[2];
        list << keysequence.d->key[3];
    }
    s << list;
    return s;
}


/*!
    \fn QDataStream &operator>>(QDataStream &stream, QKeySequence &sequence)
    \relates QKeySequence

    Reads a key sequence from the \a stream into the key \a sequence.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &s, QKeySequence &keysequence)
{
    QList<quint32> list;
    s >> list;
    for (int i = 0; i < 4; ++i)
        keysequence.d->key[i] = list.value(i);
    return s;
}

#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QKeySequence &p)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QKeySequence(" << QString(p) << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QKeySequence to QDebug");
    return dbg;
    Q_UNUSED(p);
#endif
}
#endif
