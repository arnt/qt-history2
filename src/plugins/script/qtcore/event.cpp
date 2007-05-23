#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QEvent>
#include "../global.h"

DECLARE_POINTER_METATYPE(QEvent)

static QScriptValue newEvent(QScriptEngine *eng, QEvent *e)
{
    return QScript::wrapPointer(eng, e);
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    int type = ctx->argument(0).toInt32();
    return newEvent(eng, new QEvent(QEvent::Type(type)));
}

/////////////////////////////////////////////////////////////

static QScriptValue accept(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, accept);
    self->accept();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue ignore(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, ignore);
    self->ignore();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue isAccepted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, isAccepted);
    return QScriptValue(eng, self->isAccepted());
}

/////////////////////////////////////////////////////////////

static QScriptValue setAccepted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, setAccepted);
    self->setAccepted(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue spontaneous(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, spontaneous);
    return QScriptValue(eng, self->spontaneous());
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, type);
    return QScriptValue(eng, self->type());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Event, toString);
    return QScriptValue(eng, QString::fromLatin1("QEvent"));
}

/////////////////////////////////////////////////////////////

QScriptValue constructEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newEvent(eng, new QEvent(QEvent::None));
    ADD_PROTO_FUNCTION(proto, accept);
    ADD_PROTO_FUNCTION(proto, ignore);
    ADD_PROTO_FUNCTION(proto, isAccepted);
    ADD_PROTO_FUNCTION(proto, setAccepted);
    ADD_PROTO_FUNCTION(proto, spontaneous);
    ADD_PROTO_FUNCTION(proto, type);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, QEvent, None);
    ADD_ENUM_VALUE(ctorFun, QEvent, Timer);
    ADD_ENUM_VALUE(ctorFun, QEvent, MouseButtonPress);
    ADD_ENUM_VALUE(ctorFun, QEvent, MouseButtonRelease);
    ADD_ENUM_VALUE(ctorFun, QEvent, MouseButtonDblClick);
    ADD_ENUM_VALUE(ctorFun, QEvent, MouseMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, KeyPress);
    ADD_ENUM_VALUE(ctorFun, QEvent, KeyRelease);
    ADD_ENUM_VALUE(ctorFun, QEvent, FocusIn);
    ADD_ENUM_VALUE(ctorFun, QEvent, FocusOut);
    ADD_ENUM_VALUE(ctorFun, QEvent, Enter);
    ADD_ENUM_VALUE(ctorFun, QEvent, Leave);
    ADD_ENUM_VALUE(ctorFun, QEvent, Paint);
    ADD_ENUM_VALUE(ctorFun, QEvent, Move);
    ADD_ENUM_VALUE(ctorFun, QEvent, Resize);
    ADD_ENUM_VALUE(ctorFun, QEvent, Create);
    ADD_ENUM_VALUE(ctorFun, QEvent, Destroy);
    ADD_ENUM_VALUE(ctorFun, QEvent, Show);
    ADD_ENUM_VALUE(ctorFun, QEvent, Hide);
    ADD_ENUM_VALUE(ctorFun, QEvent, Close);
    ADD_ENUM_VALUE(ctorFun, QEvent, Quit);
    ADD_ENUM_VALUE(ctorFun, QEvent, ParentChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ParentAboutToChange);
//    ADD_ENUM_VALUE(ctorFun, QEvent, Reparent);
    ADD_ENUM_VALUE(ctorFun, QEvent, ThreadChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowActivate);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowDeactivate);
    ADD_ENUM_VALUE(ctorFun, QEvent, ShowToParent);
    ADD_ENUM_VALUE(ctorFun, QEvent, HideToParent);
    ADD_ENUM_VALUE(ctorFun, QEvent, Wheel);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowTitleChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowIconChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationWindowIconChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationFontChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationLayoutDirectionChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationPaletteChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, PaletteChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, Clipboard);
    ADD_ENUM_VALUE(ctorFun, QEvent, Speech);
    ADD_ENUM_VALUE(ctorFun, QEvent, MetaCall);
    ADD_ENUM_VALUE(ctorFun, QEvent, SockAct);
    ADD_ENUM_VALUE(ctorFun, QEvent, WinEventAct);
    ADD_ENUM_VALUE(ctorFun, QEvent, DeferredDelete);
    ADD_ENUM_VALUE(ctorFun, QEvent, DragEnter);
    ADD_ENUM_VALUE(ctorFun, QEvent, DragMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, DragLeave);
    ADD_ENUM_VALUE(ctorFun, QEvent, Drop);
    ADD_ENUM_VALUE(ctorFun, QEvent, DragResponse);
    ADD_ENUM_VALUE(ctorFun, QEvent, ChildAdded);
    ADD_ENUM_VALUE(ctorFun, QEvent, ChildPolished);
//    ADD_ENUM_VALUE(ctorFun, QEvent, ChildInsertedRequest);
//    ADD_ENUM_VALUE(ctorFun, QEvent, ChildInserted);
//    ADD_ENUM_VALUE(ctorFun, QEvent, LayoutHint);
    ADD_ENUM_VALUE(ctorFun, QEvent, ChildRemoved);
    ADD_ENUM_VALUE(ctorFun, QEvent, ShowWindowRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, PolishRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, Polish);
    ADD_ENUM_VALUE(ctorFun, QEvent, LayoutRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, UpdateRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, UpdateLater);
    ADD_ENUM_VALUE(ctorFun, QEvent, EmbeddingControl);
    ADD_ENUM_VALUE(ctorFun, QEvent, ActivateControl);
    ADD_ENUM_VALUE(ctorFun, QEvent, DeactivateControl);
    ADD_ENUM_VALUE(ctorFun, QEvent, ContextMenu);
    ADD_ENUM_VALUE(ctorFun, QEvent, InputMethod);
    ADD_ENUM_VALUE(ctorFun, QEvent, AccessibilityPrepare);
    ADD_ENUM_VALUE(ctorFun, QEvent, TabletMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, LocaleChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, LanguageChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, LayoutDirectionChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, Style);
    ADD_ENUM_VALUE(ctorFun, QEvent, TabletPress);
    ADD_ENUM_VALUE(ctorFun, QEvent, TabletRelease);
    ADD_ENUM_VALUE(ctorFun, QEvent, OkRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, HelpRequest);
    ADD_ENUM_VALUE(ctorFun, QEvent, IconDrag);
    ADD_ENUM_VALUE(ctorFun, QEvent, FontChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, EnabledChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ActivationChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, StyleChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, IconTextChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ModifiedChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, MouseTrackingChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowBlocked);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowUnblocked);
    ADD_ENUM_VALUE(ctorFun, QEvent, WindowStateChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ToolTip);
    ADD_ENUM_VALUE(ctorFun, QEvent, WhatsThis);
    ADD_ENUM_VALUE(ctorFun, QEvent, StatusTip);
    ADD_ENUM_VALUE(ctorFun, QEvent, ActionChanged);
    ADD_ENUM_VALUE(ctorFun, QEvent, ActionAdded);
    ADD_ENUM_VALUE(ctorFun, QEvent, ActionRemoved);
    ADD_ENUM_VALUE(ctorFun, QEvent, FileOpen);
    ADD_ENUM_VALUE(ctorFun, QEvent, Shortcut);
    ADD_ENUM_VALUE(ctorFun, QEvent, ShortcutOverride);
//    ADD_ENUM_VALUE(ctorFun, QEvent, Accel);
//    ADD_ENUM_VALUE(ctorFun, QEvent, AccelAvailable);
//    ADD_ENUM_VALUE(ctorFun, QEvent, AccelOverride);
    ADD_ENUM_VALUE(ctorFun, QEvent, WhatsThisClicked);
//    ADD_ENUM_VALUE(ctorFun, QEvent, CaptionChange);
//    ADD_ENUM_VALUE(ctorFun, QEvent, IconChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ToolBarChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationActivated);
    ADD_ENUM_VALUE(ctorFun, QEvent, ApplicationDeactivated);
    ADD_ENUM_VALUE(ctorFun, QEvent, QueryWhatsThis);
    ADD_ENUM_VALUE(ctorFun, QEvent, EnterWhatsThisMode);
    ADD_ENUM_VALUE(ctorFun, QEvent, LeaveWhatsThisMode);
    ADD_ENUM_VALUE(ctorFun, QEvent, ZOrderChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, HoverEnter);
    ADD_ENUM_VALUE(ctorFun, QEvent, HoverLeave);
    ADD_ENUM_VALUE(ctorFun, QEvent, HoverMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, AccessibilityHelp);
    ADD_ENUM_VALUE(ctorFun, QEvent, AccessibilityDescription);
//    ADD_ENUM_VALUE(ctorFun, QEvent, EnterEditFocus);
//    ADD_ENUM_VALUE(ctorFun, QEvent, LeaveEditFocus);
    ADD_ENUM_VALUE(ctorFun, QEvent, AcceptDropsChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, MenubarUpdated);
    ADD_ENUM_VALUE(ctorFun, QEvent, ZeroTimerEvent);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneMouseMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneMousePress);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneMouseRelease);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneMouseDoubleClick);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneContextMenu);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneHoverEnter);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneHoverMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneHoverLeave);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneHelp);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneDragEnter);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneDragMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneDragLeave);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneDrop);
    ADD_ENUM_VALUE(ctorFun, QEvent, GraphicsSceneWheel);
    ADD_ENUM_VALUE(ctorFun, QEvent, KeyboardLayoutChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, DynamicPropertyChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, TabletEnterProximity);
    ADD_ENUM_VALUE(ctorFun, QEvent, TabletLeaveProximity);
    ADD_ENUM_VALUE(ctorFun, QEvent, NonClientAreaMouseMove);
    ADD_ENUM_VALUE(ctorFun, QEvent, NonClientAreaMouseButtonPress);
    ADD_ENUM_VALUE(ctorFun, QEvent, NonClientAreaMouseButtonRelease);
    ADD_ENUM_VALUE(ctorFun, QEvent, NonClientAreaMouseButtonDblClick);
    ADD_ENUM_VALUE(ctorFun, QEvent, MacSizeChange);
    ADD_ENUM_VALUE(ctorFun, QEvent, User);
    ADD_ENUM_VALUE(ctorFun, QEvent, MaxUser);

    return ctorFun;
}
