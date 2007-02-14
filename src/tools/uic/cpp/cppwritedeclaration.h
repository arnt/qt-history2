/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CPPWRITEDECLARATION_H
#define CPPWRITEDECLARATION_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

namespace CPP {

struct WriteDeclaration : public TreeWalker
{
    WriteDeclaration(Uic *uic, bool activateScripts);

    void acceptUI(DomUI *node);
    void acceptWidget(DomWidget *node);
    void acceptLayout(DomLayout *node);
    void acceptActionGroup(DomActionGroup *node);
    void acceptAction(DomAction *node);

private:
    Uic *m_uic;
    Driver *m_driver;
    QTextStream &m_output;
    const Option &m_option;
    const bool m_activateScripts;
};

} // namespace CPP

#endif // CPPWRITEDECLARATION_H
