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

/*
TRANSLATOR qdesigner_internal::PreviewWorkspace
*/

#include "previewframe.h"
#include "previewwidget.h"

#include <QtGui/QPainter>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QPaintEvent>

namespace {
    class PreviewMdiArea: public QMdiArea {
    public:
        PreviewMdiArea(QWidget *parent = 0) : QMdiArea(parent) {}
    protected:
        bool viewportEvent ( QEvent * event );
    };

    bool PreviewMdiArea::viewportEvent (QEvent * event) {
        if (event->type() != QEvent::Paint)
            return QMdiArea::viewportEvent (event);
        QPainter p(viewport());
        p.fillRect(rect(), palette().color(backgroundRole()).dark());
        p.setPen(QPen(Qt::white));
        p.drawText(0, height() / 2,  width(), height(), Qt::AlignHCenter,
                   tr("The moose in the noose\nate the goose who was loose."));
        return true;
    }
}

namespace qdesigner_internal {

PreviewFrame::PreviewFrame(QWidget *parent) : 
    QFrame(parent),
    m_mdiArea(new PreviewMdiArea(this)),
    m_previewWidget(new PreviewWidget(m_mdiArea)),
    m_mdiSubWindow(m_mdiArea->addSubWindow(m_previewWidget, Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint))     
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addWidget(m_mdiArea);

    m_mdiSubWindow->move(10,10);
    m_mdiSubWindow->show();
}

void PreviewFrame::setPreviewPalette(const QPalette &pal)
{
    m_previewWidget->setPalette(pal);
}
    
void PreviewFrame::setSubWindowActive(bool active)
{
    m_mdiArea->setActiveSubWindow (active ? m_mdiSubWindow : static_cast<QMdiSubWindow *>(0));
}

}
