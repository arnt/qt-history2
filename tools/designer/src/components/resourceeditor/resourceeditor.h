#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QTreeWidget>

#include "resourceeditor_global.h"

class AbstractFormEditor;
class AbstractFormWindow;

class QT_RESOURCEEDITOR_EXPORT ResourceEditor : public QTreeWidget
{
    Q_OBJECT
    
public:
    ResourceEditor(AbstractFormEditor *core, QWidget *parent = 0);

    QAction *addPrefixAction() const { return m_add_prefix_action; }
    QAction *addFileAction() const { return m_add_file_action; }
    QAction *deleteAction() const { return m_delete_action; }
    QAction *addResourceAction() const { return m_add_resource_action; }
    
protected:
    virtual void mousePressEvent(QMouseEvent *e);

private slots:
    void addResource();
    void addPrefix();
    void addFile();
    void deleteItem();
    
    void updateTree();
    void updateActions();
    
private:
    AbstractFormEditor *m_core;
    QAction *m_add_prefix_action;
    QAction *m_add_file_action;
    QAction *m_delete_action;
    QAction *m_add_resource_action;

    enum ItemType { ResourceItem, PrefixItem, FileItem };
    ItemType classifyItem(QTreeWidgetItem *item) const;
    QTreeWidgetItem *addToTree(const QString &path);
    QMenu *createMenu(ItemType type);
    void fixItem(QTreeWidgetItem *item);
};

#endif // RESOURCEEDITOR_H
