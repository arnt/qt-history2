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

#include "QtGui/qframe.h"
#include "QtGui/qsizepolicy.h"

QT_MODULE(Gui)

#ifndef QT_NO_SPLITTER

class QSplitterPrivate;
class QTextStream;
template <typename T> class QList;

class QSplitterHandle;

class Q_GUI_EXPORT QSplitter : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool opaqueResize READ opaqueResize WRITE setOpaqueResize)
    Q_PROPERTY(int handleWidth READ handleWidth WRITE setHandleWidth)
    Q_PROPERTY(bool childrenCollapsible READ childrenCollapsible WRITE setChildrenCollapsible)

public:
    explicit QSplitter(QWidget* parent = 0);
    explicit QSplitter(Qt::Orientation, QWidget* parent = 0);
    ~QSplitter();

    void addWidget(QWidget *widget);
    void insertWidget(int index, QWidget *widget);

    void setOrientation(Qt::Orientation);
    Qt::Orientation orientation() const;

    void setChildrenCollapsible(bool);
    bool childrenCollapsible() const;

    void setCollapsible(int index, bool);
    bool isCollapsible(int index) const;
    void setOpaqueResize(bool opaque = true);
    bool opaqueResize() const;
    void refresh();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QList<int> sizes() const;
    void setSizes(const QList<int> &list);

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    int handleWidth() const;
    void setHandleWidth(int);

    int indexOf(QWidget *w) const;
    QWidget *widget(int index) const;
    int count() const;

    void getRange(int index, int *, int *) const;
    QSplitterHandle *handle(int index) const;

    void setStretchFactor(int index, int stretch);

Q_SIGNALS:
    void splitterMoved(int pos, int index);

protected:
    virtual QSplitterHandle *createHandle();

    void childEvent(QChildEvent *);

    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);

    void changeEvent(QEvent *);
    void moveSplitter(int pos, int index);
    void setRubberBand(int position);
    int closestLegalPosition(int, int);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QSplitter(QWidget* parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QSplitter(Qt::Orientation, QWidget* parent, const char* name);
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint, Auto };
    QT3_SUPPORT void setResizeMode(QWidget *w, ResizeMode mode);
    inline QT3_SUPPORT void moveToFirst(QWidget *w) { insertWidget(0,w); }
    inline QT3_SUPPORT void moveToLast(QWidget *w) { addWidget(w); }
    inline QT3_SUPPORT void setCollapsible(QWidget *w, bool collapse)
    { setCollapsible(indexOf(w), collapse); }
#endif

private:
    Q_DISABLE_COPY(QSplitter)
    Q_DECLARE_PRIVATE(QSplitter)
private:
    friend class QSplitterHandle;
};

//#ifdef QT3_SUPPORT
#ifndef QT_NO_TEXTSTREAM
Q_GUI_EXPORT QTextStream& operator<<(QTextStream&, const QSplitter&);
Q_GUI_EXPORT QTextStream& operator>>(QTextStream&, QSplitter&);
#endif
//#endif

class QSplitterHandlePrivate;
class Q_GUI_EXPORT QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle(Qt::Orientation o, QSplitter *parent);
    void setOrientation(Qt::Orientation o);
    Qt::Orientation orientation() const;
    bool opaqueResize() const;
    QSplitter *splitter() const;

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent *);

    void moveSplitter(int p);
    int closestLegalPosition(int p);

private:
    Q_DISABLE_COPY(QSplitterHandle)
    Q_DECLARE_PRIVATE(QSplitterHandle)
};

#endif // QT_NO_SPLITTER

#endif // QSPLITTER_H
