#include "linedlg.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

DLineDlg::DLineDlg( const QString& _text, QWidget* parent, const char* name, bool modal )
  : QDialog( parent, name, modal )
{
  QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );

  QLabel* l = new QLabel( _text, this );
  vbox->addWidget( l );

  m_pLineEdit = new QLineEdit( this );
  vbox->addWidget( m_pLineEdit );

  QHBoxLayout *hbox = new QHBoxLayout( 6 );
  vbox->addLayout( hbox, AlignRight );
  
  QPushButton *ok = new QPushButton( tr("Ok"), this );
  hbox->addWidget( ok );
  QPushButton *cancel = new QPushButton( tr("Cancel"), this );
  hbox->addWidget( cancel );

  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( m_pLineEdit, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
  connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

  m_pLineEdit->setFocus();
}

DLineDlg::~DLineDlg()
{
}

QString DLineDlg::text()
{
  return m_pLineEdit->text();
}

void DLineDlg::setText( const QString& _text )
{
 m_pLineEdit->setText( _text );
}
