#include "demoviewer.h"
#include "demowidget.h"
#include "introscreen.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgenericlistview.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qsplitter.h>
#include <qstackedbox.h>
#include <qvbox.h>

#include <qdebug.h>


class ItemDelegate : public QAbstractItemDelegate
{
public:
    ItemDelegate(QAbstractItemModel *model)
        : QAbstractItemDelegate(model)
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &options,
                       const QAbstractItemModel *model, const QModelIndex &index) const;
    virtual QSize sizeHint(const QFontMetrics &, const QStyleOptionViewItem &,
                           const QAbstractItemModel *, const QModelIndex &) const
    {
        return QSize(100, 30);
    }
};

class ItemModel : public QAbstractListModel
{
public:
    QVariant data(const QModelIndex &index, int role) const {
        if ((role == DisplayRole)
            && index.type() == QModelIndex::View  && index.isValid()
            && index.row() < rowCount()
            && index.column() < columnCount()) {
            if (role == DisplayRole)
                return list.at(index.row());
        }
        return QVariant::Invalid;
    }

    int rowCount() const {
        return list.count();
    }

    bool setData(const QModelIndex &index, int role, const QVariant &value) {
        if (role == DisplayRole) {
            list[index.row()] = value.toString();
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }

    bool insertRows(int row, const QModelIndex &parent, int count) {
        if (parent.isValid() || count < 1 || row < 0 || row > rowCount())
            return false;
        while (count--)
            list.insert(row, QString());
        emit rowsInserted(parent, row, row+count-1);
        return true;
    }

private:
    QList<QString> list;
};

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options,
                         const QAbstractItemModel *model, const QModelIndex &index) const
{
    QColor base = qApp->palette().color(QPalette::Base);

    QColor g1 = base.light();
    QColor g2 = base.dark();

    bool selected = (options.state & QStyle::Style_Selected) != 0;

    if (selected) {
        g1 = g1.dark(150);
        g2 = g2.dark(150);

        g1 = QColor(g1.red(), g1.green(), g1.blue() + 70);
        g2 = QColor(g2.red(), g2.green(), g2.blue() + 70);
    }

    QRect r = options.rect;
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(r.topLeft(), g1, r.bottomLeft(), g2));
    painter->drawRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
    painter->setBrush(Qt::NoBrush);

    painter->setPen(Qt::black);
    QRect textRect = r;
    if (selected)
        textRect.moveBy(1, 1);
    painter->drawText(textRect, Qt::AlignCenter, model->data(index).toString());

    painter->setPen(selected ? Qt::white : Qt::black);
    painter->drawEdges(r, Qt::RectangleEdges(Qt::BottomEdge | Qt::RightEdge));

    painter->setPen(selected ? Qt::black : Qt::white);
    painter->drawEdges(r, Qt::RectangleEdges(Qt::TopEdge | Qt::LeftEdge));
}


DemoViewer::DemoViewer(QWidget *parent)
    : QWidget(parent),
      attributes(new Attributes)
{
    setWindowTitle(tr("Qt Paint Engine Demo"));
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);

    QSplitter *horSplit = new QSplitter(Qt::Horizontal, this);

    QVBox *vbox = new QVBox(horSplit);
    vbox->setMargin(6);
    vbox->setSpacing(6);
    QGroupBox *categories = new QGroupBox("Categories", vbox);
    QBoxLayout *glayout = new QBoxLayout(QBoxLayout::TopToBottom, categories);
    listView = new QGenericListView(categories);
    glayout->addWidget(listView);

    layout->addWidget(horSplit);

    QSplitter *verSplit = new QSplitter(Qt::Vertical, horSplit);
    widgets = new QStackedBox(verSplit);

    QGroupBox *opts = new QGroupBox("Options", vbox);
    QBoxLayout *props = new QBoxLayout(QBoxLayout::TopToBottom, opts);

    antialias = new QCheckBox(tr("Antialiasing"), opts);
    alpha = new QCheckBox(tr("Alphablended primitives"), opts);

    bgMode = new QComboBox(opts);
    QStringList items;
    items << tr("Solid Fill")
          << tr("Gradient Fill")
          << tr("Tiles")
          << tr("Background image");
    bgMode->insertStringList(items);

    props->addWidget(antialias);
    props->addWidget(alpha);
    props->addWidget(bgMode);
    props->addItem(new QSpacerItem(1, 1));

    // Setting it up...
    listView->setModel(new ItemModel);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setItemDelegate(new ItemDelegate(listView->model()));
    connect(listView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(itemSelected()));
    connect(antialias, SIGNAL(toggled(bool)), this, SLOT(antialiasChanged(bool)));
    connect(alpha, SIGNAL(toggled(bool)), this, SLOT(alphaChanged(bool)));
    connect(bgMode, SIGNAL(activated(int)), this, SLOT(fillModeChanged(int)));

    antialias->setChecked(true);
    alpha->setChecked(true);
    bgMode->setCurrentItem(1);
    attributes->fillMode = Attributes::Gradient;
}

void DemoViewer::addDemoWidget(const QString &name, DemoWidget *widget)
{
    QAbstractItemModel *model = listView->model();
    model->insertRows(model->rowCount(QModelIndex()), QModelIndex(), 1);
    model->setData(model->index(model->rowCount(QModelIndex()) - 1, 0),
                   QAbstractItemModel::DisplayRole, name);

    widget->setParent(widgets);
    widget->setAttributes(attributes);

    widgetByName[name] = widget;
}

QSize DemoViewer::sizeHint() const
{
    return QSize(800, 600);
}

void DemoViewer::itemSelected()
{
    QString name = listView->model()->data(listView->selectionModel()->currentItem()).toString();

    Q_ASSERT(!name.isEmpty());
    DemoWidget *demoWidget = widgetByName[name];

    Q_ASSERT(demoWidget);

    DemoWidget *oldDemoWidget = reinterpret_cast<DemoWidget*>(widgets->currentWidget());
    oldDemoWidget->stopAnimation();

    widgets->setCurrentIndex(widgets->indexOf(demoWidget));
    demoWidget->startAnimation();
}

void DemoViewer::antialiasChanged(bool val)
{
    attributes->antialias = val;
}

void DemoViewer::alphaChanged(bool val)
{
    attributes->alpha = val;
}

void DemoViewer::fillModeChanged(int mode)
{
    attributes->fillMode = static_cast<Attributes::BackgroundFill>(mode);
}

void DemoViewer::showEvent(QShowEvent *)
{
    if (!listView->currentItem().isValid()) {
        listView->selectionModel()->setCurrentItem(listView->model()->index(0, 0),
                                                   QItemSelectionModel::ClearAndSelect);
    }
    itemSelected();
}

void DemoViewer::hideEvent(QHideEvent *)
{
    DemoWidget *demoWidget = reinterpret_cast<DemoWidget*>(widgets->currentWidget());
    demoWidget->stopAnimation();
}
