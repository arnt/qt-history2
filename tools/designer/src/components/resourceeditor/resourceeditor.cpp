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
    m_current_form = 0;
    setHeaderLabels(QStringList() << tr("Resources"));
    setRootIsDecorated(false);
    header()->setStretchLastSection(true);
    header()->hide();

    m_insert_action = new QAction(tr("Insert"), this);
    m_insert_action->setIcon(QIcon(QLatin1String(":/trolltech/formeditor/images/filenew.png")));
    m_delete_action = new QAction(tr("Delete"), this);
    m_delete_action->setIcon(QIcon(QLatin1String(":/trolltech/formeditor/images/editdelete.png")));
    m_edit_action = new QAction(tr("Edit"), this);

    m_core = core;

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(updateTree()));
    connect(m_insert_action, SIGNAL(triggered()), this, SLOT(doInsert()));
    connect(m_delete_action, SIGNAL(triggered()), this, SLOT(doDelete()));
    connect(m_edit_action, SIGNAL(triggered()), this, SLOT(doEdit()));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(updateActions()));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(doEdit()));

    updateTree();
}

void ResourceEditor::updateTree()
{
    qDebug() << "ResourceEditor::updateTree()";

    clear();

    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();

    if (m_current_form != 0)
        disconnect(m_current_form, 0, this, 0);
    if (form != 0)
        connect(form, SIGNAL(fileNameChanged(const QString&)), this, SLOT(updateTree()));
    m_current_form = form;

    if (form != 0) {
        QTreeWidgetItem *root = new QTreeWidgetItem(this);
        QString form_name = form->fileName();
        if (form_name.isEmpty())
            form_name = tr("Untitled");
        else
            form_name = QFileInfo(form_name).fileName();
        root->setText(0, tr("Resources for \"%1\"").arg(form_name));
        QStringList res_list = form->resourceFiles();
        foreach (QString res, res_list)
            addToTree(form->absolutePath(res));
        expandItem(root);
        // ### setCurrentItem(root);
    }

    updateActions();
}

QTreeWidgetItem *ResourceEditor::addToTree(const QString &path)
{
    qDebug() << "ResourceEditor::addToTree() 1:" << path;

    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return 0;

    ResourceFile resource_file(path);
    if (!resource_file.load())
        return 0;

    QTreeWidgetItem *root = topLevelItem(0);
    if (root == 0)
        return 0;

    QTreeWidgetItem *ritem = new QTreeWidgetItem(root);
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
    qDebug() << "ResourceEditor::addToTree() 2 :" << path;

    return ritem;
}

void ResourceEditor::doInsert()
{
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);

    switch (type) {
        case RootItem:
            addResource();
            break;
        case ResourceItem:
            addPrefix();
            break;
        case PrefixItem:
            addFile();
            break;
        case FileItem:
            break;
    }
}

void ResourceEditor::doEdit()
{
    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);

    switch (type) {
        case RootItem:
            break;
        case ResourceItem:
            break;
        case PrefixItem:
            editPrefix();
            break;
        case FileItem:
            break;
    }
}

void ResourceEditor::doDelete()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;

    QTreeWidgetItem *item = currentItem();
    if (item == 0)
        return;
    ItemType type = classifyItem(item);

    switch (type) {
        case RootItem:
            return;
        case ResourceItem:
            form->removeResourceFile(form->absolutePath(item->text(0)));
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

    QTreeWidgetItem *parent = item->parent();
    Q_ASSERT(parent != 0);
    int idx = parent->indexOfChild(item);
    parent->takeChild(idx);
    delete item;
    if (parent->childCount() == 0) {
        fixItem(parent);
    } else {
        if (idx >= parent->childCount())
            idx = parent->childCount() - 1;
        fixItem(parent->child(idx));
    }
}

void ResourceEditor::addResource()
{
    AbstractFormWindow *form = m_core->formWindowManager()->activeFormWindow();
    if (form == 0)
        return;

    QTreeWidgetItem *root = topLevelItem(0);
    if (root == 0)
        return;

    QFileDialog file_dialog(this);
    file_dialog.setFilter(tr("Resource files (*.qrc)"));
    QString dir = form->absolutePath(QString());
    if (dir.isEmpty())
        dir = QDir::currentPath();
    file_dialog.setDirectory(dir);
    if (!file_dialog.exec())
        return;

    QStringList file_name_list = file_dialog.selectedFiles();
    if (file_name_list.isEmpty())
        return;
    QString file_name = file_name_list.at(0);

    if (form->resourceFiles().contains(file_name)) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("\"%1\" is already in the list").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

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

void ResourceEditor::editPrefix()
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

    QString prefix = QInputDialog::getText(this, tr("Edit prefix"), tr("Prefix"), QLineEdit::Normal,
                                            item->text(0));
    if (prefix.isEmpty())
        return;
    prefix = ResourceFile::fixPrefix(prefix);
    if (prefix == item->text(0))
        return;

    QString file_name = form->absolutePath(item->parent()->text(0));

    ResourceFile rf(file_name);
    if (!rf.load()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to open \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    if (rf.contains(prefix)) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Prefix \"%1\" already exists").arg(prefix),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    rf.changePrefix(item->text(0), prefix);

    if (!rf.save()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to save \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    item->setText(0, prefix);
    fixItem(item);
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
    prefix = ResourceFile::fixPrefix(prefix);
    QString file_name = form->absolutePath(item->text(0));

    ResourceFile rf(file_name);
    if (!rf.load()) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Failed to open \"%1\"").arg(file_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    if (rf.contains(prefix)) {
        QMessageBox::warning(this, tr("Resource error"),
                                tr("Prefix \"%1\" already exists").arg(prefix),
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

    QStringList fixed_file_name_list;
    foreach (QString file_name, file_name_list) {
        if (!rf.contains(prefix, file_name))
            fixed_file_name_list.append(file_name);
    }
    qDebug() << prefix << file_name_list << fixed_file_name_list;
    file_name_list = fixed_file_name_list;
    if (file_name_list.isEmpty())
        return;

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
        return RootItem;
    if (item->parent()->parent() == 0)
        return ResourceItem;
    if (item->parent()->parent()->parent() == 0)
        return PrefixItem;
    return FileItem;
}

QMenu *ResourceEditor::createMenu(ItemType type)
{
    QMenu *menu = new QMenu(this);
    switch (type) {
        case RootItem:
            menu->addAction(m_insert_action);
            break;
        case ResourceItem:
            menu->addAction(m_insert_action);
            menu->addAction(m_delete_action);
            break;
        case PrefixItem:
            menu->addAction(m_insert_action);
            menu->addAction(m_edit_action);
            menu->addAction(m_delete_action);
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
    QTreeWidgetItem *item = currentItem();
    if (form == 0 || item == 0) {
        m_insert_action->setEnabled(false);
        m_edit_action->setEnabled(false);
        m_delete_action->setEnabled(false);
        return;
    }

    ItemType type = classifyItem(item);

    switch (type) {
        case RootItem:
            m_insert_action->setText(tr("Insert qrc file"));
            break;
        case ResourceItem:
            m_insert_action->setText(tr("Insert prefix"));
            break;
        case PrefixItem:
            m_insert_action->setText(tr("Insert file"));
            break;
        case FileItem:
            break;
    };

    m_insert_action->setEnabled(type == RootItem || type == ResourceItem || type == PrefixItem);
    m_edit_action->setEnabled(type == PrefixItem);
    m_delete_action->setEnabled(type == ResourceItem || type == PrefixItem || type == FileItem);
}

void ResourceEditor::fixItem(QTreeWidgetItem *item)
{
    setCurrentItem(item);
    QTreeWidgetItem *it = item;
    while (it != 0) {
        expandItem(it);
        it = it->parent();
    }
    updateActions();
    scrollToItem(item);
}

void ResourceEditor::showEvent(QShowEvent *e)
{
    QTreeWidget::showEvent(e);
    updateTree();
}
