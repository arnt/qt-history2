#ifndef MULTIPAGEWIDGETCONTAINEREXTENSION_H
#define MULTIPAGEWIDGETCONTAINEREXTENSION_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionFactory>

class QExtensionManager;
class MultiPageWidget;

class MultiPageWidgetContainerExtension: public QObject,
                                         public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)

public:
    MultiPageWidgetContainerExtension(MultiPageWidget *widget, QObject *parent);

    void addWidget(QWidget *widget);
    int count() const;
    int currentIndex() const;
    void insertWidget(int index, QWidget *widget);
    void remove(int index);
    void setCurrentIndex(int index);
    QWidget *widget(int index) const;

private:
    MultiPageWidget *myWidget;
};

#endif
