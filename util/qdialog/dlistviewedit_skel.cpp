#include "dlistviewedit_skel.h"

DListViewEdit_skel::DListViewEdit_skel( QWidget* parent, const QResource& resource )
  : QDialog( parent, resource )
{
  configure( resource );

  lb_pixmap = (QLabel*)child( "lb_pixmap", "QLabel" );
  ASSERT( lb_pixmap );
  lb_text = (QLineEdit*)child( "lb_text", "QLineEdit" );
  ASSERT( lb_text );
  ok = (QPushButton*)child( "ok", "QPushButton" );
  ASSERT( ok );
  cancel = (QPushButton*)child( "cancel", "QPushButton" );
  ASSERT( cancel );
}

DListViewEdit_skel::~DListViewEdit_skel()
{
}

