/****************************************************************************
**
** Implementation of QResource and QMetaResource classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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


/* ******************** QResource ***************** */
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
};

QResource::QResource() : d_ptr(new QResourcePrivate(this))
{

}

QResource::~QResource()
{
    delete d_ptr;
    d_ptr = 0;
}

QString 
QResource::name() const
{
    return d->name;
}

const QResource 
*QResource::parent() const
{
    return d->parent;
}

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

const uchar 
*QResource::data() const
{
    if(!d->compressed) 
        return d->data;
    if(!d->decompressed)
        *d->decompressed = qUncompress(d->data, d->size);
    return (uchar *)d->decompressed->data();
}

bool QResource::isContainer() const
{
    return d->container;
}

QList<QResource *> 
QResource::children() const
{
    if(d->container)
        return d->children;
    return QList<QResource*>();
}

QResource 
*QResource::find(const QString &path)
{
    if(!qt_resource_root)
        return 0;
    if(path.isEmpty() || path[0] != '/') {
        qWarning("Invalid resource path: %s", path.latin1());
        return 0;
    }
    
    QResource *ret = qt_resource_root;
    QStringList chunks = QDir::cleanDirPath(path).split('/', QString::SkipEmptyParts);
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
            return 0;
    }
    return ret;
}

/* ******************** QMetaResource ***************** */
class QMetaResourcePrivate
{
    QMetaResource *q_ptr;
    Q_DECLARE_PUBLIC(QMetaResource)

private:
    QResource *resource;

protected:
    QMetaResourcePrivate(QMetaResource *qq) : q_ptr(qq) { }
    ~QMetaResourcePrivate() { q_ptr = 0; }
};

QMetaResource::QMetaResource(const uchar *resource) : d_ptr(new QMetaResourcePrivate(this))
{
    if(!qt_resource_root) {
        qt_resource_root = new QResource;
        qt_resource_root->d->container = true;
        qt_resource_root->d->name = "/";
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
        Q_ASSERT(!name.isEmpty() && name[0] == '/');

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
        QStringList chunks = QDir::cleanDirPath(name).split('/', QString::SkipEmptyParts);
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
        current->d->size = len;
        current->d->data = bytes;
        current->d->compressed = flags & Compressed;
    }
}

QMetaResource::~QMetaResource()
{
#if 0
    if(d->resource) {
        for(QResource *r = d->resource->d->parent; r && r != qt_resource_root; ) {
            if(r->d->container && r->d->children.count() == 1) {
                QResource *n = r->d->parent;
                n->d->children.removeAll(r);
                delete r;
                r = n;
            }
            break;
        }
        delete d->resource;
        d->resource = 0;
    }
#endif
    delete d_ptr;
    d_ptr = 0;
}

