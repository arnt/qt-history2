#ifndef FLAGBOX_P_H
#define FLAGBOX_P_H

#include "propertyeditor_global.h"
#include "flagbox_model_p.h"

#include <QComboBox>

class QT_PROPERTYEDITOR_EXPORT FlagBox: public QComboBox
{
    Q_OBJECT
public:
    FlagBox(QWidget *parent = 0);
    virtual ~FlagBox();

    inline FlagBoxModelItem itemAt(int index) const
    { return m_model->itemAt(index); }

    inline FlagBoxModelItem &item(int index)
    { return m_model->item(index); }

    inline QList<FlagBoxModelItem> items() const
    { return m_model->items(); }

    inline void setItems(const QList<FlagBoxModelItem> &items)
    { m_model->setItems(items); }

private slots:
    void slotActivated(const QModelIndex &index);

private:
    FlagBoxModel *m_model;
};

#endif // FLAGBOX_P_H
