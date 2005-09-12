#ifndef MULTIPAGEWIDGETTASKMENUEXTENSION_H
#define MULTIPAGEWIDGETTASKMENUEXTENSION_H

#include <QtDesigner/QDesignerTaskMenuExtension>

class QAction;
class MultiPageWidget;

class MultiPageWidgetTaskMenuExtension: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    MultiPageWidgetTaskMenuExtension(MultiPageWidget *widget, QObject *parent);

    QList<QAction *> taskActions() const;

private slots:
    void addPage();
    void removePage();

private:
    QAction *addPageAction;
    QAction *removePageAction;
    MultiPageWidget *myWidget;
};

#endif
