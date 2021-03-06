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

#ifndef PREVIEWFRAME_H
#define PREVIEWFRAME_H

#include <QtGui/QFrame>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QMdiArea;
class QMdiSubWindow;

namespace qdesigner_internal {

class PreviewFrame: public QFrame
{
    Q_OBJECT
public:
    explicit PreviewFrame(QWidget *parent);

    void setPreviewPalette(const QPalette &palette);
    void setSubWindowActive(bool active);
    
private:
    // The user can on some platforms close the mdi child by invoking the system menu.
    // Ensure a child is present.
    QMdiSubWindow *ensureMdiSubWindow();
    QMdiArea *m_mdiArea;
    QPointer<QMdiSubWindow> m_mdiSubWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
