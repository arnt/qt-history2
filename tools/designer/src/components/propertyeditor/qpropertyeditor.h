#ifndef QPROPERTYEDITOR_H
#define QPROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor_items_p.h"

#include <QTreeView>

namespace QPropertyEditor
{

class Model;
class Delegate;

class QT_PROPERTYEDITOR_EXPORT View: public QTreeView
{
    Q_OBJECT
public:
    View(QWidget *parent = 0);
    ~View();

    I::Property *initialInput() const;
    bool isReadOnly() const;

    inline Model *editorModel() const
    { return m_model; }

signals:
    void propertyChanged(I::Property *property);

public slots:
    void setInitialInput(I::Property *initialInput);
    void setReadOnly(bool readOnly);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    virtual void keyPressEvent(QKeyEvent *ev);

private:
    Model *m_model;
    Delegate *m_itemDelegate;
};

}; // namespace QPropertyEditor

#endif // QPROPERTYEDITOR_H

