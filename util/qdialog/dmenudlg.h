#ifndef __DMENUDLG_H__
#define __DMENUDLG_H__

#include "dmenudlg_skel.h"

#include <qlistview.h>
#include <qstring.h>

class QResource;
class QPoint;
class QListViewItem;
class QObject;
class QEvent;
class QMenuBar;
class QMenuData;

class DMenuListView : public QListView
{
  Q_OBJECT
  Q_BUILDER( "No comment", "qpushbutton.xpm" )
public:
  DMenuListView( QWidget* parent, const QResource& resource );

private slots:
  void slotRightButtonPressed( QListViewItem *item, const QPoint& point, int column );
  void slotInsertAfter();
  void slotInsertBefore();
  void slotInsertChild();
  void slotInsert();
  void slotRemove();

private:
  QListViewItem* m_item;
};

class DMenuDlg : public DMenuDlg_skel
{
  Q_OBJECT
public:
  DMenuDlg( QMenuBar* bar, QWidget* parent, const QResource& resource );

protected slots:
  virtual void slotIcon();
  virtual void slotShortcut( bool );

protected:
  bool eventFilter( QObject* _obj, QEvent* _ev );

private:
  void initListView( QMenuData* mb, QListViewItem* item = 0 );
  QString accelString( int k );

  QMenuBar* m_bar;
};

#endif
