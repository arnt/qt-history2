#ifndef MULTIPAGEWIDGETPLUGIN_H
#define MULTIPAGEWIDGETPLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

class QIcon;
class QWidget;

class MultiPageWidgetPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    MultiPageWidgetPlugin(QObject *parent = 0);

    QString name() const;
    QString group() const;
    QString toolTip() const;
    QString whatsThis() const;
    QString includeFile() const;
    QIcon icon() const;
    bool isContainer() const;
    QWidget *createWidget(QWidget *parent);
    bool isInitialized() const;
    void initialize(QDesignerFormEditorInterface *formEditor);
    QString domXml() const;

private:
    bool initialized;
};

#endif
