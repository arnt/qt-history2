#ifndef __creatdedlg_h__
#define __creatdedlg_h__

#include <qwizard.h>

#include "wizard_skel.h"

class QResource;
class QRadioButton;

class DMainWindow;

class DCreateWizard : public DCreateWizard_skel
{
  Q_OBJECT
public:
  DCreateWizard( DMainWindow* parent, const QResource& resource );
  ~DCreateWizard();

protected slots:
  void slotDialog( bool );
  void slotMainWindow( bool );
  void slotVertical( bool );
  void slotHorizontal( bool );
  void slotNameChanged( const QString&);
  void slotFinished();
  void slotSimple(bool);
  void slotFullFeatured(bool);

private:
  DMainWindow* m_pMainWindow;

  enum Type { Dialog, Wizard, MainWindow };
  enum DialogType { Dialog_VerticalButtons, Dialog_HorizontalButtons };
  enum MainWindowType { MainWindow_FullFeatured, MainWindow_Simple };

  Type m_type;
  DialogType m_dialogType;
  MainWindowType m_mainWindowType;

  QString m_name;
};

#endif
