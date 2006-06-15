#ifndef MESSAGESTREEVIEW_H
#define MESSAGESTREEVIEW_H

#include <QtGui/QTreeView>

#define TREEVIEW_ODD_COLOR QColor(235,245,255)

class MessagesTreeView : public QTreeView
{
    Q_OBJECT
public:
    MessagesTreeView(QWidget *parent = 0);
    virtual void setModel(QAbstractItemModel * model);
};


#endif // MESSAGESTREEVIEW_H

