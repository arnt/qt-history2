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

#include "demoviewer.h"
#include "demowidget.h"
#include "introscreen.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistwidget.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qsplitter.h>
#include <qstackedbox.h>
#include <qvboxwidget.h>

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
    virtual QSize sizeHint(const QStyleOptionViewItem &,
                           const QAbstractItemModel *, const QModelIndex &) const
    {
        return QSize(100, 30);
    }
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

    QVBoxWidget *vbox = new QVBoxWidget(horSplit);
    vbox->setMargin(6);
    vbox->setSpacing(6);
    QGroupBox *categories = new QGroupBox("Categories", vbox);
    QBoxLayout *glayout = new QBoxLayout(QBoxLayout::TopToBottom, categories);
    listWidget = new QListWidget(categories);
    glayout->addWidget(listWidget);

    layout->addWidget(horSplit);

    widgets = new QStackedBox(horSplit);
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

    QApplication::sendPostedEvents();

    QList<int> l;
    l.append(100);
    l.append(700);
    horSplit->setSizes(l);

    // Setting it up...
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setItemDelegate(new ItemDelegate(listWidget->model()));
    connect(listWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
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
    listWidget->appendItem(name);
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
    QString name = listWidget->model()->data(listWidget->selectionModel()->currentIndex()).toString();

    Q_ASSERT(!name.isEmpty());
    DemoWidget *demoWidget = widgetByName[name];

    Q_ASSERT(demoWidget);

    DemoWidget *oldDemoWidget = qt_cast<DemoWidget*>(widgets->currentWidget());
    oldDemoWidget->stopAnimation();

    widgets->setCurrentIndex(widgets->indexOf(demoWidget));
    demoWidget->startAnimation();
}

void DemoViewer::antialiasChanged(bool val)
{
    attributes->antialias = val;
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::alphaChanged(bool val)
{
    attributes->alpha = val;
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::fillModeChanged(int mode)
{
    attributes->fillMode = static_cast<Attributes::BackgroundFill>(mode);
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::showEvent(QShowEvent *)
{
    if (!listWidget->currentIndex().isValid()) {
        listWidget->selectionModel()->setCurrentIndex(listWidget->model()->index(0, 0),
						    QItemSelectionModel::ClearAndSelect);
    }
    itemSelected();
}

void DemoViewer::hideEvent(QHideEvent *)
{
    DemoWidget *demoWidget = qt_cast<DemoWidget*>(widgets->currentWidget());
    demoWidget->stopAnimation();
}
