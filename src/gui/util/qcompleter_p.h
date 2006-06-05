/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMPLETER_P_H
#define QCOMPLETER_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qobject_p.h"
#include "QtGui/qtreeview.h"
#include "QtGui/qabstractproxymodel.h"
#include "QtGui/qstringlistmodel.h"
#include "qcompleter.h"
#include "QtGui/qapplication.h"
#include "QtGui/qevent.h"
#include "QtGui/qheaderview.h"
#include "QtGui/qdesktopwidget.h"
#include "QtGui/qtreeview.h"
#include "QtGui/qdirmodel.h"
#include "QtGui/qheaderview.h"

class QCompletionModel;

class QCompleterPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCompleter)

public:
    QCompleterPrivate();
    ~QCompleterPrivate() { }
    void init(QWidget *widget, QAbstractItemModel *model = 0);

    QWidget *widget;
    QCompletionModel *proxy;
    QAbstractItemView *popup;
    QCompleter::CompletionMode mode;

    QString prefix;
    Qt::CaseSensitivity cs;
    int role;
    int column;
    QCompleter::ModelSorting sorting;

    bool blockCompletion;

    void showPopup(QPoint);
    void _q_completionActivated(QModelIndex);
    void _q_completionHighlighted(QModelIndex);
    void _q_selectIndex(const QModelIndex&);
};

class IndexMapper
{
public:
    IndexMapper() : v(false), f(0), t(-1) { }
    IndexMapper(int f, int t) : v(false), f(f), t(t) { }
    IndexMapper(QVector<int> vec) : v(true), vector(vec), f(-1), t(-1) { }

    inline int count() const { return v ? vector.count() : t - f + 1; }
    inline int operator[] (int index) const { return v ? vector[index] : f + index; }
    inline int indexOf(int x) const { return v ? vector.indexOf(x) : ((t < f) ? -1 : x - f); }
    inline bool isValid() const { return !isEmpty(); }
    inline bool isEmpty() const { return v ? vector.isEmpty() : (t < f); }
    inline void append(int x) { Q_ASSERT(v); vector.append(x); }
    inline int last() const { return v ? vector.last() : t; }
    inline int from() const { Q_ASSERT(!v); return f; }
    inline int to() const { Q_ASSERT(!v); return t; }

private:
    bool v;
    QVector<int> vector;
    int f, t;
};

struct MatchData {
    MatchData() : exactMatchIndex(-1) { }
    MatchData(const IndexMapper& indices, int em, bool p) : 
        indices(indices), exactMatchIndex(em), partial(p) { }
    IndexMapper indices;
    inline bool isValid() const { return indices.isValid(); }
    int  exactMatchIndex;
    bool partial;
};

class QCompletionEngine
{
public:
    typedef QMap<QString, MatchData> CacheItem;
    typedef QMap<QModelIndex, CacheItem> Cache;

    QCompletionEngine(QCompleterPrivate *c) : c(c), curRow(-1) { }
    virtual ~QCompletionEngine() { }

    void filter(const QStringList &parts);

    MatchData matchHint(QString, const QModelIndex&, bool);

    void saveInCache(QString, const QModelIndex&, const MatchData&);
    bool lookupCache(QString part, const QModelIndex& parent, MatchData *m);

    virtual void filterOnDemand(int) { }
    virtual MatchData filter(const QString&, const QModelIndex&, int) = 0;

    int matchCount() const { return curMatch.indices.count() + rootMatch.indices.count(); }

    MatchData curMatch, rootMatch;
    QCompleterPrivate *c;
    QStringList curParts;
    QModelIndex curParent;
    int curRow;

    Cache cache;
};

class SortedModelEngine : public QCompletionEngine
{
public:
    SortedModelEngine(QCompleterPrivate *c) : QCompletionEngine(c) { }
    MatchData filter(const QString&, const QModelIndex&, int);
};

class UnsortedModelEngine : public QCompletionEngine
{
public:
    UnsortedModelEngine(QCompleterPrivate *c) : QCompletionEngine(c) { }

    void filterOnDemand(int);
    MatchData filter(const QString&, const QModelIndex&, int);
private:
    int buildIndices(const QString& str, const QModelIndex& parent, int n, 
                     const IndexMapper& iv, MatchData* m);
};

class QCompletionModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    QCompletionModel(QCompleterPrivate *c, QObject *parent) : 
        QAbstractProxyModel(parent), c(c), model(0), engine(0), showAll(false) 
    { createEngine(); }
    ~QCompletionModel() { delete engine; }

    void createEngine();
    void setFiltered(bool);
    void filter(const QStringList& parts);
    int completionCount() const;
    int currentRow() const { return engine->curRow; }
    bool setCurrentRow(int row);
    QModelIndex currentIndex(bool) const;

    QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const;
    int rowCount(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex& = QModelIndex()) const { return model->columnCount(); }
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & = QModelIndex()) const { return QModelIndex(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void setSourceModel(QAbstractItemModel *sourceModel);
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

    QCompleterPrivate *c;
    QAbstractItemModel *model;
    QCompletionEngine *engine;
    bool showAll;

public Q_SLOTS:
    void invalidate();
};

#endif // QCOMPLETER_P_H
