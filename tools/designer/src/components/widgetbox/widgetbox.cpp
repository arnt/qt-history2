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
#include <pluginmanager.h>
#include <ui4.h>
#include <spacer.h>

#include <QtGui/QtGui>
#include <qdebug.h>

#define WIDGET QLatin1String("widget")
#define WIDGETBOXICON QLatin1String("widgetboxicon")
#define CATEGORY QLatin1String("category")

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


// ###
static QString userWidgetXmlDir()
{
    QDir dir = QDir::home();
    dir.mkdir(".qt4designer");
    dir.cd(".qt4designer");
    return dir.path() + QDir::separator();
}

static QIcon createIconSet(const QString &name)
{
    if (name.isEmpty())
        return QIcon();
    QString path = QString::fromUtf8(":/trolltech/formeditor/images/") + name;
    QPixmap result(path);
    if (result.isNull())
        qWarning("Failed to load \"%s\"", path.latin1());

    QIcon icon;
    icon.setPixmap(result, Qt::AutomaticIconSize, QIcon::Normal);
    return icon;
}

static QDomElement childElement(QDomNode node, const QString &tag,
                                const QString &attr_name = QString::null,
                                const QString &attr_value = QString::null)
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

static ElementList childElementList(QDomNode node, const QString &tag,
                                    const QString &attr_name = QString::null,
                                    const QString &attr_value = QString::null)
{
    ElementList result;
    _childElementList(node, tag, attr_name, attr_value, &result);
    return result;
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
    WidgetCollectionModel(AbstractFormEditor *core, QObject *parent = 0);

    // item model methods
    QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    QVariant data(const QModelIndex &index, int role = DisplayRole) const;
    ItemFlags flags(const QModelIndex &index) const;

    // classes in this file use these, which are easier to read
    QDomElement categoryElt(int cat_idx) const;
    QDomElement widgetElt(int cat_idx, int wgt_idx) const;
    QDomElement widgetElt(const QModelIndex &index) const;
    QDomElement widgetElt(const QString &name) const;
    bool widgetIdx(const QString &name, int *cat_idx, int *wgt_idx) const;
    int categoryIdx(const QString &name) const;
    int widgetCount(int cat_idx) const;
    int categoryCount() const;

    int addCategory(const QString &name, const QString &icon_file, DomUI *ui);
    void removeCategory(int cat_idx);

private:
    QDomDocument m_widget_xml;

    struct ModelItem {
        ModelItem(QDomElement elt = QDomElement());
        QString name;
        QIcon icon;
        QDomElement xml;
    };

    struct ModelKey {
        ModelKey(int cat = -1, int wgt = -1)
            : cat_idx(cat), wgt_idx(wgt) {}
        bool operator < (const ModelKey &other) const
            { return cat_idx == other.cat_idx ? wgt_idx < other.wgt_idx
                                                : cat_idx < other.cat_idx; }
        int cat_idx, wgt_idx;
    };

    typedef QMap<ModelKey, ModelItem> Model;
    Model m_widget_model;

    typedef QMap<QString, ModelKey> Index;
    Index m_widget_index;

    void loadModelFromXml();
    void loadCustomClasses(AbstractFormEditor *core);
    void saveToUserXmlFile();
};

WidgetCollectionModel::ModelItem::ModelItem(QDomElement elt)
{
    xml = elt;
    name = elt.attribute("name");
    icon = createIconSet(xml.attribute(WIDGETBOXICON));
}

WidgetCollectionModel::WidgetCollectionModel(AbstractFormEditor *core, QObject *parent)
    : QAbstractItemModel(parent)
{
    QString name = userWidgetXmlDir() + QLatin1String("widgetbox.xml");
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        QString name = QLatin1String(":/trolltech/widgetbox/widgetbox.xml");
        f.setFileName(name);
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning("WidgetBox: failed to open \"%s\"", name.latin1());
            return;
        }
    }

    QString error_msg;
    int line, col;
    if (!m_widget_xml.setContent(&f, &error_msg, &line, &col)) {
        qWarning("WidgetBox: failed to parse \"%s\": on line %d: %s",
                    name.latin1(), line, error_msg.latin1());
        return;
    }

    loadCustomClasses(core);
    loadModelFromXml();
}

QAbstractItemModel::ItemFlags WidgetCollectionModel::flags(const QModelIndex &) const
{
    return ItemIsEnabled;
}

void WidgetCollectionModel::loadCustomClasses(AbstractFormEditor *core)
{
    AbstractWidgetDataBase *db = core->widgetDataBase();
    int cnt = db->count();

    QList<DomWidget*> widget_list;
    for (int i = 0; i < cnt; ++i) {
        AbstractWidgetDataBaseItem *item = db->item(i);
        if (!item->isCustom())
            continue;
        DomWidget *dom_widget = new DomWidget;
        dom_widget->setAttributeClass(item->name());
        dom_widget->setAttributeName(item->name());
        widget_list.append(dom_widget);
    }

    if (!widget_list.isEmpty()) {
        QDomElement cat_elt = m_widget_xml.createElement("category");
        cat_elt.setAttribute("name", tr("Custom Widgets"));
        cat_elt.setAttribute("widgetboxicon", "");
        m_widget_xml.firstChild().appendChild(cat_elt);

        DomUI dom_ui;
        DomWidget *root = new DomWidget;
        dom_ui.setElementWidget(root);
        root->setElementWidget(widget_list);

        QDomElement ui_elt = dom_ui.write(m_widget_xml, QLatin1String("ui"));
        cat_elt.appendChild(ui_elt);

        ElementList list = childElementList(cat_elt, "widget", "name");
        foreach (QDomElement elt, list)
            elt.setAttribute("widgetboxicon", "");
    }
}

void WidgetCollectionModel::loadModelFromXml()
{
    m_widget_model.clear();
    m_widget_index.clear();
    if (m_widget_xml.isNull())
        return;

    ElementList cat_list = childElementList(m_widget_xml, "category");
    ElementList::iterator cat_it = cat_list.begin();
    int cat_idx = 0;
    for (; cat_it != cat_list.end(); ++cat_it, ++cat_idx) {
        QDomElement cat_elt = *cat_it;
        ModelItem cat_item(cat_elt);
        m_widget_model.insert(ModelKey(cat_idx, -1), cat_item);

//        qDebug() << cat_item.name;

        ElementList wgt_list = childElementList(cat_elt, "widget", WIDGETBOXICON);
        ElementList::iterator wgt_it = wgt_list.begin();
        int wgt_idx = 0;
        for (; wgt_it != wgt_list.end(); ++wgt_it, ++wgt_idx) {
            ModelItem wgt_item(*wgt_it);
            ModelKey key(cat_idx, wgt_idx);
            m_widget_model.insert(key, wgt_item);
            m_widget_index.insert(wgt_item.name, key);
//            qDebug() << "`-->" << wgt_item.name;
        }
    }
}

QDomElement WidgetCollectionModel::categoryElt(int cat_idx) const
{
    Q_ASSERT(cat_idx >= 0);

    ModelKey k(cat_idx, -1);
    Model::const_iterator it = m_widget_model.find(k);
    if (it == m_widget_model.constEnd())
        return QDomElement();
    return it.value().xml;
}

int WidgetCollectionModel::categoryIdx(const QString &name) const
{
    for (int i = 0; i < categoryCount(); ++i) {
        QString cat_name = categoryElt(i).attribute("name");
        if (cat_name == name)
            return i;
    }
    return -1;
}

QDomElement WidgetCollectionModel::widgetElt(int cat_idx, int wgt_idx) const
{
    Q_ASSERT(cat_idx >= 0);
    Q_ASSERT(wgt_idx >= 0);

    ModelKey k(cat_idx, wgt_idx);
    Model::const_iterator it = m_widget_model.find(k);
    if (it == m_widget_model.constEnd())
        return QDomElement();
    return it.value().xml;
}

QDomElement WidgetCollectionModel::widgetElt(const QString &name) const
{
    Index::const_iterator index_it = m_widget_index.find(name);
    if (index_it == m_widget_index.constEnd())
        return QDomElement();
    ModelKey k = index_it.value();

    Model::const_iterator model_it = m_widget_model.find(k);
    Q_ASSERT(model_it != m_widget_model.end());
    return model_it.value().xml;
}

bool WidgetCollectionModel::widgetIdx(const QString &name, int *cat_idx, int *wgt_idx) const
{
    Index::const_iterator index_it = m_widget_index.find(name);
    if (index_it == m_widget_index.constEnd())
        return false;
    ModelKey k = index_it.value();
    *cat_idx = k.cat_idx;
    *wgt_idx = k.wgt_idx;
    return true;
}

int WidgetCollectionModel::categoryCount() const
{
    if (m_widget_model.isEmpty())
        return 0;

    Model::const_iterator it = m_widget_model.end();
    --it;
    return it.key().cat_idx + 1;
}

int WidgetCollectionModel::widgetCount(int cat_idx) const
{
    Q_ASSERT(cat_idx >= 0);

    Model::const_iterator it = m_widget_model.lowerBound(ModelKey(cat_idx + 1, -1));
    if (it == m_widget_model.begin())
        return 0;
    --it;
    if (it.key().cat_idx != cat_idx)
        return 0;
    return it.key().wgt_idx + 1;
}

int WidgetCollectionModel::addCategory(const QString &name, const QString &icon_file,
                                        DomUI *ui)
{
    QDomElement root_elt = childElement(m_widget_xml, "widgetbox");
    if (root_elt.isNull()) {
        qDebug("WidgetCollectionModel::addCategory(): missing <widgetbox> element");
        return -1;
    }

    QDomElement cat_elt = m_widget_xml.createElement("category");
    root_elt.appendChild(cat_elt);

    QDomElement elt = ui->write(m_widget_xml, QLatin1String("ui"));
    elt.setAttribute("name", name);
    elt.setAttribute("widgetboxicon", icon_file);


    int cat_idx = categoryCount();
    cat_elt.appendChild(elt);

    saveToUserXmlFile();
    loadModelFromXml();
    emit reset();
    return cat_idx;
}

void WidgetCollectionModel::removeCategory(int cat_idx)
{
    QDomElement root_elt = childElement(m_widget_xml, "widgetbox");
    if (root_elt.isNull()) {
        qDebug("WidgetCollectionModel::removeCategory(): missing <widgetbox> element");
        return;
    }

    QDomElement cat_elt = categoryElt(cat_idx);
    if (cat_elt.isNull()) {
        qWarning("WidgetCollectionModel::removeCategory(): no category idx %d", cat_idx);
        return;
    }

    if (cat_elt.parentNode() != root_elt) {
        qDebug("WidgetCollectionModel::removeCategory(): category idx %d is not child of <widgetbox>", cat_idx);
        return;
    }

    root_elt.removeChild(cat_elt);
    saveToUserXmlFile();
    loadModelFromXml();
    emit reset();
}

QDomElement WidgetCollectionModel::widgetElt(const QModelIndex &index) const
{
    QDomElement result;

    if (!index.isValid())
        return QDomElement();

    int d = reinterpret_cast<long>(index.data());

    if (d == -1)
        return QDomElement();

    return widgetElt(d, index.row());
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
        return createIndex(row, 0, reinterpret_cast<void*>(-1));
    }

    int d = reinterpret_cast<long>(parent.data());
    if (d == -1) {
        Q_ASSERT(parent.row() < categoryCount());
        if (row >= widgetCount(parent.row()))
            return QModelIndex();
        return createIndex(row, 0, reinterpret_cast<void*>(parent.row()));
    }

    return QModelIndex();
}

QModelIndex WidgetCollectionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    int d = reinterpret_cast<long>(index.data());

    if (d == -1)
        return QModelIndex();

    Q_ASSERT(d < categoryCount());
    return createIndex(d, 0, reinterpret_cast<void*>(-1));
}

int WidgetCollectionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return categoryCount();

    int d = reinterpret_cast<long>(parent.data());

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

    int d = reinterpret_cast<long>(parent.data());

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

    int d = reinterpret_cast<long>(index.data());

    ModelKey key;
    if (d == -1) {
        Q_ASSERT(index.row() < categoryCount());
        key = ModelKey(index.row(), -1);
    } else {
        Q_ASSERT(d < categoryCount());
        Q_ASSERT(index.row() < widgetCount(d));
        key = ModelKey(d, index.row());
    }

    if (!m_widget_model.contains(key))
        return QVariant();

    const ModelItem &item = m_widget_model.value(key);

    switch (role) {
        case DisplayRole:
            return item.name;
        case DecorationRole:
            if (d == -1)
                return createIconSet("designer_object.png");
            return item.icon.pixmap(Qt::SmallIconSize, QIcon::Normal);
        default:
            return QVariant();
    };
}

void WidgetCollectionModel::saveToUserXmlFile()
{
    if (m_widget_xml.isNull())
        return;

    QString name = userWidgetXmlDir() + QLatin1String("widgetbox.xml");
    QFile f(name);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning("WidgetBox: failed to save \"%s\"", name.latin1());
        return;
    }

    QTextStream stream(&f);
    m_widget_xml.save(stream, 4);
}

/*******************************************************************************
** WidgetBoxView
*/

class WidgetBoxView : public QWidget
{
    Q_OBJECT
public:
    WidgetBoxView(WidgetCollectionModel *model, QWidget *parent = 0)
        : QWidget(parent), m_model(model) {}

signals:
    void pressed(const QDomElement &wgt_elt, const QRect &rect, Qt::MouseButton button);

protected:
    WidgetCollectionModel *model() const { return m_model; }

private:
    WidgetCollectionModel *m_model;
};

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
    m_model = model;

    header()->hide();
    //connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(fixSize()));
    setModel(model);
    header()->setResizeMode(QHeaderView::Stretch);

    QSettings settings;
    settings.beginGroup("widgetbox");

    QStringList open_cat = settings.value("open categories").toStringList();
    for (int i = 0; i < open_cat.size(); ++i) {
        int cat_idx = model->categoryIdx(open_cat[i]);
        if (cat_idx == -1)
            continue;
        QModelIndex idx = model->index(cat_idx, 0, QModelIndex());
        open(idx);
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
        if (isOpen(idx))
            open_cat.append(m_model->categoryElt(i).attribute("name"));
    }
    settings.setValue("open categories", open_cat);

    settings.endGroup();
}

void WidgetBoxListViewChild::fixSize()
{
    resizeColumnToContents(0);
}

class WidgetBoxListView : public WidgetBoxView
{
    Q_OBJECT

public:
    WidgetBoxListView(WidgetCollectionModel *model, QWidget *parent = 0);

private slots:
    void handleMousePress(const QModelIndex &index, Qt::MouseButton button);
};

WidgetBoxListView::WidgetBoxListView(WidgetCollectionModel *model, QWidget *parent)
    : WidgetBoxView(model, parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    WidgetBoxListViewChild *child = new WidgetBoxListViewChild(model, this);
    l->addWidget(child);
    connect(child, SIGNAL(pressed(const QModelIndex&, Qt::MouseButton,
                                             Qt::KeyboardModifiers)),
            this, SLOT(handleMousePress(const QModelIndex&, Qt::MouseButton)));
}

void WidgetBoxListView::handleMousePress(const QModelIndex &index, Qt::MouseButton button)
{
    if (!index.isValid())
        return;
    QDomElement elt = model()->widgetElt(index);
    if (elt.isNull())
        return;
    emit pressed(elt, QRect(), button);
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

    QLabel *label = new QLabel(0, Qt::WStyle_ToolTip);

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

WidgetBox::WidgetBox(AbstractFormEditor *core, ViewMode mode, QWidget *parent, Qt::WFlags flags)
    : AbstractWidgetBox(parent, flags), m_core(core)
{
    m_core = core;
    m_mode = mode;

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    m_model = new WidgetCollectionModel(core, this);
    m_view = 0;

#if 0 // ### disabled for now
    m_mode_action_group = new QActionGroup(this);
    m_mode_action_group->setExclusive(true);
    m_tree_mode_action = new QAction(tr("Tree View"), m_mode_action_group);
    m_tree_mode_action->setCheckable(true);
    m_form_mode_action = new QAction(tr("Form View"), m_mode_action_group);
    m_form_mode_action->setCheckable(true);

    addAction(m_tree_mode_action);
    addAction(m_form_mode_action);

    connect(m_mode_action_group, SIGNAL(triggered(QAction*)),
                this, SLOT(setViewMode(QAction*)));
#else
    m_mode_action_group = 0;
    m_tree_mode_action = 0;
    m_form_mode_action = 0;
#endif

    QSettings settings;

#if 0 // ### disabled for now
    setViewMode(WidgetBox::ViewMode(settings.value("widgetbox/boxmode",
                                                   WidgetBox::TreeMode).toInt()));
#else
    setViewMode(WidgetBox::TreeMode);
#endif
}

WidgetBox::~WidgetBox()
{
    QSettings settings;
    settings.setValue("widgetbox/boxmode", int(viewMode()));
    // delete view before model
    delete m_view;
}

AbstractFormEditor *WidgetBox::core() const
{
    return m_core;
}

void WidgetBox::handleMousePress(const QDomElement &wgt_elt, const QRect &geometry,
                                    Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        DomWidget *dom_widget = new DomWidget();
        dom_widget->read(wgt_elt);
        QList<AbstractDnDItem*> item_list;
        item_list.append(new WidgetBoxDnDItem(core(), dom_widget, geometry));
        m_core->formWindowManager()->dragItems(item_list, 0);
    }
}

void WidgetBox::setViewMode(QAction *action)
{
    if (action == m_tree_mode_action)
        setViewMode(TreeMode);
    else
        setViewMode(FormMode);
}

void WidgetBox::setViewMode(ViewMode mode)
{
    delete m_view;
    m_mode = mode;

    switch (m_mode) {
        case TreeMode:
            m_view = new WidgetBoxListView(m_model, this);
            // ### disabled for now
            // m_tree_mode_action->setChecked(true);
            break;

        case FormMode:
            Q_ASSERT(0); // ### not implemented yet!
    }

    connect(m_view, SIGNAL(pressed(const QDomElement&, const QRect&, Qt::MouseButton)),
            this, SLOT(handleMousePress(const QDomElement&, const QRect&, Qt::MouseButton)));

    layout()->addWidget(m_view);
    m_view->show();
}

int WidgetBox::categoryCount() const
{
    return m_model->categoryCount();
}

DomUI *WidgetBox::category(int cat_idx) const
{
    QDomElement elt = m_model->categoryElt(cat_idx);
    DomUI *ui = new DomUI();
    ui->read(elt);
    return ui;
}

int WidgetBox::widgetCount(int cat_idx) const
{
    return m_model->widgetCount(cat_idx);
}

DomUI *WidgetBox::widget(int cat_idx, int wgt_idx) const
{
    QDomElement elt = m_model->widgetElt(cat_idx, wgt_idx);
    DomUI *ui = new DomUI();
    ui->read(elt);
    return ui;
}

int WidgetBox::addCategory(const QString &name, const QString &icon_file, DomUI *ui)
{
    return m_model->addCategory(name, icon_file, ui);
}

void WidgetBox::removeCategory(int cat_idx)
{
    m_model->removeCategory(cat_idx);
}

#include "widgetbox.moc"
