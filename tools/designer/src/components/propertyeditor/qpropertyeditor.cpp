
#include "qpropertyeditor.h"
#include "qpropertyeditor_model_p.h"
#include "qpropertyeditor_delegate_p.h"

#include <qitemdelegate.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qscrollbar.h>

using namespace QPropertyEditor;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyCollection, dummy_collection, (QLatin1String("<empty>")))

View::View(QWidget *parent)
    : QTreeView(parent)
{
    m_model = new Model(this);
    setModel(m_model);
    m_itemDelegate = new Delegate(this);
    setItemDelegate(m_itemDelegate);
    setInitialInput(0);
    
    setAlternatingRowColors(true);
    setOddRowColor(QColor(250, 248, 235));
    setEvenRowColor(QColor(255, 255, 255));
    
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, SIGNAL(doubleClicked(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)),
            this, SLOT(open(const QModelIndex&)));

    connect(m_model, SIGNAL(propertyChanged(I::Property*)),
            this, SIGNAL(propertyChanged(I::Property*)));
}

View::~View()
{
}

bool View::isReadOnly() const
{
    return m_itemDelegate->isReadOnly();
}

void View::setReadOnly(bool readOnly)
{
    m_itemDelegate->setReadOnly(readOnly);
}

void View::setInitialInput(I::Property *initialInput)
{
    if (!initialInput)
        initialInput = dummy_collection();

    m_model->setInitialInput(initialInput);

    setSelectionMode(QTreeView::SingleSelection);
    setSelectionBehavior(QTreeView::SelectRows);
    setRootIsDecorated(true);

    setEditTriggers(QAbstractItemView::CurrentChanged);
    setRoot(m_model->indexOf(initialInput));

    int rc = m_model->rowCount(root());
    if (rc > 0) {
        QModelIndex current = m_model->index(0, 0, root());
        setCurrentIndex(current);
    }

    header()->setResizeMode(QHeaderView::Stretch, 1);
}


I::Property *View::initialInput() const
{
    return m_model->initialInput();
}

void View::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;

    I::Property *property = static_cast<const Model*>(model())->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    if (selectionModel()->isSelected(index))
        painter->fillRect(rect, QColor(230, 230, 230));

    painter->drawLine(rect.x(), rect.bottom(),
                      rect.right(), rect.bottom());

    if (model()->hasChildren(index)) {
        static const int size = 9;
        int left = rect.width() - (indentation() + size) / 2 ;
        int top = rect.y() + (rect.height() - size) / 2;
        painter->drawLine(left + 2, top + 4, left + 6, top + 4);
        if (!isOpen(index))
            painter->drawLine(left + 4, top + 2, left + 4, top + 6);
        QPen oldPen = painter->pen();
        painter->setPen(opt.palette.dark());
        painter->drawRect(left, top, size - 1, size - 1);
        painter->setPen(oldPen);
    }
}

void View::keyPressEvent(QKeyEvent *ev)
{
    QApplication::syncX();
    QTreeView::keyPressEvent(ev);
}

