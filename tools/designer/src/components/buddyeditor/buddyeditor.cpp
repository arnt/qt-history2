/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QLabel>
#include "buddyeditor.h"

BuddyEditor::BuddyEditor(AbstractFormWindow *form_window, QWidget *parent)
    : ConnectionEdit(parent, form_window->commandHistory())
{
    m_form_window = form_window;
}

