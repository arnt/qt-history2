#include "qressource.h"

#include "qbutton.h"
#include "qbuttongroup.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qdial.h"
#include "qframe.h"
#include "qgrid.h"
#include "qgroupbox.h"
#include "qhbox.h"
#include "qheader.h"
#include "qlabel.h"
#include "qlcdnumber.h"
#include "qlineedit.h"
#include "qlistbox.h"
#include "qlistview.h"
#include "qmainwindow.h"
#include "qmenubar.h"
#include "qmenudata.h"
#include "qml.h"
#include "qmultilineedit.h"
#include "qpopupmenu.h"
#include "qprogressbar.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qrangecontrol.h"
#include "qresizecorner.h"
#include "qscrollbar.h"
#include "qscrollview.h"
#include "qslider.h"
#include "qspinbox.h"
#include "qsplitter.h"
#include "qstatusbar.h"
#include "qtabbar.h"
#include "qtableview.h"
#include "qtoolbar.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qvalidator.h"
#include "qvbox.h"
#include "qwellarray.h"
#include "qwhatsthis.h"
#include "qwidgetstack.h"
#include "qworkspace.h"
#include "qworkspacechild.h"
#include "qdialog.h"
#include "qtabwidget.h"

QLayout* qGridLayoutFactory( const QRessource& _res, QWidget* _parent );
bool qGridLayoutConfig( const QRessource& _res, QLayout* _instance );
QLayout* qHBoxLayoutFactory( const QRessource&, QWidget* _parent );
bool qHBoxLayoutConfig( const QRessource& _res, QLayout* _instance );
QObject* qObjectFactory( const QRessource&, QObject* _parent );
bool qObjectConfig( const QRessource& res, QObject* _instance );

bool qXMLReadDialogDialogCode( const QString& _name, const QRessource& _res, QDialog::DialogCode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Rejected" ) { *_v = QDialog::Rejected; return true; }
  if ( tmp == "Accepted" ) { *_v = QDialog::Accepted; return true; }
 }
 return false;
}

bool qXMLReadMenuBarSeparator( const QString& _name, const QRessource& _res, QMenuBar::Separator* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Never" ) { *_v = QMenuBar::Never; return true; }
  if ( tmp == "InWindowsStyle" ) { *_v = QMenuBar::InWindowsStyle; return true; }
 }
 return false;
}

bool qXMLReadListViewWidthMode( const QString& _name, const QRessource& _res, QListView::WidthMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Manual" ) { *_v = QListView::Manual; return true; }
  if ( tmp == "Maximum" ) { *_v = QListView::Maximum; return true; }
 }
 return false;
}

bool qXMLReadKey( const QString& _name, const QRessource& _res, Qt::Key* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Key_Escape" ) { *_v = Qt::Key_Escape; return true; }
  if ( tmp == "Key_Tab" ) { *_v = Qt::Key_Tab; return true; }
  if ( tmp == "Key_Backtab" ) { *_v = Qt::Key_Backtab; return true; }
  if ( tmp == "Key_Backspace" ) { *_v = Qt::Key_Backspace; return true; }
  if ( tmp == "Key_Return" ) { *_v = Qt::Key_Return; return true; }
  if ( tmp == "Key_Enter" ) { *_v = Qt::Key_Enter; return true; }
  if ( tmp == "Key_Insert" ) { *_v = Qt::Key_Insert; return true; }
  if ( tmp == "Key_Delete" ) { *_v = Qt::Key_Delete; return true; }
  if ( tmp == "Key_Pause" ) { *_v = Qt::Key_Pause; return true; }
  if ( tmp == "Key_Print" ) { *_v = Qt::Key_Print; return true; }
  if ( tmp == "Key_SysReq" ) { *_v = Qt::Key_SysReq; return true; }
  if ( tmp == "Key_Home" ) { *_v = Qt::Key_Home; return true; }
  if ( tmp == "Key_End" ) { *_v = Qt::Key_End; return true; }
  if ( tmp == "Key_Left" ) { *_v = Qt::Key_Left; return true; }
  if ( tmp == "Key_Up" ) { *_v = Qt::Key_Up; return true; }
  if ( tmp == "Key_Right" ) { *_v = Qt::Key_Right; return true; }
  if ( tmp == "Key_Down" ) { *_v = Qt::Key_Down; return true; }
  if ( tmp == "Key_Prior" ) { *_v = Qt::Key_Prior; return true; }
  if ( tmp == "Key_Next" ) { *_v = Qt::Key_Next; return true; }
  if ( tmp == "Key_Shift" ) { *_v = Qt::Key_Shift; return true; }
  if ( tmp == "Key_Control" ) { *_v = Qt::Key_Control; return true; }
  if ( tmp == "Key_Meta" ) { *_v = Qt::Key_Meta; return true; }
  if ( tmp == "Key_Alt" ) { *_v = Qt::Key_Alt; return true; }
  if ( tmp == "Key_CapsLock" ) { *_v = Qt::Key_CapsLock; return true; }
  if ( tmp == "Key_NumLock" ) { *_v = Qt::Key_NumLock; return true; }
  if ( tmp == "Key_ScrollLock" ) { *_v = Qt::Key_ScrollLock; return true; }
  if ( tmp == "Key_F1" ) { *_v = Qt::Key_F1; return true; }
  if ( tmp == "Key_F2" ) { *_v = Qt::Key_F2; return true; }
  if ( tmp == "Key_F3" ) { *_v = Qt::Key_F3; return true; }
  if ( tmp == "Key_F4" ) { *_v = Qt::Key_F4; return true; }
  if ( tmp == "Key_F5" ) { *_v = Qt::Key_F5; return true; }
  if ( tmp == "Key_F6" ) { *_v = Qt::Key_F6; return true; }
  if ( tmp == "Key_F7" ) { *_v = Qt::Key_F7; return true; }
  if ( tmp == "Key_F8" ) { *_v = Qt::Key_F8; return true; }
  if ( tmp == "Key_F9" ) { *_v = Qt::Key_F9; return true; }
  if ( tmp == "Key_F10" ) { *_v = Qt::Key_F10; return true; }
  if ( tmp == "Key_F11" ) { *_v = Qt::Key_F11; return true; }
  if ( tmp == "Key_F12" ) { *_v = Qt::Key_F12; return true; }
  if ( tmp == "Key_F13" ) { *_v = Qt::Key_F13; return true; }
  if ( tmp == "Key_F14" ) { *_v = Qt::Key_F14; return true; }
  if ( tmp == "Key_F15" ) { *_v = Qt::Key_F15; return true; }
  if ( tmp == "Key_F16" ) { *_v = Qt::Key_F16; return true; }
  if ( tmp == "Key_F17" ) { *_v = Qt::Key_F17; return true; }
  if ( tmp == "Key_F18" ) { *_v = Qt::Key_F18; return true; }
  if ( tmp == "Key_F19" ) { *_v = Qt::Key_F19; return true; }
  if ( tmp == "Key_F20" ) { *_v = Qt::Key_F20; return true; }
  if ( tmp == "Key_F21" ) { *_v = Qt::Key_F21; return true; }
  if ( tmp == "Key_F22" ) { *_v = Qt::Key_F22; return true; }
  if ( tmp == "Key_F23" ) { *_v = Qt::Key_F23; return true; }
  if ( tmp == "Key_F24" ) { *_v = Qt::Key_F24; return true; }
  if ( tmp == "Key_F25" ) { *_v = Qt::Key_F25; return true; }
  if ( tmp == "Key_F26" ) { *_v = Qt::Key_F26; return true; }
  if ( tmp == "Key_F27" ) { *_v = Qt::Key_F27; return true; }
  if ( tmp == "Key_F28" ) { *_v = Qt::Key_F28; return true; }
  if ( tmp == "Key_F29" ) { *_v = Qt::Key_F29; return true; }
  if ( tmp == "Key_F30" ) { *_v = Qt::Key_F30; return true; }
  if ( tmp == "Key_F31" ) { *_v = Qt::Key_F31; return true; }
  if ( tmp == "Key_F32" ) { *_v = Qt::Key_F32; return true; }
  if ( tmp == "Key_F33" ) { *_v = Qt::Key_F33; return true; }
  if ( tmp == "Key_F34" ) { *_v = Qt::Key_F34; return true; }
  if ( tmp == "Key_F35" ) { *_v = Qt::Key_F35; return true; }
  if ( tmp == "Key_Super_L" ) { *_v = Qt::Key_Super_L; return true; }
  if ( tmp == "Key_Super_R" ) { *_v = Qt::Key_Super_R; return true; }
  if ( tmp == "Key_Menu" ) { *_v = Qt::Key_Menu; return true; }
  if ( tmp == "Key_Hyper_L" ) { *_v = Qt::Key_Hyper_L; return true; }
  if ( tmp == "Key_Hyper_R" ) { *_v = Qt::Key_Hyper_R; return true; }
  if ( tmp == "Key_Space" ) { *_v = Qt::Key_Space; return true; }
  if ( tmp == "Key_Any" ) { *_v = Qt::Key_Any; return true; }
  if ( tmp == "Key_Exclam" ) { *_v = Qt::Key_Exclam; return true; }
  if ( tmp == "Key_QuoteDbl" ) { *_v = Qt::Key_QuoteDbl; return true; }
  if ( tmp == "Key_NumberSign" ) { *_v = Qt::Key_NumberSign; return true; }
  if ( tmp == "Key_Dollar" ) { *_v = Qt::Key_Dollar; return true; }
  if ( tmp == "Key_Percent" ) { *_v = Qt::Key_Percent; return true; }
  if ( tmp == "Key_Ampersand" ) { *_v = Qt::Key_Ampersand; return true; }
  if ( tmp == "Key_Apostrophe" ) { *_v = Qt::Key_Apostrophe; return true; }
  if ( tmp == "Key_ParenLeft" ) { *_v = Qt::Key_ParenLeft; return true; }
  if ( tmp == "Key_ParenRight" ) { *_v = Qt::Key_ParenRight; return true; }
  if ( tmp == "Key_Asterisk" ) { *_v = Qt::Key_Asterisk; return true; }
  if ( tmp == "Key_Plus" ) { *_v = Qt::Key_Plus; return true; }
  if ( tmp == "Key_Comma" ) { *_v = Qt::Key_Comma; return true; }
  if ( tmp == "Key_Minus" ) { *_v = Qt::Key_Minus; return true; }
  if ( tmp == "Key_Period" ) { *_v = Qt::Key_Period; return true; }
  if ( tmp == "Key_Slash" ) { *_v = Qt::Key_Slash; return true; }
  if ( tmp == "Key_0" ) { *_v = Qt::Key_0; return true; }
  if ( tmp == "Key_1" ) { *_v = Qt::Key_1; return true; }
  if ( tmp == "Key_2" ) { *_v = Qt::Key_2; return true; }
  if ( tmp == "Key_3" ) { *_v = Qt::Key_3; return true; }
  if ( tmp == "Key_4" ) { *_v = Qt::Key_4; return true; }
  if ( tmp == "Key_5" ) { *_v = Qt::Key_5; return true; }
  if ( tmp == "Key_6" ) { *_v = Qt::Key_6; return true; }
  if ( tmp == "Key_7" ) { *_v = Qt::Key_7; return true; }
  if ( tmp == "Key_8" ) { *_v = Qt::Key_8; return true; }
  if ( tmp == "Key_9" ) { *_v = Qt::Key_9; return true; }
  if ( tmp == "Key_Colon" ) { *_v = Qt::Key_Colon; return true; }
  if ( tmp == "Key_Semicolon" ) { *_v = Qt::Key_Semicolon; return true; }
  if ( tmp == "Key_Less" ) { *_v = Qt::Key_Less; return true; }
  if ( tmp == "Key_Equal" ) { *_v = Qt::Key_Equal; return true; }
  if ( tmp == "Key_Greater" ) { *_v = Qt::Key_Greater; return true; }
  if ( tmp == "Key_Question" ) { *_v = Qt::Key_Question; return true; }
  if ( tmp == "Key_At" ) { *_v = Qt::Key_At; return true; }
  if ( tmp == "Key_A" ) { *_v = Qt::Key_A; return true; }
  if ( tmp == "Key_B" ) { *_v = Qt::Key_B; return true; }
  if ( tmp == "Key_C" ) { *_v = Qt::Key_C; return true; }
  if ( tmp == "Key_D" ) { *_v = Qt::Key_D; return true; }
  if ( tmp == "Key_E" ) { *_v = Qt::Key_E; return true; }
  if ( tmp == "Key_F" ) { *_v = Qt::Key_F; return true; }
  if ( tmp == "Key_G" ) { *_v = Qt::Key_G; return true; }
  if ( tmp == "Key_H" ) { *_v = Qt::Key_H; return true; }
  if ( tmp == "Key_I" ) { *_v = Qt::Key_I; return true; }
  if ( tmp == "Key_J" ) { *_v = Qt::Key_J; return true; }
  if ( tmp == "Key_K" ) { *_v = Qt::Key_K; return true; }
  if ( tmp == "Key_L" ) { *_v = Qt::Key_L; return true; }
  if ( tmp == "Key_M" ) { *_v = Qt::Key_M; return true; }
  if ( tmp == "Key_N" ) { *_v = Qt::Key_N; return true; }
  if ( tmp == "Key_O" ) { *_v = Qt::Key_O; return true; }
  if ( tmp == "Key_P" ) { *_v = Qt::Key_P; return true; }
  if ( tmp == "Key_Q" ) { *_v = Qt::Key_Q; return true; }
  if ( tmp == "Key_R" ) { *_v = Qt::Key_R; return true; }
  if ( tmp == "Key_S" ) { *_v = Qt::Key_S; return true; }
  if ( tmp == "Key_T" ) { *_v = Qt::Key_T; return true; }
  if ( tmp == "Key_U" ) { *_v = Qt::Key_U; return true; }
  if ( tmp == "Key_V" ) { *_v = Qt::Key_V; return true; }
  if ( tmp == "Key_W" ) { *_v = Qt::Key_W; return true; }
  if ( tmp == "Key_X" ) { *_v = Qt::Key_X; return true; }
  if ( tmp == "Key_Y" ) { *_v = Qt::Key_Y; return true; }
  if ( tmp == "Key_Z" ) { *_v = Qt::Key_Z; return true; }
  if ( tmp == "Key_BracketLeft" ) { *_v = Qt::Key_BracketLeft; return true; }
  if ( tmp == "Key_Backslash" ) { *_v = Qt::Key_Backslash; return true; }
  if ( tmp == "Key_BracketRight" ) { *_v = Qt::Key_BracketRight; return true; }
  if ( tmp == "Key_AsciiCircum" ) { *_v = Qt::Key_AsciiCircum; return true; }
  if ( tmp == "Key_Underscore" ) { *_v = Qt::Key_Underscore; return true; }
  if ( tmp == "Key_QuoteLeft" ) { *_v = Qt::Key_QuoteLeft; return true; }
  if ( tmp == "Key_BraceLeft" ) { *_v = Qt::Key_BraceLeft; return true; }
  if ( tmp == "Key_Bar" ) { *_v = Qt::Key_Bar; return true; }
  if ( tmp == "Key_BraceRight" ) { *_v = Qt::Key_BraceRight; return true; }
  if ( tmp == "Key_AsciiTilde" ) { *_v = Qt::Key_AsciiTilde; return true; }
  if ( tmp == "Key_nobreakspace" ) { *_v = Qt::Key_nobreakspace; return true; }
  if ( tmp == "Key_exclamdown" ) { *_v = Qt::Key_exclamdown; return true; }
  if ( tmp == "Key_cent" ) { *_v = Qt::Key_cent; return true; }
  if ( tmp == "Key_sterling" ) { *_v = Qt::Key_sterling; return true; }
  if ( tmp == "Key_currency" ) { *_v = Qt::Key_currency; return true; }
  if ( tmp == "Key_yen" ) { *_v = Qt::Key_yen; return true; }
  if ( tmp == "Key_brokenbar" ) { *_v = Qt::Key_brokenbar; return true; }
  if ( tmp == "Key_section" ) { *_v = Qt::Key_section; return true; }
  if ( tmp == "Key_diaeresis" ) { *_v = Qt::Key_diaeresis; return true; }
  if ( tmp == "Key_copyright" ) { *_v = Qt::Key_copyright; return true; }
  if ( tmp == "Key_ordfeminine" ) { *_v = Qt::Key_ordfeminine; return true; }
  if ( tmp == "Key_guillemotleft" ) { *_v = Qt::Key_guillemotleft; return true; }
  if ( tmp == "Key_notsign" ) { *_v = Qt::Key_notsign; return true; }
  if ( tmp == "Key_hyphen" ) { *_v = Qt::Key_hyphen; return true; }
  if ( tmp == "Key_registered" ) { *_v = Qt::Key_registered; return true; }
  if ( tmp == "Key_macron" ) { *_v = Qt::Key_macron; return true; }
  if ( tmp == "Key_degree" ) { *_v = Qt::Key_degree; return true; }
  if ( tmp == "Key_plusminus" ) { *_v = Qt::Key_plusminus; return true; }
  if ( tmp == "Key_twosuperior" ) { *_v = Qt::Key_twosuperior; return true; }
  if ( tmp == "Key_threesuperior" ) { *_v = Qt::Key_threesuperior; return true; }
  if ( tmp == "Key_acute" ) { *_v = Qt::Key_acute; return true; }
  if ( tmp == "Key_mu" ) { *_v = Qt::Key_mu; return true; }
  if ( tmp == "Key_paragraph" ) { *_v = Qt::Key_paragraph; return true; }
  if ( tmp == "Key_periodcentered" ) { *_v = Qt::Key_periodcentered; return true; }
  if ( tmp == "Key_cedilla" ) { *_v = Qt::Key_cedilla; return true; }
  if ( tmp == "Key_onesuperior" ) { *_v = Qt::Key_onesuperior; return true; }
  if ( tmp == "Key_masculine" ) { *_v = Qt::Key_masculine; return true; }
  if ( tmp == "Key_guillemotright" ) { *_v = Qt::Key_guillemotright; return true; }
  if ( tmp == "Key_onequarter" ) { *_v = Qt::Key_onequarter; return true; }
  if ( tmp == "Key_onehalf" ) { *_v = Qt::Key_onehalf; return true; }
  if ( tmp == "Key_threequarters" ) { *_v = Qt::Key_threequarters; return true; }
  if ( tmp == "Key_questiondown" ) { *_v = Qt::Key_questiondown; return true; }
  if ( tmp == "Key_Agrave" ) { *_v = Qt::Key_Agrave; return true; }
  if ( tmp == "Key_Aacute" ) { *_v = Qt::Key_Aacute; return true; }
  if ( tmp == "Key_Acircumflex" ) { *_v = Qt::Key_Acircumflex; return true; }
  if ( tmp == "Key_Atilde" ) { *_v = Qt::Key_Atilde; return true; }
  if ( tmp == "Key_Adiaeresis" ) { *_v = Qt::Key_Adiaeresis; return true; }
  if ( tmp == "Key_Aring" ) { *_v = Qt::Key_Aring; return true; }
  if ( tmp == "Key_AE" ) { *_v = Qt::Key_AE; return true; }
  if ( tmp == "Key_Ccedilla" ) { *_v = Qt::Key_Ccedilla; return true; }
  if ( tmp == "Key_Egrave" ) { *_v = Qt::Key_Egrave; return true; }
  if ( tmp == "Key_Eacute" ) { *_v = Qt::Key_Eacute; return true; }
  if ( tmp == "Key_Ecircumflex" ) { *_v = Qt::Key_Ecircumflex; return true; }
  if ( tmp == "Key_Ediaeresis" ) { *_v = Qt::Key_Ediaeresis; return true; }
  if ( tmp == "Key_Igrave" ) { *_v = Qt::Key_Igrave; return true; }
  if ( tmp == "Key_Iacute" ) { *_v = Qt::Key_Iacute; return true; }
  if ( tmp == "Key_Icircumflex" ) { *_v = Qt::Key_Icircumflex; return true; }
  if ( tmp == "Key_Idiaeresis" ) { *_v = Qt::Key_Idiaeresis; return true; }
  if ( tmp == "Key_ETH" ) { *_v = Qt::Key_ETH; return true; }
  if ( tmp == "Key_Ntilde" ) { *_v = Qt::Key_Ntilde; return true; }
  if ( tmp == "Key_Ograve" ) { *_v = Qt::Key_Ograve; return true; }
  if ( tmp == "Key_Oacute" ) { *_v = Qt::Key_Oacute; return true; }
  if ( tmp == "Key_Ocircumflex" ) { *_v = Qt::Key_Ocircumflex; return true; }
  if ( tmp == "Key_Otilde" ) { *_v = Qt::Key_Otilde; return true; }
  if ( tmp == "Key_Odiaeresis" ) { *_v = Qt::Key_Odiaeresis; return true; }
  if ( tmp == "Key_multiply" ) { *_v = Qt::Key_multiply; return true; }
  if ( tmp == "Key_Ooblique" ) { *_v = Qt::Key_Ooblique; return true; }
  if ( tmp == "Key_Ugrave" ) { *_v = Qt::Key_Ugrave; return true; }
  if ( tmp == "Key_Uacute" ) { *_v = Qt::Key_Uacute; return true; }
  if ( tmp == "Key_Ucircumflex" ) { *_v = Qt::Key_Ucircumflex; return true; }
  if ( tmp == "Key_Udiaeresis" ) { *_v = Qt::Key_Udiaeresis; return true; }
  if ( tmp == "Key_Yacute" ) { *_v = Qt::Key_Yacute; return true; }
  if ( tmp == "Key_THORN" ) { *_v = Qt::Key_THORN; return true; }
  if ( tmp == "Key_ssharp" ) { *_v = Qt::Key_ssharp; return true; }
  if ( tmp == "Key_agrave" ) { *_v = Qt::Key_agrave; return true; }
  if ( tmp == "Key_aacute" ) { *_v = Qt::Key_aacute; return true; }
  if ( tmp == "Key_acircumflex" ) { *_v = Qt::Key_acircumflex; return true; }
  if ( tmp == "Key_atilde" ) { *_v = Qt::Key_atilde; return true; }
  if ( tmp == "Key_adiaeresis" ) { *_v = Qt::Key_adiaeresis; return true; }
  if ( tmp == "Key_aring" ) { *_v = Qt::Key_aring; return true; }
  if ( tmp == "Key_ae" ) { *_v = Qt::Key_ae; return true; }
  if ( tmp == "Key_ccedilla" ) { *_v = Qt::Key_ccedilla; return true; }
  if ( tmp == "Key_egrave" ) { *_v = Qt::Key_egrave; return true; }
  if ( tmp == "Key_eacute" ) { *_v = Qt::Key_eacute; return true; }
  if ( tmp == "Key_ecircumflex" ) { *_v = Qt::Key_ecircumflex; return true; }
  if ( tmp == "Key_ediaeresis" ) { *_v = Qt::Key_ediaeresis; return true; }
  if ( tmp == "Key_igrave" ) { *_v = Qt::Key_igrave; return true; }
  if ( tmp == "Key_iacute" ) { *_v = Qt::Key_iacute; return true; }
  if ( tmp == "Key_icircumflex" ) { *_v = Qt::Key_icircumflex; return true; }
  if ( tmp == "Key_idiaeresis" ) { *_v = Qt::Key_idiaeresis; return true; }
  if ( tmp == "Key_eth" ) { *_v = Qt::Key_eth; return true; }
  if ( tmp == "Key_ntilde" ) { *_v = Qt::Key_ntilde; return true; }
  if ( tmp == "Key_ograve" ) { *_v = Qt::Key_ograve; return true; }
  if ( tmp == "Key_oacute" ) { *_v = Qt::Key_oacute; return true; }
  if ( tmp == "Key_ocircumflex" ) { *_v = Qt::Key_ocircumflex; return true; }
  if ( tmp == "Key_otilde" ) { *_v = Qt::Key_otilde; return true; }
  if ( tmp == "Key_odiaeresis" ) { *_v = Qt::Key_odiaeresis; return true; }
  if ( tmp == "Key_division" ) { *_v = Qt::Key_division; return true; }
  if ( tmp == "Key_oslash" ) { *_v = Qt::Key_oslash; return true; }
  if ( tmp == "Key_ugrave" ) { *_v = Qt::Key_ugrave; return true; }
  if ( tmp == "Key_uacute" ) { *_v = Qt::Key_uacute; return true; }
  if ( tmp == "Key_ucircumflex" ) { *_v = Qt::Key_ucircumflex; return true; }
  if ( tmp == "Key_udiaeresis" ) { *_v = Qt::Key_udiaeresis; return true; }
  if ( tmp == "Key_yacute" ) { *_v = Qt::Key_yacute; return true; }
  if ( tmp == "Key_thorn" ) { *_v = Qt::Key_thorn; return true; }
  if ( tmp == "Key_ydiaeresis" ) { *_v = Qt::Key_ydiaeresis; return true; }
  if ( tmp == "Key_unknown" ) { *_v = Qt::Key_unknown; return true; }
 }
 return false;
}

bool qXMLReadButtonToggleState( const QString& _name, const QRessource& _res, QButton::ToggleState* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Off" ) { *_v = QButton::Off; return true; }
  if ( tmp == "NoChange" ) { *_v = QButton::NoChange; return true; }
  if ( tmp == "On" ) { *_v = QButton::On; return true; }
 }
 return false;
}

bool qXMLReadLCDNumberSegmentStyle( const QString& _name, const QRessource& _res, QLCDNumber::SegmentStyle* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Outline" ) { *_v = QLCDNumber::Outline; return true; }
  if ( tmp == "Filled" ) { *_v = QLCDNumber::Filled; return true; }
  if ( tmp == "Flat" ) { *_v = QLCDNumber::Flat; return true; }
 }
 return false;
}

bool qXMLReadButtonToggleType( const QString& _name, const QRessource& _res, QButton::ToggleType* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "SingleShot" ) { *_v = QButton::SingleShot; return true; }
  if ( tmp == "Toggle" ) { *_v = QButton::Toggle; return true; }
  if ( tmp == "Tristate" ) { *_v = QButton::Tristate; return true; }
 }
 return false;
}

bool qXMLReadOrientation( const QString& _name, const QRessource& _res, Qt::Orientation* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Horizontal" ) { *_v = Qt::Horizontal; return true; }
  if ( tmp == "Vertical" ) { *_v = Qt::Vertical; return true; }
 }
 return false;
}

bool qXMLReadArrowType( const QString& _name, const QRessource& _res, Qt::ArrowType* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "UpArrow" ) { *_v = Qt::UpArrow; return true; }
  if ( tmp == "DownArrow" ) { *_v = Qt::DownArrow; return true; }
  if ( tmp == "LeftArrow" ) { *_v = Qt::LeftArrow; return true; }
  if ( tmp == "RightArrow" ) { *_v = Qt::RightArrow; return true; }
 }
 return false;
}

bool qXMLReadBGMode( const QString& _name, const QRessource& _res, Qt::BGMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "TransparentMode" ) { *_v = Qt::TransparentMode; return true; }
  if ( tmp == "OpaqueMode" ) { *_v = Qt::OpaqueMode; return true; }
 }
 return false;
}

bool qXMLReadModifier( const QString& _name, const QRessource& _res, Qt::Modifier* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "SHIFT" ) { *_v = Qt::SHIFT; return true; }
  if ( tmp == "CTRL" ) { *_v = Qt::CTRL; return true; }
  if ( tmp == "ALT" ) { *_v = Qt::ALT; return true; }
  if ( tmp == "ASCII_ACCEL" ) { *_v = Qt::ASCII_ACCEL; return true; }
 }
 return false;
}

bool qXMLReadTabWidgetTabPosition( const QString& _name, const QRessource& _res, QTabWidget::TabPosition* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Top" ) { *_v = QTabWidget::Top; return true; }
  if ( tmp == "Bottom" ) { *_v = QTabWidget::Bottom; return true; }
 }
 return false;
}

bool qXMLReadPenStyle( const QString& _name, const QRessource& _res, Qt::PenStyle* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoPen" ) { *_v = Qt::NoPen; return true; }
  if ( tmp == "SolidLine" ) { *_v = Qt::SolidLine; return true; }
  if ( tmp == "DashLine" ) { *_v = Qt::DashLine; return true; }
  if ( tmp == "DotLine" ) { *_v = Qt::DotLine; return true; }
  if ( tmp == "DashDotLine" ) { *_v = Qt::DashDotLine; return true; }
  if ( tmp == "DashDotDotLine" ) { *_v = Qt::DashDotDotLine; return true; }
 }
 return false;
}

bool qXMLReadBrushStyle( const QString& _name, const QRessource& _res, Qt::BrushStyle* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoBrush" ) { *_v = Qt::NoBrush; return true; }
  if ( tmp == "SolidPattern" ) { *_v = Qt::SolidPattern; return true; }
  if ( tmp == "Dense1Pattern" ) { *_v = Qt::Dense1Pattern; return true; }
  if ( tmp == "Dense2Pattern" ) { *_v = Qt::Dense2Pattern; return true; }
  if ( tmp == "Dense3Pattern" ) { *_v = Qt::Dense3Pattern; return true; }
  if ( tmp == "Dense4Pattern" ) { *_v = Qt::Dense4Pattern; return true; }
  if ( tmp == "Dense5Pattern" ) { *_v = Qt::Dense5Pattern; return true; }
  if ( tmp == "Dense6Pattern" ) { *_v = Qt::Dense6Pattern; return true; }
  if ( tmp == "Dense7Pattern" ) { *_v = Qt::Dense7Pattern; return true; }
  if ( tmp == "HorPattern" ) { *_v = Qt::HorPattern; return true; }
  if ( tmp == "VerPattern" ) { *_v = Qt::VerPattern; return true; }
  if ( tmp == "CrossPattern" ) { *_v = Qt::CrossPattern; return true; }
  if ( tmp == "BDiagPattern" ) { *_v = Qt::BDiagPattern; return true; }
  if ( tmp == "FDiagPattern" ) { *_v = Qt::FDiagPattern; return true; }
  if ( tmp == "DiagCrossPattern" ) { *_v = Qt::DiagCrossPattern; return true; }
  if ( tmp == "CustomPattern" ) { *_v = Qt::CustomPattern; return true; }
 }
 return false;
}

bool qXMLReadImageConversionFlags( const QString& _name, const QRessource& _res, Qt::ImageConversionFlags* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "ColorMode_Mask" ) { *_v = Qt::ColorMode_Mask; return true; }
  if ( tmp == "AutoColor" ) { *_v = Qt::AutoColor; return true; }
  if ( tmp == "ColorOnly" ) { *_v = Qt::ColorOnly; return true; }
  if ( tmp == "MonoOnly" ) { *_v = Qt::MonoOnly; return true; }
  if ( tmp == "AlphaDither_Mask" ) { *_v = Qt::AlphaDither_Mask; return true; }
  if ( tmp == "ThresholdAlphaDither" ) { *_v = Qt::ThresholdAlphaDither; return true; }
  if ( tmp == "OrderedAlphaDither" ) { *_v = Qt::OrderedAlphaDither; return true; }
  if ( tmp == "DiffuseAlphaDither" ) { *_v = Qt::DiffuseAlphaDither; return true; }
  if ( tmp == "Dither_Mask" ) { *_v = Qt::Dither_Mask; return true; }
  if ( tmp == "DiffuseDither" ) { *_v = Qt::DiffuseDither; return true; }
  if ( tmp == "OrderedDither" ) { *_v = Qt::OrderedDither; return true; }
  if ( tmp == "ThresholdDither" ) { *_v = Qt::ThresholdDither; return true; }
  if ( tmp == "DitherMode_Mask" ) { *_v = Qt::DitherMode_Mask; return true; }
  if ( tmp == "AutoDither" ) { *_v = Qt::AutoDither; return true; }
  if ( tmp == "PreferDither" ) { *_v = Qt::PreferDither; return true; }
  if ( tmp == "AvoidDither" ) { *_v = Qt::AvoidDither; return true; }
 }
 return false;
}

bool qXMLReadGUIStyle( const QString& _name, const QRessource& _res, Qt::GUIStyle* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "MacStyle" ) { *_v = Qt::MacStyle; return true; }
  if ( tmp == "WindowsStyle" ) { *_v = Qt::WindowsStyle; return true; }
  if ( tmp == "Win3Style" ) { *_v = Qt::Win3Style; return true; }
  if ( tmp == "PMStyle" ) { *_v = Qt::PMStyle; return true; }
  if ( tmp == "MotifStyle" ) { *_v = Qt::MotifStyle; return true; }
 }
 return false;
}

bool qXMLReadSliderTickSetting( const QString& _name, const QRessource& _res, QSlider::TickSetting* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoMarks" ) { *_v = QSlider::NoMarks; return true; }
  if ( tmp == "Above" ) { *_v = QSlider::Above; return true; }
  if ( tmp == "Left" ) { *_v = QSlider::Left; return true; }
  if ( tmp == "Below" ) { *_v = QSlider::Below; return true; }
  if ( tmp == "Right" ) { *_v = QSlider::Right; return true; }
  if ( tmp == "Both" ) { *_v = QSlider::Both; return true; }
 }
 return false;
}

bool qXMLReadSplitterResizeMode( const QString& _name, const QRessource& _res, QSplitter::ResizeMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Stretch" ) { *_v = QSplitter::Stretch; return true; }
  if ( tmp == "KeepSize" ) { *_v = QSplitter::KeepSize; return true; }
 }
 return false;
}

bool qXMLReadMainWindowToolBarDock( const QString& _name, const QRessource& _res, QMainWindow::ToolBarDock* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Unmanaged" ) { *_v = QMainWindow::Unmanaged; return true; }
  if ( tmp == "TornOff" ) { *_v = QMainWindow::TornOff; return true; }
  if ( tmp == "Top" ) { *_v = QMainWindow::Top; return true; }
  if ( tmp == "Bottom" ) { *_v = QMainWindow::Bottom; return true; }
  if ( tmp == "Right" ) { *_v = QMainWindow::Right; return true; }
  if ( tmp == "Left" ) { *_v = QMainWindow::Left; return true; }
 }
 return false;
}

bool qXMLReadButtonState( const QString& _name, const QRessource& _res, Qt::ButtonState* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoButton" ) { *_v = Qt::NoButton; return true; }
  if ( tmp == "LeftButton" ) { *_v = Qt::LeftButton; return true; }
  if ( tmp == "RightButton" ) { *_v = Qt::RightButton; return true; }
  if ( tmp == "MidButton" ) { *_v = Qt::MidButton; return true; }
  if ( tmp == "MouseButtonMask" ) { *_v = Qt::MouseButtonMask; return true; }
  if ( tmp == "ShiftButton" ) { *_v = Qt::ShiftButton; return true; }
  if ( tmp == "ControlButton" ) { *_v = Qt::ControlButton; return true; }
  if ( tmp == "AltButton" ) { *_v = Qt::AltButton; return true; }
  if ( tmp == "KeyButtonMask" ) { *_v = Qt::KeyButtonMask; return true; }
 }
 return false;
}

bool qXMLReadLineEditEchoMode( const QString& _name, const QRessource& _res, QLineEdit::EchoMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Normal" ) { *_v = QLineEdit::Normal; return true; }
  if ( tmp == "NoEcho" ) { *_v = QLineEdit::NoEcho; return true; }
  if ( tmp == "Password" ) { *_v = QLineEdit::Password; return true; }
 }
 return false;
}

bool qXMLReadWidgetState( const QString& _name, const QRessource& _res, Qt::WidgetState* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "WState_Created" ) { *_v = Qt::WState_Created; return true; }
  if ( tmp == "WState_Disabled" ) { *_v = Qt::WState_Disabled; return true; }
  if ( tmp == "WState_Visible" ) { *_v = Qt::WState_Visible; return true; }
  if ( tmp == "WState_ForceHide" ) { *_v = Qt::WState_ForceHide; return true; }
  if ( tmp == "WState_OwnCursor" ) { *_v = Qt::WState_OwnCursor; return true; }
  if ( tmp == "WState_MouseTracking" ) { *_v = Qt::WState_MouseTracking; return true; }
  if ( tmp == "WState_CompressKeys" ) { *_v = Qt::WState_CompressKeys; return true; }
  if ( tmp == "WState_BlockUpdates" ) { *_v = Qt::WState_BlockUpdates; return true; }
  if ( tmp == "WState_InPaintEvent" ) { *_v = Qt::WState_InPaintEvent; return true; }
  if ( tmp == "WState_Reparented" ) { *_v = Qt::WState_Reparented; return true; }
  if ( tmp == "WState_ConfigPending" ) { *_v = Qt::WState_ConfigPending; return true; }
  if ( tmp == "WState_Resized" ) { *_v = Qt::WState_Resized; return true; }
  if ( tmp == "WState_AutoMask" ) { *_v = Qt::WState_AutoMask; return true; }
  if ( tmp == "WState_Polished" ) { *_v = Qt::WState_Polished; return true; }
  if ( tmp == "WState_DND" ) { *_v = Qt::WState_DND; return true; }
  if ( tmp == "WState_USPositionX" ) { *_v = Qt::WState_USPositionX; return true; }
  if ( tmp == "WState_PaletteSet" ) { *_v = Qt::WState_PaletteSet; return true; }
  if ( tmp == "WState_PaletteFixed" ) { *_v = Qt::WState_PaletteFixed; return true; }
  if ( tmp == "WState_FontSet" ) { *_v = Qt::WState_FontSet; return true; }
  if ( tmp == "WState_FontFixed" ) { *_v = Qt::WState_FontFixed; return true; }
 }
 return false;
}

bool qXMLReadTabBarShape( const QString& _name, const QRessource& _res, QTabBar::Shape* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "RoundedAbove" ) { *_v = QTabBar::RoundedAbove; return true; }
  if ( tmp == "RoundedBelow" ) { *_v = QTabBar::RoundedBelow; return true; }
  if ( tmp == "TriangularAbove" ) { *_v = QTabBar::TriangularAbove; return true; }
  if ( tmp == "TriangularBelow" ) { *_v = QTabBar::TriangularBelow; return true; }
 }
 return false;
}

bool qXMLReadAlignmentFlags( const QString& _name, const QRessource& _res, Qt::AlignmentFlags* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "AlignLeft" ) { *_v = Qt::AlignLeft; return true; }
  if ( tmp == "AlignRight" ) { *_v = Qt::AlignRight; return true; }
  if ( tmp == "AlignHCenter" ) { *_v = Qt::AlignHCenter; return true; }
  if ( tmp == "AlignTop" ) { *_v = Qt::AlignTop; return true; }
  if ( tmp == "AlignBottom" ) { *_v = Qt::AlignBottom; return true; }
  if ( tmp == "AlignVCenter" ) { *_v = Qt::AlignVCenter; return true; }
  if ( tmp == "AlignCenter" ) { *_v = Qt::AlignCenter; return true; }
  if ( tmp == "SingleLine" ) { *_v = Qt::SingleLine; return true; }
  if ( tmp == "DontClip" ) { *_v = Qt::DontClip; return true; }
  if ( tmp == "ExpandTabs" ) { *_v = Qt::ExpandTabs; return true; }
  if ( tmp == "ShowPrefix" ) { *_v = Qt::ShowPrefix; return true; }
  if ( tmp == "WordBreak" ) { *_v = Qt::WordBreak; return true; }
  if ( tmp == "DontPrint" ) { *_v = Qt::DontPrint; return true; }
 }
 return false;
}

bool qXMLReadLCDNumberMode( const QString& _name, const QRessource& _res, QLCDNumber::Mode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Hex" ) { *_v = QLCDNumber::Hex; return true; }
  if ( tmp == "HEX" ) { *_v = QLCDNumber::HEX; return true; }
  if ( tmp == "Dec" ) { *_v = QLCDNumber::Dec; return true; }
  if ( tmp == "DEC" ) { *_v = QLCDNumber::DEC; return true; }
  if ( tmp == "Oct" ) { *_v = QLCDNumber::Oct; return true; }
  if ( tmp == "OCT" ) { *_v = QLCDNumber::OCT; return true; }
  if ( tmp == "Bin" ) { *_v = QLCDNumber::Bin; return true; }
  if ( tmp == "BIN" ) { *_v = QLCDNumber::BIN; return true; }
 }
 return false;
}

bool qXMLReadRasterOp( const QString& _name, const QRessource& _res, Qt::RasterOp* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "CopyROP" ) { *_v = Qt::CopyROP; return true; }
  if ( tmp == "OrROP" ) { *_v = Qt::OrROP; return true; }
  if ( tmp == "XorROP" ) { *_v = Qt::XorROP; return true; }
  if ( tmp == "NotAndROP" ) { *_v = Qt::NotAndROP; return true; }
  if ( tmp == "EraseROP" ) { *_v = Qt::EraseROP; return true; }
  if ( tmp == "NotCopyROP" ) { *_v = Qt::NotCopyROP; return true; }
  if ( tmp == "NotOrROP" ) { *_v = Qt::NotOrROP; return true; }
  if ( tmp == "NotXorROP" ) { *_v = Qt::NotXorROP; return true; }
  if ( tmp == "AndROP" ) { *_v = Qt::AndROP; return true; }
  if ( tmp == "NotROP" ) { *_v = Qt::NotROP; return true; }
  if ( tmp == "ClearROP" ) { *_v = Qt::ClearROP; return true; }
  if ( tmp == "SetROP" ) { *_v = Qt::SetROP; return true; }
  if ( tmp == "NopROP" ) { *_v = Qt::NopROP; return true; }
  if ( tmp == "AndNotROP" ) { *_v = Qt::AndNotROP; return true; }
  if ( tmp == "OrNotROP" ) { *_v = Qt::OrNotROP; return true; }
  if ( tmp == "NandROP" ) { *_v = Qt::NandROP; return true; }
  if ( tmp == "NorROP" ) { *_v = Qt::NorROP; return true; }
 }
 return false;
}

bool qXMLReadWidgetFlags( const QString& _name, const QRessource& _res, Qt::WidgetFlags* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "WType_TopLevel" ) { *_v = Qt::WType_TopLevel; return true; }
  if ( tmp == "WType_Modal" ) { *_v = Qt::WType_Modal; return true; }
  if ( tmp == "WType_Popup" ) { *_v = Qt::WType_Popup; return true; }
  if ( tmp == "WType_Desktop" ) { *_v = Qt::WType_Desktop; return true; }
  if ( tmp == "WType_Mask" ) { *_v = Qt::WType_Mask; return true; }
  if ( tmp == "WStyle_Customize" ) { *_v = Qt::WStyle_Customize; return true; }
  if ( tmp == "WStyle_NormalBorder" ) { *_v = Qt::WStyle_NormalBorder; return true; }
  if ( tmp == "WStyle_DialogBorder" ) { *_v = Qt::WStyle_DialogBorder; return true; }
  if ( tmp == "WStyle_NoBorder" ) { *_v = Qt::WStyle_NoBorder; return true; }
  if ( tmp == "WStyle_Title" ) { *_v = Qt::WStyle_Title; return true; }
  if ( tmp == "WStyle_SysMenu" ) { *_v = Qt::WStyle_SysMenu; return true; }
  if ( tmp == "WStyle_Minimize" ) { *_v = Qt::WStyle_Minimize; return true; }
  if ( tmp == "WStyle_Maximize" ) { *_v = Qt::WStyle_Maximize; return true; }
  if ( tmp == "WStyle_MinMax" ) { *_v = Qt::WStyle_MinMax; return true; }
  if ( tmp == "WStyle_Tool" ) { *_v = Qt::WStyle_Tool; return true; }
  if ( tmp == "WStyle_StaysOnTop" ) { *_v = Qt::WStyle_StaysOnTop; return true; }
  if ( tmp == "WStyle_Reserved1" ) { *_v = Qt::WStyle_Reserved1; return true; }
  if ( tmp == "WStyle_Reserved2" ) { *_v = Qt::WStyle_Reserved2; return true; }
  if ( tmp == "WStyle_Reserved3" ) { *_v = Qt::WStyle_Reserved3; return true; }
  if ( tmp == "WStyle_Mask" ) { *_v = Qt::WStyle_Mask; return true; }
  if ( tmp == "WDestructiveClose" ) { *_v = Qt::WDestructiveClose; return true; }
  if ( tmp == "WPaintDesktop" ) { *_v = Qt::WPaintDesktop; return true; }
  if ( tmp == "WPaintUnclipped" ) { *_v = Qt::WPaintUnclipped; return true; }
  if ( tmp == "WPaintClever" ) { *_v = Qt::WPaintClever; return true; }
  if ( tmp == "WResizeNoErase" ) { *_v = Qt::WResizeNoErase; return true; }
  if ( tmp == "WMouseNoMask" ) { *_v = Qt::WMouseNoMask; return true; }
 }
 return false;
}

bool qXMLReadWindowsVersion( const QString& _name, const QRessource& _res, Qt::WindowsVersion* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "WV_NT" ) { *_v = Qt::WV_NT; return true; }
  if ( tmp == "WV_95" ) { *_v = Qt::WV_95; return true; }
  if ( tmp == "WV_98" ) { *_v = Qt::WV_98; return true; }
  if ( tmp == "WV_32s" ) { *_v = Qt::WV_32s; return true; }
 }
 return false;
}

bool qXMLReadWidgetBackgroundMode( const QString& _name, const QRessource& _res, QWidget::BackgroundMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "FixedColor" ) { *_v = QWidget::FixedColor; return true; }
  if ( tmp == "FixedPixmap" ) { *_v = QWidget::FixedPixmap; return true; }
  if ( tmp == "NoBackground" ) { *_v = QWidget::NoBackground; return true; }
  if ( tmp == "PaletteForeground" ) { *_v = QWidget::PaletteForeground; return true; }
  if ( tmp == "PaletteButton" ) { *_v = QWidget::PaletteButton; return true; }
  if ( tmp == "PaletteLight" ) { *_v = QWidget::PaletteLight; return true; }
  if ( tmp == "PaletteMidlight" ) { *_v = QWidget::PaletteMidlight; return true; }
  if ( tmp == "PaletteDark" ) { *_v = QWidget::PaletteDark; return true; }
  if ( tmp == "PaletteMid" ) { *_v = QWidget::PaletteMid; return true; }
  if ( tmp == "PaletteText" ) { *_v = QWidget::PaletteText; return true; }
  if ( tmp == "PaletteBrightText" ) { *_v = QWidget::PaletteBrightText; return true; }
  if ( tmp == "PaletteBase" ) { *_v = QWidget::PaletteBase; return true; }
  if ( tmp == "PaletteBackground" ) { *_v = QWidget::PaletteBackground; return true; }
  if ( tmp == "PaletteShadow" ) { *_v = QWidget::PaletteShadow; return true; }
  if ( tmp == "PaletteHighlight" ) { *_v = QWidget::PaletteHighlight; return true; }
  if ( tmp == "PaletteHighlightedText" ) { *_v = QWidget::PaletteHighlightedText; return true; }
 }
 return false;
}

bool qXMLReadComboBoxPolicy( const QString& _name, const QRessource& _res, QComboBox::Policy* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoInsertion" ) { *_v = QComboBox::NoInsertion; return true; }
  if ( tmp == "AtTop" ) { *_v = QComboBox::AtTop; return true; }
  if ( tmp == "AtCurrent" ) { *_v = QComboBox::AtCurrent; return true; }
  if ( tmp == "AtBottom" ) { *_v = QComboBox::AtBottom; return true; }
  if ( tmp == "AfterCurrent" ) { *_v = QComboBox::AfterCurrent; return true; }
  if ( tmp == "BeforeCurrent" ) { *_v = QComboBox::BeforeCurrent; return true; }
 }
 return false;
}

bool qXMLReadScrollViewResizePolicy( const QString& _name, const QRessource& _res, QScrollView::ResizePolicy* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Default" ) { *_v = QScrollView::Default; return true; }
  if ( tmp == "Manual" ) { *_v = QScrollView::Manual; return true; }
  if ( tmp == "AutoOne" ) { *_v = QScrollView::AutoOne; return true; }
 }
 return false;
}

bool qXMLReadWidgetPropagationMode( const QString& _name, const QRessource& _res, QWidget::PropagationMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "NoChildren" ) { *_v = QWidget::NoChildren; return true; }
  if ( tmp == "AllChildren" ) { *_v = QWidget::AllChildren; return true; }
  if ( tmp == "SameFont" ) { *_v = QWidget::SameFont; return true; }
  if ( tmp == "SamePalette" ) { *_v = QWidget::SamePalette; return true; }
 }
 return false;
}

bool qXMLReadPaintUnit( const QString& _name, const QRessource& _res, Qt::PaintUnit* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "PixelUnit" ) { *_v = Qt::PixelUnit; return true; }
  if ( tmp == "LoMetricUnit" ) { *_v = Qt::LoMetricUnit; return true; }
  if ( tmp == "HiMetricUnit" ) { *_v = Qt::HiMetricUnit; return true; }
  if ( tmp == "LoEnglishUnit" ) { *_v = Qt::LoEnglishUnit; return true; }
  if ( tmp == "HiEnglishUnit" ) { *_v = Qt::HiEnglishUnit; return true; }
  if ( tmp == "TwipsUnit" ) { *_v = Qt::TwipsUnit; return true; }
 }
 return false;
}

bool qXMLReadScrollViewScrollBarMode( const QString& _name, const QRessource& _res, QScrollView::ScrollBarMode* _v )
{
 if ( _res.hasAttrib( _name ) ) {
  QString tmp = _res.textAttrib( _name );
  if ( tmp == "Auto" ) { *_v = QScrollView::Auto; return true; }
  if ( tmp == "AlwaysOff" ) { *_v = QScrollView::AlwaysOff; return true; }
  if ( tmp == "AlwaysOn" ) { *_v = QScrollView::AlwaysOn; return true; }
 }
 return false;
}

QWidget* qButtonFactory( const QRessource&, QWidget* _parent )
{
 return new QButton( _parent );
}

bool qButtonConfig( const QRessource& res, QWidget* _instance )
{
 QButton* self = (QButton*)_instance;
 Q_RESSOURCE_ATTRIB( "down", bool, res, qXMLReadBool, self, setDown );
 Q_RESSOURCE_ATTRIB( "accel", int, res, qXMLReadInt, self, setAccel );
 Q_RESSOURCE_ATTRIB( "autorepeat", bool, res, qXMLReadBool, self, setAutoRepeat );
 Q_RESSOURCE_ATTRIB( "autoresize", bool, res, qXMLReadBool, self, setAutoResize );
 Q_RESSOURCE_ATTRIB( "text", QString, res, qXMLReadString, self, setText );
 return true;
}

QWidget* qButtonGroupFactory( const QRessource& res, QWidget* _parent )
{
 int cols = 1; Qt::Orientation o = Qt::Horizontal;
 if ( res.hasAttrib( "columns" ) )
  cols = res.intAttrib( "columns" );
 qXMLReadOrientation( "orientation", res, &o );
 return new QButtonGroup( cols, o, _parent);
}

bool qButtonGroupConfig( const QRessource& res, QWidget* _instance )
{
 QButtonGroup* self = (QButtonGroup*)_instance;
 Q_RESSOURCE_ATTRIB( "exclusive", bool, res, qXMLReadBool, self, setExclusive );
 Q_RESSOURCE_ATTRIB( "radiobuttonexclusive", bool, res, qXMLReadBool, self, setRadioButtonExclusive );
 Q_RESSOURCE_ATTRIB( "button", int, res, qXMLReadInt, self, setButton );
 return true;
}

QWidget* qCheckBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QCheckBox( _parent );
}

bool qCheckBoxConfig( const QRessource& res, QWidget* _instance )
{
 QCheckBox* self = (QCheckBox*)_instance;
 Q_RESSOURCE_ATTRIB( "checked", bool, res, qXMLReadBool, self, setChecked );
 return true;
}

QWidget* qComboBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QComboBox( _parent );
}

bool qComboBoxConfig( const QRessource& res, QWidget* _instance )
{
 QComboBox* self = (QComboBox*)_instance;
 Q_RESSOURCE_ATTRIB( "sizelimit", int, res, qXMLReadInt, self, setSizeLimit );
 Q_RESSOURCE_ATTRIB( "currentitem", int, res, qXMLReadInt, self, setCurrentItem );
 Q_RESSOURCE_ATTRIB( "edittext", QString, res, qXMLReadString, self, setEditText );
 Q_RESSOURCE_ATTRIB( "autocompletion", bool, res, qXMLReadBool, self, setAutoCompletion );
 Q_RESSOURCE_ATTRIB( "insertionpolicy", QComboBox::Policy, res, qXMLReadComboBoxPolicy, self, setInsertionPolicy );
 Q_RESSOURCE_ATTRIB( "enabled", bool, res, qXMLReadBool, self, setEnabled );
 Q_RESSOURCE_ATTRIB( "autoresize", bool, res, qXMLReadBool, self, setAutoResize );
 Q_RESSOURCE_ATTRIB( "maxcount", int, res, qXMLReadInt, self, setMaxCount );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "backgroundcolor", QColor, res, qXMLReadColor, self, setBackgroundColor );
 return true;
}

QWidget* qDialFactory( const QRessource&, QWidget* _parent )
{
 return new QDial( _parent );
}

bool qDialConfig( const QRessource& res, QWidget* _instance )
{
 QDial* self = (QDial*)_instance;
 Q_RESSOURCE_ATTRIB( "wrapping", bool, res, qXMLReadBool, self, setWrapping );
 Q_RESSOURCE_ATTRIB( "notchtarget", double, res, qXMLReadDouble, self, setNotchTarget );
 Q_RESSOURCE_ATTRIB( "tracking", bool, res, qXMLReadBool, self, setTracking );
 Q_RESSOURCE_ATTRIB( "value", int, res, qXMLReadInt, self, setValue );
 return true;
}

QWidget* qDialogFactory( const QRessource&, QWidget* _parent )
{
 return new QDialog( _parent );
}

bool qDialogConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qFrameFactory( const QRessource&, QWidget* _parent )
{
 return new QFrame( _parent );
}

bool qFrameConfig( const QRessource& res, QWidget* _instance )
{
 QFrame* self = (QFrame*)_instance;
 Q_RESSOURCE_ATTRIB( "linewidth", int, res, qXMLReadInt, self, setLineWidth );
 Q_RESSOURCE_ATTRIB( "midlinewidth", int, res, qXMLReadInt, self, setMidLineWidth );
 Q_RESSOURCE_ATTRIB( "framestyle", int, res, qXMLReadInt, self, setFrameStyle );
 Q_RESSOURCE_ATTRIB( "margin", int, res, qXMLReadInt, self, setMargin );
 return true;
}

QWidget* qGroupBoxFactory( const QRessource& res, QWidget* _parent )
{
 int cols = 1; Qt::Orientation o = Qt::Horizontal;
 if ( res.hasAttrib( "columns" ) )
  cols = res.intAttrib( "columns" );
 qXMLReadOrientation( "orientation", res, &o );
 return new QGroupBox( cols, o, _parent);
}

bool qGroupBoxConfig( const QRessource& res, QWidget* _instance )
{
 QGroupBox* self = (QGroupBox*)_instance;
 Q_RESSOURCE_ATTRIB( "title", QString, res, qXMLReadString, self, setTitle );
 Q_RESSOURCE_ATTRIB( "alignment", int, res, qXMLReadInt, self, setAlignment );
 return true;
}

QWidget* qHBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QHBox( _parent );
}

bool qHBoxConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qHeaderFactory( const QRessource&, QWidget* _parent )
{
 return new QHeader( _parent );
}

bool qHeaderConfig( const QRessource& res, QWidget* _instance )
{
 QHeader* self = (QHeader*)_instance;
 Q_RESSOURCE_ATTRIB( "movingenabled", bool, res, qXMLReadBool, self, setMovingEnabled );
 Q_RESSOURCE_ATTRIB( "tracking", bool, res, qXMLReadBool, self, setTracking );
 Q_RESSOURCE_ATTRIB( "orientation", Qt::Orientation, res, qXMLReadOrientation, self, setOrientation );
 Q_RESSOURCE_ATTRIB( "offset", int, res, qXMLReadInt, self, setOffset );
 return true;
}

QWidget* qLCDNumberFactory( const QRessource&, QWidget* _parent )
{
 return new QLCDNumber( _parent );
}

bool qLCDNumberConfig( const QRessource& res, QWidget* _instance )
{
 QLCDNumber* self = (QLCDNumber*)_instance;
 Q_RESSOURCE_ATTRIB( "smalldecimalpoint", bool, res, qXMLReadBool, self, setSmallDecimalPoint );
 Q_RESSOURCE_ATTRIB( "segmentstyle", QLCDNumber::SegmentStyle, res, qXMLReadLCDNumberSegmentStyle, self, setSegmentStyle );
 Q_RESSOURCE_ATTRIB( "numdigits", int, res, qXMLReadInt, self, setNumDigits );
 Q_RESSOURCE_ATTRIB( "mode", QLCDNumber::Mode, res, qXMLReadLCDNumberMode, self, setMode );
 return true;
}

QWidget* qLabelFactory( const QRessource&, QWidget* _parent )
{
 return new QLabel( _parent );
}

bool qLabelConfig( const QRessource& res, QWidget* _instance )
{
 QLabel* self = (QLabel*)_instance;
 Q_RESSOURCE_ATTRIB( "num", double, res, qXMLReadDouble, self, setNum );
 Q_RESSOURCE_ATTRIB( "qml", QString, res, qXMLReadString, self, setQML );
 Q_RESSOURCE_ATTRIB( "autoresize", bool, res, qXMLReadBool, self, setAutoResize );
 Q_RESSOURCE_ATTRIB( "text", QString, res, qXMLReadString, self, setText );
 Q_RESSOURCE_ATTRIB( "automask", bool, res, qXMLReadBool, self, setAutoMask );
 Q_RESSOURCE_ATTRIB( "margin", int, res, qXMLReadInt, self, setMargin );
 Q_RESSOURCE_ATTRIB( "alignment", int, res, qXMLReadInt, self, setAlignment );
 return true;
}

QWidget* qLineEditFactory( const QRessource&, QWidget* _parent )
{
 return new QLineEdit( _parent );
}

bool qLineEditConfig( const QRessource& res, QWidget* _instance )
{
 QLineEdit* self = (QLineEdit*)_instance;
 Q_RESSOURCE_ATTRIB( "cursorposition", int, res, qXMLReadInt, self, setCursorPosition );
 Q_RESSOURCE_ATTRIB( "echomode", QLineEdit::EchoMode, res, qXMLReadLineEditEchoMode, self, setEchoMode );
 Q_RESSOURCE_ATTRIB( "text", QString, res, qXMLReadString, self, setText );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "maxlength", int, res, qXMLReadInt, self, setMaxLength );
 Q_RESSOURCE_ATTRIB( "frame", bool, res, qXMLReadBool, self, setFrame );
 Q_RESSOURCE_ATTRIB( "enabled", bool, res, qXMLReadBool, self, setEnabled );
 Q_RESSOURCE_ATTRIB( "alignment", int, res, qXMLReadInt, self, setAlignment );
 Q_RESSOURCE_ATTRIB( "edited", bool, res, qXMLReadBool, self, setEdited );
 return true;
}

QWidget* qListBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QListBox( _parent );
}

bool qListBoxConfig( const QRessource& res, QWidget* _instance )
{
 QListBox* self = (QListBox*)_instance;
 Q_RESSOURCE_ATTRIB( "multiselection", bool, res, qXMLReadBool, self, setMultiSelection );
 Q_RESSOURCE_ATTRIB( "autobottomscrollbar", bool, res, qXMLReadBool, self, setAutoBottomScrollBar );
 Q_RESSOURCE_ATTRIB( "topitem", int, res, qXMLReadInt, self, setTopItem );
 Q_RESSOURCE_ATTRIB( "currentitem", int, res, qXMLReadInt, self, setCurrentItem );
 Q_RESSOURCE_ATTRIB( "scrollbar", bool, res, qXMLReadBool, self, setScrollBar );
 Q_RESSOURCE_ATTRIB( "bottomitem", int, res, qXMLReadInt, self, setBottomItem );
 Q_RESSOURCE_ATTRIB( "smoothscrolling", bool, res, qXMLReadBool, self, setSmoothScrolling );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "autoscroll", bool, res, qXMLReadBool, self, setAutoScroll );
 Q_RESSOURCE_ATTRIB( "bottomscrollbar", bool, res, qXMLReadBool, self, setBottomScrollBar );
 Q_RESSOURCE_ATTRIB( "fixedvisiblelines", int, res, qXMLReadInt, self, setFixedVisibleLines );
 Q_RESSOURCE_ATTRIB( "autoupdate", bool, res, qXMLReadBool, self, setAutoUpdate );
 Q_RESSOURCE_ATTRIB( "autoscrollbar", bool, res, qXMLReadBool, self, setAutoScrollBar );
 Q_RESSOURCE_ATTRIB( "dragselect", bool, res, qXMLReadBool, self, setDragSelect );
 return true;
}

QWidget* qListViewFactory( const QRessource&, QWidget* _parent )
{
 return new QListView( _parent );
}

bool qListViewConfig( const QRessource& res, QWidget* _instance )
{
 QListView* self = (QListView*)_instance;
 Q_RESSOURCE_ATTRIB( "multiselection", bool, res, qXMLReadBool, self, setMultiSelection );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "treestepsize", int, res, qXMLReadInt, self, setTreeStepSize );
 Q_RESSOURCE_ATTRIB( "rootisdecorated", bool, res, qXMLReadBool, self, setRootIsDecorated );
 Q_RESSOURCE_ATTRIB( "allcolumnsshowfocus", bool, res, qXMLReadBool, self, setAllColumnsShowFocus );
 Q_RESSOURCE_ATTRIB( "itemmargin", int, res, qXMLReadInt, self, setItemMargin );
 return true;
}

QWidget* qMLBrowserFactory( const QRessource&, QWidget* _parent )
{
 return new QMLBrowser( _parent );
}

bool qMLBrowserConfig( const QRessource& res, QWidget* _instance )
{
 QMLBrowser* self = (QMLBrowser*)_instance;
 Q_RESSOURCE_ATTRIB( "contents", QString, res, qXMLReadString, self, setContents );
 Q_RESSOURCE_ATTRIB( "document", QString, res, qXMLReadString, self, setDocument );
 return true;
}

QObject* qMLProviderFactory( const QRessource&, QObject* _parent )
{
 return new QMLProvider( _parent );
}

bool qMLProviderConfig( const QRessource&, QObject* )
{
 return true;
}

QObject* qMLStyleSheetFactory( const QRessource&, QObject* _parent )
{
 return new QMLStyleSheet( _parent );
}

bool qMLStyleSheetConfig( const QRessource&, QObject* )
{
 return true;
}

QWidget* qMLViewFactory( const QRessource&, QWidget* _parent )
{
 return new QMLView( _parent );
}

bool qMLViewConfig( const QRessource& res, QWidget* _instance )
{
 QMLView* self = (QMLView*)_instance;
 Q_RESSOURCE_ATTRIB( "contents", QString, res, qXMLReadString, self, setContents );
 return true;
}

QWidget* qMainWindowFactory( const QRessource&, QWidget* _parent )
{
 return new QMainWindow( _parent );
}

bool qMainWindowConfig( const QRessource& res, QWidget* _instance )
{
 QMainWindow* self = (QMainWindow*)_instance;
 Q_RESSOURCE_ATTRIB( "usesbigpixmaps", bool, res, qXMLReadBool, self, setUsesBigPixmaps );
 Q_RESSOURCE_ATTRIB( "rightjustification", bool, res, qXMLReadBool, self, setRightJustification );
 return true;
}

QWidget* qMenuBarFactory( const QRessource&, QWidget* _parent )
{
 return new QMenuBar( _parent );
}

bool qMenuBarConfig( const QRessource& res, QWidget* _instance )
{
 QMenuBar* self = (QMenuBar*)_instance;
 Q_RESSOURCE_ATTRIB( "separator", QMenuBar::Separator, res, qXMLReadMenuBarSeparator, self, setSeparator );
 return true;
}

QWidget* qMultiLineEditFactory( const QRessource&, QWidget* _parent )
{
 return new QMultiLineEdit( _parent );
}

bool qMultiLineEditConfig( const QRessource& res, QWidget* _instance )
{
 QMultiLineEdit* self = (QMultiLineEdit*)_instance;
 Q_RESSOURCE_ATTRIB( "text", QString, res, qXMLReadString, self, setText );
 Q_RESSOURCE_ATTRIB( "fixedvisiblelines", int, res, qXMLReadInt, self, setFixedVisibleLines );
 Q_RESSOURCE_ATTRIB( "overwritemode", bool, res, qXMLReadBool, self, setOverwriteMode );
 Q_RESSOURCE_ATTRIB( "autoupdate", bool, res, qXMLReadBool, self, setAutoUpdate );
 Q_RESSOURCE_ATTRIB( "readonly", bool, res, qXMLReadBool, self, setReadOnly );
 return true;
}

QWidget* qPopupMenuFactory( const QRessource&, QWidget* _parent )
{
 return new QPopupMenu( _parent );
}

bool qPopupMenuConfig( const QRessource& res, QWidget* _instance )
{
 QPopupMenu* self = (QPopupMenu*)_instance;
 Q_RESSOURCE_ATTRIB( "checkable", bool, res, qXMLReadBool, self, setCheckable );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "activeitem", int, res, qXMLReadInt, self, setActiveItem );
 return true;
}

QWidget* qProgressBarFactory( const QRessource&, QWidget* _parent )
{
 return new QProgressBar( _parent );
}

bool qProgressBarConfig( const QRessource& res, QWidget* _instance )
{
 QProgressBar* self = (QProgressBar*)_instance;
 Q_RESSOURCE_ATTRIB( "totalsteps", int, res, qXMLReadInt, self, setTotalSteps );
 Q_RESSOURCE_ATTRIB( "indicatorfollowsstyle", bool, res, qXMLReadBool, self, setIndicatorFollowsStyle );
 Q_RESSOURCE_ATTRIB( "progress", int, res, qXMLReadInt, self, setProgress );
 Q_RESSOURCE_ATTRIB( "centerindicator", bool, res, qXMLReadBool, self, setCenterIndicator );
 return true;
}

QWidget* qPushButtonFactory( const QRessource&, QWidget* _parent )
{
 return new QPushButton( _parent );
}

bool qPushButtonConfig( const QRessource& res, QWidget* _instance )
{
 QPushButton* self = (QPushButton*)_instance;
 Q_RESSOURCE_ATTRIB( "default", bool, res, qXMLReadBool, self, setDefault );
 Q_RESSOURCE_ATTRIB( "on", bool, res, qXMLReadBool, self, setOn );
 Q_RESSOURCE_ATTRIB( "ismenubutton", bool, res, qXMLReadBool, self, setIsMenuButton );
 Q_RESSOURCE_ATTRIB( "autodefault", bool, res, qXMLReadBool, self, setAutoDefault );
 Q_RESSOURCE_ATTRIB( "togglebutton", bool, res, qXMLReadBool, self, setToggleButton );
 return true;
}

QWidget* qRadioButtonFactory( const QRessource&, QWidget* _parent )
{
 return new QRadioButton( _parent );
}

bool qRadioButtonConfig( const QRessource& res, QWidget* _instance )
{
 QRadioButton* self = (QRadioButton*)_instance;
 Q_RESSOURCE_ATTRIB( "checked", bool, res, qXMLReadBool, self, setChecked );
 return true;
}

QWidget* qScrollBarFactory( const QRessource&, QWidget* _parent )
{
 return new QScrollBar( _parent );
}

bool qScrollBarConfig( const QRessource& res, QWidget* _instance )
{
 QScrollBar* self = (QScrollBar*)_instance;
 Q_RESSOURCE_ATTRIB( "tracking", bool, res, qXMLReadBool, self, setTracking );
 Q_RESSOURCE_ATTRIB( "orientation", Qt::Orientation, res, qXMLReadOrientation, self, setOrientation );
 return true;
}

QWidget* qScrollViewFactory( const QRessource&, QWidget* _parent )
{
 return new QScrollView( _parent );
}

bool qScrollViewConfig( const QRessource& res, QWidget* _instance )
{
 QScrollView* self = (QScrollView*)_instance;
 Q_RESSOURCE_ATTRIB( "hscrollbarmode", QScrollView::ScrollBarMode, res, qXMLReadScrollViewScrollBarMode, self, setHScrollBarMode );
 Q_RESSOURCE_ATTRIB( "resizepolicy", QScrollView::ResizePolicy, res, qXMLReadScrollViewResizePolicy, self, setResizePolicy );
 Q_RESSOURCE_ATTRIB( "vscrollbarmode", QScrollView::ScrollBarMode, res, qXMLReadScrollViewScrollBarMode, self, setVScrollBarMode );
 return true;
}

QWidget* qSliderFactory( const QRessource&, QWidget* _parent )
{
 return new QSlider( _parent );
}

bool qSliderConfig( const QRessource& res, QWidget* _instance )
{
 QSlider* self = (QSlider*)_instance;
 Q_RESSOURCE_ATTRIB( "tickinterval", int, res, qXMLReadInt, self, setTickInterval );
 Q_RESSOURCE_ATTRIB( "tracking", bool, res, qXMLReadBool, self, setTracking );
 Q_RESSOURCE_ATTRIB( "tickmarks", QSlider::TickSetting, res, qXMLReadSliderTickSetting, self, setTickmarks );
 Q_RESSOURCE_ATTRIB( "orientation", Qt::Orientation, res, qXMLReadOrientation, self, setOrientation );
 Q_RESSOURCE_ATTRIB( "value", int, res, qXMLReadInt, self, setValue );
 return true;
}

QWidget* qSpinBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QSpinBox( _parent );
}

bool qSpinBoxConfig( const QRessource& res, QWidget* _instance )
{
 QSpinBox* self = (QSpinBox*)_instance;
 Q_RESSOURCE_ATTRIB( "wrapping", bool, res, qXMLReadBool, self, setWrapping );
 Q_RESSOURCE_ATTRIB( "enabled", bool, res, qXMLReadBool, self, setEnabled );
 Q_RESSOURCE_ATTRIB( "value", int, res, qXMLReadInt, self, setValue );
 return true;
}

QWidget* qSplitterFactory( const QRessource&, QWidget* _parent )
{
 return new QSplitter( _parent );
}

bool qSplitterConfig( const QRessource& res, QWidget* _instance )
{
 QSplitter* self = (QSplitter*)_instance;
 Q_RESSOURCE_ATTRIB( "orientation", Qt::Orientation, res, qXMLReadOrientation, self, setOrientation );
 return true;
}

QWidget* qStatusBarFactory( const QRessource&, QWidget* _parent )
{
 return new QStatusBar( _parent );
}

bool qStatusBarConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qTabBarFactory( const QRessource&, QWidget* _parent )
{
 return new QTabBar( _parent );
}

bool qTabBarConfig( const QRessource& res, QWidget* _instance )
{
 QTabBar* self = (QTabBar*)_instance;
 Q_RESSOURCE_ATTRIB( "shape", QTabBar::Shape, res, qXMLReadTabBarShape, self, setShape );
 Q_RESSOURCE_ATTRIB( "currenttab", int, res, qXMLReadInt, self, setCurrentTab );
 return true;
}

QWidget* qTabWidgetFactory( const QRessource&, QWidget* _parent )
{
 return new QTabWidget( _parent );
}

bool qTabWidgetConfig( const QRessource& res, QWidget* _instance )
{
 QTabWidget* self = (QTabWidget*)_instance;
 Q_RESSOURCE_ATTRIB( "tabposition", QTabWidget::TabPosition, res, qXMLReadTabWidgetTabPosition, self, setTabPosition );
 Q_RESSOURCE_ATTRIB( "margin", int, res, qXMLReadInt, self, setMargin );
 return true;
}

QWidget* qToolButtonFactory( const QRessource&, QWidget* _parent )
{
 return new QToolButton( _parent );
}

bool qToolButtonConfig( const QRessource& res, QWidget* _instance )
{
 QToolButton* self = (QToolButton*)_instance;
 Q_RESSOURCE_ATTRIB( "usesbigpixmap", bool, res, qXMLReadBool, self, setUsesBigPixmap );
 Q_RESSOURCE_ATTRIB( "usestextlabel", bool, res, qXMLReadBool, self, setUsesTextLabel );
 Q_RESSOURCE_ATTRIB( "on", bool, res, qXMLReadBool, self, setOn );
 Q_RESSOURCE_ATTRIB( "togglebutton", bool, res, qXMLReadBool, self, setToggleButton );
 return true;
}

QObject* qToolTipGroupFactory( const QRessource&, QObject* _parent )
{
 return new QToolTipGroup( _parent );
}

bool qToolTipGroupConfig( const QRessource& res, QObject* _instance )
{
 QToolTipGroup* self = (QToolTipGroup*)_instance;
 Q_RESSOURCE_ATTRIB( "delay", bool, res, qXMLReadBool, self, setDelay );
 return true;
}

QWidget* qVBoxFactory( const QRessource&, QWidget* _parent )
{
 return new QVBox( _parent );
}

bool qVBoxConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qWellArrayFactory( const QRessource&, QWidget* _parent )
{
 return new QWellArray( _parent );
}

bool qWellArrayConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qWidgetFactory( const QRessource&, QWidget* _parent )
{
 return new QWidget( _parent );
}

bool qWidgetConfig( const QRessource& res, QWidget* _instance )
{
 QWidget* self = (QWidget*)_instance;
 Q_RESSOURCE_ATTRIB( "acceptdrops", bool, res, qXMLReadBool, self, setAcceptDrops );
 Q_RESSOURCE_ATTRIB( "fixedwidth", int, res, qXMLReadInt, self, setFixedWidth );
 Q_RESSOURCE_ATTRIB( "updatesenabled", bool, res, qXMLReadBool, self, setUpdatesEnabled );
 Q_RESSOURCE_ATTRIB( "fixedheight", int, res, qXMLReadInt, self, setFixedHeight );
 Q_RESSOURCE_ATTRIB( "palettepropagation", QWidget::PropagationMode, res, qXMLReadWidgetPropagationMode, self, setPalettePropagation );
 Q_RESSOURCE_ATTRIB( "fontpropagation", QWidget::PropagationMode, res, qXMLReadWidgetPropagationMode, self, setFontPropagation );
 Q_RESSOURCE_ATTRIB( "enabled", bool, res, qXMLReadBool, self, setEnabled );
 Q_RESSOURCE_ATTRIB( "mousetracking", bool, res, qXMLReadBool, self, setMouseTracking );
 Q_RESSOURCE_ATTRIB( "minimumwidth", int, res, qXMLReadInt, self, setMinimumWidth );
 Q_RESSOURCE_ATTRIB( "automask", bool, res, qXMLReadBool, self, setAutoMask );
 Q_RESSOURCE_ATTRIB( "backgroundmode", QWidget::BackgroundMode, res, qXMLReadWidgetBackgroundMode, self, setBackgroundMode );
 Q_RESSOURCE_ATTRIB( "minimumheight", int, res, qXMLReadInt, self, setMinimumHeight );
 Q_RESSOURCE_ATTRIB( "icontext", QString, res, qXMLReadString, self, setIconText );
 Q_RESSOURCE_ATTRIB( "caption", QString, res, qXMLReadString, self, setCaption );
 Q_RESSOURCE_ATTRIB( "maximumwidth", int, res, qXMLReadInt, self, setMaximumWidth );
 Q_RESSOURCE_ATTRIB( "font", QFont, res, qXMLReadFont, self, setFont );
 Q_RESSOURCE_ATTRIB( "backgroundcolor", QColor, res, qXMLReadColor, self, setBackgroundColor );
 Q_RESSOURCE_ATTRIB( "maximumheight", int, res, qXMLReadInt, self, setMaximumHeight );
 
 QRessource ch = res.firstChild();
 for( ; ch.isValid(); ch = ch.nextSibling() )
 {
   if ( ch.type() == "Layout" )
   {
     // #### We create an intermediate widget due to QLayout problems
     // For example QButtonGroup comes with a layout that we want to use.
     // So we make the new layout a child layout of the other one.
     QWidget* p = self;
     if ( self->layout() ) p = new QWidget( self );
     QLayout* l = ch.incarnateLayout( p );
     if ( !l )
       return false;
   }
   else if ( ch.type() == "Widget" )
   {
     QWidget* w = ch.incarnateWidget( self );
     if ( !w )
       return false;
   }
 }
 return true;
}

QWidget* qWidgetStackFactory( const QRessource&, QWidget* _parent )
{
 return new QWidgetStack( _parent );
}

bool qWidgetStackConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qWorkspaceFactory( const QRessource&, QWidget* _parent )
{
 return new QWorkspace( _parent );
}

bool qWorkspaceConfig( const QRessource&, QWidget* )
{
 return true;
}

QWidget* qWorkspaceChildFactory( const QRessource&, QWidget* _parent )
{
 return new QWorkspaceChild( _parent );
}

bool qWorkspaceChildConfig( const QRessource& res, QWidget* _instance )
{
 QWorkspaceChild* self = (QWorkspaceChild*)_instance;
 Q_RESSOURCE_ATTRIB( "active", bool, res, qXMLReadBool, self, setActive );
 return true;
}

QRessourceFactory::QRessourceFactory()
{
  addFactory( "QWorkspace", qWorkspaceFactory, qWorkspaceConfig );
  addFactory( "QWellArray", qWellArrayFactory, qWellArrayConfig );
  addFactory( "QMLView", qMLViewFactory, qMLViewConfig );
  addFactory( "QListView", qListViewFactory, qListViewConfig );
  addFactory( "QStatusBar", qStatusBarFactory, qStatusBarConfig );
  addFactory( "QTabWidget", qTabWidgetFactory, qTabWidgetConfig );
  addFactory( "QLCDNumber", qLCDNumberFactory, qLCDNumberConfig );
  addFactory( "QMainWindow", qMainWindowFactory, qMainWindowConfig );
  addFactory( "QLabel", qLabelFactory, qLabelConfig );
  addFactory( "QSpinBox", qSpinBoxFactory, qSpinBoxConfig );
  addFactory( "QSlider", qSliderFactory, qSliderConfig );
  addFactory( "QMLStyleSheet", qMLStyleSheetFactory, qMLStyleSheetConfig );
  addFactory( "QScrollBar", qScrollBarFactory, qScrollBarConfig );
  addFactory( "QCheckBox", qCheckBoxFactory, qCheckBoxConfig );
  addFactory( "QHeader", qHeaderFactory, qHeaderConfig );
  addFactory( "QScrollView", qScrollViewFactory, qScrollViewConfig );
  addFactory( "QProgressBar", qProgressBarFactory, qProgressBarConfig );
  addFactory( "QTabBar", qTabBarFactory, qTabBarConfig );
  addFactory( "QLineEdit", qLineEditFactory, qLineEditConfig );
  addFactory( "QDialog", qDialogFactory, qDialogConfig );
  addFactory( "QHBox", qHBoxFactory, qHBoxConfig );
  addFactory( "QWorkspaceChild", qWorkspaceChildFactory, qWorkspaceChildConfig );
  addFactory( "QGroupBox", qGroupBoxFactory, qGroupBoxConfig );
  addFactory( "QSplitter", qSplitterFactory, qSplitterConfig );
  addFactory( "QToolTipGroup", qToolTipGroupFactory, qToolTipGroupConfig );
  addFactory( "QComboBox", qComboBoxFactory, qComboBoxConfig );
  addFactory( "QMultiLineEdit", qMultiLineEditFactory, qMultiLineEditConfig );
  addFactory( "QWidgetStack", qWidgetStackFactory, qWidgetStackConfig );
  addFactory( "QToolButton", qToolButtonFactory, qToolButtonConfig );
  addFactory( "QDial", qDialFactory, qDialConfig );
  addFactory( "QButton", qButtonFactory, qButtonConfig );
  addFactory( "QPushButton", qPushButtonFactory, qPushButtonConfig );
  addFactory( "QMLBrowser", qMLBrowserFactory, qMLBrowserConfig );
  addFactory( "QVBox", qVBoxFactory, qVBoxConfig );
  addFactory( "QWidget", qWidgetFactory, qWidgetConfig );
  addFactory( "QMLProvider", qMLProviderFactory, qMLProviderConfig );
  addFactory( "QListBox", qListBoxFactory, qListBoxConfig );
  addFactory( "QButtonGroup", qButtonGroupFactory, qButtonGroupConfig );
  addFactory( "QPopupMenu", qPopupMenuFactory, qPopupMenuConfig );
  addFactory( "QMenuBar", qMenuBarFactory, qMenuBarConfig );
  addFactory( "QFrame", qFrameFactory, qFrameConfig );
  addFactory( "QRadioButton", qRadioButtonFactory, qRadioButtonConfig );
  addFactory( "QGridLayout", qGridLayoutFactory, qGridLayoutConfig );
  addFactory( "QHBoxLayout", qHBoxLayoutFactory, qHBoxLayoutConfig );
  addFactory( "QObject", qObjectFactory, qObjectConfig );
}

