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

private slots:
    void addTab(AbstractFormWindow *form);
    void removeTab(AbstractFormWindow *form);
    void formNameChanged(const QString &name);
        
private:
    QTabWidget *m_tabs;
    QList<ResourceModel*> m_model_list;
    
    int indexOfForm(AbstractFormWindow *form);
    ResourceModel *model(const QString &file);
    
    FormTab *currentFormTab() const;
    
    friend class FormTab;
};

#endif // RESOURCEEDITOR_H
