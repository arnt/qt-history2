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

#ifndef QDND_P_H
#define QDND_P_H

#include "qobject.h"
#include "qmime.h"
#include "qdrag.h"
#include "qpixmap.h"
#include "qpoint.h"

class QDragPrivate
{
public:
    QWidget *source;
    QWidget *target;
    QMimeData *data;
    QPixmap pixmap;
    QPoint hotspot;
    QDrag::DragOperations operations;
    QDrag::DragOperation executed_op;
};

class QDropData : public QMimeData
{
    Q_OBJECT
public:
    QDropData(QObject *parent);
    ~QDropData();

    QByteArray data(const QString &mimetype) const;
    bool hasFormat(const QString &mimetype) const;
    QStringList formats() const;
};

class Q_GUI_EXPORT QDragManager: public QObject {
    Q_OBJECT

    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDrag;
    friend class QDragMoveEvent;
    friend class QDropEvent;
    friend class QApplication;

    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent*);

public:
    QDrag::DragOperation drag(QDragPrivate *, QDrag::DragOperations);

    void cancel(bool deleteSource = true);
    void move(const QPoint &);
    void drop();
    void updatePixmap();
    QWidget *source() { return object->source; }

    static QDragManager *self();
private:
    Q_DISABLE_COPY(QDragManager)

    QDragPrivate *object;
    void updateMode(Qt::KeyboardModifiers newstate);
    void updateCursor();

    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;

    QPixmap *pm_cursor;
    int n_cursor;
    static QDragManager *instance;
public:
    QDropData *dropData;
};

#endif
