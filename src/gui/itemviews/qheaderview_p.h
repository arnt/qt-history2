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

#ifndef QHEADERVIEW_P_H
#define QHEADERVIEW_P_H

#include <private/qabstractitemview_p.h>
#include <qapplication.h>
#include <qlabel.h>

class QHeaderViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QHeaderView)

public:
    QHeaderViewPrivate()
        : state(NoState),
          offset(0),
          sortIndicatorOrder(Qt::AscendingOrder),
          sortIndicatorSection(0),
          sortIndicatorShown(false),
          lastPos(-1),
          section(-1),
          target(-1),
          pressed(-1),
          movableSections(false),
          clickableSections(false),
          highlightCurrent(false),
          stretchSections(0),
          sectionIndicatorOffset(0),
          sectionIndicator(0) {}

    int sectionHandleAt(int position);
    void setupSectionIndicator(int section, int position);
    void updateSectionIndicator(int section, int position);

    inline bool reverse() const
        { return QApplication::reverseLayout() && orientation == Qt::Horizontal; }

    enum State { NoState, ResizeSection, MoveSection } state;

    int offset;
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;
    bool sortIndicatorShown;

    struct HeaderSection {
        int position;
        int section;
        uint hidden : 1;
        QHeaderView::ResizeMode mode;
        inline bool operator>(int position) const
            { return (*this).position > position; }
    };
    mutable QVector<HeaderSection> sections; // section = sections.at(index)
    mutable QVector<int> indices; // index = indices.at(section)

    int lastPos;
    int section; // used for resizing and moving sections
    int target;
    int pressed;
    bool movableSections;
    bool clickableSections;
    bool highlightCurrent;
    int stretchSections;
    int sectionIndicatorOffset;
    QLabel *sectionIndicator;
    QStyleOptionHeader getStyleOption() const;
};

static const int default_width = 100;
static const int default_height = 30;

#endif
