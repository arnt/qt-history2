#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"

#include <QPointer>
#include <QTreeWidget>

class AbstractFormEditor;
class AbstractFormWindow;
class ObjectItem;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QTreeWidget
{
    Q_OBJECT
public:
    ObjectInspector(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual AbstractFormEditor *core() const;

public slots:
    void setFormWindow(AbstractFormWindow *formWindow);
    
private slots:
    void slotSelectionChanged();
    
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

private:
    AbstractFormEditor *m_core;
    QPointer<AbstractFormWindow> m_formWindow;
    ObjectItem *m_root;
    bool m_ignoreUpdate;
};


#endif // OBJECTINSPECTOR_H

