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

    QAction *actionInsert() const { return m_insert_action; }
    QAction *actionDelete() const { return m_delete_action; }
    QAction *actionEdit() const { return m_edit_action; }
    
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void showEvent(QShowEvent *e);
    
private slots:
    void doInsert();
    void doEdit();
    void doDelete();
    
    void updateTree();
    void updateActions();
    
private:
    AbstractFormEditor *m_core;
    QAction *m_insert_action;
    QAction *m_delete_action;
    QAction *m_edit_action;
    AbstractFormWindow *m_current_form;
    
    enum ItemType { RootItem, ResourceItem, PrefixItem, FileItem };
    ItemType classifyItem(QTreeWidgetItem *item) const;
    QTreeWidgetItem *addToTree(const QString &path);
    QMenu *createMenu(ItemType type);
    void fixItem(QTreeWidgetItem *item);

    void addResource();
    void editPrefix();
    void addPrefix();
    void addFile();

};

#endif // RESOURCEEDITOR_H
