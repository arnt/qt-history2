#ifndef __connectdlg_h__
#define __connectdlg_h__

#include <qdialog.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qlistview.h>

class DObjectInfo;
class DFormEditor;

/**
 * :-) I could not resist
 */
class DIntegerServer
{
public:
  static int number() { return s_counter++; }
  static QString name();

private:
  static int s_counter;
};

class DConnectDlg : public QDialog
{
    Q_OBJECT
public:
    DConnectDlg( DFormEditor* editor, DObjectInfo* sender, DObjectInfo* receiver, QWidget* parent = 0, const char* name = 0 );
    ~DConnectDlg();

protected slots:
    void slotConnect();

private:
    DObjectInfo* m_pSender;
    DObjectInfo* m_pReceiver;
    
    QValueList<QListViewItem*> m_lstGroups;

    QListView* m_senderList;
    QListView* m_receiverList;

    QListViewItem* m_newSenderSignal;
    QListViewItem* m_newReceiverSignal;
    QListViewItem* m_newReceiverSlot;
    QListViewItem* m_receiverSignals;
};

#endif
