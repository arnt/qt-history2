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
#include <customwidget.h>
#include <pluginmanager.h>
#include <ui4.h>
#include <sheet_delegate.h>
#include <iconloader.h>

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

#include "widgetbox_dnditem.h"

/*******************************************************************************
** Tools
*/
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
** WidgetBoxTreeView
*/

class WidgetBoxTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    typedef AbstractWidgetBox::Widget Widget;
    typedef AbstractWidgetBox::Category Category;
    typedef AbstractWidgetBox::CategoryList CategoryList;

    WidgetBoxTreeView(AbstractFormEditor *core, QWidget *parent = 0);
    ~WidgetBoxTreeView();

    QString fileName() const;
    bool load(const QString &name);

    int categoryCount() const;
    Category category(int cat_idx) const;
    void addCategory(const Category &cat);
    void removeCategory(int cat_idx);

    int widgetCount(int cat_idx) const;
    Widget widget(int cat_idx, int wgt_idx) const;
    void addWidget(int cat_idx, const Widget &wgt);
    void removeWidget(int cat_idx, int wgt_idx);

signals:
    void pressed(const QString dom_xml, const QPoint &global_mouse_pos);

private slots:
    void handleMousePress(QTreeWidgetItem *item);

private:
    AbstractFormEditor *m_core;
    QString m_file_name;

    CategoryList domToCateogryList(const QDomDocument &doc) const;
    Category domToCategory(const QDomElement &cat_elt) const;
    Category loadCustomCategory() const;

    QTreeWidgetItem *widgetToItem(const Widget &wgt, QTreeWidgetItem *parent) const;
    Widget itemToWidget(const QTreeWidgetItem *item) const;

    int categoryIndex(const QString &name) const;
};

WidgetBoxTreeView::WidgetBoxTreeView(AbstractFormEditor *core, QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new SheetDelegate(this, this));
    setRootIsDecorated(false);
    setColumnCount(1);
    header()->hide();
    header()->setResizeMode(QHeaderView::Stretch);

    m_core = core;

    load(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));

    QSettings settings;
    settings.beginGroup("WidgetBox");

    QStringList open_cat;
    for (int i = 0; i < categoryCount(); ++i)
        open_cat.append(category(i).name());

    open_cat = settings.value("open categories", open_cat).toStringList();
    for (int i = 0; i < open_cat.size(); ++i) {
        int cat_idx = categoryIndex(open_cat[i]);
        if (cat_idx == -1)
            continue;
        QTreeWidgetItem *item = topLevelItem(cat_idx);
        if (item == 0)
            continue;
        setItemExpanded(item, true);
    }

    settings.endGroup();

    connect(this, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
            this, SLOT(handleMousePress(QTreeWidgetItem*)));

}

WidgetBoxTreeView::~WidgetBoxTreeView()
{
    QSettings settings;
    settings.beginGroup("WidgetBox");

    QStringList open_cat;
    for (int i = 0; i < categoryCount(); ++i) {
        QTreeWidgetItem *cat_item = topLevelItem(i);
        if (isItemExpanded(cat_item))
            open_cat.append(cat_item->text(0));
    }
    settings.setValue("open categories", open_cat);

    settings.endGroup();
}

void WidgetBoxTreeView::handleMousePress(QTreeWidgetItem *item)
{
    if (item == 0)
        return;

    if (item->parent() == 0) {
        setItemExpanded(item, !isItemExpanded(item));
        return;
    }

    AbstractWidgetBox::Widget wgt = qvariant_cast<AbstractWidgetBox::Widget>(item->data(0, Qt::UserRole));
    if (wgt.isNull())
        return;

    emit pressed(wgt.domXml(), QCursor::pos());
}

int WidgetBoxTreeView::categoryIndex(const QString &name) const
{
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->text(0) == name)
            return i;
    }
    return -1;
}

QString WidgetBoxTreeView::fileName() const
{
    return m_file_name;
}

bool WidgetBoxTreeView::load(const QString &name)
{
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("WidgetBox: failed to open \"%s\"", name.toLatin1().constData());
        return false;
    }

    QString error_msg;
    int line, col;
    QDomDocument doc;
    if (!doc.setContent(&f, &error_msg, &line, &col)) {
        qWarning("WidgetBox: failed to parse \"%s\": on line %d: %s",
                    name.toLatin1().constData(), line, error_msg.toLatin1().constData());
        return false;
    }

    CategoryList cat_list = domToCateogryList(doc);
    if (cat_list.isEmpty())
        return false;

    foreach(Category cat, cat_list)
        addCategory(cat);

    Category custom_cat = loadCustomCategory();
    if (!custom_cat.isNull())
        addCategory(custom_cat);

    m_file_name = name;

    return true;
}

WidgetBoxTreeView::CategoryList WidgetBoxTreeView::domToCateogryList(const QDomDocument &doc) const
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
            return result;
        }

        Category cat = domToCategory(cat_elt);
        if (!cat.isNull())
            result.append(cat);
    }

    return result;
}

WidgetBoxTreeView::Category WidgetBoxTreeView::domToCategory(const QDomElement &cat_elt) const
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


WidgetBoxTreeView::Category WidgetBoxTreeView::loadCustomCategory() const
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

QTreeWidgetItem *WidgetBoxTreeView::widgetToItem(const Widget &wgt, QTreeWidgetItem *parent) const
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    item->setText(0, wgt.name());
    item->setIcon(0, wgt.icon());
    item->setData(0, Qt::UserRole, qVariantFromValue(wgt));

    return item;
}

WidgetBoxTreeView::Widget WidgetBoxTreeView::itemToWidget(const QTreeWidgetItem *item) const
{
    return qvariant_cast<Widget>(item->data(0, Qt::UserRole));
}

int WidgetBoxTreeView::categoryCount() const
{
    return topLevelItemCount();
}

WidgetBoxTreeView::Category WidgetBoxTreeView::category(int cat_idx) const
{
    Category result;

    if (cat_idx >= topLevelItemCount())
        return result;

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);
    result.setName(cat_item->text(0));

    for (int i = 0; i < cat_item->childCount(); ++i) {
        QTreeWidgetItem *child = cat_item->child(i);
        result.addWidget(itemToWidget(child));
    }

    return result;
}

void WidgetBoxTreeView::addCategory(const Category &cat)
{
    QTreeWidgetItem *cat_item = new QTreeWidgetItem(this);
    cat_item->setText(0, cat.name());

    for (int i = 0; i < cat.widgetCount(); ++i)
        widgetToItem(cat.widget(i), cat_item);
}

void WidgetBoxTreeView::removeCategory(int cat_idx)
{
    if (cat_idx >= topLevelItemCount())
        return;
    delete takeTopLevelItem(cat_idx);
}

int WidgetBoxTreeView::widgetCount(int cat_idx) const
{
    if (cat_idx >= topLevelItemCount())
        return 0;

    return topLevelItem(cat_idx)->childCount();
}

WidgetBoxTreeView::Widget WidgetBoxTreeView::widget(int cat_idx, int wgt_idx) const
{
    if (cat_idx >= topLevelItemCount())
        return Widget();

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);

    if (wgt_idx >= cat_item->childCount())
        return Widget();

    return itemToWidget(cat_item->child(wgt_idx));
}

void WidgetBoxTreeView::addWidget(int cat_idx, const Widget &wgt)
{
    if (cat_idx >= topLevelItemCount())
        return;

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);

    widgetToItem(wgt, cat_item);
}

void WidgetBoxTreeView::removeWidget(int cat_idx, int wgt_idx)
{
    if (cat_idx >= topLevelItemCount())
        return;

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);

    if (wgt_idx >= cat_item->childCount())
        return;

    delete cat_item->takeChild(wgt_idx);
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
** WidgetBox
*/

WidgetBox::WidgetBox(AbstractFormEditor *core, QWidget *parent, Qt::WFlags flags)
    : AbstractWidgetBox(parent, flags), m_core(core)
{
    m_core = core;

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    m_view = new WidgetBoxTreeView(m_core, this);
    l->addWidget(m_view);

    connect(m_view, SIGNAL(pressed(const QString&, const QPoint&)),
            this, SLOT(handleMousePress(const QString&, const QPoint&)));
}

WidgetBox::~WidgetBox()
{
}

AbstractFormEditor *WidgetBox::core() const
{
    return m_core;
}

void WidgetBox::handleMousePress(const QString &xml, const QPoint &global_mouse_pos)
{
    DomWidget *dom_widget = xmlToUi(xml);
    if (dom_widget == 0)
        return;
    if (QApplication::mouseButtons() == Qt::LeftButton) {
        QList<AbstractDnDItem*> item_list;
        item_list.append(new WidgetBoxDnDItem(core(), dom_widget, global_mouse_pos));
        m_core->formWindowManager()->dragItems(item_list);
    }
}

int WidgetBox::categoryCount() const
{
    return m_view->categoryCount();
}

AbstractWidgetBox::Category WidgetBox::category(int cat_idx) const
{
    return m_view->category(cat_idx);
}

void WidgetBox::addCategory(const Category &cat)
{
    m_view->addCategory(cat);
}

void WidgetBox::removeCategory(int cat_idx)
{
    m_view->removeCategory(cat_idx);
}

int WidgetBox::widgetCount(int cat_idx) const
{
    return m_view->widgetCount(cat_idx);
}

AbstractWidgetBox::Widget WidgetBox::widget(int cat_idx, int wgt_idx) const
{
    return m_view->widget(cat_idx, wgt_idx);
}

void WidgetBox::addWidget(int cat_idx, const Widget &wgt)
{
    m_view->addWidget(cat_idx, wgt);
}

void WidgetBox::removeWidget(int cat_idx, int wgt_idx)
{
    m_view->removeWidget(cat_idx, wgt_idx);
}

void WidgetBox::reload()
{
    m_view->load(m_view->fileName());
}

void WidgetBox::dropWidgets(const QList<AbstractDnDItem*> &item_list,
                                const QPoint &global_mouse_pos)
{
    qDebug() << "WidgetBox::dropWidgets():" << item_list;
}

#include "widgetbox.moc"
