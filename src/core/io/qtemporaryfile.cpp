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

#include "qtemporaryfile.h"
#include <qfileengine.h>

#include <private/qiodevice_p.h>
#include <private/qfileengine_p.h>

#include <qplatformdefs.h>
#include <stdlib.h>
#include <errno.h>

#define d d_func()
#define q q_func()

#ifndef Q_OS_WIN
# define HAS_MKSTEMP
#endif

//************* QTemporaryFileEngine
class QTemporaryFileEngine : public QFSFileEngine
{
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QTemporaryFileEngine(const QString &file) : QFSFileEngine(file) { }

    virtual bool open(int flags);
};

bool
QTemporaryFileEngine::open(int flags)
{
    Q_ASSERT((flags & QIODevice::ReadWrite) == QIODevice::ReadWrite);

    QString qfilename = d->file;
    if(!qfilename.endsWith("XXXXXX"))
        qfilename += ".XXXXXX";
    d->external_file = 0;
    char *filename = strdup(qfilename.latin1());
#ifdef HAS_MKSTEMP
    d->fd = mkstemp(filename);
#else
    if(mktemp(filename)) 
        d->fd = d->sysOpen(filename, QT_OPEN_RDWR | QT_OPEN_CREAT);
#endif
    if(d->fd != -1) {
        d->file = filename; //changed now!
        free(filename);
        d->sequential = 0;
        return true;
    }
    free(filename);
    d->setError(errno == EMFILE ? QIODevice::ResourceError : QIODevice::OpenError, errno);
    return false;
}

//************* QTemporaryFilePrivate
class QTemporaryFilePrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QTemporaryFile)

protected:
    inline QFileEngine *getFileEngine() const { return static_cast<QFileEngine*>(q->ioEngine()); }

    QTemporaryFilePrivate();
    ~QTemporaryFilePrivate();

    bool autoRemove;
    QString templateName;
    mutable QFileEngine *fileEngine;
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

    QTemporaryFile is an I/O device that will get its input/output
    from the local disk. The filename for the temporary file will be
    guaranteed to be unique once the file is opened and will
    subsequently be removed upon destruction of the QTemporaryFile
    object.

    A temporary file will have some static part of the name and some
    part that is calculated to be unique. The default filename qt_temp
    will be placed into the temporary path as returned by
    QDir::tempPath().

    FILL IN MORE HERE**

    \sa QDir::tempPath(), QFile
*/

/*!
    Constructs a QTemporaryFile with no name.
*/
QTemporaryFile::QTemporaryFile() : QIODevice(*new QTemporaryFilePrivate)
{
    d->templateName = QDir::tempPath() + "qt_temp.XXXXXX";
}

/*!
    Constructs a QTemporaryFile with a template filename of \a
    templateName. Upon opening the temporary file this will be used to
    create a unique filename. If the \a templateName does end in
    XXXXXX it will automatically be appended and used as the dynamic
    portion of the filename.
    
    \sa QTemporaryFile::open(), QTemporaryFile::setTemplateName()
*/
QTemporaryFile::QTemporaryFile(const QString &templateName) : QIODevice(*new QTemporaryFilePrivate)
{
    d->templateName = templateName;
}

/*!
    Destroys the temporary file object, the file is automatically
    closed if necessary and if in auto remove mode it will
    automatically delete the file.

    \sa QTemporaryFile::autoRemove()
*/
QTemporaryFile::~QTemporaryFile()
{
    close();
    if(d->autoRemove) 
        remove();
}

/*!
  \fn bool QTemporaryFile::open()

  A QTemporaryFile will always be opened in QIODevice::ReadWrite mode,
  this allows easy access to the data in the file. This function will
  return true upon success and will set the fileName() to the unique
  filename used. 

  \sa QTemporaryFile::fileName()
*/

/*!
   Returns true if the QTemporaryFile is in auto remove
   mode. Auto-remove mode will automatically delete the filename from
   disk upon destruction. This makes it very easy to create your
   QTemporaryFile object on the stack, fill it with data, read from
   it, and finally on function return it will automatically clean up
   after itself.

   \sa QTemporaryFile::setAutoRemove(), QTemporaryFile::remove()
*/
bool
QTemporaryFile::autoRemove() const
{
    return d->autoRemove;
}

/*!
  Sets the QTemporaryFile into auto-remove mode. 

  \sa QTemporarFile::autoRemove(), QTemporaryFile::remove()
*/
void
QTemporaryFile::setAutoRemove(bool b)
{
    d->autoRemove = b;
}

/*!
  Remove the unique filename from disk. This is normally done
  automatically using auto-remove mode. However this can be done
  explicitly if the file needs to be removed earlier (or the
  QTemporaryFile needs to be reused).

  \sa QTemporaryFile::setAutoRemove()
*/
bool
QTemporaryFile::remove()
{
    close();    
    if(status() == QIODevice::Ok) {
        if(d->getFileEngine()->remove()) {
            resetStatus();
            return true;
        }
        setStatus(QIODevice::RemoveError, errno);
    }
    return false;
}

/*!
   Returns the complete unique filename backing the QTemporaryFile
   object. This string is null before the QTemporaryFile is opened,
   afterwards it will contain the QTemporaryFile::fileTemplate() plus
   additional characters to make it unique.

   \sa QTemporary::fileTemplate()
*/

QString
QTemporaryFile::fileName() const
{
    if(!isOpen())
        return QString::null;
    return d->getFileEngine()->fileName();
}

/*!
  Returns the set file template. The default file template will be
  called qt_temp and be placed in QDir::tempPath(). 

  \sa QTemporaryFile::setFileTemplate()
*/
QString
QTemporaryFile::fileTemplate() const
{
    return d->templateName;
}

/*!
   Sets the static portion of the file name to \a name. If the file
   template ends in XXXXXX that will automatically be replaced with
   the unique part of the filename, otherwise a filename will be
   determined automatically based on the static portion specified.

   \sa QTemporaryFile::fileTemplate()
*/
void
QTemporaryFile::setFileTemplate(const QString &name)
{
    Q_ASSERT(!isOpen());
    d->getFileEngine()->setFileName(name);
    d->templateName = name;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(). On systems that use file
  descriptors for sockets (i.e. Unix systems, but not Windows) the handle
  can be used with QSocketNotifier as well.

  If the file is not open, or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/
int
QTemporaryFile::handle() const
{
    if (!isOpen())
        return -1;
    return static_cast<QTemporaryFileEngine*>(d->getFileEngine())->handle();
}

/*!
   \reimp
*/
QIOEngine
*QTemporaryFile::ioEngine() const
{
    if(!d->fileEngine)
        d->fileEngine = new QTemporaryFileEngine(d->templateName);
    return d->fileEngine;
}


QTemporaryFile
*QTemporaryFile::createLocalFile(QFile &file)
{
    if(QFileEngine *engine = static_cast<QFileEngine*>(file.ioEngine())) {
        if(engine->fileFlags(QFileEngine::FlagsMask) & QFileEngine::LocalDiskFlag)
            return 0; //local already
        //cache
        bool wasOpen = file.isOpen();
        QFile::Offset old_off = 0;
        if(wasOpen)
            old_off = file.at();
        else
            file.open(QIODevice::ReadOnly);
        //dump data
        QTemporaryFile *ret = new QTemporaryFile;
        ret->open();
        file.seek(0);
        char buffer[1024];
        while(true) {
            Q_LONG len = file.readBlock(buffer, 1024);
            if(len < 1)
                break;
            ret->writeBlock(buffer, len);
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
