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

#include "widgetbox.h"

#include <abstractformwindowmanager.h>
#include <abstractformeditor.h>
#include <abstractwidgetfactory.h>
#include <abstractwidgetdatabase.h>
#include <qdesigner_formbuilder.h>
#include <customwidget.h>
#include <pluginmanager.h>
#include <ui4.h>
#include <spacer_widget.h>
#include <sheet_delegate.h>
#include <iconloader.h>

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

/*******************************************************************************
** Tools
*/

class WidgetBoxDnDItem : public AbstractDnDItem
{
    Q_OBJECT
public:
    WidgetBoxDnDItem(AbstractFormEditor *core, DomWidget *dom_widget, const QRect &geometry);
    virtual DomUI *domUi() const;
    virtual QWidget *decoration() const;
    virtual ~WidgetBoxDnDItem();
    virtual QPoint hotSpot() const;

protected:
    inline AbstractFormEditor *core() const
    { return m_core; }

private:
    AbstractFormEditor *m_core;
    QWidget *m_decoration;
    DomUI *m_dom_ui;
    QPoint m_hot_spot;
};

static QDomElement childElement(QDomNode node, const QString &tag,
                                const QString &attr_name,
                                const QString &attr_value)
{
    if (node.isElement()) {
        QDomElement elt = node.toElement();
        if (elt.tagName() == tag) {
            if (attr_name.isEmpty())
                return elt;
            if (elt.hasAttribute(attr_name)) {
                if (attr_value.isEmpty())
                    return elt;
                if (elt.attribute(attr_name) == attr_value)
                    return elt;
            }
        }
    }

    QDomNode child = node.firstChild();
    for (; !child.isNull(); child = child.nextSibling()) {
        QDomElement elt = childElement(child, tag, attr_name, attr_value);
        if (!elt.isNull())
            return elt;
    }

    return QDomElement();
}

typedef QList<QDomElement> ElementList;
static void _childElementList(QDomNode node, const QString &tag,
                                    const QString &attr_name,
                                    const QString &attr_value,
                                    ElementList *result)
{
    if (node.isElement()) {
        QDomElement elt = node.toElement();
        if (elt.tagName() == tag) {
            if (attr_name.isEmpty()) {
                result->append(elt);
            } else if (elt.hasAttribute(attr_name)) {
                if (attr_value.isEmpty())
                    result->append(elt);
                else if (elt.attribute(attr_name) == attr_value)
                    result->append(elt);
            }
        }
    }

    QDomNode child = node.firstChild();
    for (; !child.isNull(); child = child.nextSibling())
        _childElementList(child, tag, attr_name, attr_value, result);
}

static QString domToString(const QDomElement &elt)
{
    QString result;
    QTextStream stream(&result, QIODevice::WriteOnly);
    elt.save(stream, 2);
    stream.flush();
    return result;
}

static DomWidget *xmlToUi(QString xml)
{
    QDomDocument doc;
    QString err_msg;
    int err_line, err_col;
    if (!doc.setContent(xml, &err_msg, &err_line, &err_col)) {
        qWarning("xmlToUi: parse failed:\n%s\n:%d:%d: %s",
                    xml.toLatin1().constData(),
                    err_line, err_col,
                    err_msg.toLatin1().constData());
        return 0;
    }

    QDomElement dom_elt = doc.firstChildElement();
    if (dom_elt.nodeName() != QLatin1String("widget")) {
        qWarning("xmlToUi: invalid root element:\n%s", xml.toLatin1().constData());
        return 0;
    }

    DomWidget *widget = new DomWidget;
    widget->read(dom_elt);
    return widget;
}


/*******************************************************************************
** WidgetBoxResource
*/

class WidgetBoxResource : public QDesignerFormBuilder
{
public:
    WidgetBoxResource(AbstractFormEditor *core)
        : QDesignerFormBuilder(core) {}

    virtual QWidget *createWidget(DomWidget *ui_widget, QWidget *parentWidget)
    { return QDesignerFormBuilder::createWidget(ui_widget, parentWidget); }

protected:
    using QDesignerFormBuilder::create;
    using QDesignerFormBuilder::createWidget;

    virtual QWidget *create(DomWidget *ui_widget, QWidget *parents);
    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
};

QWidget *WidgetBoxResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    if (widgetName == QLatin1String("Spacer")) {
        Spacer *spacer = new Spacer(parentWidget);
        spacer->setObjectName(name);
        return spacer;
    }

    return QDesignerFormBuilder::createWidget(widgetName, parentWidget, name);
}

QWidget *WidgetBoxResource::create(DomWidget *ui_widget, QWidget *parent)
{
    QWidget *result = 0;

    result = QDesignerFormBuilder::create(ui_widget, parent);
    result->setFocusPolicy(Qt::NoFocus);
    result->setObjectName(ui_widget->attributeName());

    return result;
}

/*******************************************************************************
** WidgetCollectionModel
*/

class WidgetCollectionModel : public QAbstractItemModel
{
public:
    typedef AbstractWidgetBox::Widget Widget;
    typedef AbstractWidgetBox::WidgetList WidgetList;
    typedef AbstractWidgetBox::Category Category;
    typedef AbstractWidgetBox::CategoryList CategoryList;

    WidgetCollectionModel(AbstractFormEditor *core, QObject *parent = 0);
    ~WidgetCollectionModel();

    // item model methods
    QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int categoryCount() const;
    Category category(int cat_idx) const;
    void addCategory(const Category &cat);
    void removeCategory(int cat_idx);

    int widgetCount(int cat_idx) const;
    Widget widget(int cat_idx, int wgt_idx) const;
    void addWidget(int cat_idx, const Widget &wgt);
    void removeWidget(int cat_idx, int wgt_idx);

    Widget widget(const QModelIndex &index) const;
    Widget widget(const QString &name) const;
    bool widgetIdx(const QString &name, int *cat_idx, int *wgt_idx) const;
    int categoryIdx(const QString &name) const;

private:
    CategoryList m_model;

    CategoryList xmlToModel(const QDomDocument &doc);
    Category xmlToCategory(const QDomElement &cat_elt);
    Category loadCustomCategory();

    AbstractFormEditor *m_core;

    void dumpModel();
};

WidgetCollectionModel::WidgetCollectionModel(AbstractFormEditor *core, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_core = core;

    QString name = QLatin1String(":/trolltech/widgetbox/widgetbox.xml");
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("WidgetBox: failed to open \"%s\"", name.toLatin1().constData());
        return;
    }

    QString error_msg;
    int line, col;
    QDomDocument doc;
    if (!doc.setContent(&f, &error_msg, &line, &col)) {
        qWarning("WidgetBox: failed to parse \"%s\": on line %d: %s",
                    name.toLatin1().constData(), line, error_msg.toLatin1().constData());
        return;
    }

    m_model = xmlToModel(doc);
    Category custom = loadCustomCategory();
    if (custom.widgetCount() > 0)
        m_model.append(custom);

//    dumpModel();
}

void WidgetCollectionModel::dumpModel()
{
    qDebug() << "WidgetCollectionModel::dumpModel():";

    for (int i = 0; i < categoryCount(); ++i) {
        Category cat = category(i);
        qDebug() << "Category:" << cat.name();
        for (int j = 0; j < widgetCount(i); ++j) {
            Widget wgt = cat.widget(j);
            qDebug() << "Widget:" << wgt.name();
            qDebug() << wgt.domXml();
        }
    }
}

WidgetCollectionModel::~WidgetCollectionModel()
{
}

void WidgetCollectionModel::addCategory(const Category &cat)
{
    int cnt = categoryCount();
    m_model.append(cat);
    emit rowsInserted(QModelIndex(), cnt, cnt);
}

void WidgetCollectionModel::removeCategory(int cat_idx)
{
    if (category(cat_idx).isNull()) {
        qWarning("WidgetCollectionModel::removeCategory(): index out of range: %d", cat_idx);
        return;
    }
    emit rowsAboutToBeRemoved(QModelIndex(), cat_idx, cat_idx);
    m_model.removeAt(cat_idx);
}

void WidgetCollectionModel::addWidget(int cat_idx, const Widget &wgt)
{
    Category cat = category(cat_idx);
    if (cat.isNull())
        return;
    int cnt = cat.widgetCount();
    cat.addWidget(wgt);
    emit rowsInserted(index(cat_idx, 0, QModelIndex()), cnt, cnt);
}

void WidgetCollectionModel::removeWidget(int cat_idx, int wgt_idx)
{
    Category cat = category(cat_idx);
    if (cat.isNull())
        return;
    if (cat.widget(wgt_idx).isNull())
        return;
    emit rowsAboutToBeRemoved(index(cat_idx, 0, QModelIndex()), wgt_idx, wgt_idx);
    cat.removeWidget(wgt_idx);
}

WidgetCollectionModel::CategoryList WidgetCollectionModel::xmlToModel(const QDomDocument &doc)
{
    CategoryList result;

    QDomElement root = doc.firstChildElement();
    if (root.nodeName() != QLatin1String("widgetbox")) {
        qWarning("WidgetCollectionModel::xmlToModel(): not a widgetbox file");
        return result;
    }

    QDomElement cat_elt = root.firstChildElement();
    for (; !cat_elt.isNull(); cat_elt = cat_elt.nextSiblingElement()) {
        if (cat_elt.nodeName() != QLatin1String("category")) {
            qWarning("WidgetCollectionModel::xmlToModel(): bad child of widgetbox: \"%s\"", cat_elt.nodeName().toLatin1().constData());
            continue;
        }

        result.append(xmlToCategory(cat_elt));
    }

    return result;
}

WidgetCollectionModel::Category WidgetCollectionModel::xmlToCategory(const QDomElement &cat_elt)
{
    Category result(cat_elt.attribute(QLatin1String("name")));

    QDomElement widget_elt = cat_elt.firstChildElement();
    for (; !widget_elt.isNull(); widget_elt = widget_elt.nextSiblingElement()) {
        QIcon icon = createIconSet(widget_elt.attribute(QLatin1String("icon")));
        if (icon.isNull()) {
            icon = createIconSet("qtlogo.png");
        }
        Widget w(widget_elt.attribute(QLatin1String("name")), domToString(widget_elt), icon);
        result.addWidget(w);
    }

    return result;
}


WidgetCollectionModel::Category WidgetCollectionModel::loadCustomCategory()
{
    Category result(tr("Custom Widgets"));

    PluginManager *pm = m_core->pluginManager();
    AbstractWidgetDataBase *db = m_core->widgetDataBase();
    for (int i = 0; i < db->count(); ++i) {
        AbstractWidgetDataBaseItem *item = db->item(i);
        if (!item->isCustom())
            continue;
        QString path = item->pluginPath();
        if (path.isEmpty())
            continue;
        QObject *o = pm->instance(path);
        if (o == 0)
            continue;
        ICustomWidget *c = qobject_cast<ICustomWidget*>(o);
        if (c == 0)
            continue;
        QString dom_xml = c->domXml();
        if (dom_xml.isEmpty())
            continue;

        QIcon icon = c->icon();
        if (icon.isNull()) {
            icon = createIconSet("qtlogo.png");
        }

        result.addWidget(Widget(c->name(), dom_xml, icon));
    }

    return result;
}

Qt::ItemFlags WidgetCollectionModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled;
}

WidgetCollectionModel::Category WidgetCollectionModel::category(int cat_idx) const
{
    if (cat_idx < 0 || cat_idx >= m_model.size()) {
        qWarning("WidgetCollectionModel::category(): index out of range: %d", cat_idx);
        return Category();
    }

    return m_model.at(cat_idx);
}

int WidgetCollectionModel::categoryIdx(const QString &name) const
{
    for (int i = 0; i < categoryCount(); ++i) {
        if (category(i).name() == name)
            return i;
    }
    return -1;
}

WidgetCollectionModel::Widget WidgetCollectionModel::widget(int cat_idx, int wgt_idx) const
{
    Category cat = category(cat_idx);
    if (cat.isNull())
        return Widget();

    if (wgt_idx >= cat.widgetCount())
        return Widget();

    return cat.widget(wgt_idx);
}

WidgetCollectionModel::Widget WidgetCollectionModel::widget(const QString &name) const
{
    int cat_idx, wgt_idx;
    if (widgetIdx(name, &cat_idx, &wgt_idx))
        return widget(cat_idx, wgt_idx);
    return Widget();
}

bool WidgetCollectionModel::widgetIdx(const QString &name, int *cat_idx, int *wgt_idx) const
{
    for (int i = 0; i < categoryCount(); ++i) {
        for (int j = 0; j < widgetCount(i); ++j) {
            Widget w = widget(i, j);
            if (w.name() == name) {
                *cat_idx = i;
                *wgt_idx = j;
                return true;
            }
        }
    }

    return false;
}

int WidgetCollectionModel::categoryCount() const
{
    return m_model.size();
}

int WidgetCollectionModel::widgetCount(int cat_idx) const
{
    Category cat = category(cat_idx);
    if (cat.isNull())
        return 0;
    return cat.widgetCount();
}

WidgetCollectionModel::Widget WidgetCollectionModel::widget(const QModelIndex &index) const
{
    if (!index.isValid())
        return Widget();

    qint64 d = index.internalId();

    if (d == -1)
        return Widget();

    return widget(d, index.row());
}

QModelIndex WidgetCollectionModel::index(int row, int column,
                                            const QModelIndex &parent) const
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(column >= 0);

    if (column != 0)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= categoryCount())
            return QModelIndex();
        return createIndex(row, 0, -1);
    }

    qint64 d = parent.internalId();
    if (d == -1) {
        if (parent.row() >= categoryCount())
            return QModelIndex();
        if (row >= widgetCount(parent.row()))
            return QModelIndex();
        return createIndex(row, 0, parent.row());
    }

    return QModelIndex();
}

QModelIndex WidgetCollectionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    qint64 d = index.internalId();

    if (d == -1)
        return QModelIndex();

    Q_ASSERT(d < categoryCount());
    return createIndex(d, 0, -1);
}

int WidgetCollectionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return categoryCount();

    qint64 d = parent.internalId();

    if (d == -1) {
        Q_ASSERT(parent.row() < categoryCount());
        return widgetCount(parent.row());
    }

    return 0;
}

int WidgetCollectionModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;

    qint64 d = parent.internalId();

    if (d == -1) {
        Q_ASSERT(parent.row() < categoryCount());
        return 1;
    }

    return 0;
}

QVariant WidgetCollectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    qint64 d = index.internalId();

    QVariant result;

    switch (role) {
        case Qt::DisplayRole:
            if (d == -1) {
                Category cat = category(index.row());
                if (!cat.isNull())
                    result = cat.name();
            } else {
                Widget wgt = widget(d, index.row());
                if (!wgt.isNull())
                    result = wgt.name();
            }
            break;
        case Qt::DecorationRole:
            if (d != -1) {
                Widget wgt = widget(d, index.row());
                if (!wgt.isNull())
                    result = qVariantFromValue(wgt.icon());
            }
            break;
        default:
            break;
    };

    return result;
}

/*******************************************************************************
** WidgetBoxContainer
*/

class WidgetBoxContainer : public QWidget
{
    Q_OBJECT
public:
    WidgetBoxContainer(WidgetCollectionModel *model, QWidget *parent = 0)
        : QWidget(parent), m_model(model) {}

signals:
    void pressed(const QString &xml, const QRect &rect);

protected:
    WidgetCollectionModel *model() const { return m_model; }
    virtual void contextMenuEvent (QContextMenuEvent *e);


private:
    WidgetCollectionModel *m_model;
};

void WidgetBoxContainer::contextMenuEvent (QContextMenuEvent *e)
{
    e->accept();
}

/*******************************************************************************
** WidgetBoxListView
*/

class WidgetBoxListViewChild: public QTreeView
{
    Q_OBJECT

public:
    WidgetBoxListViewChild(WidgetCollectionModel *model, QWidget *parent = 0);
    ~WidgetBoxListViewChild();

private slots:
    void fixSize();

private:
    WidgetCollectionModel *m_model;
};

WidgetBoxListViewChild::WidgetBoxListViewChild(WidgetCollectionModel *model, QWidget *parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);

    m_model = model;

    header()->hide();
    //connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(fixSize()));
    setModel(model);
    header()->setResizeMode(QHeaderView::Stretch);

    QSettings settings;
    settings.beginGroup("widgetbox");

    QStringList open_cat;
    for (int i = 0; i < m_model->categoryCount(); ++i) {
        open_cat.append(m_model->category(i).name());
    }

    open_cat = settings.value("open categories", open_cat).toStringList();
    for (int i = 0; i < open_cat.size(); ++i) {
        int cat_idx = model->categoryIdx(open_cat[i]);
        if (cat_idx == -1)
            continue;
        QModelIndex idx = model->index(cat_idx, 0, QModelIndex());
        expand(idx);
    }

    settings.endGroup();
}

WidgetBoxListViewChild::~WidgetBoxListViewChild()
{
    QSettings settings;
    settings.beginGroup("widgetbox");

    QStringList open_cat;
    for (int i = 0; i < m_model->categoryCount(); ++i) {
        QModelIndex idx = m_model->index(i, 0, QModelIndex());
        if (isExpanded(idx))
            open_cat.append(m_model->category(i).name());
    }
    settings.setValue("open categories", open_cat);

    settings.endGroup();
}

void WidgetBoxListViewChild::fixSize()
{
    resizeColumnToContents(0);
}

class WidgetBoxListView : public WidgetBoxContainer
{
    Q_OBJECT

public:
    WidgetBoxListView(WidgetCollectionModel *model, QWidget *parent = 0);

private slots:
    void handleMousePress(const QModelIndex &index);

private:
    WidgetBoxListViewChild *m_list;
    WidgetCollectionModel *m_model;
};

WidgetBoxListView::WidgetBoxListView(WidgetCollectionModel *model, QWidget *parent)
    : WidgetBoxContainer(model, parent)
{
    m_model = model;

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    m_list = new WidgetBoxListViewChild(model, this);
    l->addWidget(m_list);
    m_list->setItemDelegate(new SheetDelegate(m_list, m_list));

    connect(m_list, SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(handleMousePress(const QModelIndex&)));
}

void WidgetBoxListView::handleMousePress(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!m_model->parent(index).isValid()) {
        m_list->setExpanded(index, !m_list->isExpanded(index));
        return;
    }

    WidgetCollectionModel::Widget wgt = m_model->widget(index);
    if (wgt.isNull())
        return;

    emit pressed(wgt.domXml(), QRect());
}

class CollectionFrame : public QFrame
{
    Q_OBJECT
public:
    CollectionFrame(QWidget *child, QWidget *parent = 0);
};

CollectionFrame::CollectionFrame(QWidget *child, QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);

    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setMargin(0);
    hl->addStretch(1);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    hl->addLayout(vl, 0);
    vl->addStretch(1);
    child->setParent(this);
    vl->addWidget(child, 0);
    vl->addStretch(1);

    hl->addStretch(1);

    show();
}


/*******************************************************************************
** WidgetBoxDnDItem
*/

static QSize geometryProp(DomWidget *dw)
{
    QList<DomProperty*> prop_list = dw->elementProperty();
    foreach (DomProperty *prop, prop_list) {
        if (prop->attributeName() != QLatin1String("geometry"))
            continue;
        DomRect *dr = prop->elementRect();
        if (dr == 0)
            continue;
        return QSize(dr->elementWidth(), dr->elementHeight());
    }
    return QSize();
}

static QSize domWidgetSize(DomWidget *dw)
{
    QSize size = geometryProp(dw);
    if (size.isValid())
        return size;

    foreach (DomWidget *child, dw->elementWidget()) {
        size = geometryProp(child);
        if (size.isValid())
            return size;
    }

    foreach (DomLayout *dl, dw->elementLayout()) {
        foreach (DomLayoutItem *item, dl->elementItem()) {
            DomWidget *child = item->elementWidget();
            if (child == 0)
                continue;
            size = geometryProp(child);
            if (size.isValid())
                return size;
        }
    }

    return QSize();
}

WidgetBoxDnDItem::WidgetBoxDnDItem(AbstractFormEditor *core, DomWidget *dom_widget, const QRect &geometry)
    : m_core(core)
{
    DomWidget *root_dom_widget = new DomWidget;
    QList<DomWidget*> child_list;
    child_list.append(dom_widget);
    root_dom_widget->setElementWidget(child_list);
    m_dom_ui = new DomUI();
    m_dom_ui->setElementWidget(root_dom_widget);

    QLabel *label = new QLabel(0, Qt::ToolTip);

    WidgetBoxResource builder(m_core);
    QWidget *w = builder.createWidget(dom_widget, label);
    QSize size = domWidgetSize(dom_widget);
    if (!size.isValid())
        size = w->sizeHint();
    w->setGeometry(QRect(QPoint(), size));
    label->resize(size);

    label->setWindowOpacity(0.8);
    m_decoration = label;

    QPoint pos = QCursor::pos();

    if (geometry.isValid())
        m_decoration->setGeometry(geometry);
    else
        m_decoration->move(pos - QPoint(size.width(), size.height())/2);

    m_hot_spot = pos - m_decoration->geometry().topLeft();
}

DomUI *WidgetBoxDnDItem::domUi() const
{
    return m_dom_ui;
}

QWidget *WidgetBoxDnDItem::decoration() const
{
    return m_decoration;
}

WidgetBoxDnDItem::~WidgetBoxDnDItem()
{
    delete m_dom_ui;
    m_decoration->deleteLater();
}

QPoint WidgetBoxDnDItem::hotSpot() const
{
    return m_hot_spot;
}

/*******************************************************************************
** WidgetBox
*/

WidgetBox::WidgetBox(AbstractFormEditor *core, QWidget *parent, Qt::WFlags flags)
    : AbstractWidgetBox(parent, flags), m_core(core)
{
    m_core = core;

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    m_model = new WidgetCollectionModel(core, this);
    m_view = new WidgetBoxListView(m_model, this);
    l->addWidget(m_view);

    connect(m_view, SIGNAL(pressed(const QString&, const QRect&)),
            this, SLOT(handleMousePress(const QString&, const QRect&)));
}

WidgetBox::~WidgetBox()
{
    // delete view before model
    delete m_view;
}

AbstractFormEditor *WidgetBox::core() const
{
    return m_core;
}

void WidgetBox::handleMousePress(const QString &xml, const QRect &geometry)
{
    DomWidget *dom_widget = xmlToUi(xml);
    if (dom_widget == 0)
        return;
    if (QApplication::mouseButtons() == Qt::LeftButton) {
        QList<AbstractDnDItem*> item_list;
        item_list.append(new WidgetBoxDnDItem(core(), dom_widget, geometry));
        m_core->formWindowManager()->dragItems(item_list, 0);
    }
}

int WidgetBox::categoryCount() const
{
    return m_model->categoryCount();
}

AbstractWidgetBox::Category WidgetBox::category(int cat_idx) const
{
    return m_model->category(cat_idx);
}

void WidgetBox::addCategory(const Category &cat)
{
    m_model->addCategory(cat);
}

void WidgetBox::removeCategory(int cat_idx)
{
    m_model->removeCategory(cat_idx);
}

int WidgetBox::widgetCount(int cat_idx) const
{
    return m_model->widgetCount(cat_idx);
}

AbstractWidgetBox::Widget WidgetBox::widget(int cat_idx, int wgt_idx) const
{
    return m_model->widget(cat_idx, wgt_idx);
}

void WidgetBox::addWidget(int cat_idx, const Widget &wgt)
{
    m_model->addWidget(cat_idx, wgt);
}

void WidgetBox::removeWidget(int cat_idx, int wgt_idx)
{
    m_model->removeWidget(cat_idx, wgt_idx);
}

#include "widgetbox.moc"
