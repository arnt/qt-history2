#ifndef __DMENUDLG__H_
#define __DMENUDLG__H_

class QResource;

class QListView;
class QPushButton;
class QLineEdit;
class QMultiLineEdit;

#include <qwidget.h>

class DMenuDlg_skel : public QWidget
{
  Q_OBJECT
public:
  DMenuDlg_skel( QWidget* parent, const QResource& resource );
  ~DMenuDlg_skel();

protected:
  QListView*  lv_menu;
  QPushButton*  pb_icon;
  QLineEdit*  le_text;
  QPushButton*  pb_shortcut;
  QMultiLineEdit*  le_whatsthis;
  QPushButton*  ok;
  QPushButton*  cancel;

protected slots:
  virtual void slotIcon() = 0;
  virtual void slotShortcut(bool) = 0;

signals:
};

#endif
