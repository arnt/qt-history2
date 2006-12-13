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

#include "qcolordialog.h"
#if !defined(QT_NO_COLORDIALOG) && defined(Q_WS_MAC)
#include <qapplication.h>
#include <private/qapplication_p.h>
#include <qdesktopwidget.h>
#include <private/qt_mac_p.h>
#include <qdebug.h>
#import <AppKit/AppKit.h>

@class QNSColorPickerResponder;

@interface QNSColorPickerResponder : NSObject {
    NSColor *mColor;
    BOOL mNeedAlpha;
}
- (id)initWithColor:(NSColor*)color needAlpha:(BOOL)needAlpha;
- (void)dealloc;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;
- (QRgb)qtColor;
@end

@implementation QNSColorPickerResponder
- (id)initWithColor:(NSColor*)color needAlpha:(BOOL)needAlpha {
    self = [super init];
    mColor = color;
    mNeedAlpha = needAlpha;
    [mColor retain];
    return self;
}

- (void)dealloc {
    [mColor release];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    NSColorPanel *panel = [notification object];
    [panel setTitle:(NSString*)(CFStringRef)QCFString(QColorDialog::tr("Select color"))];
    [panel setShowsAlpha:mNeedAlpha];
    [panel setColor:mColor];
    NSEnableScreenUpdates();
}
- (void)windowDidResignKey:(NSNotification *)notification {
    NSColorPanel *panel = [notification object];
    [mColor release];
    mColor = [panel color];
    [mColor retain];
}

- (QRgb)qtColor {
    NSString *colorSpace = [mColor colorSpaceName];
    QColor tmpQColor;
    if (colorSpace == NSDeviceCMYKColorSpace) {
        float cyan, magenta, yellow, black, alpha;
        [mColor getCyan:&cyan magenta:&magenta yellow:&yellow black:&black alpha:&alpha];
        tmpQColor.setCmykF(cyan, magenta, yellow, black, alpha);
    } else {
        NSColor *tmpColor;
        bool needRelease;
        if (colorSpace == NSCalibratedRGBColorSpace || colorSpace == NSDeviceRGBColorSpace) {
            tmpColor = mColor;
            needRelease = false;
        } else {
            tmpColor = [mColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            needRelease = true;
        }
        float red, green, blue, alpha;
        [tmpColor getRed:&red green:&green blue:&blue alpha:&alpha];
        tmpQColor.setRgbF(red, green, blue, alpha);
        if (needRelease)
            [tmpColor release];
    }
    return tmpQColor.rgba();
}
@end

QRgb macGetRgba(QRgb initial, bool needAlpha, bool *ok, QWidget *parent)
{
    QMacCocoaAutoReleasePool pool;
    NSColorPanel *cocoaColorPanel = [NSColorPanel sharedColorPanel];
    NSColor *nsColor = [NSColor colorWithCalibratedRed:qRed(initial) / 255.
                                                       green:qGreen(initial) / 255.
                                                       blue:qBlue(initial) / 255.
                                                       alpha:qAlpha(initial) / 255.];

    QNSColorPickerResponder *responder = [[QNSColorPickerResponder alloc] initWithColor:nsColor
                                                                          needAlpha:needAlpha];
    [nsColor release];
    [cocoaColorPanel setDelegate:responder];
    static const int sw = 420, sh = 300;
    Point p = { -1, -1 };
    if (parent) {
        parent = parent->window();
        p.h = (parent->x() + (parent->width() / 2)) - (sw / 2);
        p.v = (parent->y() + (parent->height() / 2)) - (sh / 2);
        QRect r = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(parent));
        const int border = 10;
        if(p.h + sw > r.right())
            p.h -= (p.h + sw) - r.right() + border;
        if(p.v + sh > r.bottom())
            p.v -= (p.v + sh) - r.bottom() + border;
        if(p.h < r.left())
            p.h = r.left() + border;
        if(p.v < r.top())
            p.v = r.top() + border;
    }
    RGBColor rgb, rgbout;
    rgb.red = qRed(initial) * 256;
    rgb.blue = qBlue(initial) * 256;
    rgb.green = qGreen(initial) * 256;
    Str255 title;
    bzero(title, sizeof(Str255));
    Point place;
    place.h = p.h == -1 ? 0 : p.h;
    place.v = p.v == -1 ? 0 : p.v;
    Boolean rval = false;
    {
        QMacBlockingFunction block;
        QWidget modal_widg(parent, Qt::Sheet);
        modal_widg.createWinId();
        QApplicationPrivate::enterModal(&modal_widg);
        rval = GetColor(place, title, &rgb, &rgbout);
        QApplicationPrivate::leaveModal(&modal_widg);
    }
    if (rval)
        initial = [responder qtColor];
    if (ok)
        *ok = rval;
    [cocoaColorPanel setDelegate:nil];
    [responder release];
    return initial;
}

QColor macGetColor(const QColor &initial, QWidget *parent)
{
    QRgb rgb = macGetRgba(initial.rgb(), false, 0, parent);

    QColor ret;
    if(ok)
        ret = QColor(rgb);
    return ret;
}
#endif
