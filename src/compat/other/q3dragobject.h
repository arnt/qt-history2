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

#ifndef Q3DRAGOBJECT_H
#define Q3DRAGOBJECT_H

#include "QtCore/qobject.h"
#include "QtGui/qcolor.h"
#include "QtGui/qmime.h"
#include "QtGui/qimage.h"
#include "Qt3Compat/q3strlist.h"
#include "QtCore/qlist.h"

class QWidget;
class Q3TextDragPrivate;
class Q3DragObjectPrivate;
class Q3StoredDragPrivate;
class Q3ImageDragPrivate;
class Q3ImageDrag;
class Q3TextDrag;
class Q3StrList;
class QImage;
class QPixmap;

class Q_COMPAT_EXPORT Q3DragObject : public QObject, public QMimeSource {
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3DragObject)
public:
    Q3DragObject(QWidget * dragSource = 0, const char *name = 0);
    virtual ~Q3DragObject();

    bool drag();
    bool dragMove();
    void dragCopy();
    void dragLink();

    virtual void setPixmap(QPixmap);
    virtual void setPixmap(QPixmap, const QPoint& hotspot);
    QPixmap pixmap() const;
    QPoint pixmapHotSpot() const;

    QWidget * source();
    static QWidget * target();

    enum DragMode { DragDefault, DragCopy, DragMove, DragLink, DragCopyOrMove };

protected:
    Q3DragObject(Q3DragObjectPrivate &, QWidget *dragSource = 0);
    virtual bool drag(DragMode);

private:
    friend class QDragMime;
    Q_DISABLE_COPY(Q3DragObject)
};

class Q_COMPAT_EXPORT Q3StoredDrag: public Q3DragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3StoredDrag)
public:
    Q3StoredDrag(const char *mimeType, QWidget *dragSource = 0, const char *name = 0);
    ~Q3StoredDrag();

    virtual void setEncodedData(const QByteArray &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

protected:
    Q3StoredDrag(Q3StoredDragPrivate &, const char *mimeType, QWidget *dragSource = 0);

private:
    Q_DISABLE_COPY(Q3StoredDrag)
};

class Q_COMPAT_EXPORT Q3TextDrag: public Q3DragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3TextDrag)
public:
    Q3TextDrag(const QString &, QWidget *dragSource = 0, const char *name = 0);
    Q3TextDrag(QWidget * dragSource = 0, const char * name = 0);
    ~Q3TextDrag();

    virtual void setText(const QString &);
    virtual void setSubtype(const QString &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QString& s);
    static bool decode(const QMimeSource* e, QString& s, QString& subtype);

protected:
    Q3TextDrag(Q3TextDragPrivate &, QWidget * dragSource = 0);

private:
    Q_DISABLE_COPY(Q3TextDrag)
};

class Q_COMPAT_EXPORT Q3ImageDrag: public Q3DragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3ImageDrag)
public:
    Q3ImageDrag(QImage image, QWidget * dragSource = 0, const char * name = 0);
    Q3ImageDrag(QWidget * dragSource = 0, const char * name = 0);
    ~Q3ImageDrag();

    virtual void setImage(QImage image);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QImage& i);
    static bool decode(const QMimeSource* e, QPixmap& i);

protected:
    Q3ImageDrag(Q3ImageDragPrivate &, QWidget * dragSource = 0);

private:
    Q_DISABLE_COPY(Q3ImageDrag)
};


class Q_COMPAT_EXPORT Q3UriDrag: public Q3StoredDrag {
    Q_OBJECT

public:
    Q3UriDrag(const Q3StrList &uris, QWidget * dragSource = 0, const char * name = 0);
    Q3UriDrag(QWidget * dragSource = 0, const char * name = 0);
    ~Q3UriDrag();

    void setFileNames(const QStringList & fnames);
    inline void setFileNames(const QString & fname) { setFileNames(QStringList(fname)); }
    void setFilenames(const QStringList & fnames) { setFileNames(fnames); }
    inline void setFilenames(const QString & fname) { setFileNames(QStringList(fname)); }
    void setUnicodeUris(const QStringList & uuris);
    virtual void setUris(const QList<QByteArray> &uris);

    static QString uriToLocalFile(const char*);
    static QByteArray localFileToUri(const QString&);
    static QString uriToUnicodeUri(const char*);
    static QByteArray unicodeUriToUri(const QString&);
    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, Q3StrList& i);
    static bool decodeToUnicodeUris(const QMimeSource* e, QStringList& i);
    static bool decodeLocalFiles(const QMimeSource* e, QStringList& i);

private:
    Q_DISABLE_COPY(Q3UriDrag)
};

class Q_COMPAT_EXPORT Q3ColorDrag : public Q3StoredDrag
{
    Q_OBJECT
    QColor color;

public:
    Q3ColorDrag(const QColor &col, QWidget *dragsource = 0, const char *name = 0);
    Q3ColorDrag(QWidget * dragSource = 0, const char * name = 0);
    void setColor(const QColor &col);

    static bool canDecode(QMimeSource *);
    static bool decode(QMimeSource *, QColor &col);

private:
    Q_DISABLE_COPY(Q3ColorDrag)
};

#endif // Q3DRAGOBJECT_H
