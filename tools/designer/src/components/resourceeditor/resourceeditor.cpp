#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>
#include <QtGui/QInputDialog>
#include <QtCore/qdebug.h>

#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>

#include <resourcefile.h>

#include "resourceeditor.h"

ResourceEditor::ResourceEditor(AbstractFormEditor *core, QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderLabels(QStringList() << tr("Resources"));
    header()->hide();

    m_add_prefix_action = new QAction(tr("Add prefix"), this);
    m_add_file_action = new QAction(tr("Add file"), this);
    m_delete_action = new QAction(tr("Delete"), this);
    m_add_resource_action = new QAction(tr("Add resource"), this);

    m_core = core;

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(updateTree()));
    connect(m_add_resource_action, SIGNAL(triggered()), this, SLOT(addResource()));
    connect(m_add_prefix_action, SIGNAL(triggered()), this, SLOT(addPrefix()));
    connect(m_add_file_action, SIGNAL(triggered()), this, SLOT(addFile()));
    connect(m_delete_action, SIGNAL(triggered()), this, SLOT(deleteItem()));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(updateActions()));

    updateActions();
}

static QTreeWidgetItem *findItem(QTreeWidget *widget, const QString &s)
{
    for (int i = 0; i < widget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = widget->topLevelItem(i);
        if (item->text(0) == s)
            return item;
    }
    return 0;
}

static QTreeWidgetItem *findItem(QTreeWidgetItem *parent, const QString &s)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        if (item->text(0) == s)
            return item;
    }
    return 0;
}

void ResourceEditor::updateTree()
{
    clear();

    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form != 0) {
        QStringList res_list = form->resourceFiles();
        foreach (QString res, res_list)
            addToTree(form->absolutePath(res));
    }    
    
    resizeColumnToContents(0);
    updateActions();
}

QTreeWidgetItem *ResourceEditor::addToTree(const QString &path)
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return 0;
    
    ResourceFile resource_file(path);
    if (!resource_file.load())
        return 0;

    QTreeWidgetItem *ritem = new QTreeWidgetItem(this);
    ritem->setText(0, form->relativePath(path));
        
    QStringList prefix_list = resource_file.prefixList();
    foreach (QString prefix, prefix_list) {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(ritem);
        pitem->setText(0, prefix);
        QStringList file_list = resource_file.fileList(prefix);
        foreach (QString f, file_list) {
            QTreeWidgetItem *fitem = new QTreeWidgetItem(pitem);
            fitem->setText(0, f);
        }
    }

    return ritem;
}

void ResourceEditor::addResource()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;

    QFileDialog file_dialog(this);
    file_dialog.setFilter(tr("Resource files (*.qrc)"));
    QString dir = form->relativePath(QString());
    if (dir.isEmpty())
        dir = QDir::currentPath();
    file_dialog.setDirectory(dir);
    if (!file_dialog.exec())
        return;
        
    QStringList file_name_list = file_dialog.selectedFiles();
    if (file_name_list.isEmpty())
        return;
    QString file_name = file_name_list.at(0);

    if (!QFileInfo(file_name).exists()) {
        ResourceFile rf(file_name);
        if (!rf.save()) {
            QMessageBox::warning(this, tr("Resource error"),
                                    tr("Failed to create \"%1\"").arg(file_name),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
    }
    
    QTreeWidgetItem *new_item = addToTree(file_name);
    if (new_item == 0) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to open \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    form->addResourceFile(file_name);
    fixItem(new_item);
}

void ResourceEditor::addPrefix()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);
    if (type != ResourceItem)
        return;

    QString prefix = QInputDialog::getText(this, tr("Add prefix"), tr("Prefix"));
    if (prefix.isEmpty())
        return;
    QString file_name = form->absolutePath(item->text(0));

    ResourceFile rf(file_name);
    if (!rf.load()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to open \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    rf.addPrefix(prefix);

    if (!rf.save()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to save \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    
    QTreeWidgetItem *new_item = new QTreeWidgetItem(item);
    new_item->setText(0, prefix);
    fixItem(new_item);
}

void ResourceEditor::addFile()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);
    if (type != PrefixItem)
        return;
    QString prefix = item->text(0);
    QString qrc_file_name = form->absolutePath(item->parent()->text(0));

    QStringList file_name_list = QFileDialog::getOpenFileNames(this, tr("Add file"),
                                                        QFileInfo(qrc_file_name).path(),
                                                        tr("All files (*)"));
    if (file_name_list.isEmpty())
        return;

    ResourceFile rf(qrc_file_name);
    if (!rf.load()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to open \"%1\"").arg(qrc_file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    foreach (QString file_name, file_name_list)
        rf.addFile(prefix, file_name);

    if (!rf.save()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to save \"%1\"").arg(qrc_file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    QTreeWidgetItem *new_item = 0;
    foreach (QString file_name, file_name_list) {
        new_item = new QTreeWidgetItem(item);
        new_item->setText(0, rf.relativePath(file_name));
    }

    if (file_name_list.size() > 1)
        fixItem(item);
    else
        fixItem(new_item);
}

void ResourceEditor::deleteItem()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;
    
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);

    switch (type) {
        case ResourceItem:
            form->removeResourceFile(item->text(0));
            break;
        case PrefixItem: {
            QString qrc_file_name = form->absolutePath(item->parent()->text(0));
            ResourceFile rf(qrc_file_name);
            if (!rf.load()) {
                QMessageBox::warning(this, tr("Resource error"),
                                        tr("Failed to open \"%1\"").arg(qrc_file_name),
                                        QMessageBox::Ok, QMessageBox::NoButton);
                return;
            }
            rf.removePrefix(item->text(0));
            if (!rf.save()) {
                QMessageBox::warning(this, tr("Resource error"),
                                        tr("Failed to save \"%1\"").arg(qrc_file_name),
                                        QMessageBox::Ok, QMessageBox::NoButton);
                return;
            }
            break;
        }
        case FileItem: {
            QString qrc_file_name = form->absolutePath(item->parent()->parent()->text(0));
            QString prefix = item->parent()->text(0);
            QString file_name = item->text(0);
            ResourceFile rf(qrc_file_name);
            if (!rf.load()) {
                QMessageBox::warning(this, tr("Resource error"),
                                        tr("Failed to open \"%1\"").arg(qrc_file_name),
                                        QMessageBox::Ok, QMessageBox::NoButton);
                return;
            }
            rf.removeFile(prefix, file_name);
            if (!rf.save()) {
                QMessageBox::warning(this, tr("Resource error"),
                                        tr("Failed to save \"%1\"").arg(qrc_file_name),
                                        QMessageBox::Ok, QMessageBox::NoButton);
                return;
            }
            break;
        }
    }

    if (item->parent() == 0)
        takeTopLevelItem(indexOfTopLevelItem(item));
    else
        item->parent()->takeChild(item->parent()->indexOfChild(item));
    delete item;
}

void ResourceEditor::mousePressEvent(QMouseEvent *e)
{
    QTreeWidget::mousePressEvent(e);
    
    if (e->button() == Qt::RightButton) {
        QTreeWidgetItem *item = itemAt(e->pos());
        ItemType type = classifyItem(item);
        QMenu *menu = createMenu(type);
        menu->exec(mapToGlobal(e->pos()));
        delete menu;
    }

    updateActions();
}

ResourceEditor::ItemType ResourceEditor::classifyItem(QTreeWidgetItem *item) const
{
    if (item->parent() == 0)
        return ResourceItem;
    if (item->parent()->parent() == 0)
        return PrefixItem;
    return FileItem;
}
    
QMenu *ResourceEditor::createMenu(ItemType type)
{
    QMenu *menu = new QMenu(this);
    switch (type) {
        case ResourceItem:
            menu->addAction(m_delete_action);
            menu->addAction(m_add_prefix_action);
            break;
        case PrefixItem:
            menu->addAction(m_delete_action);
            menu->addAction(m_add_file_action);
            break;
        case FileItem:
            menu->addAction(m_delete_action);
            break;
    }

    return menu;
}

void ResourceEditor::updateActions()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0) {
        m_delete_action->setEnabled(false);
        m_add_prefix_action->setEnabled(false);
        m_add_file_action->setEnabled(false);
        m_add_resource_action->setEnabled(false);
        return;
    }

    m_add_resource_action->setEnabled(true);
    
    QTreeWidgetItem *item = currentItem();
    
    if (item == 0) {
        m_delete_action->setEnabled(false);
        m_add_prefix_action->setEnabled(false);
        m_add_file_action->setEnabled(false);
    } else {
        m_delete_action->setEnabled(true);
        ItemType type = classifyItem(item);
        m_add_file_action->setEnabled(type == PrefixItem);
        m_add_prefix_action->setEnabled(type == ResourceItem);
    }
}

void ResourceEditor::fixItem(QTreeWidgetItem *item)
{
    setCurrentItem(item);
    QTreeWidgetItem *it = item;
    while (it != 0) {
        expandItem(it);
        it = it->parent();
    }
    scrollToItem(item);
}
