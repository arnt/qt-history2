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

#include "qmainwindowlayout_p.h"
#include "qdockarealayout_p.h"

#ifndef QT_NO_MAINWINDOW
#include "qdockwidget.h"
#include "qdockwidget_p.h"
#include "qtoolbar_p.h"
#include "qmainwindow.h"
#include "qmainwindowlayout_p.h"
#include "qtoolbar.h"
#include "qtoolbarlayout_p.h"
#include "qwidgetanimator_p.h"
#include "qrubberband.h"
#include "qdockwidget_p.h"
#include "qtabbar_p.h"

#include <qapplication.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstyle.h>
#include <qvarlengtharray.h>
#include <qstack.h>
#include <qmap.h>
#include <qtimer.h>

#include <qdebug.h>

#include <private/qlayoutengine_p.h>
#ifdef Q_WS_MAC
#include <private/qcore_mac_p.h>
#endif

#if 0

/******************************************************************************
** debug
*/

#include <stdio.h>
#include <QTextStream>
static QTextStream qout(stderr, QIODevice::WriteOnly);

#ifndef QT_NO_DOCKWIDGET
void dumpLayout(const QDockAreaLayoutInfo &layout, QString indent);

void dumpLayout(const QDockAreaLayoutItem &item, QString indent)
{
    qout << indent << "QDockAreaLayoutItem: "
            << "pos: " << item.pos << " size:" << item.size
            << " gap:" << item.gap << '\n';
    if (item.widgetItem != 0) {
        qout << indent << "widget: "
            << item.widgetItem->widget()->metaObject()->className()
            << ' ' << item.widgetItem->widget()->windowTitle() << '\n';
    } else if (item.subinfo != 0) {
        qout << indent << "subinfo:\n";
        dumpLayout(*item.subinfo, indent + QLatin1String("  "));
    }
    qout.flush();
}

void dumpLayout(const QDockAreaLayoutInfo &layout, QString indent)
{
    qout << indent << "QDockAreaLayoutInfo: "
            << layout.rect.left() << ','
            << layout.rect.top() << ' '
            << layout.rect.width() << 'x'
            << layout.rect.height()
            << " orient:" << layout.o
            << " tabbed:" << layout.tabbed
            << " tbshape:" << layout.tabBarShape << '\n';

    for (int i = 0; i < layout.item_list.count(); ++i) {
        qout << indent << "Item: " << i << '\n';
        dumpLayout(layout.item_list.at(i), indent + QLatin1String("  "));
    }
    qout.flush();
};

void dumpLayout(const QDockAreaLayout &layout, QString indent)
{
    qout << indent << "QDockAreaLayout: "
            << layout.rect.left() << ','
            << layout.rect.top() << ' '
            << layout.rect.width() << 'x'
            << layout.rect.height() << '\n';

    for (int i = 0; i < QInternal::DockCount; ++i) {
        qout << indent << "Dock area: " << i << '\n';
        dumpLayout(layout.docks[i], indent + QLatin1String("  "));
    }
    qout.flush();
};
#endif // QT_NO_DOCKWIDGET

#endif // 0


/******************************************************************************
** QMainWindowLayoutState
*/

// we deal with all the #ifndefferry here so QMainWindowLayout code is clean

QMainWindowLayoutState::QMainWindowLayoutState(QMainWindow *win)
    :
#ifndef QT_NO_TOOLBAR
    toolBarAreaLayout(win),
#endif
#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout(win)
#else
    centralWidgetItem(0)
#endif

{
    mainWindow = win;
}

QSize QMainWindowLayoutState::sizeHint() const
{

    QSize result(0, 0);

#ifndef QT_NO_DOCKWIDGET
    result = dockAreaLayout.sizeHint();
#else
    if (centralWidgetItem != 0)
        result = centralWidgetItem->sizeHint();
#endif

#ifndef QT_NO_TOOLBAR
    result = toolBarAreaLayout.sizeHint(result);
#endif // QT_NO_TOOLBAR

    return result;
}

QSize QMainWindowLayoutState::minimumSize() const
{
    QSize result(0, 0);

#ifndef QT_NO_DOCKWIDGET
    result = dockAreaLayout.minimumSize();
#else
    if (centralWidgetItem != 0)
        result = centralWidgetItem->minimumSize();
#endif

#ifndef QT_NO_TOOLBAR
    result = toolBarAreaLayout.minimumSize(result);
#endif // QT_NO_TOOLBAR

    return result;
}

void QMainWindowLayoutState::apply(bool animated)
{
#ifndef QT_NO_TOOLBAR
    toolBarAreaLayout.apply(animated);
#endif

#ifndef QT_NO_DOCKWIDGET
//    dumpLayout(dockAreaLayout, QString());
    dockAreaLayout.apply(animated);
#else
    if (centralWidgetItem != 0) {
        QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(mainWindow->layout());
        Q_ASSERT(layout != 0);
        layout->widgetAnimator->animate(centralWidgetItem->widget(), centralWidgetRect, animated);
    }
#endif
}

void QMainWindowLayoutState::fitLayout()
{
    QRect r;
#ifdef QT_NO_TOOLBAR
    r = rect;
#else
    toolBarAreaLayout.rect = rect;
    r = toolBarAreaLayout.fitLayout();
#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout.rect = r;
    dockAreaLayout.fitLayout();
#else
    centralWidgetRect = r;
#endif
}

void QMainWindowLayoutState::deleteAllLayoutItems()
{
#ifndef QT_NO_TOOLBAR
    toolBarAreaLayout.deleteAllLayoutItems();
#endif

#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout.deleteAllLayoutItems();
#endif
}

void QMainWindowLayoutState::deleteCentralWidgetItem()
{
#ifndef QT_NO_DOCKWIDGET
    delete dockAreaLayout.centralWidgetItem;
    dockAreaLayout.centralWidgetItem = 0;
#else
    delete centralWidgetItem;
    centralWidgetItem = 0;
#endif
}

QLayoutItem *QMainWindowLayoutState::itemAt(int index, int *x) const
{
#ifndef QT_NO_TOOLBAR
    if (QLayoutItem *ret = toolBarAreaLayout.itemAt(x, index))
        return ret;
#endif

#ifndef QT_NO_DOCKWIDGET
    if (QLayoutItem *ret = dockAreaLayout.itemAt(x, index))
        return ret;
#else
    if (centralWidgetItem != 0 && (*x)++ == index)
        return centralWidgetItem;
#endif

    return 0;
}

QLayoutItem *QMainWindowLayoutState::takeAt(int index, int *x)
{
#ifndef QT_NO_TOOLBAR
    if (QLayoutItem *ret = toolBarAreaLayout.takeAt(x, index))
        return ret;
#endif

#ifndef QT_NO_DOCKWIDGET
    if (QLayoutItem *ret = dockAreaLayout.takeAt(x, index))
        return ret;
#else
    if (centralWidgetItem != 0 && (*x)++ == index) {
        QLayoutItem *ret = centralWidgetItem;
        centralWidgetItem = 0;
        return ret;
    }
#endif

    return 0;
}

QList<int> QMainWindowLayoutState::indexOf(QWidget *widget) const
{
    QList<int> result;

#ifndef QT_NO_TOOLBAR
    // is it a toolbar?
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        result = toolBarAreaLayout.indexOf(toolBar);
        if (!result.isEmpty())
            result.prepend(0);
        return result;
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    // is it a dock widget?
    if (QDockWidget *dockWidget = qobject_cast<QDockWidget *>(widget)) {
        result = dockAreaLayout.indexOf(dockWidget);
        if (!result.isEmpty())
            result.prepend(1);
        return result;
    }
#endif //QT_NO_DOCKWIDGET

    return result;
}

bool QMainWindowLayoutState::contains(QWidget *widget) const
{
#ifndef QT_NO_DOCKWIDGET
    if (dockAreaLayout.centralWidgetItem != 0 && dockAreaLayout.centralWidgetItem->widget() == widget)
        return true;
    if (!dockAreaLayout.indexOf(widget).isEmpty())
        return true;
#else
    if (centralWidgetItem != 0 && centralWidgetItem->widget() == widget)
        return true;
#endif

#ifndef QT_NO_TOOLBAR
    if (!toolBarAreaLayout.indexOf(widget).isEmpty())
        return true;
#endif
    return false;
}

void QMainWindowLayoutState::setCentralWidget(QWidget *widget)
{
    QLayoutItem *item = 0;
    if (widget != 0)
        item = new QWidgetItem(widget);

#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout.centralWidgetItem = item;
#else
    centralWidgetItem = item;
#endif
}

QWidget *QMainWindowLayoutState::centralWidget() const
{
    QLayoutItem *item = 0;

#ifndef QT_NO_DOCKWIDGET
    item = dockAreaLayout.centralWidgetItem;
#else
    item = centralWidgetItem;
#endif

    if (item != 0)
        return item->widget();
    return 0;
}

QList<int> QMainWindowLayoutState::gapIndex(QWidget *widget,
                                            const QPoint &pos) const
{
    QList<int> result;

#ifndef QT_NO_TOOLBAR
    // is it a toolbar?
    if (qobject_cast<QToolBar*>(widget) != 0) {
        result = toolBarAreaLayout.gapIndex(pos);
        if (!result.isEmpty())
            result.prepend(0);
        return result;
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    // is it a dock widget?
    if (qobject_cast<QDockWidget *>(widget) != 0) {
        result = dockAreaLayout.gapIndex(pos);
        if (!result.isEmpty())
            result.prepend(1);
        return result;
    }
#endif //QT_NO_DOCKWIDGET

    return result;
}

bool QMainWindowLayoutState::insertGap(QList<int> path, QLayoutItem *item)
{
    if (path.isEmpty())
        return false;

    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0) {
        Q_ASSERT(qobject_cast<QToolBar*>(item->widget()) != 0);
        return toolBarAreaLayout.insertGap(path, item);
    }
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1) {
        Q_ASSERT(qobject_cast<QDockWidget*>(item->widget()) != 0);
        return dockAreaLayout.insertGap(path, item);
    }
#endif //QT_NO_DOCKWIDGET

    return false;
}

void QMainWindowLayoutState::remove(QList<int> path)
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        toolBarAreaLayout.remove(path);
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        dockAreaLayout.remove(path);
#endif //QT_NO_DOCKWIDGET
}

void QMainWindowLayoutState::clear()
{
#ifndef QT_NO_TOOLBAR
    toolBarAreaLayout.clear();
#endif

#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout.clear();
#else
    centralWidgetRect = QRect(0, 0, -1, -1);
#endif

    rect = QRect(0, 0, -1, -1);
}

bool QMainWindowLayoutState::isValid() const
{
    return rect.isValid();
}

QLayoutItem *QMainWindowLayoutState::item(QList<int> path)
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        return toolBarAreaLayout.item(path).widgetItem;
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        return dockAreaLayout.item(path).widgetItem;
#endif //QT_NO_DOCKWIDGET

    return 0;
}

QRect QMainWindowLayoutState::itemRect(QList<int> path) const
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        return toolBarAreaLayout.itemRect(path);
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        return dockAreaLayout.itemRect(path);
#endif //QT_NO_DOCKWIDGET

    return QRect();
}

QRect QMainWindowLayoutState::gapRect(QList<int> path) const
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        return toolBarAreaLayout.itemRect(path);
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        return dockAreaLayout.gapRect(path);
#endif //QT_NO_DOCKWIDGET

    return QRect();
}

QLayoutItem *QMainWindowLayoutState::plug(QList<int> path)
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        return toolBarAreaLayout.plug(path);
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        return dockAreaLayout.plug(path);
#endif //QT_NO_DOCKWIDGET

    return 0;
}

QLayoutItem *QMainWindowLayoutState::unplug(QList<int> path)
{
    int i = path.takeFirst();

#ifndef QT_NO_TOOLBAR
    if (i == 0)
        return toolBarAreaLayout.unplug(path);
#endif

#ifndef QT_NO_DOCKWIDGET
    if (i == 1)
        return dockAreaLayout.unplug(path);
#endif //QT_NO_DOCKWIDGET

    return 0;
}

void QMainWindowLayoutState::saveState(QDataStream &stream) const
{
#ifndef QT_NO_DOCKWIDGET
    dockAreaLayout.saveState(stream);
#endif
#ifndef QT_NO_TOOLBAR
    toolBarAreaLayout.saveState(stream);
#endif
}

template <typename T>
static QList<T> findChildren(const QObject *o)
{
    const QObjectList &list = o->children();
    QList<T> result;

    for (int i=0; i < list.size(); ++i) {
        if (T t = qobject_cast<T>(list[i])) {
            result.append(t);
        }
    }

    return result;
}


bool QMainWindowLayoutState::restoreState(QDataStream &stream,
                                        const QMainWindowLayoutState &oldState)
{
#ifndef QT_NO_DOCKWIDGET
    QList<QDockWidget *> dockWidgets = ::findChildren<QDockWidget*>(mainWindow);
    if (!dockAreaLayout.restoreState(stream, dockWidgets))
        return false;

    for (int i = 0; i < dockWidgets.size(); ++i) {
        QDockWidget *w = dockWidgets.at(i);
        QList<int> path = dockAreaLayout.indexOf(w);
        if (path.isEmpty()) {
            QList<int> oldPath = oldState.dockAreaLayout.indexOf(w);
            if (oldPath.isEmpty()) {
                continue;
            }
            QDockAreaLayoutInfo *info = dockAreaLayout.info(oldPath);
            if (info == 0) {
                continue;
            }
            info->item_list.append(QDockAreaLayoutItem(new QDockWidgetItem(w)));
        }
    }
#endif

#ifndef QT_NO_TOOLBAR
    QList<QToolBar *> toolBars = ::findChildren<QToolBar*>(mainWindow);
    if (!toolBarAreaLayout.restoreState(stream, toolBars))
        return false;

    for (int i = 0; i < toolBars.size(); ++i) {
        QToolBar *w = toolBars.at(i);
        QList<int> path = toolBarAreaLayout.indexOf(w);
        if (path.isEmpty()) {
            QList<int> oldPath = oldState.toolBarAreaLayout.indexOf(w);
            if (oldPath.isEmpty()) {
                continue;
            }
            toolBarAreaLayout.docks[oldPath.at(0)].insertToolBar(0, w);
        }
    }
#endif // QT_NO_TOOLBAR

    return true;
}

/******************************************************************************
** QMainWindowLayoutState - toolbars
*/

#ifndef QT_NO_TOOLBAR

static inline void validateToolBarArea(Qt::ToolBarArea &area)
{
    switch (area) {
    case Qt::LeftToolBarArea:
    case Qt::RightToolBarArea:
    case Qt::TopToolBarArea:
    case Qt::BottomToolBarArea:
        break;
    default:
        area = Qt::TopToolBarArea;
    }
}

static QInternal::DockPosition toDockPos(Qt::ToolBarArea area)
{
    switch (area) {
        case Qt::LeftToolBarArea: return QInternal::LeftDock;
        case Qt::RightToolBarArea: return QInternal::RightDock;
        case Qt::TopToolBarArea: return QInternal::TopDock;
        case Qt::BottomToolBarArea: return QInternal::BottomDock;
        default:
            break;
    }

    return QInternal::DockCount;
}

static Qt::ToolBarArea toToolBarArea(QInternal::DockPosition pos)
{
    switch (pos) {
        case QInternal::LeftDock:   return Qt::LeftToolBarArea;
        case QInternal::RightDock:  return Qt::RightToolBarArea;
        case QInternal::TopDock:    return Qt::TopToolBarArea;
        case QInternal::BottomDock: return Qt::BottomToolBarArea;
        default: break;
    }
    return Qt::NoToolBarArea;
}

static inline Qt::ToolBarArea toToolBarArea(int pos)
{
    return toToolBarArea(static_cast<QInternal::DockPosition>(pos));
}

void QMainWindowLayout::addToolBarBreak(Qt::ToolBarArea area)
{
    validateToolBarArea(area);

    layoutState.toolBarAreaLayout.addToolBarBreak(toDockPos(area));
    invalidate();
}

void QMainWindowLayout::insertToolBarBreak(QToolBar *before)
{
    layoutState.toolBarAreaLayout.insertToolBarBreak(before);
    invalidate();
}

void QMainWindowLayout::removeToolBarBreak(QToolBar *before)
{
    layoutState.toolBarAreaLayout.removeToolBarBreak(before);
    invalidate();
}

#ifdef Q_WS_MAC

void QMainWindowLayout::updateHIToolBarStatus()
{
#ifndef kWindowUnifiedTitleAndToolbarAttribute
#define kWindowUnifiedTitleAndToolbarAttribute (1 << 7)
#endif
    bool useHIToolbar = layoutState.mainWindow->unifiedTitleAndToolBarOnMac();
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        if (useHIToolbar) {
            ChangeWindowAttributes(qt_mac_window_for(layoutState.mainWindow),
                                   kWindowUnifiedTitleAndToolbarAttribute, 0);
        } else {
            ChangeWindowAttributes(qt_mac_window_for(layoutState.mainWindow),
                                   0, kWindowUnifiedTitleAndToolbarAttribute);
        }
    }

    layoutState.mainWindow->setUpdatesEnabled(false);  // reduces a little bit of flicker, not all though
    if (!useHIToolbar) {
        ShowHideWindowToolbar(qt_mac_window_for(parentWidget()), false, false);
        // Move everything out of the HIToolbar into the main toolbar.
        while (!qtoolbarsInHIToolbarList.isEmpty()) {
            // Should shrink the list by one every time.
            layoutState.mainWindow->addToolBar(Qt::TopToolBarArea, qtoolbarsInHIToolbarList.first());
        }
        SetWindowToolbar(qt_mac_window_for(parentWidget()), 0);
    } else {
        QList<QToolBar *> toolbars = layoutState.mainWindow->findChildren<QToolBar *>();
        for (int i = 0; i < toolbars.size(); ++i) {
            QToolBar *toolbar = toolbars.at(i);
            if (toolBarArea(toolbar) == Qt::TopToolBarArea) {
                removeWidget(toolbar);  // Do this here, because we are in an in-between state.
                layoutState.mainWindow->addToolBar(Qt::TopToolBarArea, toolbar);
            }
        }
    }
    layoutState.mainWindow->setUpdatesEnabled(true);
}

static const int kEventParamQToolBar = 'QTBR';
static const int kEventParamQMainWindowLayout = 'QMWL';

#define kQToolBarHIToolbarItemClassID CFSTR("com.trolltech.qt.qmainwindow.qtoolbarInHIToolbar")

const EventTypeSpec qtoolbarEvents[] =
{
    { kEventClassHIObject, kEventHIObjectConstruct },
    { kEventClassHIObject, kEventHIObjectDestruct },
    { kEventClassHIObject, kEventHIObjectInitialize },
    { kEventClassToolbarItem, kEventToolbarItemCreateCustomView }
};

struct QToolBarInHIToolbarInfo
{
    QToolBarInHIToolbarInfo(HIToolbarItemRef item)
        : toolbarItem(item), mainWindowLayout(0)
    {}
    HIToolbarItemRef toolbarItem;
    QMainWindowLayout *mainWindowLayout;
};

OSStatus QMainWindowLayout::qtoolbarInHIToolbarHandler(EventHandlerCallRef inCallRef,
                                                       EventRef event, void *data)
{
    OSStatus result = eventNotHandledErr;
    QToolBarInHIToolbarInfo *object = static_cast<QToolBarInHIToolbarInfo *>(data);

    switch (GetEventClass(event)) {
        case kEventClassHIObject:
            switch (GetEventKind(event)) {
                case kEventHIObjectConstruct:
                    {
                        HIObjectRef toolbarItem;
                        GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef,
                                          0, sizeof( HIObjectRef ), 0, &toolbarItem);

                        QToolBarInHIToolbarInfo *item = new QToolBarInHIToolbarInfo(toolbarItem);
                        SetEventParameter(event, kEventParamHIObjectInstance, typeVoidPtr,
                                          sizeof(void *), &item);
                        result = noErr;
                    }
                    break;
                case kEventHIObjectInitialize:
                    result = CallNextEventHandler(inCallRef, event);
                    if (result == noErr) {
                        QToolBar *toolbar = 0;
                        QMainWindowLayout *layout = 0;
                        GetEventParameter(event, kEventParamQToolBar, typeVoidPtr,
                                          0, sizeof(void *), 0, &toolbar);
                        GetEventParameter(event, kEventParamQMainWindowLayout, typeVoidPtr,
                                          0, sizeof(void *), 0, &layout);
                        object->mainWindowLayout = layout;
                        object->mainWindowLayout->hitoolbarHash.insert(object->toolbarItem, toolbar);
                        HIToolbarItemChangeAttributes(object->toolbarItem,
                                                      kHIToolbarItemLabelDisabled, 0);
                    }
                    break;

                case kEventHIObjectDestruct:
                    delete object;
                    result = noErr;
                    break;
            }
            break;

        case kEventClassToolbarItem:
            switch (GetEventKind(event))
            {
                case kEventToolbarItemCreateCustomView:
                    {
                        QToolBar *toolbar
                                = object->mainWindowLayout->hitoolbarHash.value(object->toolbarItem);
                        if (toolbar) {
                            HIViewRef hiview = HIViewRef(toolbar->winId());
                            SetEventParameter(event, kEventParamControlRef, typeControlRef,
                                              sizeof(HIViewRef), &hiview);
                            result = noErr;
                        }
                    }
                    break;
            }
            break;
    }

    return result;
}

void QMainWindowLayout::qtMacHIToolbarRegisterQToolBarInHIToolborItemClass()
{
    static bool registered = false;

    if (!registered) {
        HIObjectRegisterSubclass( kQToolBarHIToolbarItemClassID,
                kHIToolbarItemClassID, 0, QMainWindowLayout::qtoolbarInHIToolbarHandler,
                GetEventTypeCount(qtoolbarEvents), qtoolbarEvents, 0, 0 );
        registered = true;
    }
}

static void GetToolbarAllowedItems(CFMutableArrayRef array)
{
    CFArrayAppendValue(array, CFSTR("com.trolltech.qt.hitoolbar-qtoolbar"));
}

HIToolbarItemRef QMainWindowLayout::createQToolBarInHIToolbarItem(QToolBar *toolbar,
                                                                  QMainWindowLayout *layout)
{
    QMainWindowLayout::qtMacHIToolbarRegisterQToolBarInHIToolborItemClass();

    EventRef event;
    HIToolbarItemRef result = 0;

    CFStringRef identifier = CFSTR("com.trolltech.qt.hitoolbar-qtoolbar");
    UInt32 options = kHIToolbarItemAllowDuplicates;

    CreateEvent(0, kEventClassHIObject, kEventHIObjectInitialize,
                GetCurrentEventTime(), 0, &event);
    SetEventParameter(event, kEventParamToolbarItemIdentifier, typeCFStringRef,
                      sizeof(CFStringRef), &identifier);
    SetEventParameter(event, kEventParamAttributes, typeUInt32, sizeof(UInt32), &options);
    SetEventParameter(event, kEventParamQToolBar, typeVoidPtr, sizeof(void *), &toolbar);
    SetEventParameter(event, kEventParamQMainWindowLayout, typeVoidPtr, sizeof(void *), &layout);

    HIObjectCreate(kQToolBarHIToolbarItemClassID, event,
                                  static_cast<HIObjectRef *>(&result));

    ReleaseEvent(event);
    return result;

}

HIToolbarItemRef QMainWindowLayout::CreateToolbarItemForIdentifier(CFStringRef identifier,
                                                                   CFTypeRef data)
{
    HIToolbarItemRef item = 0;
    if (CFStringCompare(CFSTR("com.trolltech.qt.hitoolbar-qtoolbar"), identifier,
                              kCFCompareBackwards) == kCFCompareEqualTo) {
        if (data && CFGetTypeID(data) == CFArrayGetTypeID()) {
            CFArrayRef array = static_cast<CFArrayRef>(data);
            QToolBar *toolbar = static_cast<QToolBar *>(const_cast<void *>(CFArrayGetValueAtIndex(array, 0)));
            QMainWindowLayout *layout = static_cast<QMainWindowLayout *>(const_cast<void *>(CFArrayGetValueAtIndex(array, 1)));
            item = createQToolBarInHIToolbarItem(toolbar, layout);
        }
    }
    return item;
}

static const EventTypeSpec kToolbarEvents[] = {
    { kEventClassToolbar, kEventToolbarGetDefaultIdentifiers },
    { kEventClassToolbar, kEventToolbarGetAllowedIdentifiers },
    { kEventClassToolbar, kEventToolbarCreateItemWithIdentifier },
    { kEventClassToolbar, kEventToolbarItemAdded },
    { kEventClassToolbar, kEventToolbarItemRemoved }
};

OSStatus QMainWindowLayout::qtmacToolbarDelegate(EventHandlerCallRef, EventRef event, void *data)
{
    QMainWindowLayout *mainWindowLayout = static_cast<QMainWindowLayout *>(data);
    OSStatus            result = eventNotHandledErr;
    CFMutableArrayRef   array;
    CFStringRef         identifier;
    switch (GetEventKind(event)) {
    case kEventToolbarGetDefaultIdentifiers:
    case kEventToolbarGetAllowedIdentifiers:
        GetEventParameter(event, kEventParamMutableArray, typeCFMutableArrayRef, 0,
                          sizeof(CFMutableArrayRef), 0, &array);
        GetToolbarAllowedItems(array);
        result = noErr;
        break;
    case kEventToolbarCreateItemWithIdentifier: {
        HIToolbarItemRef item;
        CFTypeRef data = 0;
        OSStatus err = GetEventParameter(event, kEventParamToolbarItemIdentifier, typeCFStringRef,
                          0, sizeof(CFStringRef), 0, &identifier);
        err = GetEventParameter(event, kEventParamToolbarItemConfigData, typeCFTypeRef,
                          0, sizeof(CFTypeRef), 0, &data);
        item = CreateToolbarItemForIdentifier(identifier, data);
        if (item) {
            result = SetEventParameter(event, kEventParamToolbarItem, typeHIToolbarItemRef,
                              sizeof(HIToolbarItemRef), &item );
        }
        break;
    }
    case kEventToolbarItemAdded: {
        // Double check that our "view" of the toolbar is similar.
        HIToolbarItemRef item;
        CFIndex index;
        if (GetEventParameter(event, kEventParamToolbarItem, typeHIToolbarItemRef,
                              0, sizeof(HIToolbarItemRef), 0, &item) == noErr
            && GetEventParameter(event, kEventParamIndex, typeCFIndex, 0,
                                 sizeof(CFIndex), 0, &index) == noErr) {
            CFRetain(item); // We will watch this until it's removed from the list (or bust).
            mainWindowLayout->toolbarItemsCopy.insert(index, item);
            QToolBar *toolbar = mainWindowLayout->hitoolbarHash.value(item);
            if (toolbar) {
                int toolbarIndex = mainWindowLayout->qtoolbarsInHIToolbarList.indexOf(toolbar);
                if (index != toolbarIndex) {
                    // Dang, we must be out of sync, rebuild it from the "toolbarItemsCopy"
                    mainWindowLayout->qtoolbarsInHIToolbarList.clear();
                    for (int i = 0; i < mainWindowLayout->toolbarItemsCopy.size(); ++i) {
                        // This will either append the correct toolbar or an
                        // null toolbar. This is fine because this list
                        // is really only kept to make sure that things are but in the right order.
                        mainWindowLayout->qtoolbarsInHIToolbarList.append(
                                mainWindowLayout->hitoolbarHash.value(mainWindowLayout->
                                                                        toolbarItemsCopy.at(i)));
                    }
                }
            }
        }
        break;
    }
    case kEventToolbarItemRemoved: {
        HIToolbarItemRef item;
        if (GetEventParameter(event, kEventParamToolbarItem, typeHIToolbarItemRef,
                              0, sizeof(HIToolbarItemRef), 0, &item) == noErr) {
            mainWindowLayout->hitoolbarHash.remove(item);
            for (int i = 0; i < mainWindowLayout->toolbarItemsCopy.size(); ++i) {
                if (mainWindowLayout->toolbarItemsCopy.at(i) == item) {
                    // I know about it, so release it.
                    mainWindowLayout->toolbarItemsCopy.removeAt(i);
                    mainWindowLayout->qtoolbarsInHIToolbarList.removeAt(i);
                    CFRelease(item);
                    break;
                }
            }
        }
        break;
    }
    }
    return result;
}

void QMainWindowLayout::insertIntoMacHIToolbar(QToolBar *before, QToolBar *toolbar)
{
    if (toolbar == 0)
        return;

    toolbarSaveState.insert(toolbar, ToolBarSaveState(toolbar->isMovable(),
                                                      toolbar->maximumSize()));
    toolbar->setMovable(false);
    toolbar->setMaximumSize(toolbar->sizeHint());
    static_cast<QToolBarLayout *>(toolbar->layout())->setUsePopupMenu(true);
    layoutState.mainWindow->createWinId();
    HIToolbarRef macToolbar;
    WindowRef window = qt_mac_window_for(layoutState.mainWindow);
    if ((GetWindowToolbar(window, &macToolbar) == noErr) && !macToolbar) {
        HIToolbarCreate(CFSTR("com.trolltech.qt.qmainwindow.hitoolbar"),
                        kHIToolbarItemAllowDuplicates, &macToolbar);

        InstallEventHandler(HIObjectGetEventTarget(static_cast<HIToolbarRef>(macToolbar)),
                            QMainWindowLayout::qtmacToolbarDelegate, GetEventTypeCount(kToolbarEvents),
                            kToolbarEvents, this, 0);

        HIToolbarSetDisplaySize(macToolbar, kHIToolbarDisplaySizeNormal);
        HIToolbarSetDisplayMode(macToolbar, kHIToolbarDisplayModeIconOnly);
        SetWindowToolbar(window, macToolbar);
        if (layoutState.mainWindow->isVisible())
            ShowHideWindowToolbar(window, true, false);
        CFRelease(macToolbar);
    }


    int beforeIndex = qtoolbarsInHIToolbarList.indexOf(before);
    if (beforeIndex == -1)
        beforeIndex = qtoolbarsInHIToolbarList.size();

    int toolbarIndex = qtoolbarsInHIToolbarList.indexOf(toolbar);
    if (toolbarIndex != -1) {
        qtoolbarsInHIToolbarList.removeAt(toolbarIndex);
        HIToolbarRemoveItemAtIndex(macToolbar, toolbarIndex);
    }
    toolbar->createWinId();
    qtoolbarsInHIToolbarList.insert(beforeIndex, toolbar);
    QCFType<HIToolbarItemRef> outItem;
    const QObject *stupidArray[] = { toolbar, this };
    QCFType<CFArrayRef> array = CFArrayCreate(0, reinterpret_cast<const void **>(&stupidArray),
                                              2, 0);
    HIToolbarCreateItemWithIdentifier(macToolbar, CFSTR("com.trolltech.qt.hitoolbar-qtoolbar"),
                                      array, &outItem);
    HIToolbarInsertItemAtIndex(macToolbar, outItem, beforeIndex);
}

#endif // Q_WS_MAC


/* Removes the toolbar from the mainwindow so that it can be added again. Does not
   explicitly hide the toolbar. */
void QMainWindowLayout::removeToolBar(QToolBar *toolbar)
{
    if (toolbar) {
        QObject::disconnect(parentWidget(), SIGNAL(iconSizeChanged(QSize)),
                   toolbar, SLOT(_q_updateIconSize(QSize)));
        QObject::disconnect(parentWidget(), SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

#ifdef Q_WS_MAC
        if (usesHIToolBar(toolbar)) {
            QHash<HIToolbarItemRef, QToolBar *>::iterator it = hitoolbarHash.begin();
            while (it != hitoolbarHash.end()) {
                if (it.value() == toolbar) {
                    // Rescue our HIView and set it on the mainWindow again.
                    bool saveVisible = toolbar->isVisible();
                    toolbar->setParent(0);
                    toolbar->setParent(parentWidget());
                    toolbar->setVisible(saveVisible);
                    ToolBarSaveState saveState = toolbarSaveState.value(toolbar);
                    static_cast<QToolBarLayout *>(toolbar->layout())->setUsePopupMenu(false);
                    toolbar->setMovable(saveState.movable);
                    toolbar->setMaximumSize(saveState.maximumSize);
                    toolbarSaveState.remove(toolbar);
                    HIToolbarItemRef item = it.key();
                    HIToolbarRemoveItemAtIndex(HIToolbarItemGetToolbar(item),
                                               toolbarItemsCopy.indexOf(item));
                    break;
                }
                ++it;
            }
        } else
#endif // Q_WS_MAC
        {
            removeWidget(toolbar);
        }
    }
}

/*!
    Adds \a toolbar to \a area, continuing the current line.
*/
void QMainWindowLayout::addToolBar(Qt::ToolBarArea area,
                                   QToolBar *toolbar,
                                   bool)
{
    validateToolBarArea(area);
#ifdef Q_WS_MAC
    if ((area == Qt::TopToolBarArea)
            && layoutState.mainWindow->unifiedTitleAndToolBarOnMac()) {
        insertIntoMacHIToolbar(0, toolbar);
    } else
#endif
    {
        addChildWidget(toolbar);
        layoutState.toolBarAreaLayout.addToolBar(toDockPos(area), toolbar);
        invalidate();
    }
}

/*!
    Adds \a toolbar before \a before
*/
void QMainWindowLayout::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
#ifdef Q_WS_MAC
    if (usesHIToolBar(before)) {
        insertIntoMacHIToolbar(before, toolbar);
    } else
#endif // Q_WS_MAC
    {
        addChildWidget(toolbar);
        layoutState.toolBarAreaLayout.insertToolBar(before, toolbar);
        invalidate();
    }
}

Qt::ToolBarArea QMainWindowLayout::toolBarArea(QToolBar *toolbar) const
{
    QInternal::DockPosition pos = layoutState.toolBarAreaLayout.findToolBar(toolbar);
    switch (pos) {
        case QInternal::LeftDock:   return Qt::LeftToolBarArea;
        case QInternal::RightDock:  return Qt::RightToolBarArea;
        case QInternal::TopDock:    return Qt::TopToolBarArea;
        case QInternal::BottomDock: return Qt::BottomToolBarArea;
        default: break;
    }
#ifdef Q_WS_MAC
    if (pos == QInternal::DockCount) {
        if (qtoolbarsInHIToolbarList.contains(toolbar))
            return Qt::TopToolBarArea;
    }
#endif
    return Qt::NoToolBarArea;
}

bool QMainWindowLayout::toolBarBreak(QToolBar *toolBar) const
{
    return layoutState.toolBarAreaLayout.toolBarBreak(toolBar);
}

void QMainWindowLayout::getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const
{
    option->toolBarArea = toolBarArea(toolBar);
    layoutState.toolBarAreaLayout.getStyleOptionInfo(option, toolBar);
}

void QMainWindowLayout::toggleToolBarsVisible()
{
    layoutState.toolBarAreaLayout.visible = !layoutState.toolBarAreaLayout.visible;
    if (!layoutState.mainWindow->isMaximized()){
        QPoint topLeft = parentWidget()->geometry().topLeft();
        QRect r = parentWidget()->geometry();
        r = layoutState.toolBarAreaLayout.rectHint(r);
        r.moveTo(topLeft);
        parentWidget()->setGeometry(r);
//      widgetAnimator->animate(parentWidget(), r, true);
    } else{
        update();
    }
}

#endif // QT_NO_TOOLBAR

/******************************************************************************
** QMainWindowLayoutState - dock areas
*/

#ifndef QT_NO_DOCKWIDGET

static inline void validateDockWidgetArea(Qt::DockWidgetArea &area)
{
    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        break;
    default:
        area = Qt::LeftDockWidgetArea;
    }
}

static QInternal::DockPosition toDockPos(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::LeftDockWidgetArea: return QInternal::LeftDock;
        case Qt::RightDockWidgetArea: return QInternal::RightDock;
        case Qt::TopDockWidgetArea: return QInternal::TopDock;
        case Qt::BottomDockWidgetArea: return QInternal::BottomDock;
        default:
            break;
    }

    return QInternal::DockCount;
}

static Qt::DockWidgetArea toDockWidgetArea(QInternal::DockPosition pos)
{
    switch (pos) {
        case QInternal::LeftDock : return Qt::LeftDockWidgetArea;
        case QInternal::RightDock : return Qt::RightDockWidgetArea;
        case QInternal::TopDock : return Qt::TopDockWidgetArea;
        case QInternal::BottomDock : return Qt::BottomDockWidgetArea;
        default:
            break;
    }

    return Qt::NoDockWidgetArea;
}

inline static Qt::DockWidgetArea toDockWidgetArea(int pos)
{
    return toDockWidgetArea(static_cast<QInternal::DockPosition>(pos));
}

void QMainWindowLayout::setCorner(Qt::Corner corner, Qt::DockWidgetArea area)
{
    if (layoutState.dockAreaLayout.corners[corner] == area)
        return;
    layoutState.dockAreaLayout.corners[corner] = area;
    invalidate();
}

Qt::DockWidgetArea QMainWindowLayout::corner(Qt::Corner corner) const
{
    return layoutState.dockAreaLayout.corners[corner];
}

void QMainWindowLayout::addDockWidget(Qt::DockWidgetArea area,
                                             QDockWidget *dockwidget,
                                             Qt::Orientation orientation)
{
    addChildWidget(dockwidget);

    // If we are currently moving a separator, then we need to abort the move, since each
    // time we move the mouse layoutState is replaced by savedState modified by the move.
    if (!movingSeparator.isEmpty())
        endSeparatorMove(movingSeparatorPos);

    layoutState.dockAreaLayout.addDockWidget(toDockPos(area), dockwidget, orientation);
    emit dockwidget->dockLocationChanged(area);
    invalidate();
}

void QMainWindowLayout::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
    addChildWidget(second);
    layoutState.dockAreaLayout.tabifyDockWidget(first, second);
    emit second->dockLocationChanged(dockWidgetArea(first));
    invalidate();
}

void QMainWindowLayout::setVerticalTabsEnabled(bool enabled)
{
#ifdef QT_NO_TABBAR
    Q_UNUSED(enabled);
#else
    QDockAreaLayout &layout = layoutState.dockAreaLayout;

    const QTabBar::Shape verticalShapes[] = {
        QTabBar::RoundedWest,
        QTabBar::RoundedEast,
        QTabBar::RoundedNorth,
        QTabBar::RoundedSouth
    };

    for (int i = 0; i < QInternal::DockCount; ++i) {
        layout.docks[i].setTabBarShape(
            enabled ? verticalShapes[i] : QTabBar::RoundedSouth
        );
    }
#endif // QT_NO_TABBAR
}

void QMainWindowLayout::splitDockWidget(QDockWidget *after,
                                               QDockWidget *dockwidget,
                                               Qt::Orientation orientation)
{
    addChildWidget(dockwidget);
    layoutState.dockAreaLayout.splitDockWidget(after, dockwidget, orientation);
    emit dockwidget->dockLocationChanged(dockWidgetArea(after));
    invalidate();
}

Qt::DockWidgetArea QMainWindowLayout::dockWidgetArea(QDockWidget *widget) const
{
    QList<int> pathToWidget = layoutState.dockAreaLayout.indexOf(widget);
    if (pathToWidget.isEmpty())
        return Qt::NoDockWidgetArea;
    return toDockWidgetArea(pathToWidget.first());
}

void QMainWindowLayout::keepSize(QDockWidget *w)
{
    layoutState.dockAreaLayout.keepSize(w);
}

#ifndef QT_NO_TABBAR

class QMainWindowTabBar : public QTabBar
{
public:
    QMainWindowTabBar(QWidget *parent);
protected:
    bool event(QEvent *e);
};

QMainWindowTabBar::QMainWindowTabBar(QWidget *parent)
    : QTabBar(parent)
{
    static_cast<QTabBarPrivate*>(d_ptr)->squeezeTabs = true;
}

bool QMainWindowTabBar::event(QEvent *e)
{
    // show the tooltip if tab is too small to fit label

    if (e->type() != QEvent::ToolTip)
        return QTabBar::event(e);
    QSize size = this->size();
    QSize hint = sizeHint();
    if (shape() == QTabBar::RoundedWest || shape() == QTabBar::RoundedEast) {
        size.transpose();
        hint.transpose();
    }
    if (size.width() < hint.width())
        return QTabBar::event(e);
    e->accept();
    return true;
}

QTabBar *QMainWindowLayout::getTabBar()
{
    QTabBar *result = 0;
    if (!unusedTabBars.isEmpty()) {
        result = unusedTabBars.takeLast();
    } else {
        result = new QMainWindowTabBar(parentWidget());
        result->setShape(QTabBar::RoundedSouth);
        result->setDrawBase(true);
        result->setElideMode(Qt::ElideRight);
        connect(result, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()));
    }

    usedTabBars.insert(result);
    return result;
}

void QMainWindowLayout::tabChanged()
{
    QTabBar *tb = qobject_cast<QTabBar*>(sender());
    if (tb == 0)
        return;
    QDockAreaLayoutInfo *info = layoutState.dockAreaLayout.info(tb);
    if (info == 0)
        return;
    info->apply(false);

    if (QWidget *w = centralWidget())
        w->raise();
}
#endif // QT_NO_TABBAR

bool QMainWindowLayout::startSeparatorMove(const QPoint &pos)
{
    movingSeparator = layoutState.dockAreaLayout.findSeparator(pos);

    if (movingSeparator.isEmpty())
        return false;

    savedState = layoutState;
    movingSeparatorPos = movingSeparatorOrigin = pos;

    return true;
}

bool QMainWindowLayout::separatorMove(const QPoint &pos)
{
    if (movingSeparator.isEmpty())
        return false;
    movingSeparatorPos = pos;
    separatorMoveTimer->start();
    return true;
}

void QMainWindowLayout::doSeparatorMove()
{
    if (movingSeparator.isEmpty())
        return;
    if (movingSeparatorOrigin == movingSeparatorPos)
        return;

    layoutState = savedState;
    layoutState.dockAreaLayout.separatorMove(movingSeparator, movingSeparatorOrigin,
                                                movingSeparatorPos,
                                                &separatorMoveCache);
    movingSeparatorPos = movingSeparatorOrigin;
}

bool QMainWindowLayout::endSeparatorMove(const QPoint&)
{
    bool result = !movingSeparator.isEmpty();
    movingSeparator.clear();
    savedState.clear();
    separatorMoveCache.clear();
    return result;
}

void QMainWindowLayout::raise(QDockWidget *widget)
{
    QDockAreaLayoutInfo *info = layoutState.dockAreaLayout.info(widget);
    if (info == 0)
        return;
#ifndef QT_NO_TABBAR
    if (!info->tabbed)
        return;
    info->setCurrentTab(widget);
#endif
}

#endif // QT_NO_DOCKWIDGET


/******************************************************************************
** QMainWindowLayoutState - layout interface
*/

int QMainWindowLayout::count() const
{
    qWarning("QMainWindowLayout::count: ?");
    return 0; //#################################################
}

QLayoutItem *QMainWindowLayout::itemAt(int index) const
{
    int x = 0;

    if (QLayoutItem *ret = layoutState.itemAt(index, &x))
        return ret;

    if (statusbar && x++ == index)
        return statusbar;

    return 0;
}

QLayoutItem *QMainWindowLayout::takeAt(int index)
{
    int x = 0;

    if (QLayoutItem *ret = layoutState.takeAt(index, &x)) {
        // the widget might in fact have been destroyed by now
        if (QWidget *w = ret->widget()) {
            widgetAnimator->abort(w);
            if (w == pluggingWidget)
                pluggingWidget = 0;
        }

        return ret;
    }

    if (statusbar && x++ == index) {
        QLayoutItem *ret = statusbar;
        statusbar = 0;
        return ret;
    }

    return 0;
}

void QMainWindowLayout::setGeometry(const QRect &_r)
{
    if (savedState.isValid())
        return;

    QRect r = _r;

    QLayout::setGeometry(r);

    if (statusbar) {
        QRect sbr(QPoint(0, 0),
                  QSize(r.width(), statusbar->heightForWidth(r.width()))
                  .expandedTo(statusbar->minimumSize()));
        sbr.moveBottom(r.bottom());
        QRect vr = QStyle::visualRect(QApplication::layoutDirection(), _r, sbr);
        statusbar->setGeometry(vr);
        r.setBottom(sbr.top() - 1);
    }

    layoutState.rect = r;
    layoutState.fitLayout();
    applyState(layoutState, false);
}

void QMainWindowLayout::addItem(QLayoutItem *)
{ qWarning("QMainWindowLayout::addItem: Please use the public QMainWindow API instead"); }

QSize QMainWindowLayout::sizeHint() const
{
    if (!szHint.isValid()) {
        szHint = layoutState.sizeHint();
        const QSize sbHint = statusbar ? statusbar->sizeHint() : QSize(0, 0);
        szHint = QSize(qMax(sbHint.width(), szHint.width()),
                        sbHint.height() + szHint.height());
    }
    return szHint;
}

QSize QMainWindowLayout::minimumSize() const
{
    if (!minSize.isValid()) {
        minSize = layoutState.minimumSize();
        const QSize sbMin = statusbar ? statusbar->minimumSize() : QSize(0, 0);
        minSize = QSize(qMax(sbMin.width(), minSize.width()),
                        sbMin.height() + minSize.height());
    }
    return minSize;
}

void QMainWindowLayout::invalidate()
{
    QLayout::invalidate();
    minSize = szHint = QSize();
}

/******************************************************************************
** QMainWindowLayout - remaining stuff
*/

static void fixToolBarOrientation(QLayoutItem *item, int dockPos)
{
#ifndef QT_NO_TOOLBAR
    QToolBar *toolBar = qobject_cast<QToolBar*>(item->widget());
    if (toolBar == 0)
        return;

    QRect oldGeo = toolBar->geometry();

    QInternal::DockPosition pos
        = static_cast<QInternal::DockPosition>(dockPos);
    Qt::Orientation o = pos == QInternal::TopDock || pos == QInternal::BottomDock
                        ? Qt::Horizontal : Qt::Vertical;
    if (o != toolBar->orientation())
        toolBar->setOrientation(o);

    QSize hint = toolBar->sizeHint().boundedTo(toolBar->maximumSize())
                    .expandedTo(toolBar->minimumSize());

    if (toolBar->size() != hint) {
        QRect newGeo(oldGeo.topLeft(), hint);
        if (toolBar->layoutDirection() == Qt::RightToLeft)
            newGeo.moveRight(oldGeo.right());
        toolBar->setGeometry(newGeo);
    }

#else
    Q_UNUSED(item);
    Q_UNUSED(dockPos);
#endif
}

void QMainWindowLayout::revert(QLayoutItem *widgetItem)
{
    if (!savedState.isValid())
        return;

    QWidget *widget = widgetItem->widget();
    layoutState = savedState;
    currentGapPos = layoutState.indexOf(widget);
    fixToolBarOrientation(widgetItem, currentGapPos.at(1));
    layoutState.unplug(currentGapPos);
    layoutState.fitLayout();
    currentGapRect = layoutState.itemRect(currentGapPos);

    plug(widgetItem);
}

bool QMainWindowLayout::plug(QLayoutItem *widgetItem)
{
    if (currentGapPos.isEmpty())
        return false;

    fixToolBarOrientation(widgetItem, currentGapPos.at(1));

    QWidget *widget = widgetItem->widget();

    QList<int> previousPath = layoutState.indexOf(widget);

    QLayoutItem *it = layoutState.plug(currentGapPos);
    Q_ASSERT(it == widgetItem);
    Q_UNUSED(it);
    if (!previousPath.isEmpty())
        layoutState.remove(previousPath);

    if (dockOptions & QMainWindow::AnimatedDocks) {
        pluggingWidget = widget;
        QRect globalRect = currentGapRect;
        globalRect.moveTopLeft(parentWidget()->mapToGlobal(globalRect.topLeft()));
#ifndef QT_NO_DOCKWIDGET
        if (qobject_cast<QDockWidget*>(widget) != 0) {
            QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout*>(widget->layout());
            if (layout->nativeWindowDeco()) {
                globalRect.adjust(0, layout->titleHeight(), 0, 0);
            } else {
                int fw = widget->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, 0);
                globalRect.adjust(-fw, -fw, fw, fw);
            }
        }
#endif
        widgetAnimator->animate(widget, globalRect,
                                dockOptions & QMainWindow::AnimatedDocks);
    } else {
#ifndef QT_NO_DOCKWIDGET
        if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget))
            dw->d_func()->plug(currentGapRect);
#endif
#ifndef QT_NO_TOOLBAR
        if (QToolBar *tb = qobject_cast<QToolBar*>(widget))
            tb->d_func()->plug(currentGapRect);
#endif
        applyState(layoutState);
        savedState.clear();
#ifndef QT_NO_DOCKWIDGET
        parentWidget()->update(layoutState.dockAreaLayout.separatorRegion());
        if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget))
            emit dw->dockLocationChanged(toDockWidgetArea(currentGapPos.at(1)));
#endif
        currentGapPos.clear();
        updateGapIndicator();
    }

    return true;
}

void QMainWindowLayout::allAnimationsFinished()
{
#ifndef QT_NO_DOCKWIDGET
    parentWidget()->update(layoutState.dockAreaLayout.separatorRegion());

#ifndef QT_NO_TABBAR
    foreach (QTabBar *tab_bar, usedTabBars)
        tab_bar->show();
#endif
#endif

    updateGapIndicator();
}

void QMainWindowLayout::animationFinished(QWidget *widget)
{

    /* This signal is delivered from QWidgetAnimator over a qeued connection. The problem is that
       the widget can be deleted. This is handled as follows:

       The animator only ever animates widgets that have been added to this layout. If a widget
       is deleted during animation, the widget's destructor removes the widget form this layout.
       This in turn aborts the animation (see takeAt()) and this signal will never be delivered.

       If the widget is deleted after the animation is finished but before this qeued signal
       is delivered, the widget is no longer in the layout and we catch it here. The key is that
       QMainWindowLayoutState::contains() never dereferences the pointer. */

    if (!layoutState.contains(widget))
        return;

#ifndef QT_NO_TOOLBAR
    if (QToolBar *tb = qobject_cast<QToolBar*>(widget)) {
        QToolBarLayout *tbl = qobject_cast<QToolBarLayout*>(tb->layout());
        if (tbl->animating) {
            tbl->animating = false;
            if (tbl->expanded)
                tbl->layoutActions(tb->size());
            tb->update();
        }
    }
#endif

    if (widget != pluggingWidget)
        return;

#ifndef QT_NO_DOCKWIDGET
    if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget))
        dw->d_func()->plug(currentGapRect);
#endif
#ifndef QT_NO_TOOLBAR
    if (QToolBar *tb = qobject_cast<QToolBar*>(widget))
        tb->d_func()->plug(currentGapRect);
#endif

    applyState(layoutState, false);
#ifndef QT_NO_DOCKWIDGET
#ifndef QT_NO_TABBAR
    if (qobject_cast<QDockWidget*>(widget) != 0) {
        QDockAreaLayoutInfo *info = layoutState.dockAreaLayout.info(widget);
        Q_ASSERT(info != 0);
        info->setCurrentTab(widget);
    }
#endif
    if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget))
        emit dw->dockLocationChanged(toDockWidgetArea(currentGapPos.at(1)));
#endif
    savedState.clear();
    currentGapPos.clear();
    pluggingWidget = 0;
    updateGapIndicator();
}

void QMainWindowLayout::restore(bool keepSavedState)
{
    if (!savedState.isValid())
        return;

    layoutState = savedState;
    applyState(layoutState);
    if (!keepSavedState)
        savedState.clear();
    currentGapPos.clear();
    pluggingWidget = 0;
    updateGapIndicator();
}

QMainWindowLayout::QMainWindowLayout(QMainWindow *mainwindow)
    : QLayout(mainwindow), layoutState(mainwindow), savedState(mainwindow),
        dockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowTabbedDocks),
        statusbar(0)
{
#ifndef QT_NO_DOCKWIDGET
    separatorMoveTimer = new QTimer(this);
    separatorMoveTimer->setSingleShot(true);
    separatorMoveTimer->setInterval(0);
    connect(separatorMoveTimer, SIGNAL(timeout()), this, SLOT(doSeparatorMove()));
#endif

#ifndef QT_NO_RUBBERBAND
    gapIndicator = new QRubberBand(QRubberBand::Rectangle, mainwindow);
    gapIndicator->hide();
#endif
    pluggingWidget = 0;

    setObjectName(mainwindow->objectName() + QLatin1String("_layout"));
    widgetAnimator = new QWidgetAnimator(this);
    connect(widgetAnimator, SIGNAL(finished(QWidget*)),
            this, SLOT(animationFinished(QWidget*)), Qt::QueuedConnection);
    connect(widgetAnimator, SIGNAL(finishedAll()),
            this, SLOT(allAnimationsFinished()));
}

QMainWindowLayout::~QMainWindowLayout()
{
    layoutState.deleteAllLayoutItems();
    layoutState.deleteCentralWidgetItem();

#ifdef Q_WS_MAC
    for (int i = 0; i < toolbarItemsCopy.size(); ++i)
        CFRelease(toolbarItemsCopy.at(i));
    toolbarItemsCopy.clear();
    hitoolbarHash.clear();
#endif

    delete statusbar;
}

void QMainWindowLayout::setDockOptions(QMainWindow::DockOptions opts)
{
    if (opts == dockOptions)
        return;

    dockOptions = opts;

#ifndef QT_NO_DOCKWIDGET
    setVerticalTabsEnabled(opts & QMainWindow::VerticalTabs);
#endif

    invalidate();
}

#ifndef QT_NO_STATUSBAR
QStatusBar *QMainWindowLayout::statusBar() const
{ return statusbar ? qobject_cast<QStatusBar *>(statusbar->widget()) : 0; }

void QMainWindowLayout::setStatusBar(QStatusBar *sb)
{
    if (sb)
        addChildWidget(sb);
    delete statusbar;
    statusbar = sb ? new QWidgetItem(sb) : 0;
    invalidate();
}
#endif // QT_NO_STATUSBAR

QWidget *QMainWindowLayout::centralWidget() const
{
    return layoutState.centralWidget();
}

void QMainWindowLayout::setCentralWidget(QWidget *widget)
{
    if (widget != 0)
        addChildWidget(widget);
    layoutState.setCentralWidget(widget);
    invalidate();
}

QLayoutItem *QMainWindowLayout::unplug(QWidget *widget)
{
    QList<int> path = layoutState.indexOf(widget);
    if (path.isEmpty())
        return 0;

    QLayoutItem *item = layoutState.item(path);
    if (widget->isWindow())
        return item;

    QRect r = layoutState.itemRect(path);
    savedState = layoutState;

#ifndef QT_NO_DOCKWIDGET
    if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget)) {
        dw->d_func()->unplug(r);
    }
#endif
#ifndef QT_NO_TOOLBAR
    if (QToolBar *tb = qobject_cast<QToolBar*>(widget)) {
        tb->d_func()->unplug(r);
    }
#endif

    savedState.fitLayout();

    layoutState.unplug(path);
    currentGapPos = path;
    currentGapRect = r;
    updateGapIndicator();

    fixToolBarOrientation(item, currentGapPos.at(1));

    return item;
}

void QMainWindowLayout::updateGapIndicator()
{
#ifndef QT_NO_RUBBERBAND
    if (widgetAnimator->animating() || currentGapPos.isEmpty()) {
        gapIndicator->hide();
    } else {
        if (gapIndicator->geometry() != currentGapRect)
            gapIndicator->setGeometry(currentGapRect);
        if (!gapIndicator->isVisible())
            gapIndicator->show();
    }
#endif
}

QList<int> QMainWindowLayout::hover(QLayoutItem *widgetItem, const QPoint &mousePos)
{
    if (pluggingWidget != 0 || widgetItem == 0)
        return QList<int>();

    QWidget *widget = widgetItem->widget();
    QPoint pos = parentWidget()->mapFromGlobal(mousePos);

    if (!savedState.isValid())
        savedState = layoutState;

    QList<int> path = savedState.gapIndex(widget, pos);

    if (!path.isEmpty()) {
        bool allowed = false;

#ifndef QT_NO_DOCKWIDGET
        if (QDockWidget *dw = qobject_cast<QDockWidget*>(widget))
            allowed = dw->isAreaAllowed(toDockWidgetArea(path.at(1)));
#endif
#ifndef QT_NO_TOOLBAR
        if (QToolBar *tb = qobject_cast<QToolBar*>(widget))
            allowed = tb->isAreaAllowed(toToolBarArea(path.at(1)));
#endif

        if (!allowed)
            path.clear();
    }

    if (path == currentGapPos)
        return currentGapPos; // the gap is already there

    currentGapPos = path;
    if (path.isEmpty()) {
        fixToolBarOrientation(widgetItem, 2); // 2 = top dock, ie. horizontal
        restore(true);
        return QList<int>();
    }

    fixToolBarOrientation(widgetItem, currentGapPos.at(1));

    QMainWindowLayoutState newState = savedState;

    if (!newState.insertGap(path, widgetItem)) {
        restore(true); // not enough space
        return QList<int>();
    }

    QSize min = newState.minimumSize();
    QSize size = newState.rect.size();

    if (min.width() > size.width() || min.height() > size.height()) {
        restore(true);
        return QList<int>();
    }

    newState.fitLayout();

    currentGapRect = newState.gapRect(currentGapPos);

#ifndef QT_NO_DOCKWIDGET
    parentWidget()->update(layoutState.dockAreaLayout.separatorRegion());
#endif
    layoutState = newState;
    applyState(layoutState);

    updateGapIndicator();

    return path;
}

void QMainWindowLayout::applyState(QMainWindowLayoutState &newState, bool animate)
{
#ifndef QT_NO_DOCKWIDGET
#ifndef QT_NO_TABBAR
    QSet<QTabBar*> used = newState.dockAreaLayout.usedTabBars();
    QSet<QTabBar*> retired = usedTabBars - used;
    usedTabBars = used;
    foreach (QTabBar *tab_bar, retired) {
        tab_bar->hide();
        while (tab_bar->count() > 0)
            tab_bar->removeTab(0);
        unusedTabBars.append(tab_bar);
    }
#endif // QT_NO_TABBAR
#endif
    newState.apply(dockOptions & QMainWindow::AnimatedDocks && animate);
}

void QMainWindowLayout::saveState(QDataStream &stream) const
{
    layoutState.saveState(stream);
}

bool QMainWindowLayout::restoreState(QDataStream &stream)
{
    savedState = layoutState;
    layoutState.clear();
    layoutState.rect = savedState.rect;

    if (!layoutState.restoreState(stream, savedState)) {
        layoutState.deleteAllLayoutItems();
        layoutState = savedState;
        if (parentWidget()->isVisible())
            applyState(layoutState, false); // hides tabBars allocated by newState
        return false;
    }

    if (parentWidget()->isVisible()) {
        layoutState.fitLayout();
        applyState(layoutState, false);
    }

    savedState.deleteAllLayoutItems();
    savedState.clear();

#ifndef QT_NO_DOCKWIDGET
#ifndef QT_NO_TABBAR
    if (parentWidget()->isVisible()) {
        foreach (QTabBar *tab_bar, usedTabBars)
            tab_bar->show();
    }
#endif
#endif

    return true;
}


// Returns if this toolbar *should* be using HIToolbar. Won't work for all in between cases
// for example, you have a toolbar in the top area and then you suddenly turn on
// HIToolbar.
bool QMainWindowLayout::usesHIToolBar(QToolBar *toolbar) const
{
#ifndef Q_WS_MAC
    Q_UNUSED(toolbar);
    return false;
#else
    return qtoolbarsInHIToolbarList.contains(toolbar)
           || ((toolBarArea(toolbar) == Qt::TopToolBarArea)
                && layoutState.mainWindow->unifiedTitleAndToolBarOnMac());
#endif
}

#endif // QT_NO_MAINWINDOW
