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

enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
           Tag_Hash, Tag_SourceText, Tag_Context, Tag_Comment,
           Tag_Obsolete1 };
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

static bool match(const uchar* found, const char* target, uint len)
{
    // 0 means anything, "" means empty
    return !found || qstrncmp((const char *)found, target, len) == 0 && target[len] == '\0';
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
    Q_DECLARE_PUBLIC(QTranslator)
public:
    enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69 };

    QTranslatorPrivate() : unmapPointer(0), unmapLength(0) {}

    // for mmap'ed files, this is what needs to be unmapped.
    char *unmapPointer;
    unsigned int unmapLength;

    // for squeezed but non-file data, this is what needs to be deleted
    const uchar *messageArray;
    const uchar *offsetArray;
    const uchar *contextArray;
    uint messageLength;
    uint offsetLength;
    uint contextLength;

    bool do_load(const uchar *data, int len);

    void clear();
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
    created with \l{Qt Linguist Manual}, install it using
    QApplication::installTranslator(), and use it via QObject::tr().
    For example:

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
    Constructs an empty message file object with parent \a parent that
    is not connected to any file.
*/

QTranslator::QTranslator(QObject * parent)
    : QObject(*new QTranslatorPrivate, parent)
{
}

#ifdef QT3_SUPPORT
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
    Q_D(QTranslator);
    d->clear();
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
    d->clear();

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

    bool ok = false;

#ifdef QT_USE_MMAP

#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

    int fd = -1;
    if (!realname.startsWith(QLatin1String(":")))
        fd = QT_OPEN(QFile::encodeName(realname), O_RDONLY,
#if defined(Q_OS_WIN)
                 _S_IREAD | _S_IWRITE
#else
                 0666
#endif
                );
    if (fd >= 0) {
        struct stat st;
        if (!fstat(fd, &st)) {
            char *ptr;
            ptr = reinterpret_cast<char *>(
                        mmap(0, st.st_size,             // any address, whole file
                             PROT_READ,                 // read-only memory
                             MAP_FILE | MAP_PRIVATE,    // swap-backed map from file
                             fd, 0));                   // from offset 0 of fd
            if (ptr && ptr != reinterpret_cast<char *>(MAP_FAILED)) {
                d->unmapPointer = ptr;
                d->unmapLength = st.st_size;
                ok = true;
            }
        }
        ::close(fd);
    }
#endif // QT_USE_MMAP

    if (!ok) {
        QFile file(realname);
        if (!file.exists())
            return false;
        d->unmapLength = file.size();
        d->unmapPointer = new char[d->unmapLength];

        if (file.open(QIODevice::ReadOnly))
            ok = (d->unmapLength == (uint)file.read(d->unmapPointer, d->unmapLength));

        if (!ok) {
            delete [] d->unmapPointer;
            d->unmapPointer = 0;
            d->unmapLength = 0;
            return false;
        }
    }

    return d->do_load(reinterpret_cast<const uchar *>(d->unmapPointer), d->unmapLength);
}

/*!
  \overload
  \fn bool QTranslator::load(const uchar *data, int len)

  Loads the .qm file data \a data of length \a len into the
  translator.

  The data is not copied. The caller must be able to guarantee that \a data
  will not be deleted or modified.
*/
bool QTranslator::load(const uchar *data, int len)
{
    Q_D(QTranslator);
    d->clear();
    return d->do_load(data, len);
}

static quint8 read8(const uchar *data)
{
    return *data;
}

static quint16 read16(const uchar *data)
{
    return (data[0] << 8) | (data[1]);
}

static quint32 read32(const uchar *data)
{
    return (data[0] << 24)
        | (data[1] << 16)
        | (data[2] << 8)
        | (data[3]);
}

bool QTranslatorPrivate::do_load(const uchar *data, int len)
{
    if (len < MagicLength || memcmp(data, magic, MagicLength) != 0) {
        clear();
        return false;
    }

    bool ok = true;
    const uchar *end = data + len;

    data += MagicLength;

    while (data < end - 4) {
        quint8 tag = read8(data++);
        quint32 blockLen = read32(data);
        data += 4;
        if (!tag || !blockLen)
            break;
        if (data + blockLen > end) {
            ok = false;
            break;
        }

        if (tag == QTranslatorPrivate::Contexts) {
            contextArray = data;
            contextLength = blockLen;
        } else if (tag == QTranslatorPrivate::Hashes) {
            offsetArray = data;
            offsetLength = blockLen;
        } else if (tag == QTranslatorPrivate::Messages) {
            messageArray = data;
            messageLength = blockLen;
        }

        data += blockLen;
    }

    return ok;
}

/*!
    Empties this translator of all contents.

    This function works with stripped translator files.
*/

void QTranslatorPrivate::clear()
{
    if (unmapPointer && unmapLength) {
#if defined(QT_USE_MMAP)
        munmap(unmapPointer, unmapLength);
#else
        delete [] unmapPointer;
#endif
        unmapPointer = 0;
        unmapLength = 0;
    }

    messageArray = 0;
    contextArray = 0;
    offsetArray = 0;
    messageLength = 0;
    contextLength = 0;
    offsetLength = 0;

    QEvent ev(QEvent::LanguageChange);
    QCoreApplication::sendEvent(QCoreApplication::instance(), &ev);
}


static QString getMessage(const uchar *m, const uchar *end, const char *context, const char *sourceText, const char *comment)
{
    const uchar *tn = 0;
    uint tn_length = 0;

    for (;;) {
        uchar tag = 0;
        if (m < end)
            tag = read8(m++);
        switch((Tag)tag) {
        case Tag_End:
            goto end;
        case Tag_Translation:
            tn_length = read32(m);
            if (tn_length % 1)
                return QString();
            m += 4;
            if (tn_length == 0xffffffff)
                return QString();
            tn = m;
            m += tn_length;
            break;
        case Tag_Hash:
            m += 4;
            break;
        case Tag_SourceText: {
            quint32 len = read32(m);
            m += 4;
            if (!match(m, sourceText, len))
                return QString();
            m += len;
        }
            break;
        case Tag_Context: {
            quint32 len = read32(m);
            m += 4;
            if (*m && !match(m, context, len))
                return QString();
            m += len;
        }
            break;
        case Tag_Comment: {
            quint32 len = read32(m);
            m += 4;
            if (*m && !match(m, comment, len))
                return QString();
            m += len;
        }
            break;
        default:
            return QString();
        }
    }
end:
    if (!tn)
        return QString();
    QString str = QString::fromUtf16((const ushort *)tn, tn_length/2);
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        for (int i = 0; i < str.length(); ++i)
            str[i] = QChar((str.at(i).unicode() >> 8) + ((str.at(i).unicode() << 8) & 0xff00));
    }
    return str;
}


/*!
  Returns the translation for the key (\a context, \a sourceText,
  \a comment). If none is found, also tries (\a context, \a
  sourceText, "").
*/

QString QTranslator::translate(const char *context, const char *sourceText, const char *comment) const
{
    Q_D(const QTranslator);
    if (context == 0)
        context = "";
    if (sourceText == 0)
        sourceText = "";
    if (comment == 0)
        comment = "";

    if (!d->offsetLength)
        return QString();

    /*
        Check if the context belongs to this QTranslator. If many
        translators are installed, this step is necessary.
    */
    if (d->contextLength) {
        quint16 hTableSize = read16(d->contextArray);
        uint g = elfHash(context) % hTableSize;
        const uchar *c = d->contextArray + 2 + (g << 1);
        quint16 off = read16(c);
        c += 2;
        if (off == 0)
            return QString();
        c = d->contextArray + (2 + (hTableSize << 1) + (off << 1));

        for (;;) {
            quint8 len = read8(c++);
            if (len == 0)
                return QString();
            if (match(c, context, len))
                break;
            c += len;
        }
    }

    size_t numItems = d->offsetLength / (2 * sizeof(quint32));
    if (!numItems)
        return QString();

    for (;;) {
        quint32 h = elfHash(QByteArray(sourceText) + comment);
        uchar *r = (uchar *) bsearch(&h, d->offsetArray, numItems, 2 * sizeof(quint32),
                                     (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? cmp_uint32_big : cmp_uint32_little);
        if (r != 0) {
            // go back on equal key
            while (r != d->offsetArray && cmp_uint32_big(r - 8, r) == 0)
                r -= 8;

            while (r < d->offsetArray + d->offsetLength) {
                quint32 rh = read32(r);
                r += 4;
                if (rh != h)
                    break;
                quint32 ro = read32(r);
                r += 4;
                QString tn = getMessage(d->messageArray + ro, d->messageArray + d->messageLength, context, sourceText, comment);
                if (!tn.isNull())
                    return tn;
            }
        }
        if (!comment[0])
            break;
        comment = "";
    }
    return QString();
}




/*!
    Returns true if this translator is empty, otherwise returns false.
    This function works with stripped and unstripped translation files.
*/
bool QTranslator::isEmpty() const
{
    Q_D(const QTranslator);
    return !d->unmapPointer && !d->unmapLength && !d->messageArray &&
           !d->offsetArray && !d->contextArray;
}


#endif // QT_NO_TRANSLATION
