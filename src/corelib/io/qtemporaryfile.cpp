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
#include "qtemporaryfile.h"
#include "qabstractfileengine.h"
#include "private/qfile_p.h"
#include "private/qabstractfileengine_p.h"
#include "private/qfsfileengine_p.h"

#include <stdlib.h>
#include <errno.h>

#ifndef Q_OS_WIN
# define HAS_MKSTEMP
#endif

//************* QTemporaryFileEngine
class QTemporaryFileEngine : public QFSFileEngine
{
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QTemporaryFileEngine(const QString &file) : QFSFileEngine(file) { }
    ~QTemporaryFileEngine();

    bool open(QIODevice::OpenMode flags);
    bool remove();
    bool close();
};

QTemporaryFileEngine::~QTemporaryFileEngine()
{
}

bool QTemporaryFileEngine::open(QIODevice::OpenMode)
{
    Q_D(QFSFileEngine);

    QString qfilename = d->file;
    if(!qfilename.endsWith(QLatin1String("XXXXXX")))
        qfilename += QLatin1String(".XXXXXX");
    d->closeFileHandle = true;
    char *filename = qstrdup(qfilename.toLocal8Bit());

#ifdef HAS_MKSTEMP
    d->fd = mkstemp(filename);
#else
#if defined(_MSC_VER) && _MSC_VER >= 1400
    int len = int(strlen(filename)) + 1;
    if(_mktemp_s(filename, len) == 0) {
#else
    if(mktemp(filename)) {
#endif
        int oflags = QT_OPEN_RDWR | QT_OPEN_CREAT;
#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN32) || defined(Q_OS_OS2)
        oflags |= QT_OPEN_BINARY; // we handle all text translations our self.
#endif
        d->fd = d->sysOpen(filename, oflags);
    }
#endif
    if(d->fd != -1) {
        d->file = QString::fromLocal8Bit(filename); //changed now!
        delete [] filename;
        d->sequential = 0;
        return true;
    }
    delete [] filename;
    setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError, qt_error_string(errno));
    return false;
}

bool QTemporaryFileEngine::remove()
{
    Q_D(QFSFileEngine);
    bool removed = QFSFileEngine::remove();
    d->file.clear();
    return removed;
}

bool QTemporaryFileEngine::close()
{
    // Don't close the file, just seek to the front.
    seek(0);
    setError(QFile::UnspecifiedError, QString());
    return true;
}
 
//************* QTemporaryFilePrivate
class QTemporaryFilePrivate : public QFilePrivate
{
    Q_DECLARE_PUBLIC(QTemporaryFile)

protected:
    QTemporaryFilePrivate();
    ~QTemporaryFilePrivate();

    bool autoRemove;
    QString templateName;
    mutable QTemporaryFileEngine *fileEngine;
};

QTemporaryFilePrivate::QTemporaryFilePrivate() : autoRemove(true), fileEngine(0)
{
}

QTemporaryFilePrivate::~QTemporaryFilePrivate()
{
    delete fileEngine;
    fileEngine = 0;
}

//************* QTemporaryFile

/*!
    \class QTemporaryFile
    \reentrant
    \brief The QTemporaryFile class is an I/O device that operates on temporary files.

    \ingroup io
    \mainclass

    QTemporaryFile is used to create unique temporary files safely. The file
    itself is created by calling open(). The name of the temporary file is
    guaranteed to be unique (i.e., you are guaranteed to not overwrite an
    existing file), and the file will subsequently be removed upon destruction
    of the QTemporaryFile object. This is an important technique that avoids
    data corruption for applications that store data in temporary files. The
    file name is either auto-generated, or created based on a template, which
    is passed to QTemporaryFile's constructor.

    Example:

    \code
        QTemporaryFile *file = new QTemporaryFile;
        if (file->open(QFile::ReadWrite)) {
            // file->fileName() now contains the unique file name
        }
        delete file; // also removes the temporary file
    \endcode

    Reopening a QTemporaryFile after calling close() is safe. For as long as
    the QTemporaryFile object itself is not destroyed, the unique temporary
    file will exist and be kept open internally by QTemporaryFile.

    A temporary file will have some static part of the name and some part that
    is calculated to be unique. The default filename qt_temp will be placed
    into the temporary path as returned by QDir::tempPath().

    \sa QDir::tempPath(), QFile
*/

#ifdef QT_NO_QOBJECT
QTemporaryFile::QTemporaryFile()
    : QFile(*new QTemporaryFilePrivate)
{
    Q_D(QTemporaryFile);
    d->templateName = QDir::tempPath() + "/" + QLatin1String("qt_temp.XXXXXX");
}

QTemporaryFile::QTemporaryFile(const QString &templateName)
    : QFile(*new QTemporaryFilePrivate)
{
    Q_D(QTemporaryFile);
    d->templateName = templateName;
}

#else
/*!
    Constructs a QTemporaryFile in QDir::tempPath(), using the file template
    "qt_temp.XXXXXX".

    \sa setFileTemplate()
*/
QTemporaryFile::QTemporaryFile()
    : QFile(*new QTemporaryFilePrivate, 0)
{
    Q_D(QTemporaryFile);
    d->templateName = QDir::tempPath() + QLatin1String("/qt_temp.XXXXXX");
}

/*!
    Constructs a QTemporaryFile with a template filename of \a
    templateName. Upon opening the temporary file this will be used to create
    a unique filename. If the \a templateName does end in XXXXXX it will
    automatically be appended and used as the dynamic portion of the filename.

    \sa open(), fileTemplate()
*/
QTemporaryFile::QTemporaryFile(const QString &templateName)
    : QFile(*new QTemporaryFilePrivate, 0)
{
    Q_D(QTemporaryFile);
    d->templateName = templateName;
}

/*!
    Constructs a QTemporaryFile with the given \a parent, but with no name.

    \sa setFileTemplate()
*/
QTemporaryFile::QTemporaryFile(QObject *parent)
    : QFile(*new QTemporaryFilePrivate, parent)
{
    Q_D(QTemporaryFile);
    d->templateName = QDir::tempPath() + QLatin1String("qt_temp.XXXXXX");
}

/*!
    Constructs a QTemporaryFile with a template filename of \a
    templateName and the specified \a parent.
    Upon opening the temporary file this will be used to
    create a unique filename. If the \a templateName does end in
    XXXXXX it will automatically be appended and used as the dynamic
    portion of the filename.

    \sa open(), fileTemplate()
*/
QTemporaryFile::QTemporaryFile(const QString &templateName, QObject *parent)
    : QFile(*new QTemporaryFilePrivate, parent)
{
    Q_D(QTemporaryFile);
    d->templateName = templateName;
}
#endif

/*!
    Destroys the temporary file object, the file is automatically
    closed if necessary and if in auto remove mode it will
    automatically delete the file.

    \sa autoRemove()
*/
QTemporaryFile::~QTemporaryFile()
{
    Q_D(QTemporaryFile);
    close();
    if (!d->fileName.isEmpty() && d->autoRemove)
        remove();
}

/*!
  \fn bool QTemporaryFile::open()

  A QTemporaryFile will always be opened in QIODevice::ReadWrite mode,
  this allows easy access to the data in the file. This function will
  return true upon success and will set the fileName() to the unique
  filename used.

  \sa fileName()
*/

/*!
   Returns true if the QTemporaryFile is in auto remove
   mode. Auto-remove mode will automatically delete the filename from
   disk upon destruction. This makes it very easy to create your
   QTemporaryFile object on the stack, fill it with data, read from
   it, and finally on function return it will automatically clean up
   after itself.

   Auto-remove is on by default.

   \sa setAutoRemove(), remove()
*/
bool QTemporaryFile::autoRemove() const
{
    Q_D(const QTemporaryFile);
    return d->autoRemove;
}

/*!
    Sets the QTemporaryFile into auto-remove mode if \a b is true.

    Auto-remove is on by default.

    \sa autoRemove(), remove()
*/
void QTemporaryFile::setAutoRemove(bool b)
{
    Q_D(QTemporaryFile);
    d->autoRemove = b;
}

/*!
   Returns the complete unique filename backing the QTemporaryFile
   object. This string is null before the QTemporaryFile is opened,
   afterwards it will contain the fileTemplate() plus
   additional characters to make it unique.

   \sa fileTemplate()
*/

QString QTemporaryFile::fileName() const
{
    if(!isOpen())
        return QString();
    return fileEngine()->fileName(QAbstractFileEngine::DefaultName);
}

/*!
  Returns the set file template. The default file template will be
  called qt_temp and be placed in QDir::tempPath().

  \sa setFileTemplate()
*/
QString QTemporaryFile::fileTemplate() const
{
    Q_D(const QTemporaryFile);
    return d->templateName;
}

/*!
   Sets the static portion of the file name to \a name. If the file
   template ends in XXXXXX that will automatically be replaced with
   the unique part of the filename, otherwise a filename will be
   determined automatically based on the static portion specified.

   \sa fileTemplate()
*/
void QTemporaryFile::setFileTemplate(const QString &name)
{
    Q_ASSERT(!isOpen());
    Q_D(QTemporaryFile);
    fileEngine()->setFileName(name);
    d->templateName = name;
}

/*!
    \fn QTemporaryFile *QTemporaryFile::createLocalFile(const QString &fileName)
    \overload

    Works on the given \a fileName rather than an existing QFile
    object.
*/


/*!
    Creates and returns a local temporary file whose contents are a
    copy of the contents of the given \a file.
*/
QTemporaryFile *QTemporaryFile::createLocalFile(QFile &file)
{
    if (QAbstractFileEngine *engine = file.fileEngine()) {
        if(engine->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::LocalDiskFlag)
            return 0; //local already
        //cache
        bool wasOpen = file.isOpen();
        qint64 old_off = 0;
        if(wasOpen)
            old_off = file.pos();
        else
            file.open(QIODevice::ReadOnly);
        //dump data
        QTemporaryFile *ret = new QTemporaryFile;
        ret->open();
        file.seek(0);
        char buffer[1024];
        while(true) {
            qint64 len = file.read(buffer, 1024);
            if(len < 1)
                break;
            ret->write(buffer, len);
        }
        ret->seek(0);
        //restore
        if(wasOpen)
            file.seek(old_off);
        else
            file.close();
        //done
        return ret;
    }
    return 0;
}

/*!
   \internal
*/

QAbstractFileEngine *QTemporaryFile::fileEngine() const
{
    Q_D(const QTemporaryFile);
    if(!d->fileEngine)
        d->fileEngine = new QTemporaryFileEngine(d->templateName);
    return d->fileEngine;
}

/*!
   \reimp

    Creates a unique file name for the temporary file, and opens it.  You can
    get the unique name later by calling fileName(). The file is guaranteed to
    have been created by this function (i.e., it has never existed before).
*/
bool QTemporaryFile::open(OpenMode flags)
{
    Q_D(QTemporaryFile);
    if (!d->fileName.isEmpty()) {
        setOpenMode(flags);
        return true;
    }
    
    if (QFile::open(flags)) {
        d->fileName = d->fileEngine->fileName(QAbstractFileEngine::DefaultName);
        return true;
    }
    return false;
}
