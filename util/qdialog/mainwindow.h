#ifndef __qdialog_h__
#define __qdialog_h__

#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtabdialog.h>
#include <qtabwidget.h>
#include <qlist.h>

#include "formeditor.h"

class DWidgetsBar;
class DInspector;
class DEditorContainer;
class QXMLtag;
class QSplitter;

class DMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  DMainWindow();
  ~DMainWindow();

public slots:
  void slotOpen();
  void slotOpen( const QString& _name );
  void slotSaveAs();
  void slotNew();

  void slotPreview();

  void slotGridArrange();
  void slotHArrange();
  void slotVArrange();
  void slotApplySizeHint();

private:
  DWidgetsBar* m_widgetsBar;
  DInspector* m_inspector;
  DEditorContainer* m_editorContainer;
  QSplitter* m_splitter;
  QPtrDict<QWidget> m_dctPreviewWidgets;
};

class DEditorContainer : public QTabWidget
{
  Q_OBJECT
public:   
  DEditorContainer( QWidget* _parent );
  ~DEditorContainer();

  void addEditor( const QString& _name, DFormEditor* );
  
  void save( QXMLTag* );

  DFormEditor* currentEditor();

private:
  QList<DFormEditor> m_lstEditors;
};

#endif
