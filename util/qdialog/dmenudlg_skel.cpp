#include "dmenudlg_skel.h"

DMenuDlg_skel::DMenuDlg_skel( QWidget* parent, const QResource& resource )
  : QWidget( parent, resource )
{
  configure( resource );

  lv_menu = (QListView*)child( "lv_menu", "QListView" );
  ASSERT( lv_menu );
  pb_icon = (QPushButton*)child( "pb_icon", "QPushButton" );
  ASSERT( pb_icon );
  le_text = (QLineEdit*)child( "le_text", "QLineEdit" );
  ASSERT( le_text );
  pb_shortcut = (QPushButton*)child( "pb_shortcut", "QPushButton" );
  ASSERT( pb_shortcut );
  le_whatsthis = (QMultiLineEdit*)child( "le_whatsthis", "QMultiLineEdit" );
  ASSERT( le_whatsthis );
  ok = (QPushButton*)child( "ok", "QPushButton" );
  ASSERT( ok );
  cancel = (QPushButton*)child( "cancel", "QPushButton" );
  ASSERT( cancel );
}

DMenuDlg_skel::~DMenuDlg_skel()
{
}

