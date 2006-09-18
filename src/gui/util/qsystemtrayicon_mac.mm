#include "qsystemtrayicon_p.h"
#include <qdebug.h>

#include <private/qt_mac_p.h>
#import <AppKit/AppKit.h>

extern void qtsystray_sendActivated(QSystemTrayIcon *i, int r); //qsystemtrayicon.cpp
extern void qt_mac_get_accel(quint32 accel_key, quint32 *modif, quint32 *key); //qmenu_mac.cpp
extern QString qt_mac_no_ampersands(QString str); //qmenu_mac.cpp

@class QNSImageView;

@interface QNSStatusItem : NSObject {
    NSStatusItem *item;
    QSystemTrayIcon *icon;
    QNSImageView *imageCell;
}
-(id)initWithIcon:(QSystemTrayIcon*)icon;
-(void)free;
-(QSystemTrayIcon*)icon;
-(NSStatusItem*)item;
-(QRectF)geometry;
- (void)triggerSelector:(id)sender;
- (void)doubleClickSelector:(id)sender;
@end

@interface QNSImageView : NSImageView {
    BOOL down;
    QNSStatusItem *parent;
}
-(id)initWithParent:(QNSStatusItem*)myParent;
-(QSystemTrayIcon*)icon;
-(void)mouseDown:(NSEvent *)mouseEvent;
-(void)drawRect:(NSRect)rect;
-(void)menuTrackingDone:(NSNotification*)notification;
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
        const QRectF geom = [sys->item geometry];
        if(!geom.isNull())
            return geom.topLeft().toPoint();
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
        NSImage *nsimage = qt_mac_create_ns_image(icon.pixmap(QSize(scale, scale)));
        [(NSImageView*)[[sys->item item] view] setImage: nsimage];
        [nsimage release];
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
        QCFString string(toolTip);
        [(NSImageView*)[[sys->item item] view] setToolTip:(NSString*)static_cast<CFStringRef>(string)];
    }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return true;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &, const QString &,
                                             QSystemTrayIcon::MessageIcon, int)
{
#if 0
    if(sys) {
        QMacCocoaAutoReleasePool pool;
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
        // else do growl? we need to weak link the framework and stuff then, less than desirable - IMO.
    }
#endif
}

@implementation NSStatusItem (Qt)
@end

@implementation QNSImageView
-(id)initWithParent:(QNSStatusItem*)myParent {
    self = [super init];
    parent = myParent;
    down = NO;
    return self;
}
-(QSystemTrayIcon*)icon {
    return [parent icon];
}

-(void)menuTrackingDone:(NSNotification*)notification
{
    Q_UNUSED(notification);
    down = NO;
    if([self icon]->contextMenu())
        [self icon]->contextMenu()->hide();
    
    [self setNeedsDisplay:YES];
}

-(void)mouseDown:(NSEvent *)mouseEvent {
    int clickCount = [mouseEvent clickCount];
    down = !down;
    if(!down && [self icon]->contextMenu())
        [self icon]->contextMenu()->hide();
    [self setNeedsDisplay:YES];

    if (down && clickCount == 1)
        [parent triggerSelector:self];
    else if (clickCount >= 2)
        [parent doubleClickSelector:self];

    while (down) {
        mouseEvent = [[self window] nextEventMatchingMask: NSLeftMouseDownMask | NSLeftMouseUpMask | NSLeftMouseDraggedMask];
        switch ([mouseEvent type]) {
            case NSLeftMouseDown:
            case NSLeftMouseUp:
                [self menuTrackingDone:nil];
                break;
            case NSLeftMouseDragged:
            default:
                /* Ignore any other kind of event. */
                break;
        }
    };
}


-(void)drawRect:(NSRect)rect {
    [[parent item] drawStatusBarBackgroundInRect:rect withHighlight:down];
    [super drawRect:rect];
}
@end

@implementation QNSStatusItem
-(id)initWithIcon:(QSystemTrayIcon*)i {
    self = [super init];
    if(self) {
        icon = i;
        item = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        imageCell = [[QNSImageView alloc] initWithParent:self];
        [item setView: imageCell];
    }
    return self;
}
-(void)free {
    [[NSStatusBar systemStatusBar] removeStatusItem:item];
    [imageCell release];
    [item release];

}

-(QSystemTrayIcon*)icon {
    return icon;
}

-(NSStatusItem*)item {
    return item;
}
-(QRectF)geometry {
    if(NSWindow *window = [[item view] window]) {
        NSRect screenRect = [[window screen] frame];
        NSRect windowRect = [window frame];
        return QRectF(windowRect.origin.x, screenRect.size.height-windowRect.origin.y, windowRect.size.width, windowRect.size.height);
    }
    return QRectF();
}
- (void)triggerSelector:(id)sender {
    Q_UNUSED(sender);
    if(!icon)
        return;
    qtsystray_sendActivated(icon, QSystemTrayIcon::Trigger);
    if (icon->contextMenu()) {
#if 0
        const QRectF geom = [self geometry];
        if(!geom.isNull()) {
            icon->contextMenu()->exec(geom.topLeft().toPoint(), 0);
            [imageCell menuTrackingDone:nil];
        } else 
#endif
        {
            NSMenu *m = [[QNSMenu alloc] initWithQMenu:icon->contextMenu()];
            [[NSNotificationCenter defaultCenter] addObserver:imageCell
                                                  selector:@selector(menuTrackingDone:)
                                                  name:NSMenuDidEndTrackingNotification
                                                  object:m];
            [item popUpStatusItemMenu: m];
            [m release];
        }
    }
}
- (void)doubleClickSelector:(id)sender {
    Q_UNUSED(sender);
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
            [item setState:action->isChecked() ? NSOnState : NSOffState];
            [item setToolTip:(NSString*)QCFString::toCFStringRef(action->toolTip())];
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
    const int activated = [self indexOfItem:a];
    QAction *action = 0;
    QList<QAction*> actions = qmenu->actions();
    for(int i = 0, cnt = 0; i < actions.size(); ++i) {
        if(actions.at(i)->isVisible() && (cnt++) == activated) {
            action = actions.at(i);
            break;
        }
    }
    if(action) {
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

