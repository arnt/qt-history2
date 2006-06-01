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

/*!
    \class QCompleter
    \brief The QCompleter class provides completions based on a item model.

    You can use QCompleter to provide autocompletions in any Qt
    widget (e.g. QLineEdit, QTextEdit, or QComboBox). When the user
    starts typing a word, QCompleter suggests possible ways of
    completing the word, based on a word list. The word list is
    provided as a QAbstractItemModel. (For simple applications, where
    the word list is static, you can use QStringListModel.)

    Topics:

    \tableofcontents

    \section1 Using QCompleter with Qt widgets
    The QLineEdit and QComboBox can provide autocompletions by setting a QCompleter using
    QLineEdit::setCompleter() and QComboBox::setCompleter().

    For example, to provide completions for a QComboBox from a word list,

    \code
        QComboBox *combo = new combo;
        QStringList wordList;
        wordList << "alpha" << "omega" << "omicron" << "zeta";
        QCompleter *completer = new QCompleter(wordList, combo);
        completer->setCompletionMode(QCompleter::InlineCompletion);
        combo->setCompleter(completer);
        combo->show();
    \endcode

    You can also set a QDirModel to provide autocompletion of filenames in a QLineEdit.
    For example,

    \code
        QLineEdit *lineEdit = new QLineEdit(this);
        QDirModel *dirModel = new QDirModel(this);
        QCompleter *completer = new QCompleter(dirModel, lineEdit);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        lineEdit->setCompleter(completer);
        lineEdit->show();
    \endcode

    \section2 Setting the Model

    To set the model on which QCompleter should operate, call
    setModel(). By default, QCompleter will attempt to match a
    given string against the Qt::EditRole data stored in column 0 in
    the model case sensitively. This can be changed using
    setCompletionRole(), setCompletionColumn(), and setCaseSensitivity().

    If the model is sorted on the column and role that are used for completion,
    you can call setModelSorting() with either QtCompletor::CaseSensitivelySortedModel
    or QCompleter::CaseInsensitivelySortedModel as the argument.
    On large models, this can lead to significant performance
    improvements, because QCompleter can then use binary search
    instead of linear search.

    The model can be a \l{QAbstractListModel}{list model},
    a \l{QAbstractTableModel}{table model}, or a
    \l{QAbstractItemModel}{tree model}. Completion on tree models
    is slightly more involved and is covered in the \l{Handling
    Tree Models} section below.

    \section1 Finding Completions

    For the user, there are two main types of completion, sometimes used
    concurrently:

    \list
    \o \bold{The program suggests the most likely completion to the
       user.} This can happen automatically or be triggered by a
       shortcut key (e.g. \key Tab). Also, some programs let the user
       iterate through the list of candidates using a shortcut key.
    \o \bold{The program lists all the possible completions and lets
       the user choose among them.} The completions are typically
       shown in a QListView popup.
    \endlist

    QCompleter supports both modes of operation. The prefix to be used to
    look for completions is first set using setCompletionPrefix(). You can
    then interate through the completions using setCurrentRow() and currentCompletion().
    Alternatively, you can set up a QListView to display the completionModel()
    provided by QCompleter, and call setCompletionPrefix() to refresh the model.
    We will review both approaches in the following subsections.

    \section2 Getting the Matches One at a Time

    To retrieve a single candidate string, call setCompletionPrefix() with the text
    that needs to be completed and then use currentCompletion().

    Example:

    \code
        QStringList wordList;
        wordList << "alpha" << "omega" << "omicron" << "zeta";
        QStringListModel *wordListModel = new QStringListModel(wordList, this);

        QCompleter *completer = new QCompleter(this);
        completer->setModel(wordListModel);
        completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
        completor->setCaseSensitivity(Qt::CaseInsensitive);

        completer->setCompletionPrefix("OM");
        if (completer->currentIndex().isValid())
            qDebug() << completer->currentCompletion() << "is omega";
    \endcode

    In the above example, we set up a case-insensitive QCompleter
    based on a word list consisting of four words. The setCompletionPrefix() call
    sets up the completer to look for words starting with "OM". In this case,
    the first match is "omega". We can continue iterating using the
    setCurrentRow(). For example:

    \code
        int i = 0;
        while (completer->setCurrentRow(i)) {
            qDebug() << completer->currentCompletion() << " is match number " << i;
            ++i;
        }
    \endcode

    In the above example, we use setCurrentRow() to navigate to the i'th completion and
    obtain the same using currentCompletion(). Note that completionCount() can be used
    to obtain the number of completions (rows). However, for large unsorted models, this
    could be slow since QCompleter needs to iterate through the entire model and find all
    the completions. The above approach uses the incremental building capability of
    QCompleter.

    \section2 Using the Completion Model

    completionModel() return a list model that contains all possible
    completions for the current completion prefix, in the order in which
    they appear in the model.

    Example:

    \code
        QStringList wordList;
        wordList << "alpha" << "omega" << "omicron" << "zeta";

        QCompleter *completor = new QCompleter(wordList, this);
        completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
        completer->setCaseSensitivity(Qt::CaseInsensitive);

        QListView *completionView = new QListView(this);
        completionView->setModel(completor->completionModel());

        completor->setCompletionPrefix("OM");
        qDebug() << completor->completionModel()->rowCount() << "is 2";
    \endcode

    The example sets up a QListView that displays the completion
    model (the list of all candidates). After the call to setCompletionPrefix(),
    the completion model is updated to contain all the words that
    start with "om" ("omega" and "omicron"). As the completion model
    changes, the QListView is automatically updated. In practice, we
    would normally call setCompletionPrefix() every time the user edits the current
    word that is being completed, to update the completion model (and
    the view).

    \section3 Handling Tree Models

    QCompleter can look for completions in tree models, assuming
    that any item (or sub-item or sub-sub-item) can be unambiguously
    represented as a string by specifying the path to the item. The
    completion is then performed one level at a time.

    Let's take the example of a user typing in a file system path.
    The model is a (hierarchical) QDirModel. The completion
    occurs for every element in the path. For example, if the current
    text is \c C:\Wind, QCompleter might suggest \c Windows to
    complete the current path element. Similarly, if the current text
    is \c C:\Windows\Sy, QCompleter might suggest \c System.

    For this kind of completion to work, QCompleter needs to be able to
    split the path into a list of strings that are matched at each level.
    For C:\Windows\Sy, it needs to be split as "C:", "Windows" and "Sy".
    The default implementation of splitPath(), splits the completionPrefix
    using QDir::separator() if the model is a QDirModel. The completion
    text itself is provided by pathFromIndex().
*/

#include "qcompleter_p.h"

void QCompletionModel::setSourceModel(QAbstractItemModel *source)
{
    if (model)
        QObject::disconnect(model, 0, this, 0);

    QAbstractProxyModel::setSourceModel(source);
    model = sourceModel();

    // TODO: Optimize updates in the source model
    connect(model, SIGNAL(modelReset()), this, SLOT(invalidate()));
    connect(model, SIGNAL(layoutChanged()), this, SLOT(invalidate()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(invalidate()));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(invalidate()));
    connect(model, SIGNAL(columnsInserted(const QModelIndex&, int, int)), this, SLOT(invalidate()));
    connect(model, SIGNAL(columnsRemoved(const QModelIndex&, int, int)), this, SLOT(invalidate()));
    connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(invalidate()));

    invalidate();
}

void QCompletionModel::createEngine()
{
    bool sortedEngine = false;
    switch (c->sorting) {
    case QCompleter::UnsortedModel:
        sortedEngine = false;
        break;
    case QCompleter::CaseSensitivelySortedModel:
        sortedEngine = c->cs == Qt::CaseSensitive;
        break;
    case QCompleter::CaseInsensitivelySortedModel:
        sortedEngine = c->cs == Qt::CaseInsensitive;
        break;
    }

    delete engine;
    if (sortedEngine)
        engine = new SortedModelEngine(c);
    else
        engine = new UnsortedModelEngine(c);
}

QModelIndex QCompletionModel::mapToSource(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    int row;
    QModelIndex parent = engine->curParent;
    if (!showAll) {
        if (!engine->matchCount())
            return QModelIndex();
        Q_ASSERT(index.row() < engine->matchCount());
        IndexMapper& rootIndices = engine->rootMatch.indices;
        if (index.row() < rootIndices.count()) {
            row = rootIndices[index.row()];
            parent = QModelIndex();
        } else {
            row = engine->curMatch.indices[index.row() - rootIndices.count()];
        }
    } else {
        row = index.row();
    }

    return model->index(row, index.column(), parent);
}

QModelIndex QCompletionModel::mapFromSource(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return QModelIndex();

    int row = -1;
    if (!showAll) {
        if (!engine->matchCount())
            return QModelIndex();

        IndexMapper& rootIndices = engine->rootMatch.indices;
        if (idx.parent().isValid()) {
            if (idx.parent() != engine->curParent)
                return QModelIndex();
        } else {
            row = rootIndices.indexOf(idx.row());
            if (row == -1 && engine->curParent.isValid())
                return QModelIndex(); // source parent and our parent dont match
        }

        if (row == -1) {
            IndexMapper& indices = engine->curMatch.indices;
            engine->filterOnDemand(idx.row() - indices.last());
            row = indices.indexOf(idx.row()) + rootIndices.count();
        }

        if (row == -1)
            return QModelIndex();
    } else {
        if (idx.parent() != engine->curParent)
            return QModelIndex();
        row = idx.row();
    }

    return createIndex(row, idx.column());
}

bool QCompletionModel::setCurrentRow(int row)
{
    if (row < 0 || !engine->matchCount())
        return false;

    if (row >= engine->matchCount())
        engine->filterOnDemand(row + 1 - engine->matchCount());

    if (row >= engine->matchCount()) // invalid row
        return false;

    engine->curRow = row;
    return true;
}

QModelIndex QCompletionModel::currentIndex(bool sourceIndex) const
{
    if (!engine->matchCount())
        return QModelIndex();

    if (sourceIndex) {
        QModelIndex idx = createIndex(engine->curRow, c->column);
        return mapToSource(idx);
    }

    int row = engine->curRow;
    if (showAll)
        row = engine->curMatch.indices[engine->curRow];
    return createIndex(row, c->column);
}

QModelIndex QCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || column >= columnCount(parent) || parent.isValid())
        return QModelIndex();

    if (!showAll) {
        if (!engine->matchCount())
            return QModelIndex();
        if (row >= engine->rootMatch.indices.count()) {
            engine->filterOnDemand(row + 1 - engine->matchCount());
            if (row >= engine->matchCount())
                return QModelIndex();
        }
    } else {
        if (row >= model->rowCount(engine->curParent))
            return QModelIndex();
    }

    return createIndex(row, column);
}

int QCompletionModel::completionCount() const
{
    if (!engine->matchCount())
        return 0;

    engine->filterOnDemand(INT_MAX);
    return engine->matchCount();
}

int QCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (showAll) {
        // Show all items below the current parent, even if we have no
        // valid matches
        if (engine->curParts.count() != 1  && !engine->matchCount()
            && !engine->curParent.isValid())
            return 0;
        return model->rowCount(engine->curParent);
    }

    return completionCount();
}

void QCompletionModel::setFiltered(bool filtered)
{
    if (showAll == !filtered)
        return;
    showAll = !filtered;
    reset();
}

bool QCompletionModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    if (showAll)
        return model->hasChildren(mapToSource(parent));

    if (!engine->matchCount())
        return false;

    return true;
}

QVariant QCompletionModel::data(const QModelIndex& index, int role) const
{
    return model->data(mapToSource(index), role);
}

void QCompletionModel::invalidate()
{
    engine->cache.clear();
    engine->filter(engine->curParts); // refilter
    reset();
}

void QCompletionModel::filter(const QStringList& parts)
{
    engine->filter(parts);
    reset();
}

///////////////////////////////////////////////
void QCompletionEngine::filter(const QStringList& parts)
{
    const QAbstractItemModel *model = c->proxy->sourceModel();
    curParts = parts;
    if (curParts.isEmpty())
        curParts.append(QString());

    curRow = -1;
    curParent = QModelIndex();
    curMatch = rootMatch = MatchData();

//     if (curParts.count() > 1 && !c->showAll)
//         // when matching on tree models, match the entire text on the root
//         // this is useful when storing histories on the root index
//         rootMatch = filter(c->prefix, QModelIndex(), INT_MAX);

    QModelIndex parent;
    for (int i = 0; i < curParts.count() - 1; i++) {
        QString part = curParts[i];
        int emi = filter(part, parent, -1).exactMatchIndex;
        if (emi == -1)
            return;
        parent = model->index(emi, c->column, parent);
    }

    // Note that we set the curParent to a valid parent, even if we have no matches
    // When filtering is disabled, we show all the items under this parent
    curParent = parent;
    if (curParts.last().isEmpty())
        curMatch = MatchData(IndexMapper(0, model->rowCount(curParent) - 1), -1, false);
    else
        curMatch = filter(curParts.last(), curParent, 1); // build atleast one
    curRow = curMatch.isValid() ? 0 : -1;
}

MatchData QCompletionEngine::matchHint(QString part, const QModelIndex& parent, bool sorted)
{
    const QAbstractItemModel *model = c->proxy->sourceModel();

    if (c->cs == Qt::CaseInsensitive)
        part = part.toLower();

    const CacheItem& map = cache[parent];

    // Try chopping the search string and look them up in the cache
    // (i.e) the matches for 'abcd' will be subset of 'abc', or 'ab' or 'a'
    QString key = part;
    while (!key.isEmpty()) {
        key.chop(1);
        if (map.contains(key))
            return map[key];
    }

    int to = model->rowCount(parent) - 1;

    if (!sorted || part.isEmpty())
        return MatchData(IndexMapper(0, to), -1, false);

    // Try to find a lower and upper bound for the search from previous results
    int from = 0;
    const CacheItem::const_iterator it = map.lowerBound(part);

    // look backward for first valid hint
    for(CacheItem::const_iterator it1 = it; it1-- != map.constBegin();) {
        const MatchData& value = it1.value();
        if (value.isValid()) {
            from = value.indices.last() + 1;
            break;
        }
    }

    // look forward for first valid hint
    for(CacheItem::const_iterator it2 = it; it2 != map.constEnd(); ++it2) {
        const MatchData& value = it2.value();
        if (value.isValid() && !it2.key().startsWith(part)) {
            to = value.indices[0] - 1;
            break;
        }
    }

    return MatchData(IndexMapper(from, to), -1, false);
}

bool QCompletionEngine::lookupCache(QString part, const QModelIndex& parent, MatchData *m)
{
   if (c->cs == Qt::CaseInsensitive)
        part = part.toLower();
   const CacheItem& map = cache[parent];
   if (!map.contains(part))
       return false;
   *m = map[part];
   return true;
}

void QCompletionEngine::saveInCache(QString part, const QModelIndex& parent, const MatchData& m)
{
    // FIXME: Need to regulate the size of cache
    if (c->cs == Qt::CaseInsensitive)
        part = part.toLower();
    cache[parent][part] = m;
}

///////////////////////////////////////////////////////////////////////////////////
MatchData SortedModelEngine::filter(const QString& part, const QModelIndex& parent, int)
{
    // binary search the model within 'range' for 'part' under 'parent'
    const QAbstractItemModel *model = c->proxy->sourceModel();

    MatchData hint;
    if (lookupCache(part, parent, &hint))
        return hint;

    hint = matchHint(part, parent, true);
    if (!hint.isValid())
        return MatchData();

    IndexMapper indices(0, model->rowCount(parent) - 1); // scan entire model
    if (hint.isValid())
        indices = hint.indices; // just scan the hint

    int high = indices.to() + 1;
    int low = indices.from() - 1;
    int probe;
    QModelIndex probeIndex;
    QString probeData;

    while (high - low > 1)
    {
        probe = (high + low) / 2;
        probeIndex = model->index(probe, c->column, parent);
        probeData = model->data(probeIndex, c->role).toString();
        if (QString::compare(probeData, part, c->cs) >= 0)
            high = probe;
        else
            low = probe;
    }

    if (low == indices.to()) { // not found
        saveInCache(part, parent, MatchData());
        return MatchData();
    }

    probeIndex = model->index(low + 1, c->column, parent);
    probeData = model->data(probeIndex, c->role).toString();
    if (!probeData.startsWith(part, c->cs)) {
        saveInCache(part, parent, MatchData());
        return MatchData();
    }

    int emi = QString::compare(probeData, part, c->cs) == 0 ? low+1 : -1;

    int from = low + 1;
    high = indices.to() + 1;
    low = from;

    while (high - low > 1)
    {
        probe = (high + low) / 2;
        probeIndex = model->index(probe, c->column, parent);
        probeData = model->data(probeIndex, c->role).toString();
        if (probeData.startsWith(part, c->cs))
            low = probe;
        else
            high = probe;
    }

    MatchData m(IndexMapper(from, high - 1), emi, false);
    saveInCache(part, parent, m);
    return m;
}

////////////////////////////////////////////////////////////////////////////////////////
int UnsortedModelEngine::buildIndices(const QString& str, const QModelIndex& parent, int n,
                                     const IndexMapper& indices, MatchData* m)
{
    Q_ASSERT(m->partial);
    Q_ASSERT(n != -1 || m->exactMatchIndex == -1);
    const QAbstractItemModel *model = c->proxy->sourceModel();
    int i, count = 0;

    for (i = 0; i < indices.count() && count != n; ++i) {
        QModelIndex idx = model->index(indices[i], c->column, parent);
        QString data = model->data(idx, c->role).toString();
        if (!data.startsWith(str, c->cs))
            continue;
        m->indices.append(indices[i]);
        ++count;
        if (m->exactMatchIndex == -1 && QString::compare(data, str, c->cs) == 0) {
            m->exactMatchIndex = indices[i];
            if (n == -1)
                return indices[i];
        }
    }
    return indices[i-1];
}

void UnsortedModelEngine::filterOnDemand(int n)
{
    Q_ASSERT(matchCount());
    if (!curMatch.partial)
        return;
    const QAbstractItemModel *model = c->proxy->sourceModel();
    int lastRow = model->rowCount(curParent) - 1;
    IndexMapper im(curMatch.indices.last() + 1, lastRow);
    int lastIndex = buildIndices(curParts.last(), curParent, n, im, &curMatch);
    curMatch.partial = (lastRow != lastIndex);
    saveInCache(curParts.last(), curParent, curMatch);
}

MatchData UnsortedModelEngine::filter(const QString& part, const QModelIndex& parent, int n)
{
    MatchData hint;

    QVector<int> v;
    IndexMapper im(v);
    MatchData m(im, -1, true);

    const QAbstractItemModel *model = c->proxy->sourceModel();
    bool foundInCache = lookupCache(part, parent, &m);

    if (!foundInCache) {
        hint = matchHint(part, parent, false);
        if (!hint.isValid())
            return MatchData();
    }

    if (!foundInCache && !hint.isValid()) {
        const int lastRow = model->rowCount(curParent) - 1;
        IndexMapper all(0, lastRow);
        int lastIndex = buildIndices(part, parent, n, all, &m);
        m.partial = (lastIndex != lastRow);
    } else {
        if (!foundInCache) { // build from hint as much as we can
            buildIndices(part, parent, INT_MAX, hint.indices, &m);
            m.partial = hint.partial;
        }
        if (m.partial && ((n == -1 && m.exactMatchIndex == -1) || (m.indices.count() < n))) {
            // need more and have more
            const int lastRow = model->rowCount(curParent) - 1;
            IndexMapper rest(hint.indices.last() + 1, lastRow);
            int want = n == -1 ? -1 : n - m.indices.count();
            int lastIndex = buildIndices(part, parent, want, rest, &m);
            m.partial = (lastRow != lastIndex);
        }
    }

    saveInCache(part, parent, m);
    return m;
}

///////////////////////////////////////////////////////////////////////////////
QCompleterPrivate::QCompleterPrivate()
   : widget(0), proxy(0), popup(0), mode(QCompleter::InlineCompletion),
     cs(Qt::CaseSensitive), role(Qt::EditRole), column(0),
     sorting(QCompleter::UnsortedModel), blockCompletion(false)
{
}

void QCompleterPrivate::init(QWidget *w, QAbstractItemModel *m)
{
    Q_Q(QCompleter);
    Q_ASSERT(w);
    widget = w;
    proxy = new QCompletionModel(this, q);
    q->setModel(m);
    q->setCompletionMode(QCompleter::PopupCompletion);
}

void QCompleterPrivate::_q_selectIndex(const QModelIndex& index)
{
    Q_ASSERT(popup != 0);
    blockCompletion = true;
    if (index.isValid()) {
        popup->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect
                                              | QItemSelectionModel::Rows);
        popup->setCurrentIndex(index);
        popup->scrollTo(index);
    } else
        popup->selectionModel()->clear();
    blockCompletion = false;
}

void QCompleterPrivate::_q_completionActivated(QModelIndex index)
{
    Q_Q(QCompleter);
    QString completion;

    if (!index.isValid())
        completion = prefix;
    else {
        index = proxy->mapToSource(index);
        index = index.sibling(index.row(), column); // for clicked()
        completion = q->pathFromIndex(index);
    }

    emit q->activated(index);
    emit q->activated(completion);
}

void QCompleterPrivate::_q_completionHighlighted(QModelIndex index)
{
    Q_Q(QCompleter);
    if (blockCompletion)
        return;

    QString completion;
    if (!index.isValid())
        completion = prefix;
    else {
        index = proxy->mapToSource(index);
        completion = q->pathFromIndex(index);
    }

    emit q->highlighted(index);
    emit q->highlighted(completion);
}

void QCompleterPrivate::showPopup(QPoint pos)
{
    const QRect screen = QApplication::desktop()->availableGeometry(widget);
    int h = (popup->sizeHintForRow(0) * qMin(7, popup->model()->rowCount()) + 3) + 3;
    int w = widget->width();

    if (pos.isNull())
        pos =  widget->mapToGlobal(QPoint(0, widget->height() - 2));
    if ((pos.x() + w) > (screen.x() + screen.width()))
        pos.setX(screen.x() + screen.width() - w);
    if (pos.x() < screen.x())
        pos.setX(screen.x());
    if (((pos.y() + h) > (screen.y() + screen.height()))
        && ((pos.y() - h - widget->height()) >= 0))
        pos.setY(pos.y() - qMax(h, popup->minimumHeight()) - widget->height());

    popup->setGeometry(pos.x(), pos.y(), w, h);

    if (!popup->isVisible())
        popup->show();
}

/*!
    Constructs a QCompleter object with the given \a parent.
*/
QCompleter::QCompleter(QWidget *parent)
: QObject(*new QCompleterPrivate(), parent)
{
    Q_D(QCompleter);
    d->init(parent);
}

/*!
    Constructs a QCompleter object that completes from \a model
    and with the given \a parent.
*/
QCompleter::QCompleter(QAbstractItemModel *model, QWidget *parent)
    : QObject(*new QCompleterPrivate(), parent)
{
    Q_D(QCompleter);
    d->init(parent, model);
}

/*!
    Constructs a QCompleter object that completes from the \a completions
    and with the given \a parent.
*/
QCompleter::QCompleter(const QStringList& completions, QWidget *parent)
: QObject(*new QCompleterPrivate(), parent)
{
    Q_D(QCompleter);
    d->init(parent, new QStringListModel(completions, this));
}

/*!
    Destroys the QCompleter object.
*/
QCompleter::~QCompleter()
{
}

/*!
    Sets the model from which completions are obtained to \a model.

    \sa completionModel(), modelSorting
*/
void QCompleter::setModel(QAbstractItemModel *model)
{
    Q_D(QCompleter);
    QAbstractItemModel *oldModel = d->proxy->model;
    d->proxy->setSourceModel(model);
    if (d->popup)
        setPopup(d->popup); // set the model and make new connections
    if (oldModel && oldModel->QObject::parent() == this)
        delete oldModel;
}

/*!
    Returns the model from which completions are obtained.

    \sa completionModel()
*/
QAbstractItemModel *QCompleter::model() const
{
    Q_D(const QCompleter);
    return d->proxy->sourceModel();
}

/*
    \enum QCompleter::CompletionMode

    This enum specifies how completions are provided to the user.

    \value PopupCompletion                Displays a popup contains the current completions
    \value InlineCompletion               Completions appear inline (as selected text)
    \value UnfilteredPopupCompletion      Displays a popup containing all the possible completions \
                                          with the most likely suggestion selected in the popup.
*/

/*!
    \property QCompleter::completionMode
    \brief how the completions are provided to the user

    The default value is QCompleter::PopupCompletion.

    \sa QCompleter::CompletionMode
*/
void QCompleter::setCompletionMode(QCompleter::CompletionMode mode)
{
    Q_D(QCompleter);

    if (d->mode == mode)
        return;

    d->mode = mode;
    d->proxy->setFiltered(d->mode != QCompleter::UnfilteredPopupCompletion);

    if (d->mode == QCompleter::InlineCompletion) {
        delete d->popup;
        d->popup = 0;
        d->widget->removeEventFilter(this);
        return;
    }

    d->widget->installEventFilter(this);

    if (!d->popup) { // create default popup
        QTreeView *treeView = new QTreeView;
        treeView->header()->hide();
        treeView->setRootIsDecorated(false);
        treeView->setItemsExpandable(false);
        treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        treeView->setSelectionMode(QAbstractItemView::SingleSelection);
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setPopup(treeView);
    }
}

QCompleter::CompletionMode QCompleter::completionMode() const
{
    Q_D(const QCompleter);
    return d->mode;
}

/*!
    Sets the popup used to display completions to \a popup. QCompleter reparents the
    view to be a child of the widget for which completions are being provided.

    A popup is automatically created when the completionMode() is set to QCompleter::PopupCompletion
    or QCompleter::UnfilteredPopupCompletion. The default popup display all the columns
    in the model. This function can be used to set a custom view that displays selected
    columns in the model (or reorders them).

    Ensure that this function is called before the view settings are modified. This is
    required since view's properties may require that a model has been set on the view
    (for example, hiding certain columns in the view).
*/
void QCompleter::setPopup(QAbstractItemView *popup)
{
    Q_D(QCompleter);
    Q_ASSERT(popup != 0);
    if (d->popup)
        QObject::disconnect(d->popup, 0, this, 0);
    if (d->popup != popup)
        delete d->popup;
    popup->hide();
    popup->setParent(d->widget, Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->installEventFilter(this);
    popup->setModel(d->proxy);
    popup->setMouseTracking(true); // required for entered()

    QObject::connect(popup, SIGNAL(clicked(const QModelIndex&)),
                     this, SLOT(_q_completionActivated(QModelIndex)));
    QObject::connect(popup, SIGNAL(clicked(const QModelIndex&)), popup, SLOT(hide()));
    QObject::connect(popup, SIGNAL(entered(const QModelIndex&)),
                     this, SLOT(_q_selectIndex(const QModelIndex&)));
    QObject::connect(popup->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                     this, SLOT(_q_completionHighlighted(QModelIndex)));
    d->popup = popup;
}

/*!
    Returns the popup used to display completions.
*/
QAbstractItemView *QCompleter::popup() const
{
    Q_D(const QCompleter);
    return d->popup;
}

/*!
  \reimp
*/
bool QCompleter::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QCompleter);

    if (o == d->widget && e->type() == QEvent::FocusOut) {
        if (d->popup && d->popup->isVisible())
                return true;
    }

    if (o != d->popup)
        return QObject::eventFilter(o, e);

    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        QModelIndex curIndex = d->popup->currentIndex();
        switch (ke->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            d->popup->hide();
            d->_q_completionActivated(curIndex);
            return true;

        case Qt::Key_Tab:
            d->popup->hide();
            if (d->mode == QCompleter::PopupCompletion)
                break;
            d->_q_completionActivated(curIndex);
            return true;

        case Qt::Key_Backtab:
            d->popup->hide();
            break;

        case Qt::Key_F4:
            if (ke->modifiers() & Qt::AltModifier) {
                d->popup->hide();
                return true;
            }
            break;

        case Qt::Key_Escape:
            d->popup->hide();
            return true;

        case Qt::Key_End:
        case Qt::Key_Home:
            if (ke->modifiers() & Qt::ControlModifier)
                return false;
            break;

        case Qt::Key_Up:
            if (!curIndex.isValid()) {
                int rowCount = d->proxy->rowCount();
                QModelIndex lastIndex = d->proxy->index(rowCount - 1, 0);
                d->_q_selectIndex(lastIndex);
                d->_q_completionHighlighted(lastIndex);
                return true;
            } else if (curIndex.row() == 0) {
                d->_q_selectIndex(QModelIndex());
                d->_q_completionHighlighted(QModelIndex());
                return true;
            }
            return false;

        case Qt::Key_Down:
            if (!curIndex.isValid()) {
                QModelIndex firstIndex = d->proxy->index(0, 0);
                d->_q_selectIndex(firstIndex);
                d->_q_completionHighlighted(firstIndex);
                return true;
            } else if (curIndex.row() == d->proxy->rowCount() - 1) {
                d->_q_selectIndex(QModelIndex());
                d->_q_completionHighlighted(QModelIndex());
                return true;
            }
            return false;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            return false;

        default:
            break;
        }

        QApplication::sendEvent(d->widget, ke);
        return true;
    }

    case QEvent::MouseButtonPress:
        if (!d->popup->underMouse()) {
            d->popup->hide();
            return true;
        }
        return false;

    default:
        return false;
    }
}

/*!
    Triggers auto completions. The point \a globalPos can be specified to
    indicate the position of the popup for QCompleter::UnfilteredPopupCompletion
    and QCompleter::PopupCompletion. If \a globalPos is invalid, QCompleter
    positions the popup at the bottom of the widget.
*/
void QCompleter::complete(const QPoint& globalPos)
{
    Q_D(QCompleter);
    QModelIndex idx = d->proxy->currentIndex(false);
    if (d->mode == QCompleter::InlineCompletion) {
        if (idx.isValid())
            d->_q_completionHighlighted(idx);
        return;
    }

    if ((d->mode == QCompleter::PopupCompletion && !idx.isValid())
        || (d->mode == QCompleter::UnfilteredPopupCompletion && d->proxy->rowCount() == 0)) {
        d->popup->hide(); // no suggestion, hide
    } else {
        if (d->mode == QCompleter::UnfilteredPopupCompletion)
            d->_q_selectIndex(idx); // select the probable completion
        d->showPopup(globalPos);
    }
}

/*!
    Sets the current row to \a row. This function may be used along with
    currentCompletion() to iterate through all the possible completions.
*/
bool QCompleter::setCurrentRow(int row)
{
    Q_D(QCompleter);
    return d->proxy->setCurrentRow(row);
}

/*!
    Returns the current row

    \sa setCurrentRow
*/
int QCompleter::currentRow() const
{
    Q_D(const QCompleter);
    return d->proxy->currentRow();
}

/*!
  Returns the number of completions for the current prefix. For an unsorted model with
  a large number of items this call can be expensive.
*/
int QCompleter::completionCount() const
{
    Q_D(const QCompleter);
    return d->proxy->completionCount();
}

/*!
    \enum QCompleter::ModelSorting

    This enum specifies how the items in the model is sorted

    \value UnsortedModel                    The model is unsorted
    \value CaseSensitivelySortedModel       The model is sorted case sensitively
    \value CaseInsensitivelySortedModel     The model is sorted case insensitively

    \sa completionRole, completionColumn
*/

/*!
    \property QCompleter::sourceModelSorting
    \brief whether and how the model is sorted

    By default, no assumptions are made about the order of the items
    in the model from which completions are obtained. If the model's data
    for the matchColumn() and matchRole() is sorted in ascending order, you
    can set this property to CaseSensitivelySortedModel or
    CaseInsensitivelySortedModel. On large models, this can lead to
    significant performance improvements, because QCompleter can
    then use binary search instead of linear search. (Be aware that
    these performance improvements cannot take place when
    QCompleter's caseSensitivity and the model's sorting case
    differ.)

    \sa setCaseSensitivity(), QCompleter::ModelSorting
*/
void QCompleter::setModelSorting(QCompleter::ModelSorting sorting)
{
    Q_D(QCompleter);
    if (d->sorting == sorting)
        return;
    d->sorting = sorting;
    d->proxy->createEngine();
    d->proxy->invalidate();
}

QCompleter::ModelSorting QCompleter::modelSorting() const
{
    Q_D(const QCompleter);
    return d->sorting;
}

/*!
    \property QCompleter::completionColumn
    \brief the column in the model in which completions are searched for

    By default, the match column is 0.

    \sa completionRole, caseSensitivity
*/
void QCompleter::setCompletionColumn(int column)
{
    Q_D(QCompleter);
    if (d->column == column)
        return;
    d->column = column;
    d->proxy->invalidate();
}

int QCompleter::completionColumn() const
{
    Q_D(const QCompleter);
    return d->column;
}

/*!
    \property QCompleter::completionRole
    \brief the item role to be used to query the contents of items for matching.

    The default role is Qt::EditRole.

    \sa completionColumn, caseSensitivity
*/
void QCompleter::setCompletionRole(int role)
{
    Q_D(QCompleter);
    if (d->role == role)
        return;
    d->role = role;
    d->proxy->invalidate();
}

int QCompleter::completionRole() const
{
    Q_D(const QCompleter);
    return d->role;
}

/*!
    \property QCompleter::caseSensitivity
    \brief the case sensitivity of the matching

    The default is Qt::CaseSensitive.

    \sa completionColumn, completionRole, modelSorting
*/
void QCompleter::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    Q_D(QCompleter);
    if (d->cs == cs)
        return;
    d->cs = cs;
    d->proxy->createEngine();
    d->proxy->invalidate();
}

Qt::CaseSensitivity QCompleter::caseSensitivity() const
{
    Q_D(const QCompleter);
    return d->cs;
}

/*!
    \property QCompleter::completionPrefix
    \brief the completion prefix used to provide completions

    The completion model is updated to reflect the list of possible
    matches for \a prefix.
*/
void QCompleter::setCompletionPrefix(const QString &prefix)
{
    Q_D(QCompleter);
    d->prefix = prefix;
    d->proxy->filter(splitPath(prefix));
}

QString QCompleter::completionPrefix() const
{
    Q_D(const QCompleter);
    return d->prefix;
}

/*!
    Returns the current completion index.

    \sa setCurrentRow()
*/
QModelIndex QCompleter::currentIndex() const
{
    Q_D(const QCompleter);
    return d->proxy->currentIndex(true);
}

/*!
    Returns the current completion string. This includes the completionPrefix.
    When used alongside setCurrentRow(), it can be used to iterate through
    all the matches.

    \sa setCurrentRow()
*/
QString QCompleter::currentCompletion() const
{
    return pathFromIndex(currentIndex());
}

/*!
    Returns the completion model. The completion model is a list
    model that contains all the possible matches for the current
    completion prefix.

    \sa completionPrefix, model()
*/
const QAbstractProxyModel *QCompleter::completionModel() const
{
    Q_D(const QCompleter);
    return d->proxy;
}

/*!
    Returns the path from index \a index. QCompleter uses this
    to provide the completion text for the index \a index.

    The default implementation returns the Qt::Edit role of the
    item for list models. It returns the absolute file path
    if the model is a QDirModel.

    \sa splitPath()
*/
QString QCompleter::pathFromIndex(const QModelIndex& index) const
{
    Q_D(const QCompleter);
    if (!index.isValid())
        return QString();

    QAbstractItemModel *sourceModel = d->proxy->sourceModel();
    QDirModel *dirModel = qobject_cast<QDirModel *>(sourceModel);
    if (!dirModel)
        return sourceModel->data(index, Qt::EditRole).toString();

    // FIXME: Investigate bug in QDirModel returning wrong filePath
    // return QDir::convertSeparators(dirModel->filePath(index));

    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = sourceModel->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

#ifndef Q_OS_WIN
    if (list.count() == 1) // only the separator or some other text
        return list[0];
    list[0].clear() ; // the join below will provide the separator
#endif

    return list.join(QDir::separator());
}

/*!
    Splits \a path into strings that are used to match at each level in the source model.
    The default implementation of splitPath() splits a file system path based on
    QDir::separator() when the sourceModel() is a QDirModel.

    Note that the \a path can be modified in any way (for example it can be capitalized).
    When used with list models, the first item in the returned list is used for matching.

    \sa pathFromIndex(), {Handling Tree Models}
*/
QStringList QCompleter::splitPath(const QString& path) const
{
    Q_D(const QCompleter);
    bool isDirModel = qobject_cast<QDirModel *>(d->proxy->sourceModel()) != 0;

    if (!isDirModel || path.isEmpty())
        return QStringList(completionPrefix());

    QString sep = QDir::separator();
#ifdef Q_OS_WIN
    sep += QLatin1Char('/');
#endif

    QRegExp re("[" + QRegExp::escape(sep) + "]");
    QStringList parts = path.split(re);

#ifndef Q_OS_WIN
    if (path[0] == sep[0]) // readd the "/" at the beginning as the split would have removed it
        parts[0] = sep[0];
#else
    if (path.startsWith(QLatin1String("\\\\"))) { // network share
        if (parts.isEmpty())
            parts[0] = QLatin1String("\\\\");
        else
            parts[0].prepend(QLatin1String("\\\\"));
    }
#endif

    return parts;
}

#include "moc_qcompleter.cpp"
