#ifndef __DLISTVIEW_H__
#define __DLISTVIEW_H__

#include <qlistview.h>

class QResource;
class QPoint;
class QListViewItem;

class DListView : public QListView
{
  Q_OBJECT
  Q_BUILDER( "A list view", "qpushbutton.xpm" )
public:
  DListView( QWidget* parent, const QResource& resource );

private slots:
  void slotRightButtonPressed( QListViewItem *item, const QPoint& point, int column );
  void slotInsertAfter();
  void slotInsertBefore();
  void slotInsertChild();
  void slotInsert();
  void slotRemove();
  void slotEdit();

private:
  int columns();

  QListViewItem* m_item;
  int m_column;
};

#endif
