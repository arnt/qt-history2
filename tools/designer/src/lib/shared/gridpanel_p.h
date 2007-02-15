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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Qt tools.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef GRIDPANEL_H
#define GRIDPANEL_H

#include "shared_global_p.h"

#include <QtGui/QGroupBox>

class QSpinBox;
class QCheckBox;

namespace qdesigner_internal {

class Grid;

class  QDESIGNER_SHARED_EXPORT GridPanel : public QGroupBox
{
    Q_OBJECT
public:
    GridPanel(QWidget *parentWidget = 0);

    void setGrid(const Grid &g);
    Grid grid() const;

private slots:
    void reset();

private:
    QCheckBox *m_visibleCheckBox;
    QCheckBox *m_snapXCheckBox;
    QCheckBox *m_snapYCheckBox;
    QSpinBox *m_deltaXSpinBox;
    QSpinBox *m_deltaYSpinBox;
};
}
#endif // GRIDPANEL_H
