#ifndef __linedlg_h__
#define __linedlg_h__

#include <qdialog.h>
#include <string.h>

class QLineEdit;

class DLineDlg : public QDialog
{
  Q_OBJECT
public:
  DLineDlg( const QString& label, QWidget* parent = 0, const char* name = 0, bool modal = TRUE );
  ~DLineDlg();

  QString text();
  void setText( const QString& );

private:
  QLineEdit* m_pLineEdit;
};

#endif
