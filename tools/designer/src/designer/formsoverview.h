#ifndef FORMSOVERVIEW_H
#define FORMSOVERVIEW_H

#include <QTreeWidget>
#include <QHash>

class AbstractFormEditor;
class AbstractFormWindowManager;
class AbstractFormWindow;
class QTimer;

class FormsOverview: public QTreeWidget
{
    Q_OBJECT
public:
    FormsOverview(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~FormsOverview();

    AbstractFormEditor *core() const;
    AbstractFormWindowManager *formWindowManager() const;

private slots:
    void activeFormWindowChanged(AbstractFormWindow *formWindow);
    void formWindowAdded(AbstractFormWindow *formWindow);
    void formWindowRemoved(AbstractFormWindow *formWindow);
    void formActivated(QTreeWidgetItem *item, int column);

    void updateForms();
    void updateFormsNow();

private:
    AbstractFormWindow *formWindow(QTreeWidgetItem *item) const;

private:
    AbstractFormEditor *m_core;
    QHash<AbstractFormWindow*, QTreeWidgetItem*> m_items;
    QTimer *m_updateFormsTimer;
};

#endif // FORMSOVERVIEW_H
