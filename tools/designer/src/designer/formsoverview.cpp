
#include "formsoverview.h"

// sdk
#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>
#include <abstractformwindow.h>
#include <qdesigner_formbuilder.h>

#include <QTimer>
#include <QPainter>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QItemDelegate>
#include <QApplication>

#include <qdebug.h>

Q_DECLARE_METATYPE(AbstractFormWindow*)

class FormsOverviewDelegate: public QItemDelegate
{
public:
    FormsOverviewDelegate(QObject *parent = 0)
        : QItemDelegate(parent)
    {}

    virtual ~FormsOverviewDelegate()
    {}

    virtual void paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
    {
        bool selected = option.state & QStyle::State_Selected;
        painter->fillRect(option.rect, selected ? Qt::lightGray : Qt::white);

        const QAbstractItemModel *model = index.model();
        QString text = model->data(index).toString();
        QIcon icon = model->data(index, QAbstractItemModel::DecorationRole).toIcon();

        QSize iconSize(128, 128);
        QPoint topLeft = option.rect.topLeft();
        painter->drawPixmap(QRect(topLeft, iconSize),
                            icon.pixmap(iconSize));

        int h = painter->fontMetrics().height();
        int x = topLeft.x() + iconSize.width() + 8;
        int y = topLeft.y() + h;
        painter->drawText(x, y, text);
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
    { return QItemDelegate::sizeHint(option, index).expandedTo(QSize(128, 128 + 2)); }
};


FormsOverview::FormsOverview(AbstractFormEditor *core, QWidget *parent)
    : QTreeWidget(parent),
      m_core(core)
{
    setItemDelegate(new FormsOverviewDelegate(this));
    m_updateFormsTimer = new QTimer(this);
    connect(m_updateFormsTimer, SIGNAL(timeout()),
            this, SLOT(updateFormsNow()));

    setColumnCount(1);
    setItemHidden(headerItem(), true);

    connect(formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
            this, SLOT(activeFormWindowChanged(AbstractFormWindow*)));
    connect(formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow*)),
            this, SLOT(formWindowAdded(AbstractFormWindow*)));
    connect(formWindowManager(), SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            this, SLOT(formWindowRemoved(AbstractFormWindow*)));

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)),
            this, SLOT(formActivated(QTreeWidgetItem*, int)));
}

FormsOverview::~FormsOverview()
{
}

AbstractFormEditor *FormsOverview::core() const
{
    return m_core;
}

AbstractFormWindowManager *FormsOverview::formWindowManager() const
{
    return m_core->formWindowManager();
}

void FormsOverview::activeFormWindowChanged(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
}

void FormsOverview::formWindowAdded(AbstractFormWindow *formWindow)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    QString fileName = formWindow->fileName();
    if (fileName.isEmpty())
        fileName = formWindow->windowTitle();

    item->setText(0, tr("File name: %1\n").arg(fileName));

    QVariant v;
    qVariantSet(v, formWindow);
    item->setData(0, QAbstractItemModel::UserRole + 1, v);

    connect(formWindow, SIGNAL(changed()), this, SLOT(updateForms()));

    m_items.insert(formWindow, item);
}

void FormsOverview::formWindowRemoved(AbstractFormWindow *formWindow)
{
    if (QTreeWidgetItem *item = m_items.value(formWindow)) {
        m_items.remove(formWindow);
        delete item;
    }
}

void FormsOverview::formActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (AbstractFormWindow *fw = formWindow(item)) {
        fw->activateWindow();
        fw->show();
        fw->raise();
    }
}

AbstractFormWindow *FormsOverview::formWindow(QTreeWidgetItem *item) const
{
    if (!item)
        return 0;

    QVariant v = item->data(0, QAbstractItemModel::UserRole + 1);
    AbstractFormWindow *formWindow = 0;
    qVariantGet(v, formWindow);

    return formWindow;
}

void FormsOverview::updateForms()
{
    m_updateFormsTimer->start(250);
}

void FormsOverview::updateFormsNow()
{
    m_updateFormsTimer->stop();

    QDesignerFormBuilder builder(core());

    QHashIterator<AbstractFormWindow*, QTreeWidgetItem*> it(m_items);
    while (it.hasNext()) {
        it.next();

        AbstractFormWindow *formWindow = it.key();
        QTreeWidgetItem *item = it.value();

        QWidget *w = builder.createWidgetFromContents(formWindow->contents(), 0);
        w->setGeometry(10, 10, w->size().width(), w->size().height());
        qApp->processEvents(); // ### remove me
        w->ensurePolished();
        QPixmap pix = QPixmap::grabWidget(w);
        QImage img = pix.toImage();
        img.scale(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pix = img;
        item->setIcon(0, pix);
        delete w;

        QString fileName = formWindow->fileName();
        if (fileName.isEmpty())
            fileName = formWindow->windowTitle();

        item->setText(0, tr("File name: %1\n").arg(fileName));
    }
}

