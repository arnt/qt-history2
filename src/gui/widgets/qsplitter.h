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

#ifndef QSPLITTER_H
#define QSPLITTER_H

#include "qframe.h"
#include "qsizepolicy.h"

#ifndef QT_NO_SPLITTER

class QSplitterPrivate;
class QTextStream;
template <typename T> class QList;

class Q_GUI_EXPORT QSplitter : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool opaqueResize READ opaqueResize WRITE setOpaqueResize)
    Q_PROPERTY(int handleWidth READ handleWidth WRITE setHandleWidth)
    Q_PROPERTY(bool childrenCollapsible READ childrenCollapsible WRITE setChildrenCollapsible)

public:
    QSplitter(QWidget* parent = 0, const char* name = 0);
    QSplitter(Qt::Orientation, QWidget* parent = 0, const char* name = 0);
    ~QSplitter();

    void setOrientation(Qt::Orientation);
    Qt::Orientation orientation() const;

    void setChildrenCollapsible(bool);
    bool childrenCollapsible() const;

    void setCollapsible(QWidget *w, bool);
    void setOpaqueResize(bool = true);
    bool opaqueResize() const;
    void refresh();

    void moveToFirst(QWidget *);
    void moveToLast(QWidget *);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QList<int> sizes() const;
    void setSizes(const QList<int> &list);

    int handleWidth() const;
    void setHandleWidth(int);

protected:
    void childEvent(QChildEvent *);

    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);

    int idAfter(QWidget*) const;

    void moveSplitter(int pos, int id);
    void changeEvent(QEvent *);
    int adjustPos(int, int);
    virtual void setRubberband(int);
    void getRange(int id, int *, int *);

#ifdef QT_COMPAT
public:
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint, Auto };
    QT_COMPAT void setResizeMode(QWidget *w, ResizeMode mode);
#endif

private:
    Q_DISABLE_COPY(QSplitter)
    Q_DECLARE_PRIVATE(QSplitter)
    friend class QSplitterHandle;
#ifndef QT_NO_TEXTSTREAM
    friend Q_GUI_EXPORT QTextStream& operator<<(QTextStream&, const QSplitter&);
    friend Q_GUI_EXPORT QTextStream& operator>>(QTextStream&, QSplitter&);
#endif
};

#ifndef QT_NO_TEXTSTREAM
Q_GUI_EXPORT QTextStream& operator<<(QTextStream&, const QSplitter&);
Q_GUI_EXPORT QTextStream& operator>>(QTextStream&, QSplitter&);
#endif

#endif // QT_NO_SPLITTER

#endif // QSPLITTER_H
