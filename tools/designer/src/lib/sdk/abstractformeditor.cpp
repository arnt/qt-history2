
#include "abstractformeditor.h"

AbstractFormEditor::AbstractFormEditor(QObject *parent)
    : QObject(parent),
      m_topLevel(0),
      m_widgetBox(0),
      m_propertyEditor(0),
      m_formManager(0),
      m_extensionManager(0),
      m_metaDataBase(0),
      m_widgetDataBase(0),
      m_widgetFactory(0),
      m_undoManager(0),
      m_objectInspector(0)
{
}

AbstractFormEditor::~AbstractFormEditor()
{
}

