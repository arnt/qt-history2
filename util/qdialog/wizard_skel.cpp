#include "wizard_skel.h"

DCreateWizard_skel::DCreateWizard_skel( QWidget* parent, const QResource& resource )
  : QWizard( parent, resource )
{
  configure( resource );

  first = (QGridLayout*)child( "first", "QGridLayout" );
  ASSERT( first );
  rb_dialog = (QRadioButton*)child( "rb_dialog", "QRadioButton" );
  ASSERT( rb_dialog );
  rb_mainwindow = (QRadioButton*)child( "rb_mainwindow", "QRadioButton" );
  ASSERT( rb_mainwindow );
  dlgtmpl = (QGridLayout*)child( "dlgtmpl", "QGridLayout" );
  ASSERT( dlgtmpl );
  rb_horizontal = (QRadioButton*)child( "rb_horizontal", "QRadioButton" );
  ASSERT( rb_horizontal );
  rb_vertical = (QRadioButton*)child( "rb_vertical", "QRadioButton" );
  ASSERT( rb_vertical );
  mwtmpl = (QGridLayout*)child( "mwtmpl", "QGridLayout" );
  ASSERT( mwtmpl );
  rb_mainwindow_simple = (QRadioButton*)child( "rb_mainwindow_simple", "QRadioButton" );
  ASSERT( rb_mainwindow_simple );
  rb_mainwindow_full = (QRadioButton*)child( "rb_mainwindow_full", "QRadioButton" );
  ASSERT( rb_mainwindow_full );
  last = (QVBoxLayout*)child( "last", "QVBoxLayout" );
  ASSERT( last );
  le_name = (QLineEdit*)child( "le_name", "QLineEdit" );
  ASSERT( le_name );
}

DCreateWizard_skel::~DCreateWizard_skel()
{
}

