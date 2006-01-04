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

#include "private/qabstractitemview_p.h"

#ifndef QT_NO_ITEMVIEWS
#include "QtCore/qbitarray.h"
#include "QtGui/qapplication.h"
#include "QtGui/qlabel.h"

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
          firstPos(-1),
          section(-1),
          target(-1),
          pressed(-1),
          hover(-1),
          length(0),
          sectionCount(0),
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
    void resizeSections(QHeaderView::ResizeMode globalMode, bool useGlobalMode = false);
    void sectionsRemoved(const QModelIndex &,int,int);

    bool isSectionSelected(int section) const;

    inline void prepareSectionSelected() {
        if (!selectionModel->hasSelection())
            sectionSelected.clear();
        else if (sectionSelected.count() != sectionCount * 2)
            sectionSelected.fill(false, sectionCount * 2);
        else sectionSelected.fill(false);
    }

    inline bool reverse() const {
        return q_func()->isRightToLeft() && orientation == Qt::Horizontal;
    }

    inline int logicalIndex(int visualIndex) const {
        return logicalIndices.isEmpty() ? visualIndex : logicalIndices.at(visualIndex);
    }

    inline void setDefaultValues(Qt::Orientation o) {
        defaultSectionSize = (o == Qt::Horizontal ? 100 : 30);
        defaultAlignment = (o == Qt::Horizontal
                            ? Qt::Alignment(Qt::AlignCenter)
                            : Qt::AlignLeft|Qt::AlignVCenter);
        orientation = o;
    }

    inline bool isVisualIndexHidden(int visual) const {
        return !sectionHidden.isEmpty() && sectionHidden.at(visual);
    }

    inline void setVisualIndexHidden(int visual, bool hidden) {
        if (!sectionHidden.isEmpty()) sectionHidden.setBit(visual, hidden);
    }

    inline QHeaderView::ResizeMode visualIndexResizeMode(int visual) const {
        return (sectionResizeMode.count() <= visual
                ? globalResizeMode : sectionResizeMode.at(visual));
    }

    QStyleOptionHeader getStyleOption() const;

    void clear();

    enum State { NoState, ResizeSection, MoveSection } state;

    int offset;
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;
    bool sortIndicatorShown;

    mutable QVector<QHeaderView::ResizeMode> sectionResizeMode;
    mutable QVector<int> visualIndices; // visualIndex = visualIndices.at(logicalIndex)
    mutable QVector<int> logicalIndices; // logicalIndex = row or column in the model
    mutable QBitArray sectionSelected;
    mutable QBitArray sectionHidden;
    mutable QHash<int, int> hiddenSectionSize; // from logical index to section size
    
    int lastPos;
    int firstPos;
    int section; // used for resizing and moving sections
    int target;
    int pressed;
    int hover;
    int length;
    int sectionCount;
    bool movableSections;
    bool clickableSections;
    bool highlightSelected;
    bool stretchLastSection;
    int stretchSections;
    int sectionIndicatorOffset;
    int defaultSectionSize;
    Qt::Alignment defaultAlignment;
    QLabel *sectionIndicator;
    QHeaderView::ResizeMode globalResizeMode;

    // header section spans
    
    struct SectionSpan {
        int size;
        int count;
        inline SectionSpan() : size(0), count(0) {}
        inline SectionSpan(int length, int sections) : size(length), count(sections) {}
        inline int sectionSize() const { return size / count; }
    };

    QVector<SectionSpan> sectionSpans;

    void createSectionSpan(int start, int end, int size);
    void removeSectionsFromSpans(int start, int end);

    inline int headerSectionCount() const { // for debugging
        int count = 0;
        for (int i = 0; i < sectionSpans.count(); ++i)
            count += sectionSpans.at(i).count;
        return count;
    }

    inline int headerLength() const { // for debugging
        int len = 0;
        for (int i = 0; i < sectionSpans.count(); ++i)
            len += sectionSpans.at(i).size;
        return len;
    }

    int headerSectionSize(int visual) const;
    int headerSectionPosition(int visual) const;
    int headerVisualIndexAt(int position) const;
};

#endif // QT_NO_ITEMVIEWS

#endif // QHEADERVIEW_P_H
