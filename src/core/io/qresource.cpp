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

#include "qresource.h"
#include <qdir.h>
#include <qbytearray.h>

#define d d_func()
#define q q_func()

//#define QRESOURCE_DEBUG

enum {
    Compressed = 0x01
};
static QResource *qt_resource_root = 0;
static QStringList qt_resource_search_paths;


/* ******************** QResource ***************** */

/*!
    \class QResource qresource.h
    \reentrant

    \brief The QResource class provides access to application resource data.

    \ingroup io
    \mainclass

    QResources are created and used internally although they can also
    be used for direct access to the application's current resources.
    Resources must be accessed using absolute paths. Compression and
    encryption are taken care of internally.

    Use find() to find a resource. A resource has a name(), a size(),
    and either data() or children() (child resources). A resource may
    also have another resource as its parent().
*/

class QResourcePrivate
{
    QResource *q_ptr;
    friend class QMetaResource;
    Q_DECLARE_PUBLIC(QResource)

private:
    uint compressed : 1;
    uint container : 1;
    uint size;
    const uchar *data;
    QResource *parent;
    QString name;
    QList<QResource*> children;
    mutable QByteArray *decompressed;

protected:
    QResourcePrivate(QResource *qq) : q_ptr(qq), compressed(0),
                                      container(0), size(0), data(0), parent(0), decompressed(0) { }
    ~QResourcePrivate() {
        for(int i = 0; i < children.size(); i++)
            delete children[i];
        children.clear();
        delete decompressed;
        q_ptr = 0;
    }
    static QResource *locateResource(const QString &resource);
};

QResource *QResourcePrivate::locateResource(const QString &resource)
{
    QResource *ret = qt_resource_root;
    QStringList chunks = QDir::cleanPath(resource).split(QLatin1Char('/'), QString::SkipEmptyParts);
    for(int i = 0; i < chunks.size(); i++) {
        QResource *parent = ret;
        ret = 0;
        for(int subi = 0; subi < parent->d->children.size(); subi++) {
            if(parent->d->children.at(subi)->d->name == chunks[i]) {
                ret = parent->d->children.at(subi);
                break;
            }
        }
        if(!ret)
            break;
    }
    return ret;
}

/*!
  \internal

  Constructs a null QResource. Never attempt to create a QResource,
  instead they are created internally and access via QResource::find.

  \sa find
*/
QResource::QResource() : d_ptr(new QResourcePrivate(this))
{

}

/*!
  \internal

   Destroys a QResource. never attempt to destroy a QResource this
   will happen internally.
*/
QResource::~QResource()
{
    if(d->parent) {
        d->parent->d->children.removeAll(this);
        if(d->parent->d->children.isEmpty())
            delete d->parent;
        d->parent = 0;
    }
    delete d_ptr;
    d_ptr = 0;
}

/*!
    Returns the resource's node name, which is "/" if this resource is
    at the root of the resource system.
*/
QString
QResource::name() const
{
    return d->name;
}

/*!
    Returns the resource that contains this resource, or 0 if this
    resource is at the root of the resource system.
*/
const QResource
*QResource::parent() const
{
    return d->parent;
}

/*!
    Returns the uncompressed size of the resource.
*/
uint
QResource::size() const
{
    if(!d->compressed)
        return d->size;
    if(!d->decompressed) {
        d->decompressed = new QByteArray;
        *d->decompressed = qUncompress(d->data, d->size);
    }
    return d->decompressed->size();
}

/*!
    Returns the resource's data (uncompressed).

    \sa isContainer() children()
*/
const uchar
*QResource::data() const
{
    if(!d->compressed)
        return d->data;
    if(!d->decompressed) {
        d->decompressed = new QByteArray;
        *d->decompressed = qUncompress(d->data, d->size);
    }
    return (uchar *)d->decompressed->data();
}

/*!
    Returns true if this resource contains at least one other resource
    (and thus has no data); otherwise returns false.

    \sa children() parent()
*/
bool QResource::isContainer() const
{
    return d->container;
}

/*!
    Returns a list of the resource's child nodes. If there are no
    children or isContainer() returns false, an empty list is
    returned.

    \sa isContainer()
*/
QList<QResource *>
QResource::children() const
{
    if(d->container)
        return d->children;
    return QList<QResource*>();
}

/*!
    Returns the resource located at path \a resource, or 0 if no
    resource is found. Relative resources will be searched for through
    the search path. Resources are separated from their containing
    resource with forward slashes ('/') regardless of local file
    system.

    \sa QResource::addSearchPath()
*/
QResource
*QResource::find(const QString &resource)
{
    if(!qt_resource_root)
        return 0;
    if(resource[0] == QLatin1Char('/'))
        return QResourcePrivate::locateResource(resource);
    QStringList searchPaths = qt_resource_search_paths;
    if(searchPaths.isEmpty())
        searchPaths << QLatin1String("/");
    for(int i = 0; i < searchPaths.count(); i++) {
        if(QResource *ret = QResourcePrivate::locateResource(searchPaths.at(i) + QLatin1String("/") + resource))
            return ret;
    }
    return 0;
}

/*!
  Adds \a path to the search paths searched in to find resources that
  are not specified with an absolute path. The default search path is
  to search only in the root ('/').

  \sa QResource::find()
*/
void
QResource::addSearchPath(const QString &path)
{
    if(path[0] != QLatin1Char('/')) {
        qWarning("QResource::addSearchPath: Search paths must be absolute (start with /) [%s]", path.latin1());
        return;
    }
    qt_resource_search_paths.append(path);
}


/* ******************** QMetaResource ***************** */
// ### DOC: A class is \internal or public; it can't be "sort of" both.
/*!
    \class QMetaResource qresource.h
    \reentrant

    \brief The QMetaResource class is used to create QResource's from supplied data.

    \ingroup io
    \internal

    The QMetaResource is mostly an internal class that may only be
    used by code generated by the Qt resource compiler (rcc), once
    created a QResource will be referenced via its path name encoded
    in its data, once a QMetaResource is destroyed all references to
    QResource's representing the data are void, and QResource::find
    will no longer find a valid resource.
*/

class QMetaResourcePrivate
{
    QMetaResource *q_ptr;
    Q_DECLARE_PUBLIC(QMetaResource)

private:
    QResource *resource;

protected:
    QMetaResourcePrivate(QMetaResource *qq) : q_ptr(qq), resource(0) { }
    ~QMetaResourcePrivate() {
        delete resource;
        resource = 0;
        q_ptr = 0;
    }
};

/*!
   \internal

   Constructs a QMetaResource out of the resource data \a
   resource. This will normally happen automatically, QMetaResources
   must use specifically crafted data, if the data is invalid memory
   errors can occur.
*/
QMetaResource::QMetaResource(const uchar *resource) : d_ptr(new QMetaResourcePrivate(this))
{
    qInitResourceIO(); //just to be sure it has been loaded
    if(!qt_resource_root) {
        qt_resource_root = new QResource;
        qt_resource_root->d->container = true;
        qt_resource_root->d->name = QLatin1Char('/');
    }
    Q_ASSERT(resource[0] == 0x12 && resource[1] == 0x15 && resource[2] == 0x19 && resource[3] == 0x78);
    resource += 4;

    if(resource[0] == 0x01) { //version 1
        //flags
        uchar flags = resource[1];
        int off = 2;

        //name
        QString name;
        for( ; true; off += 2) {
            uchar c1 = resource[off], c2 = resource[off+1];
            if(!c1 && !c2) {
                off += 2;
                break;
	    }
	    name += QChar(c2, c1);
        }
        Q_ASSERT(!name.isEmpty() && name.at(0) == QLatin1Char('/'));

        //bytes
        uchar bytes_in_len = resource[off];
        Q_ASSERT(bytes_in_len <= 4);
        int len = 0;
        for(off += 1; bytes_in_len > 0; off++)
            len += (resource[off] << ((--bytes_in_len) * 8));
        const uchar *bytes = len ? resource+off : 0;

#ifdef QRESOURCE_DEBUG
        qDebug("created %s %d %p", name.latin1(), len, bytes);
#endif

        //now create the nodes
        bool creation_path = false;
        QResource *current = qt_resource_root;
        QStringList chunks = QDir::cleanPath(name).split(QLatin1Char('/'), QString::SkipEmptyParts);
        for(int i = 0; i < chunks.size(); i++) {
            QResource *parent = current;
            current = 0;
            if(!creation_path) {
                for(int subi = 0; subi < parent->d->children.size(); subi++) {
                    if(parent->d->children.at(subi)->d->name == chunks[i]) {
                        current = parent->d->children.at(subi);
                        break;
                    }
                }
            }
            if(!current) {
                creation_path = true;
                current = new QResource;
                current->d->name = chunks[i];
                current->d->container = (i != (chunks.size()-1));
                current->d->parent = parent;
                parent->d->children.append(current);
            }
        }
        Q_ASSERT(current && !current->d->container);
        d->resource = current;
        current->d->size = len;
        current->d->data = bytes;
        current->d->compressed = flags & Compressed;
    }
}

/*!
  \internal

  Destroys a QMetaResource. This normally will happen implicitly at
  the end of your application's existence.
*/
QMetaResource::~QMetaResource()
{
    delete d_ptr;
    d_ptr = 0;
}

