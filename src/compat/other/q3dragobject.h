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

#include "qobject.h"
#include "qcolor.h"
#include "qmime.h"
#include "qimage.h"
#include "qlist.h"

class QWidget;
class QTextDragPrivate;
class QDragObjectPrivate;
class QStoredDragPrivate;
class QImageDragPrivate;
class QImageDrag;
class QTextDrag;
class QImage;
class QPixmap;

class Q_GUI_EXPORT QDragObject : public QObject, public QMimeSource {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDragObject)
public:
    QDragObject(QWidget * dragSource = 0, const char *name = 0);
    virtual ~QDragObject();

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
    QDragObject(QDragObjectPrivate &, QWidget *dragSource = 0);
    virtual bool drag(DragMode);

private:
    Q_DISABLE_COPY(QDragObject)
};

class Q_GUI_EXPORT QStoredDrag: public QDragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStoredDrag)
public:
    QStoredDrag(const char *mimeType, QWidget *dragSource = 0, const char *name = 0);
    ~QStoredDrag();

    virtual void setEncodedData(const QByteArray &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

protected:
    QStoredDrag(QStoredDragPrivate &, const char *mimeType, QWidget *dragSource = 0);

private:
    Q_DISABLE_COPY(QStoredDrag)
};

class Q_GUI_EXPORT QTextDrag: public QDragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextDrag)
public:
    QTextDrag(const QString &, QWidget *dragSource = 0, const char *name = 0);
    QTextDrag(QWidget * dragSource = 0, const char * name = 0);
    ~QTextDrag();

    virtual void setText(const QString &);
    virtual void setSubtype(const QString &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QString& s);
    static bool decode(const QMimeSource* e, QString& s, QString& subtype);

protected:
    QTextDrag(QTextDragPrivate &, QWidget * dragSource = 0);

private:
    Q_DISABLE_COPY(QTextDrag)
};

class Q_GUI_EXPORT QImageDrag: public QDragObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QImageDrag)
public:
    QImageDrag(QImage image, QWidget * dragSource = 0, const char * name = 0);
    QImageDrag(QWidget * dragSource = 0, const char * name = 0);
    ~QImageDrag();

    virtual void setImage(QImage image);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QImage& i);
    static bool decode(const QMimeSource* e, QPixmap& i);

protected:
    QImageDrag(QImageDragPrivate &, QWidget * dragSource = 0);

private:
    Q_DISABLE_COPY(QImageDrag)
};


class Q_GUI_EXPORT QUriDrag: public QStoredDrag {
    Q_OBJECT

public:
    QUriDrag(const QList<QByteArray> &uris, QWidget * dragSource = 0, const char * name = 0);
    QUriDrag(QWidget * dragSource = 0, const char * name = 0);
    ~QUriDrag();

    void setFilenames(const QStringList & fnames) { setFileNames(fnames); }
    void setFileNames(const QStringList & fnames);
    void setUnicodeUris(const QStringList & uuris);
    virtual void setUris(const QList<QByteArray> &uris);

    static QString uriToLocalFile(const char*);
    static QByteArray localFileToUri(const QString&);
    static QString uriToUnicodeUri(const char*);
    static QByteArray unicodeUriToUri(const QString&);
    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QList<QByteArray>& i);
    static bool decodeToUnicodeUris(const QMimeSource* e, QStringList& i);
    static bool decodeLocalFiles(const QMimeSource* e, QStringList& i);

private:
    Q_DISABLE_COPY(QUriDrag)
};

class Q_GUI_EXPORT QColorDrag : public QStoredDrag
{
    Q_OBJECT
    QColor color;

public:
    QColorDrag(const QColor &col, QWidget *dragsource = 0, const char *name = 0);
    QColorDrag(QWidget * dragSource = 0, const char * name = 0);
    void setColor(const QColor &col);

    static bool canDecode(QMimeSource *);
    static bool decode(QMimeSource *, QColor &col);

private:
    Q_DISABLE_COPY(QColorDrag)
};

#ifdef QT_COMPAT
typedef QUriDrag QUrlDrag;
#endif

#endif // QDRAGOBJECT_H
