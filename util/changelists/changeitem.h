#ifndef CHANGEITEM_H
#define CHANGEITEM_H

#include <qlistview.h>

class ChangeItem : public QListViewItem
{
public:
    ChangeItem( QListView*, int );
    ~ChangeItem();

    QString key( int column, bool ascending ) const;
};

#endif // CHANGEITEM_H
