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

#include "qplatformdefs.h"

#include "qtranslator.h"

#ifndef QT_NO_TRANSLATION

#include "qfileinfo.h"
#include "qstring.h"
#include "qcoreapplication.h"
#include "qdatastream.h"
#include "qfile.h"
#include "qmap.h"
#include "qalgorithms.h"
#include "qhash.h"
#include "qglobal.h"

#if defined(Q_OS_UNIX)
#define QT_USE_MMAP
#endif

// most of the headers below are already included in qplatformdefs.h
// also this lacks Large File support but that's probably irrelevant
#if defined(QT_USE_MMAP)
// for mmap
#include <sys/mman.h>
#include <errno.h>
#endif

#include <stdlib.h>

#include "qobject_p.h"

/*
$ mcookie
3cb86418caef9c95cd211cbf60a1bddd
$
*/

// magic number for the file
static const int MagicLength = 16;
static const uchar magic[MagicLength] = {
    0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
    0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};

static bool match(const char* found, const char* target)
{
    // 0 means anything, "" means empty
    return found == 0 || qstrcmp(found, target) == 0;
}

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

/*
  Yes, unfortunately, we have code here that depends on endianness.
  The candidate is big endian (it comes from a .qm file) whereas the
  target endianness depends on the system Qt is running on.
*/
#ifdef Q_OS_TEMP
static int __cdecl cmp_uint32_little(const void* target, const void* candidate)
#else
static int cmp_uint32_little(const void* target, const void* candidate)
#endif
{
    const uchar* t = (const uchar*) target;
    const uchar* c = (const uchar*) candidate;
    return t[3] != c[0] ? (int) t[3] - (int) c[0]
         : t[2] != c[1] ? (int) t[2] - (int) c[1]
         : t[1] != c[2] ? (int) t[1] - (int) c[2]
                   : (int) t[0] - (int) c[3];
}

#ifdef Q_OS_TEMP
static int __cdecl cmp_uint32_big(const void* target, const void* candidate)
#else
static int cmp_uint32_big(const void* target, const void* candidate)
#endif
{
    const uint* t = (const uint*) target;
    const uint* c = (const uint*) candidate;
    return (*t > *c ? 1 : (*t == *c ? 0 : -1));
}

#if defined(Q_C_CALLBACKS)
}
#endif

static uint elfHash(const char * name)
{
    const uchar *k;
    uint h = 0;
    uint g;

    if (name) {
        k = (const uchar *) name;
        while (*k) {
            h = (h << 4) + *k++;
            if ((g = (h & 0xf0000000)) != 0)
                h ^= g >> 24;
            h &= ~g;
        }
    }
    if (!h)
        h = 1;
    return h;
}

extern bool qt_detectRTLLanguage();

class QTranslatorPrivate : public QObjectPrivate
{
public:
    struct Offset {
        Offset()
            : h(0), o(0) { }
        Offset(const QTranslatorMessage& m, int offset)
            : h(m.hash()), o(offset) { }

        bool operator<(const Offset &other) const {
            return (h != other.h) ? h < other.h : o < other.o;
        }
        bool operator==(const Offset &other) const {
            return h == other.h && o == other.o;
        }
        uint h;
        uint o;
    };

    enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69 };

    QTranslatorPrivate() : unmapPointer(0), unmapLength(0) {}
    // QTranslator must finalize this before deallocating it

    // for mmap'ed files, this is what needs to be unmapped.
    char * unmapPointer;
    unsigned int unmapLength;

    // for squeezed but non-file data, this is what needs to be deleted
    QByteArray messageArray;
    QByteArray offsetArray;
    QByteArray contextArray;

#ifndef QT_NO_TRANSLATION_BUILDER
    QMap<QTranslatorMessage, void *> messages;
#endif
};


/*!
    \class QTranslator

    \brief The QTranslator class provides internationalization support for text
    output.

    \ingroup i18n
    \ingroup environment
    \mainclass

    An object of this class contains a set of QTranslatorMessage
    objects, each of which specifies a translation from a source
    language to a target language. QTranslator provides functions to
    look up translations, add new ones, remove them, load and save
    them, etc.

    The most common use of QTranslator is to: load a translator file
    created with \link linguist-manual.book Qt Linguist\endlink,
    install it using QApplication::installTranslator(), and use it via
    QObject::tr(). For example:

    \code
    int main(int argc, char ** argv)
    {
        QApplication app(argc, argv);

        QTranslator translator(0);
        translator.load("french.qm", ".");
        app.installTranslator(&translator);

        MyWidget m;
        app.setMainWidget(&m);
        m.show();

        return app.exec();
    }
    \endcode
    Note that the translator must be created \e before the
    application's main window.

    Most applications will never need to do anything else with this
    class. The other functions provided by this class are useful for
    applications that work on translator files.

    We call a translation a "messsage". For this reason, translation
    files are sometimes referred to as "message files".

    It is possible to lookup a translation using findMessage() (as
    tr() and QApplication::translate() do) and contains(), to insert a
    new translation messsage using insert(), and to remove one using
    remove().

    Translation tools often need more information than the bare source
    text and translation, for example, context information to help
    the translator. But end-user programs that are using translations
    usually only need lookup. To cater for these different needs,
    QTranslator can use stripped translator files that use the minimum
    of memory and which support little more functionality than
    findMessage().

    Thus, load() may not load enough information to make anything more
    than findMessage() work. save() has an argument indicating
    whether to save just this minimum of information or to save
    everything.

    "Everything" means that for each translation item the following
    information is kept:

    \list
    \i The \e {translated text} - the return value from tr().
    \i The input key:
        \list
        \i The \e {source text} - usually the argument to tr().
        \i The \e context - usually the class name for the tr() caller.
        \i The \e comment - a comment that helps disambiguate different uses
           of the same text in the same context.
        \endlist
    \endlist

    The minimum for each item is just the information necessary for
    findMessage() to return the right text. This may include the
    source, context and comment, but usually it is just a hash value
    and the translated text.

    For example, the "Cancel" in a dialog might have "Anuluj" when the
    program runs in Polish (in this case the source text would be
    "Cancel"). The context would (normally) be the dialog's class
    name; there would normally be no comment, and the translated text
    would be "Anuluj".

    But it's not always so simple. The Spanish version of a printer
    dialog with settings for two-sided printing and binding would
    probably require both "Activado" and "Activada" as translations
    for "Enabled". In this case the source text would be "Enabled" in
    both cases, and the context would be the dialog's class name, but
    the two items would have disambiguating comments such as
    "two-sided printing" for one and "binding" for the other. The
    comment enables the translator to choose the appropriate gender
    for the Spanish version, and enables Qt to distinguish between
    translations.

    Note that when QTranslator loads a stripped file, most functions
    do not work. The functions that do work with stripped files are
    explicitly documented as such.

    \sa QTranslatorMessage QApplication::installTranslator()
    QApplication::removeTranslator() QObject::tr() QApplication::translate()
*/

/*!
    \enum QTranslator::SaveMode

    This enum type defines how QTranslator writes translation
    files. There are two modes:

    \value Everything  files are saved with all available information
    \value Stripped  files are saved with just enough information for
        end-user applications

    Note that when QTranslator loads a stripped file, most functions do
    not work. The functions that do work with stripped files are
    explicitly documented as such.
*/

/*!
    Constructs an empty message file object with parent \a parent that
    is not connected to any file.
*/

QTranslator::QTranslator(QObject * parent)
    : QObject(*new QTranslatorPrivate, parent)
{
}

#ifdef QT_COMPAT
/*!
    \overload
    \obsolete
 */
QTranslator::QTranslator(QObject * parent, const char * name)
    : QObject(*new QTranslatorPrivate, parent)
{
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the object and frees any allocated resources.
*/

QTranslator::~QTranslator()
{
    if (QCoreApplication::instance())
        QCoreApplication::instance()->removeTranslator(this);
    clear();
}

/*!
    Loads \a filename + \a suffix (".qm" if the \a suffix is
    not specified), which may be an absolute file name or relative
    to \a directory. The previous contents of this translator object
    is discarded.

    If the file name does not exist, other file names are tried
    in the following order:

    \list 1
    \i File name without \a suffix appended.
    \i File name with text after a character in \a search_delimiters
       stripped ("_." is the default for \a search_delimiters if it is
       an empty string) and \a suffix.
    \i File name stripped without \a suffix appended.
    \i File name stripped further, etc.
    \endlist

    For example, an application running in the fr_CA locale
    (French-speaking Canada) might call load("foo.fr_ca",
    "/opt/foolib"). load() would then try to open the first existing
    readable file from this list:

    \list 1
    \i /opt/foolib/foo.fr_ca.qm
    \i /opt/foolib/foo.fr_ca
    \i /opt/foolib/foo.fr.qm
    \i /opt/foolib/foo.fr
    \i /opt/foolib/foo.qm
    \i /opt/foolib/foo
    \endlist

    \sa save()
*/

bool QTranslator::load(const QString & filename, const QString & directory,
                        const QString & search_delimiters,
                        const QString & suffix)
{
    Q_D(QTranslator);
    clear();

    QString prefix;

    if (filename[0] == QLatin1Char('/')
#ifdef Q_WS_WIN
         || (filename[0].isLetter() && filename[1] == QLatin1Char(':')) || filename[0] == QLatin1Char('\\')
#endif
        )
        prefix = QLatin1String("");
    else
        prefix = directory;

    if (prefix.length()) {
        if (prefix[int(prefix.length()-1)] != QLatin1Char('/'))
            prefix += QLatin1Char('/');
    }

    QString fname = filename;
    QString realname;
    QString delims;
    delims = search_delimiters.isNull() ? QString::fromLatin1("_.") : search_delimiters;

    for (;;) {
        QFileInfo fi;

        realname = prefix + fname + (suffix.isNull() ? QString::fromLatin1(".qm") : suffix);
        fi.setFile(realname);
        if (fi.isReadable())
            break;

        realname = prefix + fname;
        fi.setFile(realname);
        if (fi.isReadable())
            break;

        int rightmost = 0;
        for (int i = 0; i < (int)delims.length(); i++) {
            int k = fname.lastIndexOf(delims[i]);
            if (k > rightmost)
                rightmost = k;
        }

        // no truncations? fail
        if (rightmost == 0)
            return false;

        fname.truncate(rightmost);
    }

    // realname is now the fully qualified name of a readable file.

#if defined(QT_USE_MMAP)

#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

    int f;

    f = QT_OPEN(QFile::encodeName(realname), O_RDONLY,
#if defined(Q_OS_WIN)
            _S_IREAD | _S_IWRITE
#else
            0666
#endif
            );
    if (f < 0) {
        // qDebug("can't open %s: %s", realname.ascii(), strerror(errno));
        return false;
    }

    struct stat st;
    if (fstat(f, &st)) {
        // qDebug("can't stat %s: %s", realname.ascii(), strerror(errno));
        return false;
    }
    char * tmp;
    tmp = (char*)mmap(0, st.st_size, // any address, whole file
                       PROT_READ, // read-only memory
                       MAP_FILE | MAP_PRIVATE, // swap-backed map from file
                       f, 0); // from offset 0 of f
    if (!tmp || tmp == (char*)MAP_FAILED) {
        // qDebug("can't mmap %s: %s", filename.ascii(), strerror(errno));
        return false;
    }

    ::close(f);

    d->unmapPointer = tmp;
    d->unmapLength = st.st_size;
#else
    QFile f(realname);
    if (!f.exists())
        return false;
    d->unmapLength = f.size();
    d->unmapPointer = new char[d->unmapLength];
    bool ok = false;
    if (f.open(IO_ReadOnly)) {
        ok = d->unmapLength == (uint)f.read(d->unmapPointer, d->unmapLength);
        f.close();
    }
    if (!ok) {
        delete [] d->unmapPointer;
        d->unmapPointer = 0;
        return false;
    }
#endif

    return do_load((const uchar *) d->unmapPointer, d->unmapLength);
}

/*!
  \overload
  \fn bool QTranslator::load(const uchar *data, int len)

  Loads the .qm file data \a data of length \a len into the
  translator.

  The data is not copied. The caller must be able to guarantee that \a data
  will not be deleted or modified.
*/

bool QTranslator::do_load(const uchar *data, int len)
{
    Q_D(QTranslator);
    if (len < MagicLength || memcmp(data, magic, MagicLength) != 0) {
        clear();
        return false;
    }

    QByteArray array = QByteArray::fromRawData((const char *) data, len);
    QDataStream s(&array, IO_ReadOnly);
    bool ok = true;

    s.device()->seek(MagicLength);

    Q_UINT8 tag = 0;
    Q_UINT32 blockLen = 0;
    s >> tag >> blockLen;
    while (tag && blockLen) {
        if ((Q_UINT32) s.device()->at() + blockLen > (Q_UINT32) len) {
            ok = false;
            break;
        }

        if (tag == QTranslatorPrivate::Contexts) {
            d->contextArray = QByteArray(array.constData() + s.device()->at(), blockLen);
        } else if (tag == QTranslatorPrivate::Hashes) {
            d->offsetArray = QByteArray(array.constData() + s.device()->at(), blockLen);
        } else if (tag == QTranslatorPrivate::Messages) {
            d->messageArray = QByteArray(array.constData() + s.device()->at(), blockLen);
        }

        if (!s.device()->seek(s.device()->at() + blockLen)) {
            ok = false;
            break;
        }
        tag = 0;
        blockLen = 0;
        if (!s.atEnd())
            s >> tag >> blockLen;
    }

    return ok;
}

#ifndef QT_NO_TRANSLATION_BUILDER

/*!
    Saves this message file to \a filename, overwriting the previous
    contents of \a filename. If \a mode is \c Everything (the
    default), all the information is preserved. If \a mode is \c
    Stripped, any information that is not necessary for findMessage()
    is stripped away.

    \sa load()
*/

bool QTranslator::save(const QString & filename, SaveMode mode)
{
    Q_D(QTranslator);
    QFile f(filename);
    if (f.open(IO_WriteOnly)) {
        squeeze(mode);

        QDataStream s(&f);
        s.writeRawData((const char *)magic, MagicLength);
        Q_UINT8 tag;

        if (!d->offsetArray.isEmpty()) {
            tag = (Q_UINT8)QTranslatorPrivate::Hashes;
            Q_UINT32 oas = (Q_UINT32)d->offsetArray.size();
            s << tag << oas;
            s.writeRawData(d->offsetArray, oas);
        }
        if (!d->messageArray.isEmpty()) {
            tag = (Q_UINT8)QTranslatorPrivate::Messages;
            Q_UINT32 mas = (Q_UINT32)d->messageArray.size();
            s << tag << mas;
            s.writeRawData(d->messageArray, mas);
        }
        if (!d->contextArray.isEmpty()) {
            tag = (Q_UINT8)QTranslatorPrivate::Contexts;
            Q_UINT32 cas = (Q_UINT32)d->contextArray.size();
            s << tag << cas;
            s.writeRawData(d->contextArray, cas);
        }
        return true;
    }
    return false;
}

#endif

/*!
    Empties this translator of all contents.

    This function works with stripped translator files.
*/

void QTranslator::clear()
{
    Q_D(QTranslator);
    if (d->unmapPointer && d->unmapLength) {
#if defined(QT_USE_MMAP)
        munmap(d->unmapPointer, d->unmapLength);
#else
        delete [] d->unmapPointer;
#endif
        d->unmapPointer = 0;
        d->unmapLength = 0;
    }

    d->messageArray.clear();
    d->offsetArray.clear();
    d->contextArray.clear();
#ifndef QT_NO_TRANSLATION_BUILDER
    d->messages.clear();
#endif

    QEvent ev(QEvent::LanguageChange);
    QCoreApplication::sendEvent(QCoreApplication::instance(), &ev);
}

#ifndef QT_NO_TRANSLATION_BUILDER

/*!
    Converts this message file to the compact format used to store
    message files on disk.

    You should never need to call this directly; save() and other
    functions call it as necessary. \a mode is for internal use.

    \sa save() unsqueeze()
*/

void QTranslator::squeeze(SaveMode mode)
{
    Q_D(QTranslator);
    if (d->messages.isEmpty()) {
        if (mode == Stripped)
            unsqueeze();
        else
            return;
    }

    QMap<QTranslatorMessage, void *> messages = d->messages;
    clear();

    QMap<QTranslatorPrivate::Offset, void *> offsets;

    QDataStream ms(&d->messageArray, IO_WriteOnly);
    QMap<QTranslatorMessage, void *>::const_iterator it, next;
    int cpPrev = 0, cpNext = 0;
    for (it = messages.constBegin(); it != messages.constEnd(); ++it) {
        cpPrev = cpNext;
        next = it;
        ++next;
        if (next == messages.constEnd())
            cpNext = 0;
        else
            cpNext = (int) it.key().commonPrefix(next.key());
        offsets.insert(QTranslatorPrivate::Offset(it.key(), ms.device()->at()), (void *)0);
        it.key().write(ms, mode == Stripped, (QTranslatorMessage::Prefix)qMax(cpPrev, cpNext + 1));
    }

    QMap<QTranslatorPrivate::Offset, void *>::Iterator offset;
    offset = offsets.begin();
    QDataStream ds(&d->offsetArray, IO_WriteOnly);
    while (offset != offsets.end()) {
        QTranslatorPrivate::Offset k = offset.key();
        ++offset;
        ds << (Q_UINT32)k.h << (Q_UINT32)k.o;
    }

    if (mode == Stripped) {
        QMap<const char *, int> contextSet;
        for (it = messages.constBegin(); it != messages.constEnd(); ++it)
            ++contextSet[it.key().context()];

        Q_UINT16 hTableSize;
        if (contextSet.size() < 200)
            hTableSize = (contextSet.size() < 60) ? 151 : 503;
        else if (contextSet.size() < 2500)
            hTableSize = (contextSet.size() < 750) ? 1511 : 5003;
        else
            hTableSize = (contextSet.size() < 10000) ? 15013 : 3 * contextSet.size() / 2;

        QMultiMap<int, const char *> hashMap;
        QMap<const char *, int>::const_iterator c;
        for (c = contextSet.constBegin(); c != contextSet.constEnd(); ++c)
            hashMap.insert(elfHash(c.key()) % hTableSize, c.key());

        /*
          The contexts found in this translator are stored in a hash
          table to provide fast lookup. The context array has the
          following format:

              Q_UINT16 hTableSize;
              Q_UINT16 hTable[hTableSize];
              Q_UINT8  contextPool[...];

          The context pool stores the contexts as Pascal strings:

              Q_UINT8  len;
              Q_UINT8  data[len];

          Let's consider the look-up of context "FunnyDialog".  A
          hash value between 0 and hTableSize - 1 is computed, say h.
          If hTable[h] is 0, "FunnyDialog" is not covered by this
          translator. Else, we check in the contextPool at offset
          2 * hTable[h] to see if "FunnyDialog" is one of the
          contexts stored there, until we find it or we meet the
          empty string.
        */
        d->contextArray.resize(2 + (hTableSize << 1));
        QDataStream t(&d->contextArray, IO_WriteOnly);

        Q_UINT16 *hTable = new Q_UINT16[hTableSize];
        memset(hTable, 0, hTableSize * sizeof(Q_UINT16));

        t << hTableSize;
        t.device()->seek(2 + (hTableSize << 1));
        t << (Q_UINT16)0; // the entry at offset 0 cannot be used
        uint upto = 2;

        QMap<int, const char *>::const_iterator entry = hashMap.constBegin();
        while (entry != hashMap.constEnd()) {
            int i = entry.key();
            const char *con = entry.value();
            hTable[i] = (Q_UINT16)(upto >> 1);
            uint len = (uint)qstrlen(con);
            len = qMin(len, 255u);
            t << (Q_UINT8)len;
            t.writeRawData(con, len);
            upto += 1 + len;

            ++entry;
            if (entry == hashMap.constEnd() || entry.key() != i) {
                do {
                    t << (Q_UINT8) 0; // empty string
                    ++upto;
                } while ((upto & 0x1) != 0); // offsets have to be even
            }
        }
        t.device()->seek(2);
        for (int j = 0; j < hTableSize; j++)
            t << hTable[j];
        delete [] hTable;

        if (upto > 131072) {
            qWarning("QTranslator::squeeze: Too many contexts");
            d->contextArray.clear();
        }
    }
}


/*!
    Converts this message file into an easily modifiable data
    structure, less compact than the format used in the files.

    You should never need to call this function; it is called by
    insert() and friends as necessary.

    \sa squeeze()
*/

void QTranslator::unsqueeze()
{
    Q_D(QTranslator);
    if (!d->messages.isEmpty() || d->messageArray.isEmpty())
        return;

    QDataStream s(&d->messageArray, IO_ReadOnly);
    for (;;) {
        QTranslatorMessage m(s);
        if (m.hash() == 0)
            break;
        d->messages.insert(m, (void *)0);
    }
}


/*!
    Returns true if this message file contains a message with the key
    (\a context, \a sourceText, \a comment); otherwise returns false.

    This function works with stripped translator files.

    (This is is a one-liner that calls findMessage().)
*/

bool QTranslator::contains(const char* context, const char* sourceText,
                            const char* comment) const
{
    return !findMessage(context, sourceText, comment).translation().isNull();
}


/*!
    Inserts \a message into this message file.

    This function does \e not work with stripped translator files. It
    may appear to, but that is not dependable.

    \sa remove()
*/

void QTranslator::insert(const QTranslatorMessage& message)
{
    Q_D(QTranslator);
    unsqueeze();
    d->messages.remove(message); // safer
    d->messages.insert(message, (void *) 0);
}

/*!
  \fn void QTranslator::insert(const char *context, const char
 *sourceText, const QString &translation)
  \overload
  \obsolete

  Inserts the \a sourceText and \a translation into the translator
  with the given \a context.
*/

/*!
    Removes \a message from this translator.

    This function works with stripped translator files.

    \sa insert()
*/

void QTranslator::remove(const QTranslatorMessage& message)
{
    Q_D(QTranslator);
    unsqueeze();
    d->messages.remove(message);
}


/*!
  \fn void QTranslator::remove(const char *, const char *)
  \overload
  \obsolete

  Removes the translation associated to the key (\a context, \a sourceText,
  "") from this translator.
*/
#endif

/*!  Returns the QTranslatorMessage for the key
     (\a context, \a sourceText, \a comment). If none is found,
     also tries (\a context, \a sourceText, "").
*/

QTranslatorMessage QTranslator::findMessage(const char *context, const char *sourceText,
                                            const char *comment) const
{
    Q_D(const QTranslator);
    if (context == 0)
        context = "";
    if (sourceText == 0)
        sourceText = "";
    if (comment == 0)
        comment = "";

#ifndef QT_NO_TRANSLATION_BUILDER
    if (!d_func()->messages.isEmpty()) {
        QMap<QTranslatorMessage, void *>::const_iterator it;

        it = d->messages.find(QTranslatorMessage(context, sourceText, comment));
        if (it != d->messages.constEnd())
            return it.key();

        if (comment[0]) {
            it = d->messages.find(QTranslatorMessage(context, sourceText, ""));
            if (it != d->messages.constEnd())
                return it.key();
        }
        return QTranslatorMessage();
    }
#endif

    if (d->offsetArray.isEmpty())
        return QTranslatorMessage();

    /*
        Check if the context belongs to this QTranslator. If many
        translators are installed, this step is necessary.
    */
    if (!d->contextArray.isEmpty()) {
        Q_UINT16 hTableSize = 0;
        QDataStream t(d->contextArray);
        t >> hTableSize;
        uint g = elfHash(context) % hTableSize;
        t.device()->seek(2 + (g << 1));
        Q_UINT16 off;
        t >> off;
        if (off == 0)
            return QTranslatorMessage();
        t.device()->seek(2 + (hTableSize << 1) + (off << 1));

        Q_UINT8 len;
        char con[256];
        for (;;) {
            t >> len;
            if (len == 0)
                return QTranslatorMessage();
            t.readRawData(con, len);
            con[len] = '\0';
            if (qstrcmp(con, context) == 0)
                break;
        }
    }

    size_t numItems = d->offsetArray.size() / (2 * sizeof(Q_UINT32));
    if (!numItems)
        return QTranslatorMessage();

    for (;;) {
        Q_UINT32 h = elfHash(QByteArray(sourceText) + comment);

        char *r = (char *) bsearch(&h, d->offsetArray, numItems,
                                   2 * sizeof(Q_UINT32),
                                   (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? cmp_uint32_big
                                   : cmp_uint32_little);
        if (r != 0) {
            // go back on equal key
            while (r != d->offsetArray.constData() && cmp_uint32_big(r - 8, r) == 0)
                r -= 8;

            QDataStream s(d->offsetArray);
            s.device()->seek(r - d->offsetArray.constData());

            Q_UINT32 rh, ro;
            s >> rh >> ro;

            QDataStream ms(d->messageArray);
            while (rh == h) {
                ms.device()->seek(ro);
                QTranslatorMessage m(ms);
                if (match(m.context(), context)
                        && match(m.sourceText(), sourceText)
                        && match(m.comment(), comment))
                    return m;
                if (s.atEnd())
                    break;
                s >> rh >> ro;
            }
        }
        if (!comment[0])
            break;
        comment = "";
    }
    return QTranslatorMessage();
}

/*!
    Returns true if this translator is empty, otherwise returns false.
    This function works with stripped and unstripped translation files.
*/
bool QTranslator::isEmpty() const
{
    Q_D(const QTranslator);
    return !d->unmapPointer && !d->unmapLength && d->messageArray.isEmpty() &&
           d->offsetArray.isEmpty() && d->contextArray.isEmpty() && d->messages.isEmpty();
}


#ifndef QT_NO_TRANSLATION_BUILDER

/*!
    Returns a list of the messages in the translator. This function is
    rather slow. Because it is seldom called, it's optimized for
    simplicity and small size, rather than speed.

    If you want to iterate over the list, you should iterate over a
    copy, e.g.
    \code
    QList<QTranslatorMessage> list = myTranslator.messages();
    QList<QTranslatorMessage>::Iterator it = list.begin();
    while (it != list.end()) {
        process_message(*it);
        ++it;
    }
  \endcode
*/

QList<QTranslatorMessage> QTranslator::messages() const
{
    Q_D(const QTranslator);
    ((QTranslator *) this)->unsqueeze();
    return d->messages.keys();
}

#endif

/*!
    \class QTranslatorMessage

    \brief The QTranslatorMessage class contains a translator message and its
    properties.

    \ingroup i18n
    \ingroup environment

    This class is of no interest to most applications. It is useful
    for translation tools such as \link linguist-manual.book Qt
    Linguist\endlink. It is provided simply to make the API complete
    and regular.

    For a QTranslator object, a lookup key is a triple (\e context, \e
    {source text}, \e comment) that uniquely identifies a message. An
    extended key is a quadruple (\e hash, \e context, \e {source
    text}, \e comment), where \e hash is computed from the source text
    and the comment. Unless you plan to read and write messages
    yourself, you need not worry about the hash value.

    QTranslatorMessage stores this triple or quadruple and the relevant
    translation if there is any.

    \sa QTranslator
*/

/*!
    Constructs a translator message with the extended key (0, 0, 0, 0)
    and an empty string as translation.
*/

QTranslatorMessage::QTranslatorMessage()
    : h(0)
{
}


/*!
    Constructs an translator message with the extended key (\e h, \a
    context, \a sourceText, \a comment), where \e h is computed from
    \a sourceText and \a comment, and possibly with a \a translation.
*/

QTranslatorMessage::QTranslatorMessage(const char * context,
                                        const char * sourceText,
                                        const char * comment,
                                        const QString& translation)
    : cx(context), st(sourceText), cm(comment), tn(translation)
{
    // 0 means we don't know, "" means empty
    if (cx == (const char*)0)
        cx = "";
    if (st == (const char*)0)
        st = "";
    if (cm == (const char*)0)
        cm = "";
    h = elfHash(st + cm);
}


/*!
    Constructs a translator message read from the \a stream. The
    resulting message may have any combination of content.

    \sa QTranslator::save()
*/

QTranslatorMessage::QTranslatorMessage(QDataStream & stream)
    : h(0)
{
    QString str16;
    char tag;
    Q_UINT8 obs1;

    for (;;) {
        tag = 0;
        if (!stream.atEnd())
            stream.readRawData(&tag, 1);
        switch((Tag)tag) {
        case Tag_End:
            if (h == 0)
                h = elfHash(st + cm);
            return;
        case Tag_SourceText16: // obsolete
            stream >> str16;
            st = str16.latin1();
            break;
        case Tag_Translation:
            stream >> tn;
            break;
        case Tag_Context16: // obsolete
            stream >> str16;
            cx = str16.latin1();
            break;
        case Tag_Hash:
            stream >> h;
            break;
        case Tag_SourceText:
            stream >> st;
            break;
        case Tag_Context:
            stream >> cx;
            if (cx == "") // for compatibility
                cx = 0;
            break;
        case Tag_Comment:
            stream >> cm;
            break;
        case Tag_Obsolete1: // obsolete
            stream >> obs1;
            break;
        default:
            h = 0;
            st = 0;
            cx = 0;
            cm = 0;
            tn = QString::null;
            return;
        }
    }
}


/*!
    Constructs a copy of translator message \a m.
*/

QTranslatorMessage::QTranslatorMessage(const QTranslatorMessage & m)
    : cx(m.cx), st(m.st), cm(m.cm), tn(m.tn)
{
    h = m.h;
}


/*!
    Assigns message \a m to this translator message and returns a
    reference to this translator message.
*/

QTranslatorMessage & QTranslatorMessage::operator=(
        const QTranslatorMessage & m)
{
    h = m.h;
    cx = m.cx;
    st = m.st;
    cm = m.cm;
    tn = m.tn;
    return *this;
}


/*!
    \fn uint QTranslatorMessage::hash() const

    Returns the hash value used internally to represent the lookup
    key. This value is zero only if this translator message was
    constructed from a stream containing invalid data.

    The hashing function is unspecified, but it will remain unchanged
    in future versions of Qt.
*/

/*!
    \fn const char *QTranslatorMessage::context() const

    Returns the context for this message (e.g. "MyDialog").
*/

/*!
    \fn const char *QTranslatorMessage::sourceText() const

    Returns the source text of this message (e.g. "&Save").
*/

/*!
    \fn const char *QTranslatorMessage::comment() const

    Returns the comment for this message (e.g. "File|Save").
*/

/*!
    \fn void QTranslatorMessage::setTranslation(const QString & translation)

    Sets the translation of the source text to \a translation.

    \sa translation()
*/

/*!
    \fn QString QTranslatorMessage::translation() const

    Returns the translation of the source text (e.g., "&Sauvegarder").

    \sa setTranslation()
*/

/*!
    \enum QTranslatorMessage::Prefix

    Let (\e h, \e c, \e s, \e m) be the extended key. The possible
    prefixes are

    \value NoPrefix  no prefix
    \value Hash  only (\e h)
    \value HashContext  only (\e h, \e c)
    \value HashContextSourceText  only (\e h, \e c, \e s)
    \value HashContextSourceTextComment  the whole extended key, (\e
        h, \e c, \e s, \e m)

    \sa write() commonPrefix()
*/

/*!
    Writes this translator message to the \a stream. If \a strip is
    false (the default), all the information in the message is
    written. If \a strip is true, only the part of the extended key
    specified by \a prefix is written with the translation (\c
    HashContextSourceTextComment by default).

    \sa commonPrefix()
*/

void QTranslatorMessage::write(QDataStream & stream, bool strip,
                                Prefix prefix) const
{
    char tag;

    tag = (char)Tag_Translation;
    stream.writeRawData(&tag, 1);
    stream << tn;

    if (!strip)
        prefix = HashContextSourceTextComment;

    switch (prefix) {
    case HashContextSourceTextComment:
        tag = (char)Tag_Comment;
        stream.writeRawData(&tag, 1);
        stream << cm;
        // fall through
    case HashContextSourceText:
        tag = (char)Tag_SourceText;
        stream.writeRawData(&tag, 1);
        stream << st;
        // fall through
    case HashContext:
        tag = (char)Tag_Context;
        stream.writeRawData(&tag, 1);
        stream << cx;
        // fall through
    default:
        tag = (char)Tag_Hash;
        stream.writeRawData(&tag, 1);
        stream << h;
    }

    tag = (char)Tag_End;
    stream.writeRawData(&tag, 1);
}


/*!
    Returns the widest lookup prefix that is common to this translator
    message and to message \a m.

    For example, if the extended key is for this message is (71,
    "PrintDialog", "Yes", "Print?") and that for \a m is (71,
    "PrintDialog", "No", "Print?"), this function returns \c
    HashContext.

    \sa write()
*/

QTranslatorMessage::Prefix QTranslatorMessage::commonPrefix(
        const QTranslatorMessage& m) const
{
    if (h != m.h)
        return NoPrefix;
    if (cx != m.cx)
        return Hash;
    if (st != m.st)
        return HashContext;
    if (cm != m.cm)
        return HashContextSourceText;
    return HashContextSourceTextComment;
}


/*!
 Returns true if the extended key of this object is equal to that of
 \a m; otherwise returns false.
*/

bool QTranslatorMessage::operator==(const QTranslatorMessage& m) const
{
    return h == m.h && cx == m.cx && st == m.st && cm == m.cm;
}


/*!
    \fn bool QTranslatorMessage::operator!=(const QTranslatorMessage& m) const

    Returns true if the extended key of this object is different from
    that of \a m; otherwise returns false.
*/


/*!
    Returns true if the extended key of this object is
    lexicographically before than that of \a m; otherwise returns
    false.
*/

bool QTranslatorMessage::operator<(const QTranslatorMessage& m) const
{
    return h != m.h ? h < m.h
           : (cx != m.cx ? cx < m.cx
             : (st != m.st ? st < m.st : cm < m.cm));
}


/*!
    \fn bool QTranslatorMessage::operator<=(const QTranslatorMessage& m) const

    Returns true if the extended key of this object is
    lexicographically before that of \a m or if they are equal;
    otherwise returns false.
*/

/*!
    \fn bool QTranslatorMessage::operator>(const QTranslatorMessage& m) const

    Returns true if the extended key of this object is
    lexicographically after that of \a m; otherwise returns false.
*/

/*!
    \fn bool QTranslatorMessage::operator>=(const QTranslatorMessage& m) const

    Returns true if the extended key of this object is
    lexicographically after that of \a m or if they are equal;
    otherwise returns false.
*/

/*!
    \fn QString QTranslator::find(const char *context, const char *sourceText, const char * comment) const

    Use findMessage() instead.
*/

#endif // QT_NO_TRANSLATION
