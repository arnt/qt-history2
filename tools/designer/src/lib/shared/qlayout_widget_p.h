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
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QLAYOUT_WIDGET_H
#define QLAYOUT_WIDGET_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerLayoutDecorationExtension>

#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtGui/QWidget>
#include <QtGui/QLayout>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QGridLayout;

namespace qdesigner_internal {
// ---- LayoutProperties: Helper struct that stores all layout-relevant properties
//      with functions to retrieve and apply to property sheets. Can be used to store the state
//      for undo commands and while rebuilding layouts.
struct QDESIGNER_SHARED_EXPORT LayoutProperties
{
    LayoutProperties();
    void clear();

    enum Margins { LeftMargin, TopMargin, RightMargin, BottomMargin, MarginCount };
    enum Spacings { Spacing, HorizSpacing, VertSpacing, SpacingsCount };

    // Property names
    static const QVector<QString> &marginPropertyNames();
    static const QVector<QString> &spacingProperyNames();

    enum PropertyMask {
        ObjectNameProperty  = 0x1,
        LeftMarginProperty = 0x2, TopMarginProperty = 0x4, RightMarginProperty = 0x8, BottomMarginProperty = 0x10,
        SpacingProperty = 0x20, HorizSpacingProperty = 0x40, VertSpacingProperty = 0x80,
        SizeConstraintProperty = 0x100,
        AllProperties = 0xFFFF};

    // return a PropertyMask of visible properties
    static int visibleProperties(const  QLayout *layout);

    // Retrieve from /apply to sheet: A property mask is returned indicating the properties found in the sheet
    int fromPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask = AllProperties);
    int toPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask = AllProperties, bool applyChanged = true) const;

    int m_margins[MarginCount];
    bool m_marginsChanged[MarginCount];

    int m_spacings[SpacingsCount];
    bool m_spacingsChanged[SpacingsCount];

    QString m_objectName;
    bool m_objectNameChanged;
    QVariant m_sizeConstraint;
    bool m_sizeConstraintChanged;
};

// -- LayoutHelper: For use with the 'insert widget'/'delete widget' command,
//    able to store and restore states.
//    This could become part of 'QDesignerLayoutDecorationExtensionV2',
//    but to keep any existing old extensions working, it is provided as
//    separate class with factory function.
class LayoutHelper {
protected:
    LayoutHelper();

public:
    virtual ~LayoutHelper();

    static LayoutHelper *createLayoutHelper(int type);

    static int indexOf(const QLayout *lt, const QWidget *widget);

    // Return area of an item (x == columns)
    QRect itemInfo(QLayout *lt, const QWidget *widget) const;

    virtual QRect itemInfo(QLayout *lt, int index) const = 0;
    virtual void insertWidget(QLayout *lt, const QRect &info, QWidget *w) = 0;
    virtual void removeWidget(QLayout *lt, QWidget *widget) = 0;

    // Simplify a grid, remove empty columns, rows within the rectangle
    virtual bool canSimplify(const QWidget *widgetWithManagedLayout, const QRect &restrictionArea) const = 0;
    virtual void simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout, const QRect &restrictionArea) = 0;

    // Push and pop a state. Can be used for implementing undo for
    // simplify/row, column insertion commands, provided that
    // the widgets remain the same.
    virtual void pushState(const QWidget *widgetWithManagedLayout)  = 0;
    virtual void popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout) = 0;
};

// Base class for layout decoration extensions.
class QDESIGNER_SHARED_EXPORT QLayoutSupport: public QObject, public QDesignerLayoutDecorationExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerLayoutDecorationExtension)

protected:
    QLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent = 0);

public:
    virtual ~QLayoutSupport();

    inline QDesignerFormWindowInterface *formWindow() const   { return m_formWindow; }

    // DecorationExtension V2
    LayoutHelper* helper() const                              { return m_helper; }

    // DecorationExtension
    virtual int currentIndex() const                          { return m_currentIndex; }

    virtual InsertMode currentInsertMode() const              { return m_currentInsertMode; }

    virtual QPair<int, int> currentCell() const               { return m_currentCell; }

    virtual int findItemAt(const QPoint &pos) const;
    virtual int indexOf(QWidget *widget) const;
    virtual int indexOf(QLayoutItem *item) const;

    virtual void adjustIndicator(const QPoint &pos, int index);

    virtual QList<QWidget*> widgets(QLayout *layout) const;

    // Pad empty cells with dummy spacers. Called by layouting commands.
    static void createEmptyCells(QGridLayout *gridLayout);

    // remove dummy spacers in the area. Returns false if there are non-empty items in the way
    static bool removeEmptyCells(QGridLayout *gridLayout, const QRect &area);

    // grid helpers: find item index
    static int findItemAt(QGridLayout *, int row, int column);
    // grid helpers: Quick check whether simplify should be enabled for grids. May return false positives.
    static bool canSimplifyQuickCheck(const QGridLayout *);
    // Factory function, create layout support according to layout type of widget
    static QLayoutSupport *createLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent = 0);

protected:
    // figure out insertion position and mode from indicator on empty cell if supported
    virtual void setCurrentCellFromIndicatorOnEmptyCell(int index) = 0;
    // figure out insertion position and mode from indicator
    virtual void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment) = 0;

    // Overwrite to return the extended geometry of an item, that is,
    // if it is a border item, include the widget border for the indicator to work correctly
    virtual QRect extendedGeometry(int index) const = 0;
    virtual bool supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const = 0;

    QRect itemInfo(int index) const;
    inline QLayout *layout() const       { return m_widget->layout(); }
    QGridLayout *gridLayout() const;
    QWidget *widget() const              { return m_widget; }

    void setInsertMode(InsertMode im);
    void setCurrentCell(const QPair<int, int> &cell);

private:
    enum Indicator { LeftIndicator, TopIndicator, RightIndicator, BottomIndicator, NumIndicators };

    void hideIndicator(Indicator i);
    void showIndicator(Indicator i, const QRect &geometry, const QPalette &);

    QDesignerFormWindowInterface *m_formWindow;
    LayoutHelper* m_helper;

    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_indicators[NumIndicators];
    int m_currentIndex;
    InsertMode m_currentInsertMode;
    QPair<int, int> m_currentCell;
};
} // namespace qdesigner_internal

// Red layout widget.
class QDESIGNER_SHARED_EXPORT QLayoutWidget: public QWidget
{
    Q_OBJECT
public:
    explicit QLayoutWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent = 0);

    int layoutLeftMargin() const;
    void setLayoutLeftMargin(int layoutMargin);

    int layoutTopMargin() const;
    void setLayoutTopMargin(int layoutMargin);

    int layoutRightMargin() const;
    void setLayoutRightMargin(int layoutMargin);

    int layoutBottomMargin() const;
    void setLayoutBottomMargin(int layoutMargin);

    inline QDesignerFormWindowInterface *formWindow() const    { return m_formWindow; }

protected:
    virtual bool event(QEvent *e);
    virtual void paintEvent(QPaintEvent *e);

private:
    QDesignerFormWindowInterface *m_formWindow;
    int m_leftMargin;
    int m_topMargin;
    int m_rightMargin;
    int m_bottomMargin;
};

#endif // QDESIGNER_WIDGET_H
