
#include <QtGui/QScrollBar>
#include "oubliette.h"
#include "oublietteview.h"


OublietteView::OublietteView()
{
    m_dungeon = new Oubliette;
    setWidget(m_dungeon);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(m_dungeon, SIGNAL(characterMoved(const QPoint &)),
            this, SLOT(scrollToCharacter(const QPoint &)));
    setFocusPolicy(Qt::NoFocus);
    m_dungeon->setFocus();
    scrollToCharacter(m_dungeon->visualCursorPos());
}

OublietteView::~OublietteView()
{
}

void OublietteView::scrollToCharacter(const QPoint &pt)
{
    bool needUpdate = false;
    if (qAbs(pt.x() - horizontalScrollBar()->value()) >= 10 * 32) {
        horizontalScrollBar()->setValue(pt.x() - width() / 2);
        needUpdate = true;
    }
    if (qAbs(pt.y() - (verticalScrollBar()->value())) >= 6 * 32) {
        verticalScrollBar()->setValue(pt.y() - height() / 2);
        needUpdate = true;
    }
    if (needUpdate)
        scrollContentsBy(0, 0);
}
