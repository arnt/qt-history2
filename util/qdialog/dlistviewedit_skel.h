#ifndef __DLISTVIEWEDIT__H_
#define __DLISTVIEWEDIT__H_

class QResource;

class QLabel;
class QLineEdit;
class QPushButton;

#include <qdialog.h>

class DListViewEdit_skel : public QDialog
{
  Q_OBJECT
public:
  DListViewEdit_skel( QWidget* parent, const QResource& resource );
  ~DListViewEdit_skel();

protected:
  QLabel*  lb_pixmap;
  QLineEdit*  lb_text;
  QPushButton*  ok;
  QPushButton*  cancel;

protected slots:

signals:
};

#endif
