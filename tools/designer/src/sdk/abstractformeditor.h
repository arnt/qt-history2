#ifndef ABSTRACTFORMEDITOR_H
#define ABSTRACTFORMEDITOR_H

#include "sdk_global.h"

#include <QObject>

class AbstractWidgetBox;
class AbstractPropertyEditor;
class AbstractFormWindowManager;
class AbstractWidgetDataBase;
class AbstractMetaDataBase;
class AbstractWidgetFactory;
class AbstractDnDManager;
class AbstractUndoManager;

class QWidget;

class QExtensionManager;
class ObjectInspector;

class QT_SDK_EXPORT AbstractFormEditor: public QObject
{
    Q_OBJECT
public:
    AbstractFormEditor(QObject *parent = 0);
    virtual ~AbstractFormEditor();

    inline QExtensionManager *extensionManager() const;

    inline QWidget *topLevel() const;
    inline AbstractWidgetBox *widgetBox() const;
    inline AbstractPropertyEditor *propertyEditor() const;
    inline AbstractFormWindowManager *formManager() const;
    inline AbstractWidgetDataBase *widgetDataBase() const;
    inline AbstractMetaDataBase *metaDataBase() const;
    inline AbstractWidgetFactory *widgetFactory() const;
    inline AbstractUndoManager *undoManager() const;

    inline ObjectInspector *objectInspector() const; // ### abstract

    inline void setTopLevel(QWidget *topLevel);
    inline void setWidgetBox(AbstractWidgetBox *widgetBox);
    inline void setPropertyEditor(AbstractPropertyEditor *propertyEditor);
    inline void setObjectInspector(ObjectInspector *objectInspector); // ### abstract

protected:
    inline void setFormManager(AbstractFormWindowManager *formManager);
    inline void setMetaDataBase(AbstractMetaDataBase *metaDataBase);
    inline void setWidgetDataBase(AbstractWidgetDataBase *widgetDataBase);
    inline void setWidgetFactory(AbstractWidgetFactory *widgetFactory);
    inline void setExtensionManager(QExtensionManager *extensionManager);
    inline void setUndoManager(AbstractUndoManager *undoManager);

private:
    QWidget *m_topLevel;
    AbstractWidgetBox *m_widgetBox;
    AbstractPropertyEditor *m_propertyEditor;
    AbstractFormWindowManager *m_formManager;
    QExtensionManager *m_extensionManager;
    AbstractMetaDataBase *m_metaDataBase;
    AbstractWidgetDataBase *m_widgetDataBase;
    AbstractWidgetFactory *m_widgetFactory;
    AbstractUndoManager *m_undoManager;
    ObjectInspector *m_objectInspector;

private:
    AbstractFormEditor(const AbstractFormEditor &other);
    void operator = (const AbstractFormEditor &other);
};

inline AbstractWidgetBox *AbstractFormEditor::widgetBox() const
{ return m_widgetBox; }

inline void AbstractFormEditor::setWidgetBox(AbstractWidgetBox *widgetBox)
{ m_widgetBox = widgetBox; }

inline AbstractPropertyEditor *AbstractFormEditor::propertyEditor() const
{ return m_propertyEditor; }

inline void AbstractFormEditor::setPropertyEditor(AbstractPropertyEditor *propertyEditor)
{ m_propertyEditor = propertyEditor; }

inline QWidget *AbstractFormEditor::topLevel() const
{ return m_topLevel; }

inline void AbstractFormEditor::setTopLevel(QWidget *topLevel)
{ m_topLevel = topLevel; }

inline AbstractFormWindowManager *AbstractFormEditor::formManager() const
{ return m_formManager; }

inline void AbstractFormEditor::setFormManager(AbstractFormWindowManager *formManager)
{ m_formManager = formManager; }

inline QExtensionManager *AbstractFormEditor::extensionManager() const
{ return m_extensionManager; }

inline void AbstractFormEditor::setUndoManager(AbstractUndoManager *undoManager)
{ m_undoManager = undoManager; }

inline AbstractUndoManager *AbstractFormEditor::undoManager() const
{ return m_undoManager; }

inline void AbstractFormEditor::setExtensionManager(QExtensionManager *extensionManager)
{ m_extensionManager = extensionManager; }

inline AbstractMetaDataBase *AbstractFormEditor::metaDataBase() const
{ return m_metaDataBase; }

inline void AbstractFormEditor::setMetaDataBase(AbstractMetaDataBase *metaDataBase)
{ m_metaDataBase = metaDataBase; }

inline AbstractWidgetDataBase *AbstractFormEditor::widgetDataBase() const
{ return m_widgetDataBase; }

inline void AbstractFormEditor::setWidgetDataBase(AbstractWidgetDataBase *widgetDataBase)
{ m_widgetDataBase = widgetDataBase; }

inline AbstractWidgetFactory *AbstractFormEditor::widgetFactory() const
{ return m_widgetFactory; }

inline void AbstractFormEditor::setWidgetFactory(AbstractWidgetFactory *widgetFactory)
{ m_widgetFactory = widgetFactory; }

inline ObjectInspector *AbstractFormEditor::objectInspector() const
{ return m_objectInspector; }

inline void AbstractFormEditor::setObjectInspector(ObjectInspector *objectInspector)
{ m_objectInspector = objectInspector; }


#endif // ABSTRACTFORMEDITOR_H

