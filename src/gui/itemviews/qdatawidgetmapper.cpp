#include "qdatawidgetmapper.h"

#include "qabstractitemmodel.h"
#include "qitemdelegate.h"
#include "qmetaobject.h"
#include "qwidget.h"
#include "private/qobject_p.h"

class QDataWidgetMapperPrivate: public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QDataWidgetMapper)

    QDataWidgetMapperPrivate()
        : delegate(0), orientation(Qt::Horizontal), submitPolicy(QDataWidgetMapper::AutoSubmit)
    {
    }

    QPointer<QAbstractItemModel> model;
    QAbstractItemDelegate *delegate;
    Qt::Orientation orientation;
    QDataWidgetMapper::SubmitPolicy submitPolicy;
    QPersistentModelIndex rootIndex;
    QPersistentModelIndex currentTopLeft;

    inline int itemCount()
    {
        return orientation == Qt::Horizontal
            ? model->rowCount(rootIndex)
            : model->columnCount(rootIndex);
    }

    inline int currentIdx() const
    {
        return orientation == Qt::Horizontal ? currentTopLeft.row() : currentTopLeft.column();
    }

    inline QModelIndex indexAt(int itemPos)
    {
        return orientation == Qt::Horizontal
            ? model->index(currentIdx(), itemPos, rootIndex)
            : model->index(itemPos, currentIdx(), rootIndex);
    }

    inline void flipEventFilters(QAbstractItemDelegate *oldDelegate,
                                 QAbstractItemDelegate *newDelegate)
    {
        for (int i = 0; i < widgetMap.count(); ++i) {
            QWidget *w = widgetMap.at(i).widget;
            if (!w)
                continue;
            w->removeEventFilter(oldDelegate);
            w->installEventFilter(newDelegate);
        }
    }

    void populate();

    // private slots
    void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void _q_commitData(QWidget *);
    void _q_closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint);

    struct WidgetMapper
    {
        inline WidgetMapper(QWidget *w = 0, int c = 0)
            : widget(w), section(c) {}

        QPointer<QWidget> widget;
        int section;
        QPersistentModelIndex currentIndex;
    };
    void populate(WidgetMapper &m);
    int findWidget(QWidget *w) const;

    QList<WidgetMapper> widgetMap;
};

int QDataWidgetMapperPrivate::findWidget(QWidget *w) const
{
    for (int i = 0; i < widgetMap.count(); ++i) {
        if (widgetMap.at(i).widget == w)
            return i;
    }
    return -1;
}

void QDataWidgetMapperPrivate::populate(WidgetMapper &m)
{
    if (m.widget.isNull())
        return;

    m.currentIndex = indexAt(m.section);
    delegate->setEditorData(m.widget, m.currentIndex);
}

void QDataWidgetMapperPrivate::populate()
{
    Q_ASSERT(!model.isNull());

    for (int i = 0; i < widgetMap.count(); ++i)
        populate(widgetMap[i]);
}

static bool qContainsIndex(const QModelIndex &idx, const QModelIndex &topLeft,
                           const QModelIndex &bottomRight)
{
    return idx.row() >= topLeft.row() && idx.row() <= bottomRight.row()
           && idx.column() >= topLeft.column() && idx.column() <= bottomRight.column();
}

void QDataWidgetMapperPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.parent() != rootIndex)
        return; // not in our hierarchy

    for (int i = 0; i < widgetMap.count(); ++i) {
        WidgetMapper &m = widgetMap[i];
        if (qContainsIndex(m.currentIndex, topLeft, bottomRight))
            populate(m);
    }
}

void QDataWidgetMapperPrivate::_q_commitData(QWidget *w)
{
    if (submitPolicy == QDataWidgetMapper::ManualSubmit)
        return;

    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    delegate->setModelData(w, model, widgetMap.at(idx).currentIndex);
}

class QFocusHelper: public QWidget
{
public:
    bool focusNextPrevChild(bool next)
    {
        return QWidget::focusNextPrevChild(next);
    }

    static inline void focusNextPrevChild(QWidget *w, bool next)
    {
        static_cast<QFocusHelper *>(w)->focusNextPrevChild(next);
    }
};

void QDataWidgetMapperPrivate::_q_closeEditor(QWidget *w, QAbstractItemDelegate::EndEditHint hint)
{
    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    switch (hint) {
    case QAbstractItemDelegate::RevertModelCache: {
        const WidgetMapper &m = widgetMap.at(idx);
        delegate->setEditorData(m.widget, m.currentIndex);
        break; }
    case QAbstractItemDelegate::EditNextItem:
        QFocusHelper::focusNextPrevChild(w, true);
        break;
    case QAbstractItemDelegate::EditPreviousItem:
        QFocusHelper::focusNextPrevChild(w, false);
        break;
    case QAbstractItemDelegate::SubmitModelCache:
    case QAbstractItemDelegate::NoHint:
        // nothing
        break;
    }
}

/*!
    \class QDataWidgetMapper qdatawidgetmapper.h

    \brief The QDataWidgetMapper class provides mapping between a section
    of a data model to widgets.

    \since 4.2

    \ingroup model-view

    QDataWidgetMapper can be used to create data-aware widgets by mapping
    them to sections of an item model. A section is a column of a model
    if the orientation is horizontal (the default), otherwise a row.

    Every time the current index changes, all widgets are updated
    with the contents from the model. If user edits the contents of
    the widget, the changes are written back to the model.

    It is possible to set an item delegate to support custom widgets. By default,
    a QItemDelegate is used to synchronize the model with the widgets.

    Let us assume that we have an item model named \c{model} with the following contents:

    \table
    \row \o 1 \o Trolltech AS    \o Oslo
    \row \o 2 \o Trolltech Pty   \o Brisbane
    \row \o 3 \o Trolltech Inc   \o Palo Alto
    \row \o 4 \o Trolltech China \o Beijing
    \row \o 5 \o Trolltech GmbH  \o Berlin
    \endtable

    The following code will map the columns of the model to widgets called \c mySpinBox,
    \c myLineEdit and \c{myCountryChooser}:

    \code
    QDataWidgetMapper *mapper = new QDataWidgetMapper;
    mapper->setModel(model);
    mapper->addMapping(mySpinBox, 0);
    mapper->addMapping(myLineEdit, 1);
    mapper->addMapping(myCountryChooser, 2);
    mapper->first();
    \endcode

    After the call to first(), \c mySpinBox displays the value \c{1}, \c myLineEdit
    displays \c {Trolltech AS} and \c myCountryChooser displays \c{Oslo}. The
    navigational functions first(), next(), previous(), last() and setCurrentIndex()
    can be used to navigate in the model and update the widgets with contents from
    the model.

    QDataWidgetMapper supports two submit policies, \c AutoSubmit and \c{ManualSubmit}.
    \c AutoSubmit will update the model as soon as the current widget loses focus,
    \c ManualSubmit will not update the model unless submit() is called. \c ManualSubmit
    is useful when displaying a dialog that lets the user cancel all modifications.
    Also, other views that display the model won't update until the user finishes
    all his modifications and submits.

    Note that QDataWidgetMapper keeps track of external modifications. If the contents
    of the model are updated in another module of the application, the widgets are
    updated as well.

    \sa QAbstractItemModel, QAbstractItemDelegate
 */

/*! \enum QDataWidgetMapper::SubmitPolicy

    This enum describes the possible submit policies a QDataWidgetMapper
    supports.

    \value AutoSubmit    Whenever a widget loses focus, the widget's current
                         value is set to the item model.
    \value ManualSubmit  The model is not updated until submit() is called.
 */

/*!
    \fn void currentIndexChanged(int index)

    This signal is emitted after the current index has changed and all widgets
    were populated with new data.

    \sa currentIndex(), setCurrentIndex()
 */

/*!
    Constructs a new QDataWidgetMapper with parent object \a parent.
    By default, the orientation is horizontal and the submit policy
    is \c{AutoSubmit}.

    \sa setOrientation(), setSubmitPolicy()
 */
QDataWidgetMapper::QDataWidgetMapper(QObject *parent)
    : QObject(*new QDataWidgetMapperPrivate, parent)
{
    setItemDelegate(new QItemDelegate(this));
}

/*!
    Destroys the object.
 */
QDataWidgetMapper::~QDataWidgetMapper()
{
}

/*!
     Sets the current model to \a model. If another model was set,
     all mappings to that old model are cleared.

     \sa model()
 */
void QDataWidgetMapper::setModel(QAbstractItemModel *model)
{
    Q_D(QDataWidgetMapper);

    if (d->model == model)
        return;

    if (d->model)
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this,
                   SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
    clearMapping();
    d->rootIndex = QModelIndex();
    d->currentTopLeft = QModelIndex();

    d->model = model;

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
}

/*!
    Returns the current model.

    \sa setModel()
 */
QAbstractItemModel *QDataWidgetMapper::model() const
{
    Q_D(const QDataWidgetMapper);
    return d->model;
}

/*!
    Sets the item delegate to \a delegate. The delegate will be used to write
    data from the model into the widget and from the widget to the model,
    using QAbstractItemDelegate::setEditorData() and QAbstractItemDelegate::setModelData().

    The delegate also decides when to apply data and when to change the editor,
    using QAbstractItemDelegate::commitData() and QAbstractItemDelegate::closeEditor().

    \sa QAbstractItemDelegate, itemDelegate()
 */
void QDataWidgetMapper::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_D(QDataWidgetMapper);
    QAbstractItemDelegate *oldDelegate = d->delegate;
    if (oldDelegate) {
        disconnect(oldDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(_q_commitData(QWidget*)));
        disconnect(oldDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(_q_closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
    }

    d->delegate = delegate;

    if (delegate) {
        connect(delegate, SIGNAL(commitData(QWidget*)), SLOT(_q_commitData(QWidget*)));
        connect(delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                SLOT(_q_closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
    }

    d->flipEventFilters(oldDelegate, delegate);
}

/*!
    Returns the current item delegate.

    \sa setDelegate()
 */
QAbstractItemDelegate *QDataWidgetMapper::itemDelegate() const
{
    Q_D(const QDataWidgetMapper);
    return d->delegate;
}

/*!
    Sets the root item to \a index. This can be used to display
    a branch of a tree. Pass an invalid model index to display
    the top-most branch.

    \sa rootIndex()
 */
void QDataWidgetMapper::setRootIndex(const QModelIndex &index)
{
    Q_D(QDataWidgetMapper);
    d->rootIndex = index;
}

/*!
    Returns the current root index.

    \sa setRootIndex()
*/
QModelIndex QDataWidgetMapper::rootIndex() const
{
    Q_D(const QDataWidgetMapper);
    return d->rootIndex;
}

/*!
    Adds a mapping between a \a widget and a \section from the model.
    The \a section is a column in the model if the orientation is
    horizontal (the default), otherwise a row.

    For the following example, we assume a model \c myModel that
    has two columns, the first one containing the name of a person,
    the second column his age. The first column is mapped to the
    QLineEdit \c nameLineEdit and the second to the QSpinBox
    \c{ageSpinBox}:

    \code
    QDataWidgetMapper *mapper = new QDataWidgetMapper();
    mapper->setModel(myModel);
    mapper->addMapping(nameLineEdit, 0);
    mapper->addMapping(ageSpinBox, 1);
    \endcode

    Note: If the \a widget is already mapped to a section, the
    old mapping will be replaced by the new one. A widget can never
    be mapped to more than one section at a time.

    \sa removeMapping(), mappedSection(), clearMapping()
 */
void QDataWidgetMapper::addMapping(QWidget *widget, int section)
{
    Q_D(QDataWidgetMapper);

    removeMapping(widget);
    d->widgetMap.append(QDataWidgetMapperPrivate::WidgetMapper(widget, section));
    widget->installEventFilter(d->delegate);
}

/*!
    Removes the mapping for the given \a widget.

    \sa addMapping(), clearMapping()
 */
void QDataWidgetMapper::removeMapping(QWidget *widget)
{
    Q_D(QDataWidgetMapper);

    int idx = d->findWidget(widget);
    if (idx == -1)
        return;

    d->widgetMap.removeAt(idx);
    widget->removeEventFilter(d->delegate);
}

/*!
    Returns the section the \a widget is mapped to or -1
    if the widget is not mapped.

    \sa addMapping(), removeMapping()
 */
int QDataWidgetMapper::mappedSection(QWidget *widget) const
{
    Q_D(const QDataWidgetMapper);

    int idx = d->findWidget(widget);
    if (idx == -1)
        return -1;

    return d->widgetMap.at(idx).section;
}

/*!
    Repopulates all widgets with the current data of the model.
    All unsubmitted changes will be lost.

    \sa submit(), setSubmitPolicy()
 */
void QDataWidgetMapper::revert()
{
    Q_D(QDataWidgetMapper);

    if (d->model.isNull())
        return;
    d->populate();
}

/*!
    Submits all changes from the mapped widgets to the model.

    For every mapped section, the item delegate reads the current
    value from the widget and sets it in the model. Finally, the
    model's submit() method is invoked.

    Returns true if all the values were submitted, otherwise false.

    Note: For database models, QSqlQueryModel::lastError() can be
    used to retrieve the last error.

    \sa revert(), setSumbitPolicy()
 */
bool QDataWidgetMapper::submit()
{
    Q_D(QDataWidgetMapper);

    if (d->model.isNull())
        return false;

    for (int i = 0; i < d->widgetMap.count(); ++i) {
        const QDataWidgetMapperPrivate::WidgetMapper &m = d->widgetMap.at(i);
        if (m.widget.isNull())
            continue;
        if (!m.currentIndex.isValid())
            return false;
        d->delegate->setModelData(m.widget, d->model, m.currentIndex);
    }

    return d->model->submit();
}

/*!
    Populates the widgets with data from the first row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the first column.

    This is equivalent to calling setCurrentIndex(0).

    \sa last(), setCurrentIndex()
 */
void QDataWidgetMapper::first()
{
    setCurrentIndex(0);
}

/*!
    Populates the widgets with data from the last row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the last column.

    Calls setCurrentIndex() internally.

    \sa last(), setCurrentIndex()
 */
void QDataWidgetMapper::last()
{
    Q_D(QDataWidgetMapper);
    setCurrentIndex(d->itemCount() - 1);
}


/*!
    Populates the widgets with data from the next row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the next column.

    Calls setCurrentIndex() internally. Does nothing if there is
    no next row in the model.

    \sa previous(), setCurrentIndex()
 */
void QDataWidgetMapper::next()
{
    Q_D(QDataWidgetMapper);
    setCurrentIndex(d->currentIdx() + 1);
}

/*!
    Populates the widgets with data from the previous row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the previous column.

    Calls setCurrentIndex() internally. Does nothing if there is
    no previous row in the model.

    \sa next(), setCurrentIndex()
 */
void QDataWidgetMapper::previous()
{
    Q_D(QDataWidgetMapper);
    setCurrentIndex(d->currentIdx() - 1);
}

/*!
    Populates the widgets with data from the row at \a index
    if the orientation is horizontal (the default), otherwise
    with data from the column at \a index.

    Does nothing if no such row/column exists.

    \sa setCurrentModelIndex(), first(), next(), previous(), last()
 */
void QDataWidgetMapper::setCurrentIndex(int index)
{
    Q_D(QDataWidgetMapper);

    if (d->model.isNull() || index >= d->itemCount())
        return;
    d->currentTopLeft = d->orientation == Qt::Horizontal
                            ? d->model->index(index, 0, d->rootIndex)
                            : d->model->index(0, index, d->rootIndex);
    d->populate();

    emit currentIndexChanged(index);
}

/*
    Sets the current index to the row of the \a index if the
    orientation is horizontal (the default), otherwise to the
    column of the \a index.

    Calls setCurrentIndex() internally. This convenience slot
    can be connected to the signal currentRowChanged() or
    currentColumnChanged() of another view's selection model.

    The following example illustrates how to update all widgets
    with new data whenever the selection of a QTableView named
    \c myTableView changes:

    \code
    QDataWidgetMapper *mapper = new QDataWidgetMapper();
    connect(myTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex)),
            mapper, SLOT(setCurrentModelIndex(QModelIndex)));
    \endcode

    \sa currentModelIndex()
 */
void QDataWidgetMapper::setCurrentModelIndex(const QModelIndex &index)
{
    Q_D(QDataWidgetMapper);

    if (!index.isValid()
        || index.model() != static_cast<QAbstractItemModel *>(d->model)
        || index.parent() != d->rootIndex)
        return;

    setCurrentIndex(d->orientation == Qt::Horizontal ? index.row() : index.column());
}

/*!
    Clears all mappings.

    \sa addMapping(), removeMapping()
 */
void QDataWidgetMapper::clearMapping()
{
    Q_D(QDataWidgetMapper);

    while (!d->widgetMap.isEmpty()) {
        QWidget *w = d->widgetMap.takeLast().widget;
        if (w)
            w->removeEventFilter(d->delegate);
    }
}

/*! \fn void QDataWidgetMapper::setOrientation(Qt::Orientation orientation)

    Sets the orientation to \a orientation. If the orientation is
    horizontal (the default), a widget is mapped to a column of a
    data model. The widget will be populated with the model's data
    from its mapped column and the row that currentIndex() points at.

    Use horizontal orientation on tabular data like follows:

    \table
    \row \o 1 \o Trolltech AS    \o Oslo
    \row \o 2 \o Trolltech Pty   \o Brisbane
    \row \o 3 \o Trolltech Inc   \o Palo Alto
    \row \o 4 \o Trolltech China \o Beijing
    \row \o 5 \o Trolltech GmbH  \o Berlin
    \endtable

    If the orientation is set to vertical, a widget is mapped to a
    row. Calling setCurrentIndex() will change the current column.
    The widget will be populates with the model's data from its mapped
    row and the column that currentIndex() points at.

    Use vertical orientation on tabular data like follows:

    \table
    \row \o 1 \o 2 \o 3 \o 4 \o 5
    \row \o Trolltech AS \o Trolltech Pty \o Trolltech Inc \o Trolltech China \o Trolltech GmbH
    \row \o Oslo \o Brisbane \o Palo Alto \o Beijing \i Berlin
    \endtable

    Note: Changing the orientation clears all existing mappings.

    \sa orientation()
 */
void QDataWidgetMapper::setOrientation(Qt::Orientation aOrientation)
{
    Q_D(QDataWidgetMapper);

    if (d->orientation == aOrientation)
        return;

    clearMapping();
    d->orientation = aOrientation;
}

/*!
    Returns the current orientation of the model.

    \sa setOrientation()
 */
Qt::Orientation QDataWidgetMapper::orientation() const
{
    Q_D(const QDataWidgetMapper);
    return d->orientation;
}

/*!
    Sets the submit policy to \a policy. See SubmitPolicy for an
    explanation which submit policies are supported.

    Note: Changing the current submit policy will revert all widgets
    to the current data from the model.

    \sa SubmitPolicy, submitPolicy()
 */
void QDataWidgetMapper::setSubmitPolicy(SubmitPolicy policy)
{
    Q_D(QDataWidgetMapper);
    if (policy == d->submitPolicy)
        return;

    revert();
    d->submitPolicy = policy;
}

/*!
    Returns the current submit policy. See SubmitPolicy for an
    explanation which policies are supported.

    \sa setSubmitPolicy()
 */
QDataWidgetMapper::SubmitPolicy QDataWidgetMapper::submitPolicy() const
{
    Q_D(const QDataWidgetMapper);
    return d->submitPolicy;
}

/*!
    Returns the current row if the orientation is horizontal (the default)
    or the current column if the orientation is vertical.

    \sa setCurrentIndex()
 */
int QDataWidgetMapper::currentIndex() const
{
    Q_D(const QDataWidgetMapper);
    return d->currentIdx();
}

#include "moc_qdatawidgetmapper.cpp"
