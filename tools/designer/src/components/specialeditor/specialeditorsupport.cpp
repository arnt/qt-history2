#include "specialeditorsupport.h"
#include "defaultspecialeditor.h"

// Sdk
#include <abstractformeditor.h>
#include <qextensionmanager.h>

SpecialEditorSupport::SpecialEditorSupport(AbstractFormEditor *core)
    : QObject(core),
      m_core(core)
{
    m_factory = new QDesignerSpecialEditorFactory(core->extensionManager());
    core->extensionManager()->registerExtensions(m_factory, Q_TYPEID(ISpecialEditor));
}

SpecialEditorSupport::~SpecialEditorSupport()
{
//    m_core->extensionManager()->unregisterExtensions(m_factory, Q_TYPEID(ISpecialEditor));
}
