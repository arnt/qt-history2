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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qabstractitemview_p.h>

#ifndef QT_NO_ITEMVIEWS
#include <qbitarray.h>
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
          hover(-1),
          movableSections(false),
          clickableSections(false),
          highlightSelected(false),
          stretchLastSection(false),
          stretchSections(0),
          sectionIndicatorOffset(0),
          sectionIndicator(0),
          globalResizeMode(QHeaderView::Interactive) {}

    int sectionHandleAt(int position);
    void setupSectionIndicator(int section, int position);
    void updateSectionIndicator(int section, int position);

    bool isSectionSelected(int section) const;
    inline void prepareSectionSelected()
        { if (sectionSelection.count() != sections.count() * 2)
            sectionSelection.fill(false, sections.count() * 2);
          else sectionSelection.fill(false); }

    inline int defaultSectionSize() const
        { return (orientation == Qt::Horizontal ? 100 : 30); }

    inline bool reverse() const
        { return q_func()->isRightToLeft() && orientation == Qt::Horizontal; }

    enum State { NoState, ResizeSection, MoveSection } state;

    int offset;
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;
    bool sortIndicatorShown;

    struct HeaderSection {
        int position;
        int logical;
        uint hidden : 1;
        QHeaderView::ResizeMode mode;
        inline bool operator>(int position) const
            { return (*this).position > position; }
    };
    mutable QVector<HeaderSection> sections; // HeaderSection = sections.at(visualIndex)
    mutable QVector<int> visualIndices; // visualIndex = visualIndices.at(logicalIndex)
    mutable QBitArray sectionSelection;

    int lastPos;
    int section; // used for resizing and moving sections
    int target;
    int pressed;
    int hover;
    bool movableSections;
    bool clickableSections;
    bool highlightSelected;
    bool stretchLastSection;
    int stretchSections;
    int sectionIndicatorOffset;
    QLabel *sectionIndicator;
    QStyleOptionHeader getStyleOption() const;
    QHeaderView::ResizeMode globalResizeMode;
};

#endif // QT_NO_ITEMVIEWS
#endif // QHEADERVIEW_P_H
