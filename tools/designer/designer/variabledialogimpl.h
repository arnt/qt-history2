/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VARIABLEDIALOGIMPL_H
#define VARIABLEDIALOGIMPL_H

#include "variabledialog.h"

class FormWindow;
class QListView;

class VariableDialog : public VariableDialogBase
{
    Q_OBJECT
public:
    VariableDialog( FormWindow *fw, QWidget* parent = 0 );
    ~VariableDialog();

    void setCurrentItem( const QString &text );

protected slots:
    void okClicked();
    void addVariable();
    void deleteVariable();
    void nameChanged();
    void accessChanged();
    void currentItemChanged( QListViewItem *i );

private:
    FormWindow *formWindow;
};

#endif
