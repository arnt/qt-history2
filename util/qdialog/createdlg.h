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

private:
  DMainWindow* m_pMainWindow;
};

#endif
