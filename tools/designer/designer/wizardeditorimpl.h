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

#ifndef WIZARDEDITORIMPL_H
#define WIZARDEDITORIMPL_H

class QWizard;
class FormWindow;

#include "wizardeditor.h"
#include "command.h"

class WizardEditor : public WizardEditorBase
{
    Q_OBJECT

public:
    WizardEditor( QWidget *parent, QWizard *wizard, FormWindow *fw );
    ~WizardEditor();

protected slots:
    void okClicked();
    void applyClicked();
    void cancelClicked();
    void helpClicked();

    void addClicked();
    void removeClicked();
    void upClicked();
    void downClicked();

    void itemHighlighted( int );
    void itemSelected( int );

    void itemDragged( QListBoxItem * );
    void itemDropped( QListBoxItem * );

private:
    void updateButtons();
    void fillListBox();

private:
    FormWindow *formwindow;
    QWizard *wizard;
    QList<Command*> commands;
    int draggedItem;
};

#endif
