#ifndef SPECIALEDITORSUPPORT_H
#define SPECIALEDITORSUPPORT_H

#include "specialeditor_global.h"
#include <QObject>

class AbstractFormEditor;
class QDesignerSpecialEditorFactory;

class QT_SPECIALEDITORSUPPORT_EXPORT SpecialEditorSupport: public QObject
{
    Q_OBJECT
public:
    SpecialEditorSupport(AbstractFormEditor *core);
    virtual ~SpecialEditorSupport();

    inline AbstractFormEditor *core() const
    { return m_core; }

private:
    AbstractFormEditor *m_core;
    QDesignerSpecialEditorFactory *m_factory;
};

#endif // SPECIALEDITORSUPPORT_H
