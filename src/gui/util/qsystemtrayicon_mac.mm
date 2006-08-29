#include "qsystemtrayicon_p.h"
#include <qdebug.h>

#include <private/qt_mac_p.h>
#import <AppKit/AppKit.h>

extern void qtsystray_sendActivated(QSystemTrayIcon *i, int r); //qsystemtrayicon.cpp
extern void qt_mac_get_accel(quint32 accel_key, quint32 *modif, quint32 *key); //qmenu_mac.cpp
extern QString qt_mac_no_ampersands(QString str); //qmenu_mac.cpp


@interface QNSStatusItem : NSObject {
    NSStatusItem *item;
    QSystemTrayIcon *icon;
}
-(id)initWithIcon:(QSystemTrayIcon*)icon;
-(void)free;
-(NSStatusItem*)item;
- (void)triggerSelector:(id)sender;
- (void)doubleClickSelector:(id)sender;
@end

@interface QNSMenu : NSMenu {
    QMenu *qmenu;
}
-(id)initWithQMenu:(QMenu*)qmenu;
-(void)menuNeedsUpdate:(QNSMenu*)menu;
-(void)selectedAction:(id)item;
@end

void qt_mac_trayicon_activate_action(QMenu *menu, QAction *action)
{
    emit menu->triggered(action);
}

NSImage *qt_mac_create_ns_image(const QPixmap &pm) 
{
    QMacCocoaAutoReleasePool pool;
    if(CGImageRef image = pm.toMacCGImageRef()) {
        NSRect imageRect = NSMakeRect(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image));
        NSImage *newImage = [[NSImage alloc] initWithSize:imageRect.size]; 
        [newImage lockFocus];
        {
            CGContextRef imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
            CGContextDrawImage(imageContext, *(CGRect*)&imageRect, image);
        }
        [newImage unlockFocus];
        return newImage;
    }
    return 0;
}

class QSystemTrayIconSys
{
public:
    QSystemTrayIconSys(QSystemTrayIcon *icon) { 
        QMacCocoaAutoReleasePool pool;
        item = [[QNSStatusItem alloc] initWithIcon:icon];
    }
    ~QSystemTrayIconSys() { 
        QMacCocoaAutoReleasePool pool;
        [item free];
        [item release]; 
    }
    QNSStatusItem *item;
};

void QSystemTrayIconPrivate::install_sys()
{
    Q_Q(QSystemTrayIcon);
    if (!sys) {
        sys = new QSystemTrayIconSys(q);
        updateIcon_sys();
        updateMenu_sys();
        updateToolTip_sys();
    }
}

QPoint QSystemTrayIconPrivate::globalPos_sys() const
{
    if(sys) {
        if(NSWindow *window = [[[sys->item item] view] window]) {
            NSRect rect = [window frame];
            return QPoint(qRound(rect.origin.x), qRound(rect.origin.y));
        }
    }
    return QPoint();
}

void QSystemTrayIconPrivate::remove_sys()
{
    delete sys;
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
    if(sys && !icon.isNull()) {
        QMacCocoaAutoReleasePool pool;
        const short scale = GetMBarHeight()-4;
        [[sys->item item] setImage:qt_mac_create_ns_image(icon.pixmap(QSize(scale, scale)))];
    }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
    if(sys) {
        QMacCocoaAutoReleasePool pool;
        if(menu && !menu->isEmpty()) {
            [[sys->item item] setHighlightMode:YES];
        } else {
            [[sys->item item] setHighlightMode:NO];
        }
    }
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
    if(sys) {
        QMacCocoaAutoReleasePool pool;
        [[sys->item item] setToolTip:(NSString*)QCFString::toCFStringRef(toolTip)];
    }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return true;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message, const QString &title,
                                             QSystemTrayIcon::MessageIcon icon, int msecs)
{
    if(sys) {
        QMacCocoaAutoReleasePool pool;
#if 0        
        //[[sys->item item:] setTitle:(NSString*)QCFString::toCFStringRef(message)];
#elif 0
        Q_Q(QSystemTrayIcon);
        NSView *v = [[sys->item item] view];
        NSWindow *w = [v window];
        w = [[sys->item item] window];
        qDebug() << w << v;
        QPoint p(qRound([w frame].origin.x), qRound([w frame].origin.y));
        qDebug() << p;
        QBalloonTip::showBalloon(icon, message, title, q, QPoint(0, 0), msecs);
#else
        //do growl? we need to weak link the framework and stuff then, less than desirable - IMO.
#endif        
    }
}

@implementation NSStatusItem (Qt)
@end

@implementation QNSStatusItem
-(id)initWithIcon:(QSystemTrayIcon*)i {
    self = [super init];
    if(self) {
        icon = i;
        item = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        //[item setView:[[NSView alloc] init]];
        [item setTarget:self];
        [item setAction:@selector(triggerSelector:)];
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
            [item setDoubleAction:@selector(doubleClickSelector:)];
    }
    return self;
}
-(void)free {
    [[NSStatusBar systemStatusBar] removeStatusItem:item];
    [item release];
}
-(NSStatusItem*)item {
    return item;
}
- (void)triggerSelector:(id)sender {
    if(!icon)
        return;
    qtsystray_sendActivated(icon, QSystemTrayIcon::Trigger);
    if(icon->contextMenu()) {
        NSMenu *m = [[QNSMenu alloc] initWithQMenu:icon->contextMenu()];
        [item popUpStatusItemMenu:m];
        [m release];
    }
}
- (void)doubleClickSelector:(id)sender {
    if(!icon)
        return;
    qtsystray_sendActivated(icon, QSystemTrayIcon::DoubleClick);
}
@end

@implementation QNSMenu
-(id)initWithQMenu:(QMenu*)qm {
    self = [super init];
    if(self) {
        self->qmenu = qm;
        [self setDelegate:self];
    }
    return self;
}
-(void)menuNeedsUpdate:(QNSMenu*)menu {
    for(int i = [menu numberOfItems]-1; i >= 0; --i) 
        [menu removeItemAtIndex:i];
    QList<QAction*> actions = menu->qmenu->actions();;
    for(int i = 0; i < actions.size(); ++i) {
        const QAction *action = actions[i];
        if(!action->isVisible())
            continue;

        NSMenuItem *item = 0;
        if(action->isSeparator()) {
            item = [NSMenuItem separatorItem];
        } else {
            item = [[NSMenuItem alloc] init];
            QString text = action->text();
            QKeySequence accel = action->shortcut();
            {
                int st = text.lastIndexOf('\t');
                if(st != -1) {
                    accel = QKeySequence(text.right(text.length()-(st+1)));
                    text.remove(st, text.length()-st);
                }
            }
            if(accel.count() > 1)
                text += QString(" (****)"); //just to denote a multi stroke shortcut

            [item setTitle:(NSString*)QCFString::toCFStringRef(qt_mac_no_ampersands(text))];
            [item setEnabled:action->isEnabled()];
            [item setTarget:self];
            [item setAction:@selector(selectedAction:)];
            if(action->menu()) {
                QNSMenu *sub = [[QNSMenu alloc] initWithQMenu:action->menu()];
                [item setSubmenu:sub];
            }
            if(!accel.isEmpty()) {
                quint32 modifier, key;
                qt_mac_get_accel(accel[0], &modifier, &key);
                [item setKeyEquivalentModifierMask:modifier];
                [item setKeyEquivalent:(NSString*)QCFString::toCFStringRef(QString((QChar*)&key, 2))];
            }
        }
        if(item)
            [menu addItem:item];
    }
}
-(void)selectedAction:(id)a {
    const int index = [self indexOfItem:a];
    if(QAction *action = qmenu->actions()[index]) {
        action->activate(QAction::Trigger);
        qt_mac_trayicon_activate_action(qmenu, action);
    }
}
@end


/* Done here because this is the only .mm for now! -Sam */
QMacCocoaAutoReleasePool::QMacCocoaAutoReleasePool()
{
    NSApplicationLoad();
    pool = (void*)[[NSAutoreleasePool alloc] init];
}

QMacCocoaAutoReleasePool::~QMacCocoaAutoReleasePool()
{
    [(NSAutoreleasePool*)pool release];
}

