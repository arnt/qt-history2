#ifndef __WIZARD__H_
#define __WIZARD__H_

class QResource;

class QGridLayout;
class QRadioButton;
class QVBoxLayout;
class QLineEdit;

#include <qwizard.h>

class DCreateWizard_skel : public QWizard
{
  Q_OBJECT
public:
  DCreateWizard_skel( QWidget* parent, const QResource& resource );
  ~DCreateWizard_skel();

protected:
  QGridLayout*  first;
  QRadioButton*  rb_dialog;
  QRadioButton*  rb_mainwindow;
  QGridLayout*  dlgtmpl;
  QRadioButton*  rb_horizontal;
  QRadioButton*  rb_vertical;
  QGridLayout*  mwtmpl;
  QRadioButton*  rb_mainwindow_simple;
  QRadioButton*  rb_mainwindow_full;
  QVBoxLayout*  last;
  QLineEdit*  le_name;

protected slots:
  virtual void slotDialog(bool) = 0;
  virtual void slotMainWindow(bool) = 0;
  virtual void slotVertical(bool) = 0;
  virtual void slotHorizontal(bool) = 0;
  virtual void slotSimple(bool) = 0;
  virtual void slotFullFeatured(bool) = 0;
  virtual void slotNameChanged( const QString&) = 0;

signals:
};

#endif
