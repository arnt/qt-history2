#ifndef QDRAGOBJECT_P_H
#define QDRAGOBJECT_P_H

#include <private/qobject_p.h>
#define d d_func()
#define q q_func()

class QDragObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDragObject)
public:
    QDragObjectPrivate(): hot(0,0) {}
    QPixmap pixmap;
    QPoint hot;
    // store default cursors
    QPixmap *pm_cursor;
};

class QTextDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextDrag)
public:
    QTextDragPrivate() { setSubType("plain"); }
    void setSubType(const QString & st);

    enum { nfmt=4 };

    QString txt;
    QByteArray fmt[nfmt];
    QString subtype;
};

class QStoredDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QStoredDrag)
public:
    QStoredDragPrivate() {}
    const char* fmt;
    QByteArray enc;
};

class QImageDragPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QImageDrag)
public:
    QImage img;
    QList<QByteArray> ofmts;
};

#endif
