#ifndef CHANGEITEM_H
#define CHANGEITEM_H

#include <qlistview.h>

class ChangeItem : public QListViewItem
{
public:
    ChangeItem( QListView*, int, const QString& );
    ChangeItem( QListViewItem*, int, const QString& );
    ~ChangeItem();

    void setVisitedEnable( bool v );
    bool isVisited() const;

    // reimplemented functions
    int compare( QListViewItem *i, int col, bool ascending ) const ;
    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );

    static int changeRTTI() { return 925823; }
    int rtti() const { return changeRTTI(); }

private:
    bool visited;
};

#endif // CHANGEITEM_H
