#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QWidget>

#include "resourceeditor_global.h"

class AbstractFormEditor;
class AbstractFormWindow;
class QTabWidget;
class ResourceModel;
class FormTab;

class QT_RESOURCEEDITOR_EXPORT ResourceEditor : public QWidget
{
    Q_OBJECT
    
public:
    ResourceEditor(AbstractFormEditor *core, QWidget *parent = 0);

    QAction *newQrcAction() const { return m_new_qrc_action; }
    QAction *openQrcAction() const { return m_open_qrc_action; }
    QAction *saveQrcAction() const { return m_save_qrc_action; }
    QAction *removeQrcAction() const { return m_remove_qrc_action; }
    QAction *reloadQrcAction() const { return m_reload_qrc_action; }

public slots:
    void addTab(AbstractFormWindow *form);
    void removeTab(AbstractFormWindow *form);
    void formNameChanged(const QString &name);

    void newQrc();
    void openQrc();
    void saveQrc();
    void removeQrc();
    void reloadQrc();
        
private:
    QAction *m_new_qrc_action;
    QAction *m_open_qrc_action;
    QAction *m_save_qrc_action;
    QAction *m_remove_qrc_action;
    QAction *m_reload_qrc_action;

    QTabWidget *m_tabs;
    QList<ResourceModel*> m_model_list;
    
    int indexOfForm(AbstractFormWindow *form);
    ResourceModel *model(const QString &file);
    
    FormTab *currentFormTab() const;
    
    friend class FormTab;
};

#endif // RESOURCEEDITOR_H
