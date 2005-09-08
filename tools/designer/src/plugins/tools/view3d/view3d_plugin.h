#ifndef Q3VIEW3D_PLUGIN_H
#define Q3VIEW3D_PLUGIN_H

#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtDesigner/QtDesigner>
#include "view3d_global.h"

class QView3DTool;
class QAction;

class VIEW3D_EXPORT QView3DPlugin : public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerFormEditorPluginInterface)

public:
    QView3DPlugin();
    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);
    virtual QAction *action() const;
    virtual QDesignerFormEditorInterface *core() const;

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

private slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);

private:
    QPointer<QDesignerFormEditorInterface> m_core;
    QHash<QDesignerFormWindowInterface*, QView3DTool*> m_tool_list;
    QAction *m_action;
};

#endif // QVIEW3D_PLUGIN_H
