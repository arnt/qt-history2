#ifndef QLINEDIALOG_H
#define QLINEDIALOG_H

#include <qdialog.h>
#include <qstring.h>

class QLineEdit;

class QLineDialog : public QDialog
{
  Q_OBJECT
public:
  QLineDialog( const QString& label, QWidget* parent = 0, const char* name = 0, bool modal = TRUE );
  ~QLineDialog();

  QString text();
  void setText( const QString& );

private:
  QLineEdit* m_pLineEdit;
};

#endif
