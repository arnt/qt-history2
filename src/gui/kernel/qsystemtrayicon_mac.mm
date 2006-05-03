#include "qsystemtrayicon_p.h"
#include <qdebug.h>

#include <private/qt_mac_p.h>
#import <AppKit/AppKit.h>

extern void qt_mac_get_accel(quint32 accel_key, quint32 *modif, quint32 *key); //qmenu_mac.cpp
extern QString qt_mac_no_ampersands(QString str); //qmenu_mac.cpp

@interface NSStatusItem (Qt)
- (void)clickSelector:(id)sender;
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

NSImage *qt_mac_create_ns_image(const QPixmap &pm) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
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
    [pool release];
    return 0;
}

class QSystemTrayIconSys
{
public:
    QSystemTrayIconSys() { 
        item = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        [item setTarget:item];
        [item setAction:@selector(clickSelector:)];
        [item setDoubleAction:@selector(doubleClickSelector:)];
    }
    ~QSystemTrayIconSys() { 
        [[NSStatusBar systemStatusBar] removeStatusItem:item];
        [item release]; 
    }
    NSStatusItem *item;
};

void QSystemTrayIconPrivate::install()
{
    if (!sys) {
        sys = new QSystemTrayIconSys;
        updateIcon();
        updateMenu();
        updateToolTip();
    }
}

void QSystemTrayIconPrivate::remove()
{
    delete sys;
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon()
{
    if(sys && !icon.isNull()) {
        const short scale = GetMBarHeight()-4;
        [sys->item setImage:qt_mac_create_ns_image(icon.pixmap(QSize(scale, scale)))];
    }
}

void QSystemTrayIconPrivate::updateMenu()
{
    if(sys) {
        if(menu && !menu->isEmpty()) {
            [sys->item setHighlightMode:YES];
            NSMenu *m = [[QNSMenu alloc] initWithQMenu:menu];
            [sys->item setMenu:m];
            [m release];
        } else {
            [sys->item setHighlightMode:NO];
        }
    }
}

void QSystemTrayIconPrivate::updateToolTip()
{
    if(sys) {
        [sys->item setToolTip:(NSString*)QCFString::toCFStringRef(toolTip)];
    }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable()
{
    return true;
}

void QSystemTrayIconPrivate::showMessage(const QString &message, const QString &,
                                         QSystemTrayIcon::MessageIcon, int)
{
    if(sys) {
        //[sys->item setTitle:(NSString*)QCFString::toCFStringRef(message)];
    }
}

@implementation NSStatusItem (Qt)
- (void)clickSelector:(id)sender {
    printf("hi\n");
}
- (void)doubleClickSelector:(id)sender {
    printf("double hi\n");
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
                text += " (****)"; //just to denote a multi stroke shortcut

            [item setTitle:(NSString*)QCFString::toCFStringRef(qt_mac_no_ampersands(text))];
            [item setEnabled:action->isEnabled()];
            [item setTarget:self];
            [item setAction:@selector(selectedAction:)];
            if(action->menu()) {
                QNSMenu *sub = [[QNSMenu alloc] initWithQMenu:action->menu()];
                [sub setSupermenu:menu];
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














/* General cocoa cleanup, done here because this is the only .mm for now! -Sam */
static NSAutoreleasePool *qt_mac_pool = 0;
void qt_mac_cocoa_cleanup()
{
    if(qt_mac_pool) {
        [qt_mac_pool release];
        qt_mac_pool = 0;
    }
}
void qt_mac_cocoa_init()
{
    if(!qt_mac_pool) {
        NSApplicationLoad();
        qt_mac_pool = [[NSAutoreleasePool alloc] init];
    }
}
