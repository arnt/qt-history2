#ifndef __dinspector_h__
#define __dinspector_h__

#include <qtabwidget.h>
#include <qlist.h>
#include <qframe.h>
#include <qproperty.h>
#include <qlistview.h>

#include "qform.h"

class QLineEdit;
class QLabel;
class QPushButton;
class DFormEditor;
class DObjectInfo;

class DPropertyEditor : public QFrame
{
  Q_OBJECT
public:
  DPropertyEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
		   QWidget* _parent = 0, const char* _name = 0 );
  ~DPropertyEditor();

  QString propertyName() const { return prop_name; }
  QProperty& property() { return prop; }
  
  bool isValid() const { return ( prop.type() != QProperty::Empty ); }
  
public slots:
  virtual void slotUpdate();
  /**
   * Called if the user switches between default/custom value.
   */
  void slotToggled( bool );
 
protected:
  void apply();
  
private:
  QString prop_name;
  DObjectInfo* obj;
  QProperty prop;
  /**
   * Pointer to the button which switches between default/custom value.
   *
   * @see #slotToggled
   */
  QPushButton* toggle;
};

class DPropEditorFactory
{
public: 
  static DPropertyEditor* create( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
				  QWidget* _parent = 0, const char* _name = 0 );

protected:
  DPropEditorFactory();
  ~DPropEditorFactory();
};

class DStringPropEditor : public DPropertyEditor
{
  Q_OBJECT
public:
  DStringPropEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
		   QWidget* _parent = 0, const char* _name = 0 );
  ~DStringPropEditor();

public slots:
  virtual void slotUpdate();

private slots:
  void returnPressed();
  
private:
  QLineEdit* lineedit;
};

class DPixmapPropEditor : public DPropertyEditor
{
  Q_OBJECT
public:
  DPixmapPropEditor( DObjectInfo* _inspect, const QString& _prop_name, QPushButton* _toggle,
		   QWidget* _parent = 0, const char* _name = 0 );
  ~DPixmapPropEditor();

public slots:
  virtual void slotUpdate();

protected:
  virtual void mousePressEvent( QMouseEvent* );
  virtual bool eventFilter( QObject* o, QEvent* e );
  
  void openFile();
  
private:
  QLabel* label;
  QString filename;
};

class DConnectionInspector : public QListView
{
  Q_OBJECT
public:
  DConnectionInspector( QWidget* _parent = 0, const char* _name = 0 );
  ~DConnectionInspector();

public slots:
  virtual void inspect( DFormEditor*, DObjectInfo* _obj );

private:
  DObjectInfo* m_info;
};

class DPropertyInspector : public QForm
{
  Q_OBJECT
public:
  DPropertyInspector( QWidget* _parent = 0, const char* _name = 0 );
  ~DPropertyInspector();
  
public slots:
  virtual void inspect( DFormEditor*, DObjectInfo* _info );

private:
  DObjectInfo* m_info;

  QList<QWidget> m_widgets;
};

class DInspector : public QTabWidget
{
  Q_OBJECT
public:
  DInspector( QWidget* _parent = 0, const char* _name = 0 );
  ~DInspector();

  DPropertyInspector* propertyInspector() { return props; }
  DConnectionInspector* connectionInspector() { return connects; }

public slots:
  virtual void inspect( DFormEditor*, DObjectInfo* _info );

private:
  DPropertyInspector *props;
  DConnectionInspector* connects;

  QWidget* m_editor;
  QWidget* m_editorChild;
};

#endif
