#ifndef __qdialog_h__
#define __qdialog_h__

#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtabdialog.h>

class DWidgetsBar;
class DInspector;
class DFormEditor;

class DMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  DMainWindow();
  ~DMainWindow();

private:
  DWidgetsBar* m_widgetsBar;
  DInspector* m_inspector;
  DFormEditor* m_formEditor;
};

#endif
