#include "connectdlg.h"
#include "formeditor.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <qlabel.h>
#include <qmessagebox.h>

DConnectDlg::DConnectDlg( DFormEditor* editor, DObjectInfo* sender, DObjectInfo* receiver, QWidget* parent, const char* name )
  : QDialog( parent, name )
{
  m_pSender = sender;
  m_pReceiver = receiver;
    
  m_senderList = new QListView( this );
  m_senderList->addColumn( "Signals" );
  m_senderList->setRootIsDecorated( TRUE );

  if ( editor->isTopLevelWidget( sender->widget() ) )
  {
    QListViewItem *item = new QListViewItem( m_senderList, tr("Custom signals") );
    m_lstGroups.append( item );
    (void)new QListViewItem( item, tr("(Create new signal)") );

    QStringList::ConstIterator it = sender->customSignals().begin();
    QStringList::ConstIterator end = sender->customSignals().end();
    for( ; it != end; ++it )
      (void)new QListViewItem( item, *it );
  }

  QMetaObject *meta = m_pSender->widget()->metaObject();
  if ( meta )
  {
    int n = meta->nSignals( TRUE );
    for( int i = 0; i < n; ++i )
    {
      QMetaData* d = meta->signal( i, TRUE );
      QListViewItem* item = new QListViewItem( m_senderList, (const char*)d->name );
    }
  }

  m_receiverList = new QListView( this );
  m_receiverList->addColumn( "Slots/Signals" );
  m_receiverList->setRootIsDecorated( TRUE );

  QListViewItem* signal = new QListViewItem( m_receiverList, tr("Signals") );
  QListViewItem* slot = new QListViewItem( m_receiverList, tr("Slots") );
  m_lstGroups.append( signal );
  m_lstGroups.append( slot );
  
  if ( editor->isTopLevelWidget( receiver->widget() ) )
  {
    QListViewItem* item = new QListViewItem( m_receiverList, tr("Custom signals") );
    m_lstGroups.append( item );
    (void)new QListViewItem( item, tr("(Create new signal)") );
    item = new QListViewItem( m_receiverList, tr("Custom slots") );

    QStringList::ConstIterator it = sender->customSignals().begin();
    QStringList::ConstIterator end = sender->customSignals().end();
    for( ; it != end; ++it )
      (void)new QListViewItem( item, *it );

    m_lstGroups.append( item );
    (void)new QListViewItem( item, tr("(Create new slot)") );

    it = sender->customSlots().begin();
    end = sender->customSlots().end();
    for( ; it != end; ++it )
      (void)new QListViewItem( item, *it );
  }

  meta = m_pReceiver->widget()->metaObject();
  if ( meta )
  {
    int n = meta->nSignals( TRUE );
    int i;
    for( i = 0; i < n; ++i )
    {
      QMetaData* d = meta->signal( i, TRUE );
      QListViewItem* item = new QListViewItem( signal, (const char*)d->name );
    }

    n = meta->nSlots( TRUE );
    for( i = 0; i < n; ++i )
    {
      QMetaData* d = meta->slot( i, TRUE );
      QListViewItem* item = new QListViewItem( slot, (const char*)d->name );
    }
  }

  QVBoxLayout* vbox = new QVBoxLayout( this, 6, 6 );
  vbox->addWidget( new QLabel( tr("Sender"), this ) );
  vbox->addWidget( m_senderList );
  vbox->addSpacing( 6 );
  vbox->addWidget( new QLabel( tr("Receiver"), this ) );
  vbox->addWidget( m_receiverList );

  QHBoxLayout* hbox = new QHBoxLayout( vbox, 6 );
  vbox->addLayout( hbox );

  QPushButton* connect = new QPushButton( tr("Connect" ), this );
  QPushButton* cancel = new QPushButton( tr("Cancel" ), this );
  hbox->addStretch( 1 );
  hbox->addWidget( connect );
  hbox->addWidget( cancel );

  QObject::connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  QObject::connect( connect, SIGNAL( clicked() ), this, SLOT( slotConnect() ) );
}

DConnectDlg::~DConnectDlg()
{
}

void DConnectDlg::slotConnect()
{
  QListViewItem* from = m_senderList->selectedItem();
  if ( !from || m_lstGroups.contains( from ) )
  {
    QMessageBox::critical( this, tr("Error"), tr("You did not select a sender."), tr("Ok") );
    return;
  }

  QListViewItem* to = m_receiverList->selectedItem();
  if ( !to || m_lstGroups.contains( to ) )
  {
    QMessageBox::critical( this, tr("Error"), tr("You did not select a receiver."), tr("Ok") );
    return;
  }

  DObjectInfo::Connection con;
  con.slot = to->text( 0 );
  con.signal = from->text( 0 );
  con.receiverName = m_pReceiver->widget()->name();
  con.senderName = m_pSender->widget()->name();

  // Check wether sender and receiver have a name
  if ( con.receiverName == "unnamed" || con.receiverName.isEmpty() )
  {
    con.receiverName = DIntegerServer::name();
    m_pReceiver->widget()->setName( con.receiverName );
    // TODO: emit signal so that the property inspector can update
  }
  if ( con.senderName == "unnamed" || con.senderName.isEmpty() )
  {
    con.senderName = DIntegerServer::name();
    m_pSender->widget()->setName( con.senderName );
    // TODO: emit signal so that the property inspector can update
  }

  // Store the connection at the sender
  m_pSender->addConnection( con );

  // TODO: Emit a signal so that the inspector can update

  accept();
}


/***********************************************************
 *
 * DIntegerServer
 *
 ***********************************************************/

int DIntegerServer::s_counter = 1;

QString DIntegerServer::name()
{
  QString tmp( "unnamed%1" );
  tmp = tmp.arg( number() );

  return tmp;
}
