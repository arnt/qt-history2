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
#include <qlocale.h>
#include <qbytearray.h>

#define d d_func()
#define q q_func()

//#define QRESOURCE_DEBUG

enum {
    Compressed = 0x01
};
Q_GLOBAL_STATIC(QStringList, qt_resource_search_paths)


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

struct QResourceNode
{
    QString name;
    QList<QResource*> resources;
    QResource *localeResource(); //find the resource for locale

    uint container : 1;
    QResourceNode *parent;
    QList<QResourceNode*> children;

    inline QResourceNode() : container(0), parent(0) { }
    inline ~QResourceNode() {
        qDeleteAll(children);
        children.clear();
        for(int i = 0; i < resources.size(); i++)
            delete resources[i];
        resources.clear();
    }
};
static QResourceNode *qt_resource_root = 0;

class QResourcePrivate
{
    QResource *q_ptr;
    friend class QMetaResource;
    Q_DECLARE_PUBLIC(QResource)

private:
    QLocale::Language lang;
    QLocale::Country country;
    uint compressed : 1;
    uint size;
    const uchar *data;
    mutable QByteArray *decompressed;

    QResourceNode *node;

protected:
    friend struct QResourceNode;
    QResourcePrivate(QResource *qq) : q_ptr(qq), lang(QLocale::C),
                                      country(QLocale::AnyCountry), compressed(0),
                                      size(0), data(0), decompressed(0), node(0) { }
    ~QResourcePrivate() {
        delete decompressed;
        q_ptr = 0;
    }
    static QResource *locateResource(const QString &resource);
};

QResource *QResourceNode::localeResource()
{
    if(container) {
        if(resources.isEmpty()) { //create a resource as needed
            QResource *ret = new QResource;
            ret->d->node = this;
            resources.append(ret);
            return ret;
        }
        Q_ASSERT(resources.count() == 1);
        return resources.first(); //containers do not get localized
    } else if(!resources.isEmpty()) {
        QResource *ret = 0;
        QLocale systemLocale = QLocale::system();
        for(int i = 0; i < resources.count(); i++) {
            QResource *resource = resources.at(i);
            if(resource->d->lang == systemLocale.language() &&
               resource->d->country == systemLocale.country())
                return resource;
            if(!ret && resource->d->lang == systemLocale.language())
                ret = resource;
            else if(!ret && resource->d->lang == QLocale::C && //default
                    resource->d->country == QLocale::AnyCountry)
                ret = resource;
        }
        return ret;
    }
    return 0;
}

QResource *QResourcePrivate::locateResource(const QString &resource)
{
    QResourceNode *ret = qt_resource_root;
    QStringList chunks = QDir::cleanPath(resource).split(QLatin1Char('/'), QString::SkipEmptyParts);
    for(int i = 0; i < chunks.size(); i++) {
        QResourceNode *parent = ret;
        ret = 0;
        for(int subi = 0; subi < parent->children.size(); subi++) {
            QResourceNode *child = parent->children.at(subi);
            if(child->name == chunks[i]) {
                ret = child;
                break;
            }
        }
        if(!ret)
            break;
    }
#ifdef QRESOURCE_DEBUG
    fprintf(stderr, "Looking for resources %s found %p[%p]\n", resource.latin1(),
            ret, ret ? ret->localeResource() : 0);
#endif
    return ret ? ret->localeResource() : 0;
}

/*!
  \internal

  Constructs a null QResource. Never attempt to create a QResource,
  instead they are created internally and accessed via QResource::find.

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
    if(d->node->parent) {
        d->node->parent->children.removeAll(d->node);
        if(d->node->parent->children.isEmpty())
            delete d->node->parent;
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
    return d->node->name;
}

/*!
    Returns the resource that contains this resource, or 0 if this
    resource is at the root of the resource system.
*/
const QResource
*QResource::parent() const
{
    if(QResourceNode *ret = d->node->parent)
        return ret->localeResource();
    return 0;
}

/*!
    Returns the uncompressed size of the resource.
*/
uint
QResource::size() const
{
    if(d->compressed) {
        if(!d->decompressed) {
            d->decompressed = new QByteArray;
            *d->decompressed = qUncompress(d->data, d->size);
        }
        return d->decompressed->size();
    }
    return d->size;
}

/*!
    Returns the resource's data (uncompressed).

    \sa isContainer() children()
*/
const uchar
*QResource::data() const
{
    if(d->compressed) {
        if(!d->decompressed) {
            d->decompressed = new QByteArray;
            *d->decompressed = qUncompress(d->data, d->size);
        }
        return (uchar *)d->decompressed->data();
    }
    return d->data;
}

/*!
    Returns true if this resource contains at least one other resource
    (and thus has no data); otherwise returns false.

    \sa children() parent()
*/
bool QResource::isContainer() const
{
    return d->node->container;
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
    QList<QResource *> ret;
    if(d->node->container) {
        for(int i = 0; i < d->node->children.count(); i++)
            ret.append(d->node->children.at(i)->localeResource());
    }
    return ret;
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
    QStringList searchPaths = *qt_resource_search_paths();
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
    qt_resource_search_paths()->append(path);
}


/* ******************** QMetaResource ***************** */
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
        qt_resource_root = new QResourceNode;
        qt_resource_root->container = true;
        qt_resource_root->name = QLatin1Char('/');
    }
    Q_ASSERT(resource[0] == 0x12 && resource[1] == 0x15 && resource[2] == 0x19 && resource[3] == 0x78);
    resource += 4;

    if(resource[0] == 0x01) { //version 1
        int off = 1;

        //lang
        uchar lang = resource[off++];
        uchar country = resource[off++];

        //flags
        uchar flags = resource[off];
        off++;

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
        fprintf(stderr, "created %s %d %p [%d/%d]\n", name.latin1(), len, bytes, lang, country);
#endif

        //now create the nodes
        bool creation_path = false;
        QResourceNode *node = qt_resource_root;
        QStringList chunks = QDir::cleanPath(name).split(QLatin1Char('/'), QString::SkipEmptyParts);
        for(int i = 0; i < chunks.size(); i++) {
            QResourceNode *parent = node;
            node = 0;
            if(!creation_path) {
                for(int subi = 0; subi < parent->children.size(); subi++) {
                    if(parent->children.at(subi)->name == chunks[i]) {
                        node = parent->children.at(subi);
                        break;
                    }
                }
            }
            if(!node) {
                creation_path = true;
                node = new QResourceNode;
                node->name = chunks[i];
                node->container = (i != (chunks.size()-1));
                node->parent = parent;
                parent->children.append(node);
            }
        }

        //create a resource for this node
        Q_ASSERT(node && !node->container);
        QResource *resource = new QResource;
        resource->d->node = node;
        resource->d->lang = (QLocale::Language)lang;
        resource->d->country = (QLocale::Country)country;
        resource->d->size = len;
        resource->d->data = bytes;
        resource->d->compressed = flags & Compressed;
        node->resources.append(resource);
        d->resource = resource;
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

