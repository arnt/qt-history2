#include <qvariant.h>
#include "abstractpropertyeditor.h"

AbstractPropertyEditor::AbstractPropertyEditor(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
}

AbstractPropertyEditor::~AbstractPropertyEditor()
{
}

AbstractFormEditor *AbstractPropertyEditor::core() const
{
    return 0;
}

