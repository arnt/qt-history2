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

#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetfactory.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstracticoncache.h>
#include <QtDesigner/customwidget.h>
#include <pluginmanager.h>
#include <QtDesigner/ui4.h>
#include <sheet_delegate.h>
#include <iconloader.h>

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

#include "widgetbox_dnditem.h"

#define SCRATCHPAD_ITEM 1
#define CUSTOM_ITEM     2

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

static QDomDocument stringToDom(const QString &xml)
{
    QDomDocument result;
    result.setContent(xml);
    return result;
}

static DomWidget *xmlToUi(QString xml)
{
    QDomDocument doc;
    QString err_msg;
    int err_line, err_col;
    if (!doc.setContent(xml, &err_msg, &err_line, &err_col)) {
        qWarning("xmlToUi: parse failed:\n%s\n:%d:%d: %s",
                    xml.toUtf8().constData(),
                    err_line, err_col,
                    err_msg.toUtf8().constData());
        return 0;
    }

    QDomElement dom_elt = doc.firstChildElement();
    if (dom_elt.nodeName() != QLatin1String("widget")) {
        qWarning("xmlToUi: invalid root element:\n%s", xml.toUtf8().constData());
        return 0;
    }

    DomWidget *widget = new DomWidget;
    widget->read(dom_elt);
    return widget;
}

/*******************************************************************************
** WidgetBoxItemDelegate
*/

class WidgetBoxItemDelegate : public SheetDelegate
{
public:
    WidgetBoxItemDelegate(QTreeWidget *tree, QWidget *parent = 0)
        : SheetDelegate(tree, parent) {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
};

QWidget *WidgetBoxItemDelegate::createEditor(QWidget *parent,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const
{
    QWidget *result = SheetDelegate::createEditor(parent, option, index);
    QLineEdit *line_edit = qobject_cast<QLineEdit*>(result);
    if (line_edit == 0)
        return result;
    line_edit->setValidator(new QRegExpValidator(QRegExp(QLatin1String("[_a-zA-Z][_a-zA-Z0-9]*")), line_edit));
    return result;
}

/*******************************************************************************
** WidgetBoxTreeView
*/

class WidgetBoxTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    typedef QDesignerWidgetBoxInterface::Widget Widget;
    typedef QDesignerWidgetBoxInterface::Category Category;
    typedef QDesignerWidgetBoxInterface::CategoryList CategoryList;

    WidgetBoxTreeView(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    ~WidgetBoxTreeView();

    int categoryCount() const;
    Category category(int cat_idx) const;
    void addCategory(const Category &cat);
    void removeCategory(int cat_idx);

    int widgetCount(int cat_idx) const;
    Widget widget(int cat_idx, int wgt_idx) const;
    void addWidget(int cat_idx, const Widget &wgt);
    void removeWidget(int cat_idx, int wgt_idx);

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list);

    void setFileName(const QString &file_name);
    QString fileName() const;
    bool load();
    bool save();

signals:
    void pressed(const QString dom_xml, const QPoint &global_mouse_pos);

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private slots:
    void handleMousePress(QTreeWidgetItem *item);
    void removeCurrentItem();
    void editCurrentItem();
    void updateItemData(QTreeWidgetItem *item);

private:
    QDesignerFormEditorInterface *m_core;
    QString m_file_name;

    CategoryList domToCateogryList(const QDomDocument &doc) const;
    Category domToCategory(const QDomElement &cat_elt) const;
    Category loadCustomCategory() const;
    QDomDocument categoryListToDom(const CategoryList &cat_list) const;

    QTreeWidgetItem *widgetToItem(const Widget &wgt, QTreeWidgetItem *parent,
                                    bool editable = false);
    Widget itemToWidget(const QTreeWidgetItem *item) const;

    int indexOfCategory(const QString &name) const;
    int indexOfScratchpad();
};

WidgetBoxTreeView::WidgetBoxTreeView(QDesignerFormEditorInterface *core, QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new WidgetBoxItemDelegate(this, this));
    setRootIsDecorated(false);
    setColumnCount(1);
    header()->hide();
    header()->setResizeMode(QHeaderView::Stretch);

    m_core = core;

    connect(this, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(handleMousePress(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updateItemData(QTreeWidgetItem*)));

    setEditTriggers(QAbstractItemView::AnyKeyPressed);
}

WidgetBoxTreeView::~WidgetBoxTreeView()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("WidgetBox"));

    QStringList open_cat;
    for (int i = 0; i < categoryCount(); ++i) {
        QTreeWidgetItem *cat_item = topLevelItem(i);
        if (isItemExpanded(cat_item))
            open_cat.append(cat_item->text(0));
    }
    settings.setValue(QLatin1String("open categories"), open_cat);

    settings.endGroup();
}

void WidgetBoxTreeView::setFileName(const QString &file_name)
{
    m_file_name = file_name;
}

QString WidgetBoxTreeView::fileName() const
{
    return m_file_name;
}

bool WidgetBoxTreeView::save()
{
    if (fileName().isEmpty())
        return false;

    QFile file(fileName());
    if (!file.open(QIODevice::WriteOnly))
        return false;

    CategoryList cat_list;
    for (int i = 0; i < categoryCount(); ++i)
        cat_list.append(category(i));

    QDomDocument doc = categoryListToDom(cat_list);
    QTextStream stream(&file);
    doc.save(stream, 4);

    return true;
}

void WidgetBoxTreeView::handleMousePress(QTreeWidgetItem *item)
{
    if (item == 0)
        return;

    if (item->parent() == 0) {
        setItemExpanded(item, !isItemExpanded(item));
        return;
    }

    QDesignerWidgetBoxInterface::Widget wgt = qvariant_cast<QDesignerWidgetBoxInterface::Widget>(item->data(0, Qt::UserRole));
    if (wgt.isNull())
        return;

    emit pressed(wgt.domXml(), QCursor::pos());
}

int WidgetBoxTreeView::indexOfScratchpad()
{
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->data(0, Qt::UserRole).toInt() == SCRATCHPAD_ITEM)
            return i;
    }

    QTreeWidgetItem *scratch_item = new QTreeWidgetItem(this);
    scratch_item->setText(0, tr("Scratchpad"));
    scratch_item->setData(0, Qt::UserRole, SCRATCHPAD_ITEM);

    return categoryCount() - 1;
}

int WidgetBoxTreeView::indexOfCategory(const QString &name) const
{
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->text(0) == name)
            return i;
    }
    return -1;
}

bool WidgetBoxTreeView::load()
{
    QString name = fileName();

    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("WidgetBox: failed to open \"%s\"", name.toUtf8().constData());
        return false;
    }

    QString error_msg;
    int line, col;
    QDomDocument doc;
    if (!doc.setContent(&f, &error_msg, &line, &col)) {
        qWarning("WidgetBox: failed to parse \"%s\": on line %d: %s",
                    name.toUtf8().constData(), line, error_msg.toUtf8().constData());
        return false;
    }

    CategoryList cat_list = domToCateogryList(doc);
    if (cat_list.isEmpty())
        return false;

    // make sure the scratchpad is always at the end
    int scratch_idx = -1;
    for (int i = 0; i < cat_list.size(); ++i) {
        if (cat_list.at(i).type() == Category::Scratchpad) {
            scratch_idx = i;
            break;
        }
    }

    clear();
    foreach(Category cat, cat_list) {
        if (cat.type() != Category::Scratchpad)
            addCategory(cat);
    }

    Category custom_cat = loadCustomCategory();
    if (!custom_cat.isNull())
        addCategory(custom_cat);

    if (scratch_idx != -1)
        addCategory(cat_list.at(scratch_idx));

    // Restore which items are expanded

    QSettings settings;
    settings.beginGroup(QLatin1String("WidgetBox"));

    QStringList open_cat;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = topLevelItem(i);
        if (isItemExpanded(item))
            open_cat.append(item->text(0));
    }

    open_cat = settings.value(QLatin1String("open categories"), open_cat).toStringList();
    for (int i = 0; i < open_cat.size(); ++i) {
        int cat_idx = indexOfCategory(open_cat[i]);
        if (cat_idx == -1)
            continue;
        QTreeWidgetItem *item = topLevelItem(cat_idx);
        if (item == 0)
            continue;
        setItemExpanded(item, true);
    }

    settings.endGroup();

    return true;
}

QDomDocument WidgetBoxTreeView::categoryListToDom(const CategoryList &cat_list) const
{
    QDomDocument doc;
    QDomElement root = doc.createElement(QLatin1String("widgetbox"));
    doc.appendChild(root);

    foreach (Category cat, cat_list) {
        if (cat.type() == Category::Custom)
            continue;

        QDomElement cat_elt = doc.createElement(QLatin1String("category"));
        root.appendChild(cat_elt);
        cat_elt.setAttribute(QLatin1String("name"), cat.name());
        if (cat.type() == Category::Scratchpad)
            cat_elt.setAttribute(QLatin1String("type"), QLatin1String("scratchpad"));
        for (int i = 0; i < cat.widgetCount(); ++i) {
            Widget wgt = cat.widget(i);
            DomWidget *dom_wgt = xmlToUi(wgt.domXml());
            QDomElement wgt_elt = dom_wgt->write(doc);
            wgt_elt.setAttribute(QLatin1String("name"), wgt.name());
            wgt_elt.setAttribute(QLatin1String("icon"), wgt.iconName());
            cat_elt.appendChild(wgt_elt);
        }
    }

    return doc;
}

WidgetBoxTreeView::CategoryList
    WidgetBoxTreeView::domToCateogryList(const QDomDocument &doc) const
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
            qWarning("WidgetCollectionModel::xmlToModel(): bad child of widgetbox: \"%s\"", cat_elt.nodeName().toUtf8().constData());
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
    if (cat_elt.attribute(QLatin1String("type"))
                                        == QLatin1String("scratchpad"))
        result.setType(Category::Scratchpad);

    QDomElement widget_elt = cat_elt.firstChildElement();
    for (; !widget_elt.isNull(); widget_elt = widget_elt.nextSiblingElement()) {
        Widget w(widget_elt.attribute(QLatin1String("name")),
                    domToString(widget_elt),
                    widget_elt.attribute(QLatin1String("icon")));
        result.addWidget(w);
    }

    return result;
}


WidgetBoxTreeView::Category WidgetBoxTreeView::loadCustomCategory() const
{
    Category result(tr("Custom Widgets"), Category::Custom);

    PluginManager *pm = m_core->pluginManager();
    QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
    for (int i = 0; i < db->count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *item = db->item(i);
        if (!item->isCustom())
            continue;
        QString path = item->pluginPath();
        if (path.isEmpty())
            continue;
        QObject *o = pm->instance(path);
        if (o == 0)
            continue;
        QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o);
        if (c == 0)
            continue;
        QString dom_xml = c->domXml();
        if (dom_xml.isEmpty())
            continue;

        QIcon icon = c->icon();
        QString icon_name;
        if (icon.isNull())
            icon_name = QLatin1String("qtlogo.png");
        else
            icon_name = m_core->iconCache()->iconToFilePath(icon);
        result.addWidget(Widget(c->name(), dom_xml, icon_name));
    }

    return result;
}

QTreeWidgetItem *WidgetBoxTreeView::widgetToItem(const Widget &wgt,
                                                    QTreeWidgetItem *parent,
                                                    bool editable)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    QString icon_name = wgt.iconName();
    if (icon_name.isEmpty())
        icon_name = QLatin1String("qtlogo.png");

    bool block = blockSignals(true);
    item->setText(0, wgt.name());
    item->setIcon(0, createIconSet(icon_name));
    item->setData(0, Qt::UserRole, qVariantFromValue(wgt));
    blockSignals(block);

    if (editable) {
        item->setFlags(Qt::ItemIsSelectable
                        | Qt::ItemIsEditable
                        | Qt::ItemIsEnabled);
    }

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

    int i = cat_item->data(0, Qt::UserRole).toInt();
    switch (i) {
        case SCRATCHPAD_ITEM:
            result.setType(Category::Scratchpad);
            break;
        case CUSTOM_ITEM:
            result.setType(Category::Custom);
            break;
        default:
            break;
    }

    return result;
}

void WidgetBoxTreeView::addCategory(const Category &cat)
{
    QTreeWidgetItem *cat_item = new QTreeWidgetItem(this);
    cat_item->setText(0, cat.name());

    switch (cat.type()) {
        case Category::Scratchpad:
            cat_item->setData(0, Qt::UserRole, SCRATCHPAD_ITEM);
            break;
        case Category::Custom:
            cat_item->setData(0, Qt::UserRole, CUSTOM_ITEM);
            break;
        default:
            break;
    }

    for (int i = 0; i < cat.widgetCount(); ++i)
        widgetToItem(cat.widget(i), cat_item, cat.type() == Category::Scratchpad);
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

    bool scratch = cat_item->data(0, Qt::UserRole).toInt() == SCRATCHPAD_ITEM;
    widgetToItem(wgt, cat_item, scratch);
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

void WidgetBoxTreeView::removeCurrentItem()
{
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;

    QTreeWidgetItem *parent = item->parent();
    if (parent == 0) {
        takeTopLevelItem(indexOfTopLevelItem(item));
    } else {
        parent->takeChild(parent->indexOfChild(item));
        setItemExpanded(parent, true);
        if (parent->data(0, Qt::UserRole).toInt() == SCRATCHPAD_ITEM
                && parent->childCount() == 0)
            delete takeTopLevelItem(indexOfTopLevelItem(parent));
    }
    delete item;

    save();
}

void WidgetBoxTreeView::updateItemData(QTreeWidgetItem *item)
{
    if (item->parent() == 0)
        return;

    Widget widget = qvariant_cast<Widget>(item->data(0, Qt::UserRole));

    if (item->text(0).isEmpty()) {
        item->setText(0, widget.name());
        return;
    }

    widget.setName(item->text(0));
    QDomDocument doc = stringToDom(widget.domXml());
    QDomElement widget_elt = doc.firstChildElement(QLatin1String("widget"));
    if (!widget_elt.isNull()) {
        widget_elt.setAttribute(QLatin1String("name"), item->text(0));
        widget.setDomXml(domToString(widget_elt));
    }

    bool block = blockSignals(true);
    item->setData(0, Qt::UserRole, qVariantFromValue(widget));
    blockSignals(block);

    save();
}

void WidgetBoxTreeView::editCurrentItem()
{
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    edit(index);
}

void WidgetBoxTreeView::contextMenuEvent(QContextMenuEvent *e)
{
    QPoint global_pos = mapToGlobal(e->pos());
    QTreeWidgetItem *item = itemAt(e->pos());
    if (item == 0)
        return;

    QTreeWidgetItem *parent = item->parent();
    if (parent == 0)
        return;

    if (!parent->data(0, Qt::UserRole).isValid())
        return;

    setCurrentItem(item);
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Remove"), this, SLOT(removeCurrentItem()));
    menu->addAction(tr("Edit name"), this, SLOT(editCurrentItem()));
    menu->exec(global_pos);

    e->accept();
}

void WidgetBoxTreeView::dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list)
{
    QTreeWidgetItem *last_item = 0;

    foreach (QDesignerDnDItemInterface *item, item_list) {
        QWidget *w = item->widget();
        if (w == 0)
            continue;

        DomUI *dom_ui = item->domUi();
        if (dom_ui == 0)
            continue;

        int scratch_idx = indexOfScratchpad();
        QTreeWidgetItem *scratch_item = topLevelItem(scratch_idx);

        QDomDocument dom;
        QDomElement elt = dom_ui->write(dom);
        QString xml = domToString(elt
                                    .firstChildElement(QLatin1String("widget"))
                                    .firstChildElement(QLatin1String("widget")));

        last_item = widgetToItem(Widget(w->objectName(), xml), scratch_item, true);
        setItemExpanded(scratch_item, true);
    }

    if (last_item != 0) {
        save();
        QApplication::setActiveWindow(this);
        setCurrentItem(last_item);
        editCurrentItem();
    }
}

/*******************************************************************************
** WidgetBox
*/

WidgetBox::WidgetBox(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WFlags flags)
    : QDesignerWidgetBoxInterface(parent, flags), m_core(core)
{
    m_core = core;

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    m_view = new WidgetBoxTreeView(m_core, this);
    l->addWidget(m_view);

    connect(m_view, SIGNAL(pressed(QString,QPoint)),
            this, SLOT(handleMousePress(QString,QPoint)));
}

WidgetBox::~WidgetBox()
{
}

QDesignerFormEditorInterface *WidgetBox::core() const
{
    return m_core;
}

void WidgetBox::handleMousePress(const QString &xml, const QPoint &global_mouse_pos)
{
    DomWidget *dom_widget = xmlToUi(xml);
    if (dom_widget == 0)
        return;
    if (QApplication::mouseButtons() == Qt::LeftButton) {
        QList<QDesignerDnDItemInterface*> item_list;
        item_list.append(new WidgetBoxDnDItem(core(), dom_widget, global_mouse_pos));
        m_core->formWindowManager()->dragItems(item_list);
    }
}

int WidgetBox::categoryCount() const
{
    return m_view->categoryCount();
}

QDesignerWidgetBoxInterface::Category WidgetBox::category(int cat_idx) const
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

QDesignerWidgetBoxInterface::Widget WidgetBox::widget(int cat_idx, int wgt_idx) const
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

void WidgetBox::dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint&)
{
    m_view->dropWidgets(item_list);
}

void WidgetBox::setFileName(const QString &file_name)
{
    m_view->setFileName(file_name);
}

QString WidgetBox::fileName() const
{
    return m_view->fileName();
}

bool WidgetBox::load()
{
    return m_view->load();
}

bool WidgetBox::save()
{
    return m_view->save();
}

#include "widgetbox.moc"
