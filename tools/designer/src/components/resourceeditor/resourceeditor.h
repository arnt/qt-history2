#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QtGui/QWidget>

#include "resourceeditor_global.h"
#include "ui_resourceeditor.h"

class AbstractFormEditor;
class AbstractFormWindow;

class QT_RESOURCEEDITOR_EXPORT ResourceEditor : public QWidget,
                                                    public Ui::ResourceEditor
{
    Q_OBJECT
    
public:
    ResourceEditor(AbstractFormEditor *core, QWidget *parent = 0);

private slots:
    void updateTree(AbstractFormWindow *form);
    void openResourceFile();
    void createResourceFile();
    
private:
    AbstractFormEditor *m_core;

    void addToTree(const QString &path);
};

#endif // RESOURCEEDITOR_H
