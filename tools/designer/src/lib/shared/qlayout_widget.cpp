/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlayout_widget_p.h"
#include "layout_p.h"
#include "invisible_widget_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerWidgetFactoryInterface>

#include <QtGui/QPainter>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QMap>
#include <QtCore/QStack>
#include <QtCore/QPair>
#include <QtCore/QSet>

namespace {
    enum { ShiftValue = 1 };
    enum { debugLayout = 0 };
}

static const char *objectNameC = "objectName";
static const char *sizeConstraintC = "sizeConstraint";

static inline bool isEmptyItem(QLayoutItem *item)
{
    return item->spacerItem() != 0;
}

static inline QSpacerItem *createGridSpacer()
{
    return new QSpacerItem(20, 20);
}

// Grid/form Helpers: get info (overloads to make templates work)

namespace { // Do not use static, will break HP-UX due to templates 
inline int gridRowCount(const QGridLayout *gridLayout)    { return  gridLayout->rowCount(); }
inline int gridColumnCount(const QGridLayout *gridLayout) { return  gridLayout->columnCount(); }

// QGridLayout/QFormLayout Helpers: get item position (overloads to make templates work)
 inline void getGridItemPosition(QGridLayout *gridLayout, int index, int *row, int *column, int *rowspan, int *colspan)
{
    gridLayout->getItemPosition(index, row, column, rowspan, colspan);
}

QRect gridItemInfo(QGridLayout *grid, int index)
{
    int row, column, rowSpan, columnSpan;
    // getItemPosition is not const, grmbl..
    grid->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
    return QRect(column, row, columnSpan, rowSpan);
}
}

// QGridLayout/QFormLayout Helpers: Debug items of GridLikeLayout
template <class GridLikeLayout>
static QDebug debugGridLikeLayout(QDebug str, const GridLikeLayout &gl)
{
    const int count = gl.count();
    str << "Grid: " << gl.objectName() <<   gridRowCount(&gl) << " rows x " <<  gridColumnCount(&gl)
        << " cols " << count << " items\n";
    for (int i = 0; i < count; i++) {
        QLayoutItem *item = gl.itemAt(i);
        str << "Item " << i << item << item->widget() << gridItemInfo(const_cast<GridLikeLayout *>(&gl), i) << " empty " << isEmptyItem(item) << "\n";
    }
    return str;
}

static inline QDebug operator<<(QDebug str, const QGridLayout &gl) { return debugGridLikeLayout(str, gl); }

// recreate a managed grid in case it needs to shrink
static QGridLayout *recreateManagedGrid(const QDesignerFormEditorInterface *core, QWidget *w, QGridLayout *grid)
{
    qdesigner_internal::LayoutProperties properties;
    const int mask = properties.fromPropertySheet(core, grid, qdesigner_internal::LayoutProperties::AllProperties);
    qdesigner_internal::LayoutInfo::deleteLayout(core, w);
    QGridLayout *rc = static_cast<QGridLayout*>(core->widgetFactory()->createLayout(w, 0, qdesigner_internal::LayoutInfo::Grid));
    properties.toPropertySheet(core, rc, mask, true);
    return rc;
}

// QGridLayout/QFormLayout Helpers: find an item on a form/grid. Return index
template <class GridLikeLayout>
int findGridItemAt(GridLikeLayout *gridLayout, int at_row, int at_column)
{
    Q_ASSERT(gridLayout);
    const int count = gridLayout->count();
    for (int index = 0; index <  count; index++) {
        int row, column, rowspan, colspan;
        getGridItemPosition(gridLayout, index, &row, &column, &rowspan, &colspan);
        if (at_row >= row && at_row < (row + rowspan)
            && at_column >= column && at_column < (column + colspan)) {
            return index;
        }
    }
    return -1;
}
// QGridLayout/QFormLayout  Helpers: remove dummy spacers on form/grid
template <class GridLikeLayout>
static bool removeEmptyCellsOnGrid(GridLikeLayout *grid, const QRect &area)
{
    // check if there are any items in the way. Should be only spacers
    // Unique out items that span rows/columns.
    QVector<int> indexesToBeRemoved;
    indexesToBeRemoved.reserve(grid->count());
    const int rightColumn = area.x() + area.width();
    const int bottomRow = area.y() + area.height();
    for (int c = area.x(); c < rightColumn; c++)
        for (int r = area.y(); r < bottomRow; r++) {
            const int index = findGridItemAt(grid, r ,c);
            if (index != -1)
                if (QLayoutItem *item = grid->itemAt(index))
                    if (isEmptyItem(item)) {
                        if (indexesToBeRemoved.indexOf(index) == -1)
                            indexesToBeRemoved.push_back(index);
                    } else {
                        return false;
                    }
        }
    // remove, starting from last
    if (!indexesToBeRemoved.empty()) {
        qStableSort(indexesToBeRemoved.begin(), indexesToBeRemoved.end());
        for (int i = indexesToBeRemoved.size() - 1; i >= 0; i--)
            delete grid->takeAt(indexesToBeRemoved[i]);
    }
    return true;
}

namespace qdesigner_internal {
// --------- LayoutProperties

LayoutProperties::LayoutProperties()
{
    clear();
}

void LayoutProperties::clear()
{
    qFill(m_margins, m_margins + MarginCount, 0);
    qFill(m_marginsChanged, m_marginsChanged + MarginCount, false);
    qFill(m_spacings, m_spacings + SpacingsCount, 0);
    qFill(m_spacingsChanged, m_spacingsChanged + SpacingsCount, false);

    m_objectName.clear();
    m_objectNameChanged = false;
    m_sizeConstraint = QVariant(QLayout::SetDefaultConstraint);
    m_sizeConstraintChanged = false;
}

int LayoutProperties::visibleProperties(const  QLayout *layout)
{
    // Grid like layout have 2 spacings.
    const bool isGridLike = qobject_cast<const QGridLayout*>(layout);
    int rc = ObjectNameProperty|LeftMarginProperty|TopMarginProperty|RightMarginProperty|BottomMarginProperty|
             SizeConstraintProperty;
    rc |= isGridLike ? (HorizSpacingProperty|VertSpacingProperty) : SpacingProperty;
    return rc;
}
const QVector<QString> &LayoutProperties::marginPropertyNames()
{
    static QVector<QString> rc;
    if (rc.empty()) {
        rc.reserve(MarginCount);
        rc.push_back(QLatin1String("leftMargin"));
        rc.push_back(QLatin1String("topMargin"));
        rc.push_back(QLatin1String("rightMargin"));
        rc.push_back(QLatin1String("bottomMargin"));
    }
    return rc;
}

const QVector<QString> &LayoutProperties::spacingProperyNames()
{
    static QVector<QString> rc;
    if (rc.empty()) {
        rc.reserve(SpacingsCount);
        rc.push_back(QLatin1String("spacing"));
        rc.push_back(QLatin1String("horizontalSpacing"));
        rc.push_back(QLatin1String("verticalSpacing"));
    }
    return rc;
}

static bool intValueFromSheet(const QDesignerPropertySheetExtension *sheet, const QString &name, int *value, bool *changed)
{
    const int sheetIndex = sheet->indexOf(name);
    if (sheetIndex == -1)
        return false;
    *value = sheet->property(sheetIndex).toInt();
    *changed = sheet->isChanged(sheetIndex);
    return true;
}

int LayoutProperties::fromPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask)
{
    int rc = 0;
    const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), l);
    Q_ASSERT(sheet);
    // name
    if (mask & ObjectNameProperty) {
        const int nameIndex = sheet->indexOf(QLatin1String(objectNameC));
        Q_ASSERT(nameIndex != -1);
        m_objectName = sheet->property(nameIndex).toString();
        m_objectNameChanged =  sheet->isChanged(nameIndex);
        rc |= ObjectNameProperty;
    }
    // -- Margins
    const QVector<QString> &marginNames = marginPropertyNames();
    const int marginFlags[MarginCount] = { LeftMarginProperty, TopMarginProperty, RightMarginProperty, BottomMarginProperty};
    for (int i = 0; i < MarginCount; i++)
        if (mask & marginFlags[i])
            if (intValueFromSheet(sheet, marginNames[i], m_margins + i, m_marginsChanged + i))
                rc |= marginFlags[i];

    const QVector<QString> &spacingNames = spacingProperyNames();
    const int spacingFlags[] = { SpacingProperty, HorizSpacingProperty, VertSpacingProperty};
    for (int i = 0; i < SpacingsCount; i++)
        if (mask & spacingFlags[i])
            if (intValueFromSheet(sheet, spacingNames[i], m_spacings + i, m_spacingsChanged + i))
                rc |= spacingFlags[i];
    // sizeConstraint
    if (mask & SizeConstraintProperty) {
        const int scIndex = sheet->indexOf(QLatin1String(sizeConstraintC));
        if (scIndex != -1) {
            m_sizeConstraint = sheet->property(scIndex);
            m_sizeConstraintChanged =  sheet->isChanged(scIndex);
            rc |= SizeConstraintProperty;
        }
    }
    return rc;
}

static bool intValueToSheet(QDesignerPropertySheetExtension *sheet, const QString &name, int value, bool changed, bool applyChanged)

{

    const int sheetIndex = sheet->indexOf(name);
    if (sheetIndex == -1) {
        qWarning() << " LayoutProperties: Attempt to set property " << name << " that does not exist for the layout.";
        return false;
    }
    sheet->setProperty(sheetIndex, QVariant(value));
    if (applyChanged)
        sheet->setChanged(sheetIndex, changed);
    return true;
}

int LayoutProperties::toPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask, bool applyChanged) const
{
    int rc = 0;
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), l);
    Q_ASSERT(sheet);
    // name
    if (mask & ObjectNameProperty) {
        const int nameIndex = sheet->indexOf(QLatin1String(objectNameC));
        Q_ASSERT(nameIndex != -1);
        sheet->setProperty(nameIndex, m_objectName);
        if (applyChanged)
           sheet->setChanged(nameIndex, m_objectNameChanged);
        rc |= ObjectNameProperty;
    }
    // margins
    const QVector<QString> &marginNames = marginPropertyNames();
    const int marginFlags[MarginCount] = { LeftMarginProperty, TopMarginProperty, RightMarginProperty, BottomMarginProperty};
    for (int i = 0; i < MarginCount; i++)
        if (mask & marginFlags[i])
            if (intValueToSheet(sheet, marginNames[i], m_margins[i], m_marginsChanged[i], applyChanged))
                rc |= marginFlags[i];

    const QVector<QString> &spacingNames = spacingProperyNames();
    const int spacingFlags[] = { SpacingProperty, HorizSpacingProperty, VertSpacingProperty};
    for (int i = 0; i < SpacingsCount; i++)
        if (mask & spacingFlags[i])
            if (intValueToSheet(sheet, spacingNames[i], m_spacings[i], m_spacingsChanged[i], applyChanged))
                rc |= spacingFlags[i];
    // sizeConstraint
    if (mask &  SizeConstraintProperty) {
        const int scIndex = sheet->indexOf(QLatin1String(sizeConstraintC));
        if (scIndex != -1) {
            sheet->setProperty(scIndex, m_sizeConstraint);
            if (applyChanged)
                sheet->setChanged(scIndex, m_sizeConstraintChanged);
            rc |= SizeConstraintProperty;
        }
    }
    return rc;
}

// ---------------- LayoutHelper
LayoutHelper::LayoutHelper()
{
}

LayoutHelper::~LayoutHelper()
{
}

int LayoutHelper::indexOf(const QLayout *lt, const QWidget *widget)
{
    if (!lt)
        return -1;

    const int itemCount = lt->count();
    for (int i = 0; i < itemCount; i++)
        if (lt->itemAt(i)->widget() == widget)
            return i;
    return -1;
}

QRect LayoutHelper::itemInfo(QLayout *lt, const QWidget *widget) const
{
    const int index = indexOf(lt, widget);
    if (index == -1) {
        qWarning() << "LayoutHelper::itemInfo: " << widget << " not in layout " << lt;
        return QRect(0, 0, 1, 1);
    }
    return itemInfo(lt, index);
}

namespace {
    // ---------------- BoxLayoutHelper
    class BoxLayoutHelper : public  LayoutHelper {
    public:
        BoxLayoutHelper(const Qt::Orientation orientation) : m_orientation(orientation) {}

        virtual QRect itemInfo(QLayout *lt, int index) const;
        virtual void insertWidget(QLayout *lt, const QRect &info, QWidget *w);
        virtual void removeWidget(QLayout *lt, QWidget *widget);

        virtual void pushState(const QWidget *);
        virtual void popState(const QDesignerFormEditorInterface *, QWidget *);

        virtual bool canSimplify(const QWidget *, const QRect &) const { return  false; }
        virtual void simplify(const QDesignerFormEditorInterface *, QWidget *, const QRect &) {}

        // Helper for restoring layout states
        typedef QVector <QLayoutItem *> LayoutItemVector;
        static LayoutItemVector disassembleLayout(QLayout *lt);
        static QLayoutItem *findItemOfWidget(const LayoutItemVector &lv, QWidget *w);

    private:
        typedef QVector<QWidget *> BoxLayoutState;

        static BoxLayoutState state(const QBoxLayout*lt);

        QStack<BoxLayoutState> m_states;
        const Qt::Orientation m_orientation;
    };

    QRect BoxLayoutHelper::itemInfo(QLayout * /*lt*/, int index) const
    {
        return m_orientation == Qt::Horizontal ?  QRect(index, 0, 1, 1) : QRect(0, index, 1, 1);
    }

    void BoxLayoutHelper::insertWidget(QLayout *lt, const QRect &info, QWidget *w)
    {
        QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(lt);
        Q_ASSERT(boxLayout);
        boxLayout->insertWidget(m_orientation == Qt::Horizontal ? info.x() : info.y(), w);
    }

    void BoxLayoutHelper::removeWidget(QLayout *lt, QWidget *widget)
    {
        QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(lt);
        Q_ASSERT(boxLayout);
        boxLayout->removeWidget(widget);
    }

    BoxLayoutHelper::BoxLayoutState BoxLayoutHelper::state(const QBoxLayout*lt)
    {
        BoxLayoutState rc;
        if (const int count = lt->count()) {
            rc.reserve(count);
            for (int i = 0; i < count; i++)
                if (QWidget *w = lt->itemAt(i)->widget())
                    rc.push_back(w);
        }
        return rc;
    }

    void BoxLayoutHelper::pushState(const QWidget *w)
    {
        const QBoxLayout *boxLayout = qobject_cast<const QBoxLayout *>(w->layout());
        Q_ASSERT(boxLayout);
        m_states.push(state(boxLayout));
    }

    QLayoutItem *BoxLayoutHelper::findItemOfWidget(const LayoutItemVector &lv, QWidget *w)
    {
        const LayoutItemVector::const_iterator cend = lv.constEnd();
        for (LayoutItemVector::const_iterator it = lv.constBegin(); it != cend; ++it)
            if ( (*it)->widget() == w)
                return *it;

        return 0;
    }

    BoxLayoutHelper::LayoutItemVector BoxLayoutHelper::disassembleLayout(QLayout *lt)
    {
        // Take items
        const int count = lt->count();
        if (count == 0)
            return LayoutItemVector();
        LayoutItemVector rc;
        rc.reserve(count);
        for (int i = count - 1; i >= 0; i--)
            rc.push_back(lt->takeAt(i));
        return rc;
    }

    void BoxLayoutHelper::popState(const QDesignerFormEditorInterface *, QWidget *w)
    {
        QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(w->layout());
        Q_ASSERT(boxLayout);
        const BoxLayoutState savedState = m_states.pop();
        const BoxLayoutState currentState = state(boxLayout);
        // Check for equality/empty. Note that this will currently
        // always trigger as box layouts do not have a state apart from
        // the order and there is no layout order editor yet.
        if (savedState == state(boxLayout))
            return;

        const int count = savedState.size();
        Q_ASSERT(count == currentState.size());
        // Take items and reassemble in saved order
        const LayoutItemVector items = disassembleLayout(boxLayout);
        for (int i = 0; i < count; i++) {
            QLayoutItem *item = findItemOfWidget(items, savedState[i]);
            Q_ASSERT(item);
            boxLayout->addItem(item);
        }
    }

    // Grid Layout state. Datatypically store the state of a GridLayout as a map of
    // widgets to QRect(columns, rows) and size. Used to store the state for undo operations
    // that do not change the widgets within the layout; also provides some manipulation
    // functions and ability to apply the state to a layout provided its widgets haven't changed.
    struct GridLayoutState {
        GridLayoutState();

        void fromLayout(QGridLayout *l);
        void applyToLayout(const QDesignerFormEditorInterface *core, QWidget *w) const;

        void insertRow(int row);
        void insertColumn(int column);

        bool simplify(const QRect &r, bool testOnly);
        void removeFreeRow(int row);
        void removeFreeColumn(int column);


        // State of a cell in one dimension
        enum DimensionCellState {
            Free,
            Spanned,  // Item spans it
            Occupied  // Item bordering on it
        };
        // Horiontal, Vertical pair of state
        typedef QPair<DimensionCellState, DimensionCellState> CellState;
        typedef QVector<CellState> CellStates;

        // Figure out states of a cell and return as a flat vector of
        // [column1, column2,...] (address as  row * columnCount + col)
        static CellStates cellStates(const QList<QRect> &rects, int numRows, int numColumns);

        typedef QMap<QWidget *, QRect> WidgetItemMap;
        WidgetItemMap widgetItemMap;
        int rowCount;
        int colCount;
    };

    static inline bool needsSpacerItem(const GridLayoutState::CellState &cs) {
        return cs.first == GridLayoutState::Free && cs.second == GridLayoutState::Free;
    }

    static inline QDebug operator<<(QDebug str, const GridLayoutState &gs)
    {
        str << "GridLayoutState: " <<  gs.rowCount << " rows x " <<  gs.colCount
            << " cols " << gs.widgetItemMap.size() << " items\n";

        const GridLayoutState::WidgetItemMap::const_iterator wcend = gs.widgetItemMap.constEnd();
        for (GridLayoutState::WidgetItemMap::const_iterator it = gs.widgetItemMap.constBegin(); it != wcend; ++it)
            str << "Item " << it.key() << it.value() << '\n';
        return str;
    }

    GridLayoutState::GridLayoutState() :
         rowCount(0),
         colCount(0)
    {
    }

    GridLayoutState::CellStates GridLayoutState::cellStates(const QList<QRect> &rects, int numRows, int numColumns)
    {
        CellStates rc = CellStates(numRows * numColumns, CellState(Free, Free));
        const QList<QRect>::const_iterator rcend = rects.constEnd();
        for (QList<QRect>::const_iterator it = rects.constBegin(); it != rcend; ++it) {
            const int leftColumn = it->x();
            const int topRow = it->y();
            const int rightColumn = leftColumn + it->width() - 1;
            const int bottomRow = topRow + it->height() - 1;
            for (int r = topRow; r <= bottomRow; r++)
                for (int c = leftColumn; c <= rightColumn; c++) {
                    const int flatIndex = r * numColumns + c;
                    // Bordering horizontally?
                    DimensionCellState &horizState = rc[flatIndex].first;
                    if (c == leftColumn || c == rightColumn) {
                        horizState = Occupied;
                    } else {
                        if (horizState < Spanned)
                            horizState = Spanned;
                    }
                    // Bordering vertically?
                    DimensionCellState &vertState = rc[flatIndex].second;
                    if (r == topRow || r == bottomRow) {
                        vertState = Occupied;
                    } else {
                        if (vertState < Spanned)
                            vertState = Spanned;
                    }
                }
        }
        if (debugLayout) {
            qDebug() << "GridLayoutState::cellStates: " << numRows << " x " << numColumns;
            for (int r = 0; r < numRows; r++)
                for (int c = 0; c < numColumns; c++)
                    qDebug() << "  Row: " << r << " column: " << c <<  rc[r * numColumns + c];
        }
        return rc;
    }

    void GridLayoutState::fromLayout(QGridLayout *l)
    {
        rowCount = l->rowCount();
        colCount = l->columnCount();
        const int count = l->count();
        for (int i = 0; i < count; i++) {
            QLayoutItem *item = l->itemAt(i);
            if (!isEmptyItem(item))
                widgetItemMap.insert(item->widget(), gridItemInfo(l, i));
        }
    }

    void GridLayoutState::applyToLayout(const QDesignerFormEditorInterface *core, QWidget *w) const
    {
        typedef QMap<QLayoutItem *, QRect> LayoutItemRectMap;
        QGridLayout *grid = qobject_cast<QGridLayout *>(w->layout());
        Q_ASSERT(grid);
        if (debugLayout)
            qDebug() << ">GridLayoutState::applyToLayout" <<  *this << *grid;
        const bool shrink = grid->rowCount() > rowCount || grid->columnCount() > colCount;
        // Build a map of existing items to rectangles via widget map, delete spacers
        LayoutItemRectMap itemMap;
        while (grid->count()) {
            QLayoutItem *item = grid->takeAt(0);
            if (!isEmptyItem(item)) {
                QWidget *itemWidget = item->widget();
                const WidgetItemMap::const_iterator it = widgetItemMap.constFind(itemWidget);
                if (it == widgetItemMap.constEnd())
                    qFatal("GridLayoutState::applyToLayout: Attempt to apply to a layout that has a widget '%s'/'%s' added after saving the state.",
                           itemWidget->metaObject()->className(), itemWidget->objectName().toUtf8().constData());
                itemMap.insert(item, it.value());
            } else {
                delete item;
            }
        }
        Q_ASSERT(itemMap.size() == widgetItemMap.size());
        // recreate if shrink
        if (shrink)
            grid = recreateManagedGrid(core, w, grid);

        // Add widgets items
        const LayoutItemRectMap::const_iterator icend = itemMap.constEnd();
        for (LayoutItemRectMap::const_iterator it = itemMap.constBegin(); it != icend; ++it) {
            const QRect info = it.value();
            grid->addItem(it.key(), info.y(), info.x(), info.height(), info.width());
        }
        // create spacers
        const CellStates cs = cellStates(itemMap.values(), rowCount, colCount);
        for (int r = 0; r < rowCount; r++)
            for (int c = 0; c < colCount; c++)
                if (needsSpacerItem(cs[r * colCount  + c]))
                    grid->addItem(createGridSpacer(), r, c);
        grid->activate();
        if (debugLayout)
            qDebug() << "<GridLayoutState::applyToLayout" <<  *grid;
    }

    void GridLayoutState::insertRow(int row)
    {
        rowCount++;
        const WidgetItemMap::iterator iend = widgetItemMap.end();
        for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
            const int topRow = it.value().y();
            if (topRow >= row) {
                it.value().translate(0, 1);
            } else {  //Over  it: Does it span it -> widen?
                const int rowSpan = it.value().height();
                if (rowSpan > 1 && topRow + rowSpan > row)
                    it.value().setHeight(rowSpan + 1);
            }
        }
    }

    void GridLayoutState::insertColumn(int column)
    {
        colCount++;
        const WidgetItemMap::iterator iend = widgetItemMap.end();
        for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
            const int leftColumn = it.value().x();
            if (leftColumn >= column) {
                it.value().translate(1, 0);
            } else { // Left of it: Does it span it -> widen?
                const int colSpan = it.value().width();
                if (colSpan  > 1 &&  leftColumn + colSpan > column)
                    it.value().setWidth(colSpan + 1);
            }
        }
    }

    // Simplify: Remove empty columns/rows and such ones that are only spanned (shrink
    // spanning items).
    // 'AB.C.'           'ABC'
    // 'DDDD.'     ==>   'DDD'
    // 'EF.G.'           'EFG'
    bool GridLayoutState::simplify(const QRect &r, bool testOnly)
    {
        // figure out free rows/columns.
        QVector<bool> occupiedRows(rowCount, false);
        QVector<bool> occupiedColumns(colCount, false);
        // Mark everything outside restriction rectangle as occupied
        const int restrictionLeftColumn = r.x();
        const int restrictionRightColumn = restrictionLeftColumn + r.width();
        const int restrictionTopRow = r.y();
        const int restrictionBottomRow = restrictionTopRow + r.height();
        if (restrictionLeftColumn > 0 || restrictionRightColumn < colCount ||
            restrictionTopRow     > 0 || restrictionBottomRow   < rowCount) {
            for (int r = 0; r <  rowCount; r++)
                if (r < restrictionTopRow || r >= restrictionBottomRow)
                    occupiedRows[r] = true;
            for (int c = 0; c < colCount; c++)
                if (c < restrictionLeftColumn ||  c >= restrictionRightColumn)
                    occupiedColumns[c] = true;
        }
        // figure out free fields and tick off occupied rows and columns
        const CellStates cs = cellStates(widgetItemMap.values(), rowCount, colCount);
        for (int r = 0; r < rowCount; r++)
            for (int c = 0; c < colCount; c++) {
                const CellState &state = cs[r * colCount  + c];
                if (state.first == Occupied)
                    occupiedColumns[c] = true;
                if (state.second == Occupied)
                    occupiedRows[r] = true;
            }
        // Any free rows/columns?
        if (occupiedRows.indexOf(false) ==  -1 && occupiedColumns.indexOf(false) == -1)
            return false;
        if (testOnly)
            return true;
        // remove rows
        for (int r = rowCount - 1; r >= 0; r--)
            if (!occupiedRows[r])
                removeFreeRow(r);
        // remove columns
        for (int c = colCount - 1; c >= 0; c--)
            if (!occupiedColumns[c])
                removeFreeColumn(c);
        return true;
    }

    void GridLayoutState::removeFreeRow(int removeRow)
    {
        const WidgetItemMap::iterator iend = widgetItemMap.end();
        for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
            const int r = it.value().y();
            Q_ASSERT(r != removeRow); // Free rows only
            if (r < removeRow) { // Does the item span it? - shrink it
                const int rowSpan = it.value().height();
                if (rowSpan > 1) {
                    const int bottomRow = r + rowSpan;
                    if (bottomRow > removeRow)
                        it.value().setHeight(rowSpan - 1);
                }
            } else
                if (r > removeRow) // Item below it? - move.
                    it.value().translate(0, -1);
        }
        rowCount--;
    }

    void GridLayoutState::removeFreeColumn(int removeColumn)
    {
        const WidgetItemMap::iterator iend = widgetItemMap.end();
        for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
            const int c = it.value().x();
            Q_ASSERT(c != removeColumn); // Free columns only
            if (c < removeColumn) { // Does the item span it? - shrink it
                const int colSpan = it.value().width();
                if (colSpan > 1) {
                    const int rightColumn = c + colSpan;
                    if (rightColumn > removeColumn)
                        it.value().setWidth(colSpan - 1);
                }
            } else
                if (c > removeColumn) // Item to the right of it?  - move.
                    it.value().translate(-1, 0);
        }
        colCount--;
    }

    // ---------------- GridLayoutHelper
    class GridLayoutHelper : public  LayoutHelper {
    public:
        GridLayoutHelper() {}

        virtual QRect itemInfo(QLayout *lt, int index) const;
        virtual void insertWidget(QLayout *lt, const QRect &info, QWidget *w);
        virtual void removeWidget(QLayout *lt, QWidget *widget);

        virtual void pushState(const QWidget *widgetWithManagedLayout);
        virtual void popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout);

        virtual bool canSimplify(const QWidget *widgetWithManagedLayout, const QRect &restrictionArea) const;
        virtual void simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout, const QRect &restrictionArea);

    private:
        QStack<GridLayoutState> m_states;
    };

    QRect GridLayoutHelper::itemInfo(QLayout * lt, int index) const
    {
        QGridLayout *grid = qobject_cast<QGridLayout *>(lt);
        Q_ASSERT(grid);
        return gridItemInfo(grid, index);
    }

    void GridLayoutHelper::insertWidget(QLayout *lt, const QRect &info, QWidget *w)
    {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(lt);
        Q_ASSERT(gridLayout);
        // check if there are any items. Should be only spacers, else something is wrong
        if (!removeEmptyCellsOnGrid(gridLayout, info)) {
            qWarning() << "GridLayoutHelper::insertWidget : Unable to insert " << w << " at " << info << " The cell is not empty.";
            return;
        }
        gridLayout->addWidget(w, info.y(), info.x(), info.height(), info.width());
    }

    void GridLayoutHelper::removeWidget(QLayout *lt, QWidget *widget)
    {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(lt);
        Q_ASSERT(gridLayout);
        const int index = gridLayout->indexOf(widget);
        if (index == -1) {
            qWarning() << "GridLayoutHelper::removeWidget : Attempt to remove " << widget <<  " which is not in the layout.";
            return;
        }
        // delete old item and pad with  by spacer items
        int row, column, rowspan, colspan;
        gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
        delete gridLayout->takeAt(index);
        const int rightColumn = column + colspan;
        const int bottomRow = row +  rowspan;
        for (int c = column; c < rightColumn; c++)
            for (int r = row; r < bottomRow; r++)
                gridLayout->addItem(createGridSpacer(), r, c);
    }

    void GridLayoutHelper::pushState(const QWidget *widgetWithManagedLayout)
    {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(widgetWithManagedLayout->layout());
        Q_ASSERT(gridLayout);
        GridLayoutState gs;
        gs.fromLayout(gridLayout);
        m_states.push(gs);
    }

    void GridLayoutHelper::popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout)
    {
        Q_ASSERT(!m_states.empty());
        const GridLayoutState state = m_states.pop();
        state.applyToLayout(core, widgetWithManagedLayout);
    }

    bool GridLayoutHelper::canSimplify(const QWidget *widgetWithManagedLayout, const QRect &restrictionArea) const
    {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(widgetWithManagedLayout->layout());
        Q_ASSERT(gridLayout);
        GridLayoutState gs;
        gs.fromLayout(gridLayout);
        return gs.simplify(restrictionArea, true);
    }

    void GridLayoutHelper::simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout, const QRect &restrictionArea)
    {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(widgetWithManagedLayout->layout());
        Q_ASSERT(gridLayout);
        if (debugLayout)
            qDebug() << ">GridLayoutHelper::simplify" <<  *gridLayout;
        GridLayoutState gs;
        gs.fromLayout(gridLayout);
        if (gs.simplify(restrictionArea, false))
            gs.applyToLayout(core, widgetWithManagedLayout);
        if (debugLayout)
            qDebug() << "<GridLayoutHelper::simplify" <<  *gridLayout;
   }
} //  anonymous namespace

LayoutHelper *LayoutHelper::createLayoutHelper(int type)
{
    LayoutHelper *rc = 0;
    switch (type) {
    case LayoutInfo::HBox:
        rc = new BoxLayoutHelper(Qt::Horizontal);
        break;
    case LayoutInfo::VBox:
        rc = new BoxLayoutHelper(Qt::Vertical);
        break;
    case LayoutInfo::Grid:
        rc = new GridLayoutHelper;
        break;
     default:
        break;
    }
    Q_ASSERT(rc);
    return rc;
}

// ---- QLayoutSupport (LayoutDecorationExtension)
QLayoutSupport::QLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent)  :
      QObject(parent),
      m_formWindow(formWindow),
      m_helper(helper),
      m_widget(widget),
      m_currentIndex(-1),
      m_currentInsertMode(QDesignerLayoutDecorationExtension::InsertWidgetMode)
{
}

void QLayoutSupport::hideIndicator(Indicator i)
{
    if (m_indicators[i])
        m_indicators[i]->hide();
}

void QLayoutSupport::showIndicator(Indicator i, const QRect &geometry, const QPalette &p)
{
    if (!m_indicators[i])
        m_indicators[i] = new qdesigner_internal::InvisibleWidget(m_widget);
    QWidget *indicator = m_indicators[i];
    indicator->setAutoFillBackground(true);
    indicator->setPalette(p);
    indicator->setGeometry(geometry);
    indicator->show();
    indicator->raise();
}

QLayoutSupport::~QLayoutSupport()
{
    delete m_helper;
    for (int i = 0; i < NumIndicators; i++)
        if (m_indicators[i])
            m_indicators[i]->deleteLater();
}

QGridLayout * QLayoutSupport::gridLayout() const
{
    return qobject_cast<QGridLayout*>(m_widget->layout());
}

QRect QLayoutSupport::itemInfo(int index) const
{
    return m_helper->itemInfo(m_widget->layout(), index);
}

void QLayoutSupport::setInsertMode(InsertMode im)
{
    m_currentInsertMode = im;
}

void QLayoutSupport::setCurrentCell(const QPair<int, int> &cell)
{
    m_currentCell = cell;
}

void QLayoutSupport::adjustIndicator(const QPoint &pos, int index)
{
    if (index == -1) { // first item goes anywhere
        hideIndicator(LeftIndicator);
        hideIndicator(TopIndicator);
        hideIndicator(RightIndicator);
        hideIndicator(BottomIndicator);
        return;
    }
    m_currentIndex = index;
    m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertWidgetMode;

    QLayoutItem *item = layout()->itemAt(index);
   const QRect g = extendedGeometry(index);
    // ### cleanup
    if (isEmptyItem(item)) {
        QPalette redPalette;
        redPalette.setColor(QPalette::Window, Qt::red);

        showIndicator(LeftIndicator,   QRect(g.x(),     g.y(),      2,         g.height()), redPalette);
        showIndicator(TopIndicator,    QRect(g.x(),     g.y(),      g.width(), 2), redPalette);
        showIndicator(RightIndicator,  QRect(g.right(), g.y(),      2,         g.height()), redPalette);
        showIndicator(BottomIndicator, QRect(g.x(),     g.bottom(), g.width(), 2), redPalette);
        setCurrentCellFromIndicatorOnEmptyCell(m_currentIndex);
    } else {
        QPalette bluePalette;
        bluePalette.setColor(QPalette::Window, Qt::blue);
        hideIndicator(LeftIndicator);
        hideIndicator(TopIndicator);

        const int fromRight = g.right() - pos.x();
        const int fromBottom = g.bottom() - pos.y();

        const int fromLeft = pos.x() - g.x();
        const int fromTop = pos.y() - g.y();

        const int fromLeftRight = qMin(fromRight, fromLeft );
        const int fromBottomTop = qMin(fromBottom, fromTop);

        const Qt::Orientation indicatorOrientation =  fromLeftRight < fromBottomTop ? Qt::Vertical :  Qt::Horizontal;

        if (supportsIndicatorOrientation(indicatorOrientation)) {
            const QRect r(layout()->geometry().topLeft(), layout()->parentWidget()->size());
            switch (indicatorOrientation) {
            case  Qt::Vertical: {
                hideIndicator(BottomIndicator);
                const bool closeToLeft = fromLeftRight == fromLeft;
                showIndicator(RightIndicator, QRect(closeToLeft ? g.x() : g.right(), 0, 2, r.height()), bluePalette);

                const int incr = closeToLeft ? 0 : +1;
                setCurrentCellFromIndicator(indicatorOrientation, m_currentIndex, incr);
            }
            break;
            case  Qt::Horizontal: {
                hideIndicator(RightIndicator);
                const bool closeToTop = fromBottomTop == fromTop;
                showIndicator(BottomIndicator, QRect(r.x(), closeToTop ? g.y() : g.bottom(), r.width(), 2), bluePalette);

                const int incr = closeToTop ? 0 : +1;
                setCurrentCellFromIndicator(indicatorOrientation, m_currentIndex, incr);
            }
                break;
            }
        } else {
            hideIndicator(RightIndicator);
            hideIndicator(BottomIndicator);
        } // can handle indicatorOrientation
    }
}

int QLayoutSupport::indexOf(QLayoutItem *i) const
{
    const QLayout *lt = layout();
    if (!lt)
        return -1;

    int index = 0;

    while (QLayoutItem *item = lt->itemAt(index)) {
        if (item == i)
            return index;

        ++index;
    }

    return -1;
}

int QLayoutSupport::indexOf(QWidget *widget) const
{
    const QLayout *lt = layout();
    if (!lt)
        return -1;

    int index = 0;
    while (QLayoutItem *item = lt->itemAt(index)) {
        if (item->widget() == widget)
            return index;

        ++index;
    }

    return -1;
}

QList<QWidget*> QLayoutSupport::widgets(QLayout *layout) const
{
    if (!layout)
        return QList<QWidget*>();

    QList<QWidget*> lst;
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        ++index;

        QWidget *widget = item->widget();
        if (widget && formWindow()->isManaged(widget))
            lst.append(widget);
    }

    return lst;
}

int QLayoutSupport::findItemAt(QGridLayout *gridLayout, int at_row, int at_column)
{
    return findGridItemAt(gridLayout, at_row, at_column);
}

// Quick check whether simplify should be enabled for grids. May return false positives.
// Note: Calculating the occupied area does not work as spanning items may also be simplified.

bool QLayoutSupport::canSimplifyQuickCheck(const QGridLayout *gl)
{
    if (!gl)
        return false;
    const int colCount = gl->columnCount();
    const int rowCount = gl->rowCount();
    if (colCount < 2 || rowCount < 2)
        return false;
    // try to find a spacer.
    const int count = gl->count();
    for (int index = 0; index < count; index++)
        if (isEmptyItem(gl->itemAt(index)))
            return true;
    return false;
}

// remove dummy spacers
bool QLayoutSupport::removeEmptyCells(QGridLayout *grid, const QRect &area)
{
    return removeEmptyCellsOnGrid(grid, area);
}

void QLayoutSupport::createEmptyCells(QGridLayout *gridLayout)
{
    Q_ASSERT(gridLayout);
    GridLayoutState gs;
    gs.fromLayout(gridLayout);

    const GridLayoutState::CellStates cs = GridLayoutState::cellStates(gs.widgetItemMap.values(), gs.rowCount, gs.colCount);
    for (int c = 0; c < gs.colCount; c++)
        for (int r = 0; r < gs.rowCount; r++)
            if (needsSpacerItem(cs[r * gs.colCount + c])) {
                const int existingItemIndex = findItemAt(gridLayout, r, c);
                if (existingItemIndex == -1)
                    gridLayout->addItem(createGridSpacer(), r, c);
            }
}

int QLayoutSupport::findItemAt(const QPoint &pos) const
{
    if (!layout())
        return -1;

    int best = -1;
    int bestIndex = -1;

    int index = 0;
    const QLayout *lt = layout();
    while (QLayoutItem *item = lt->itemAt(index)) {

        const QRect g = item->geometry();

        const int dist = (g.center() - pos).manhattanLength();
        if (best == -1 || dist < best) {
            best = dist;
            bestIndex = index;
        }

        ++index;
    }

    return bestIndex;
}

// ------------ QBoxLayoutSupport (LayoutDecorationExtension)
namespace {
class QBoxLayoutSupport: public QLayoutSupport
{
public:
    QBoxLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, Qt::Orientation orientation, QObject *parent = 0);

    virtual void insertWidget(QWidget *widget, const QPair<int, int> &cell);
    virtual void removeWidget(QWidget *widget) { helper()->removeWidget(layout(), widget); }

    virtual void simplify() {}
    virtual void insertRow(int /*row*/) {}
    virtual void insertColumn(int /*column*/) {}

    virtual int findItemAt(int /*at_row*/, int /*at_column*/) const {    return -1; }

private:
    virtual void setCurrentCellFromIndicatorOnEmptyCell(int index);
    virtual void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment);
    virtual bool supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const;
    virtual QRect extendedGeometry(int index) const;

    const Qt::Orientation m_orientation;
};

QBoxLayoutSupport::QBoxLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, Qt::Orientation orientation, QObject *parent) :
    QLayoutSupport(formWindow, widget, new BoxLayoutHelper(orientation), parent),
    m_orientation(orientation)
{
}

void QBoxLayoutSupport::setCurrentCellFromIndicatorOnEmptyCell(int index)
{
    qDebug() << "QBoxLayoutSupport::setCurrentCellFromIndicatorOnEmptyCell(): Warning: found a fake spacer inside a vbox layout at " << index;
    setCurrentCell(qMakePair(0, 0));
}

void QBoxLayoutSupport::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
    switch (m_orientation) {
    case  Qt::Horizontal:
        helper()->insertWidget(layout(), QRect(cell.second, 0, 1, 1), widget);
        break;
    case  Qt::Vertical:
        helper()->insertWidget(layout(), QRect(0, cell.first, 1, 1), widget);
        break;
    }
}

void QBoxLayoutSupport::setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment)
{
    if (m_orientation == Qt::Horizontal && indicatorOrientation == Qt::Vertical) {
        setCurrentCell(qMakePair(0, index + increment));
    } else if (m_orientation == Qt::Vertical && indicatorOrientation == Qt::Horizontal) {
        setCurrentCell(qMakePair(index + increment, 0));
    }
}

bool QBoxLayoutSupport::supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const
{
    return m_orientation != indicatorOrientation;
}

QRect QBoxLayoutSupport::extendedGeometry(int index) const
{
    QLayoutItem *item = layout()->itemAt(index);
    // start off with item geometry
    QRect g = item->geometry();

    const QRect info = itemInfo(index);

    // On left border: extend to widget border
    if (info.x() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.rx() = layout()->geometry().left();
        g.setTopLeft(topLeft);
    }

    // On top border: extend to widget border
    if (info.y() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.ry() = layout()->geometry().top();
        g.setTopLeft(topLeft);
    }

    // is this the last item?
    const QBoxLayout *box = static_cast<const QBoxLayout*>(layout());
    if (index < box->count() -1)
        return g; // Nope.

    // extend to widget border
    QPoint bottomRight = g.bottomRight();
    switch (m_orientation) {
    case Qt::Vertical:
        bottomRight.ry() = layout()->geometry().bottom();
        break;
    case Qt::Horizontal:
        bottomRight.rx() = layout()->geometry().right();
        break;
    }
    g.setBottomRight(bottomRight);
    return g;
}

// --------------  Base class for QGridLayout-like support classes (LayoutDecorationExtension)
template <class GridLikeLayout>
class GridLikeLayoutSupportBase: public QLayoutSupport
{
public:

    GridLikeLayoutSupportBase(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent = 0) :
        QLayoutSupport(formWindow, widget, helper, parent) {}

    void insertWidget(QWidget *widget, const QPair<int, int> &cell);
    virtual void removeWidget(QWidget *widget) { helper()->removeWidget(layout(), widget); }
    virtual int findItemAt(int row, int column) const;

protected:
    GridLikeLayout *gridLikeLayout() const { return qobject_cast<GridLikeLayout*>(widget()->layout()); }

private:

    virtual void setCurrentCellFromIndicatorOnEmptyCell(int index);
    virtual void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment);
    virtual bool supportsIndicatorOrientation(Qt::Orientation) const { return true; }

    virtual QRect extendedGeometry(int index) const;
};

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::setCurrentCellFromIndicatorOnEmptyCell(int index)
{
    GridLikeLayout *grid = gridLikeLayout();
    Q_ASSERT(grid);

    setInsertMode(InsertWidgetMode);
    int row, column, rowspan, colspan;

    getGridItemPosition(grid, index, &row, &column, &rowspan, &colspan);
    setCurrentCell(qMakePair(row, column));
}

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment) {
    const QRect info = itemInfo(index);
    switch (indicatorOrientation) {
    case Qt::Vertical:
        setInsertMode(InsertColumnMode);
        setCurrentCell(qMakePair(info.top(), increment ? info.right() + 1 : info.left()));
        break;
    case Qt::Horizontal:
        setInsertMode(InsertRowMode);
        setCurrentCell(qMakePair(increment ? info.bottom() + 1 : info.top(), info.left()));
        break;
    }
}

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
    helper()->insertWidget(layout(), QRect(cell.second, cell.first, 1, 1), widget);
}

template <class GridLikeLayout>
int GridLikeLayoutSupportBase<GridLikeLayout>::findItemAt(int at_row, int at_column) const
{
    GridLikeLayout *grid = gridLikeLayout();
    Q_ASSERT(grid);
    return findGridItemAt(grid, at_row, at_column);
}

template <class GridLikeLayout>
QRect GridLikeLayoutSupportBase<GridLikeLayout>::extendedGeometry(int index) const
{
    QLayoutItem *item = layout()->itemAt(index);
    // start off with item geometry
    QRect g = item->geometry();

    const QRect info = itemInfo(index);

    // On left border: extend to widget border
    if (info.x() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.rx() = layout()->geometry().left();
        g.setTopLeft(topLeft);
    }

    // On top border: extend to widget border
    if (info.y() == 0) {
        QPoint topLeft = g.topLeft();
        topLeft.ry() = layout()->geometry().top();
        g.setTopLeft(topLeft);
    }
    const GridLikeLayout *grid = gridLikeLayout();
    Q_ASSERT(grid);

    // extend to widget border
    QPoint bottomRight = g.bottomRight();
    if (gridRowCount(grid) == info.y())
        bottomRight.ry() = layout()->geometry().bottom();
    if (gridColumnCount(grid) == info.x())
        bottomRight.rx() = layout()->geometry().right();
    g.setBottomRight(bottomRight);
    return g;
}

// --------------  QGridLayoutSupport (LayoutDecorationExtension)
class QGridLayoutSupport: public GridLikeLayoutSupportBase<QGridLayout>
{
public:

    QGridLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent = 0);

    virtual void simplify();
    virtual void insertRow(int row);
    virtual void insertColumn(int column);

private:
};

QGridLayoutSupport::QGridLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent) :
    GridLikeLayoutSupportBase<QGridLayout>(formWindow, widget, new GridLayoutHelper, parent)
{
}

void QGridLayoutSupport::insertRow(int row)
{
    QGridLayout *grid = gridLayout();
    Q_ASSERT(grid);
    GridLayoutState state;
    state.fromLayout(grid);
    state.insertRow(row);
    state.applyToLayout(formWindow()->core(), widget());
}

void QGridLayoutSupport::insertColumn(int column)
{
    QGridLayout *grid = gridLayout();
    Q_ASSERT(grid);
    GridLayoutState state;
    state.fromLayout(grid);
    state.insertColumn(column);
    state.applyToLayout(formWindow()->core(), widget());
}

void QGridLayoutSupport::simplify()
{
    QGridLayout *grid = gridLayout();
    Q_ASSERT(grid);
    GridLayoutState state;
    state.fromLayout(grid);

    const QRect fullArea = QRect(0, 0, state.colCount, state.rowCount);
    if (state.simplify(fullArea, false))
        state.applyToLayout(formWindow()->core(), widget());
}
} //  anonymous namespace

QLayoutSupport *QLayoutSupport::createLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent)
{
    const QLayout *layout = widget->layout();
    Q_ASSERT(layout);
    QLayoutSupport *rc = 0;
    switch (LayoutInfo::layoutType(formWindow->core(), layout)) {
    case LayoutInfo::HBox:
        rc = new QBoxLayoutSupport(formWindow, widget, Qt::Horizontal, parent);
        break;
    case LayoutInfo::VBox:
        rc = new QBoxLayoutSupport(formWindow, widget, Qt::Vertical, parent);
        break;
    case LayoutInfo::Grid:
        rc = new QGridLayoutSupport(formWindow, widget, parent);
        break;
    default:
        break;
    }
    Q_ASSERT(rc);
    return rc;
}
} // namespace qdesigner_internal
// -------------- QLayoutWidget
QLayoutWidget::QLayoutWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow),
      m_leftMargin(0), m_topMargin(0), m_rightMargin(0), m_bottomMargin(0)
{
}

void QLayoutWidget::paintEvent(QPaintEvent*)
{
    if (!m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature))
        return;

    if (m_formWindow->currentTool() != 0)
        return;

    // only draw red borders if we're editting widgets

    QPainter p(this);

    if (const QLayout *lt = layout()) {
        if (const int count = lt->count()) {
            p.setPen(QPen(QColor(255, 0, 0, 35), 1));
            for (int i = 0; i < count; i++) {
                QLayoutItem *item = lt->itemAt(i);
                if (item->spacerItem()) {
                    const QRect geometry = item->geometry();
                    if (!geometry.isNull())
                        p.drawRect(QRect(geometry.x() + 1, geometry.y() + 1, geometry.width() - 2, geometry.height() - 2));
                }
            }
        }
    }
    p.setPen(QPen(Qt::red, 1));
    p.drawRect(0, 0, width() - 1, height() - 1);
}

bool QLayoutWidget::event(QEvent *e)
{
    switch (e->type()) {
        case QEvent::LayoutRequest: {
            (void) QWidget::event(e);
            // Magic: We are layouted, but the parent is not..
            if (layout() && qdesigner_internal::LayoutInfo::layoutType(formWindow()->core(), parentWidget()) == qdesigner_internal::LayoutInfo::NoLayout) {
                resize(layout()->totalMinimumSize().expandedTo(size()));
            }

            update();

            return true;
        }

        default:
            break;
    }

    return QWidget::event(e);
}

int QLayoutWidget::layoutLeftMargin() const
{
    if (m_leftMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(&margin, 0, 0, 0);
        return margin;
    }
    return m_leftMargin;
}

void QLayoutWidget::setLayoutLeftMargin(int layoutMargin)
{
    m_leftMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_leftMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(newMargin, top, right, bottom);
    }
}

int QLayoutWidget::layoutTopMargin() const
{
    if (m_topMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, &margin, 0, 0);
        return margin;
    }
    return m_topMargin;
}

void QLayoutWidget::setLayoutTopMargin(int layoutMargin)
{
    m_topMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_topMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, newMargin, right, bottom);
    }
}

int QLayoutWidget::layoutRightMargin() const
{
    if (m_rightMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, 0, &margin, 0);
        return margin;
    }
    return m_rightMargin;
}

void QLayoutWidget::setLayoutRightMargin(int layoutMargin)
{
    m_rightMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_rightMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, top, newMargin, bottom);
    }
}

int QLayoutWidget::layoutBottomMargin() const
{
    if (m_bottomMargin < 0 && layout()) {
        int margin;
        layout()->getContentsMargins(0, 0, 0, &margin);
        return margin;
    }
    return m_bottomMargin;
}

void QLayoutWidget::setLayoutBottomMargin(int layoutMargin)
{
    m_bottomMargin = layoutMargin;
    if (layout()) {
        int newMargin = m_bottomMargin;
        if (newMargin >= 0 && newMargin < ShiftValue)
            newMargin = ShiftValue;
        int left, top, right, bottom;
        layout()->getContentsMargins(&left, &top, &right, &bottom);
        layout()->setContentsMargins(left, top, right, newMargin);
    }
}
