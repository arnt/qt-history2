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

#ifndef LAYOUT_H
#define LAYOUT_H

#include "shared_global_p.h"
#include "layoutinfo_p.h"

#include <QtCore/QPointer>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QHash>

#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class QDESIGNER_SHARED_EXPORT Layout : public QObject
{
    Q_OBJECT

protected:
    Layout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, LayoutInfo::Type layoutType);

public:
    static  Layout* createLayout(const QWidgetList &widgets,  QWidget *parentWidget,
                                 QDesignerFormWindowInterface *fw,
                                 QWidget *layoutBase, LayoutInfo::Type layoutType);

    virtual ~Layout();

    virtual void sort() = 0;
    virtual void doLayout() = 0;

    virtual void setup();
    virtual void undoLayout();
    virtual void breakLayout();

    const QWidgetList &widgets() const { return m_widgets; }
    QWidget *parentWidget() const      { return m_parentWidget; }
    QWidget *layoutBaseWidget() const  { return m_layoutBase; }

protected:
    virtual void finishLayout(bool needMove, QLayout *layout = 0);
    virtual bool prepareLayout(bool &needMove, bool &needReparent);

    void setWidgets(const  QWidgetList &widgets) { m_widgets = widgets; }
    QLayout *createLayout(int type);
    void reparentToLayoutBase(QWidget *w);

private slots:
    void widgetDestroyed();

private:
    Layout(const  Layout &);
    Layout &operator=(const  Layout &);

    QWidgetList m_widgets;
    QWidget *m_parentWidget;
    typedef QHash<QWidget *, QRect> WidgetGeometryHash;
    WidgetGeometryHash m_geometries;
    QWidget *m_layoutBase;
    QDesignerFormWindowInterface *m_formWindow;
    const LayoutInfo::Type m_layoutType;
    QPoint m_startPoint;
    QRect m_oldGeometry;
    const bool m_isBreak;
};

namespace Utils
{

inline int indexOfWidget(QLayout *layout, QWidget *widget)
{
    int index = 0;
    while (QLayoutItem *item = layout->itemAt(index)) {
        if (item->widget() == widget)
            return index;

        ++index;
    }

    return -1;
}

} // namespace Utils

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LAYOUT_H
