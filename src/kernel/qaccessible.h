#ifndef QACCESSIBLE_H
#define QACCESSIBLE_H

#ifndef QT_H
#include "qstring.h"
#include "qcom.h"
#include "qrect.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QObject;

class Q_EXPORT QAccessible
{
public:
    enum Reason {
	SoundPlayed	    = 0x0001,
	Alert		    = 0x0002,
	ForegroundChange    = 0x0003,
	MenuStart	    = 0x0004,
	MenuEnd		    = 0x0005,
	PopupMenuStart	    = 0x0006,
	PopupMenuEnd	    = 0x0007,
	DragDropStart	    = 0x000E,
	DragDropEnd	    = 0x000F,
	DialogStart	    = 0x0010,
	DialogEnd	    = 0x0011,
	ScrollingStart	    = 0x0012,
	ScrollingEnd	    = 0x0013,
	ObjectCreated	    = 0x8000,
	ObjectDestroyed	    = 0x8001,
	ObjectShow	    = 0x8002,
	ObjectHide	    = 0x8003,
	ObjectReorder	    = 0x8004,
	Focus		    = 0x8005,
	Selection	    = 0x8006,
	SelectionAdd	    = 0x8007,
	SelectionRemove	    = 0x8008,
	SelectionWithin	    = 0x8009,
	StateChange	    = 0x800A,
	LocationChange	    = 0x800B,
	NameChange	    = 0x800C,
	DescriptionChange   = 0x800D,
	ValueChange	    = 0x800E,
	ParentChange	    = 0x800F,
	HelpChange	    = 0x80A0,
	DefaultActionChange = 0x80B0,
	AcceleratorChange   = 0x80C0
    };

    enum State {
	Normal		= 0x00000000,
	Unavailable	= 0x00000001,
	Selected	= 0x00000002,
	Focused		= 0x00000004,
	Pressed		= 0x00000008,
	Checked		= 0x00000010,
	Mixed		= 0x00000020,
	Readonly	= 0x00000040,
	Hottracked	= 0x00000080,
	Default		= 0x00000100,
	Expanded	= 0x00000200,
	Collapsed	= 0x00000400,
	Busy		= 0x00000800,
	Floating	= 0x00001000,
	Marqueed	= 0x00002000,
	Animated	= 0x00004000,
	Invisible	= 0x00008000,
	Offscreen	= 0x00010000,
	Sizeable	= 0x00020000,
	Moveable	= 0x00040000,
	Selfvoicing	= 0x00080000,
	Focusable	= 0x00100000,
	Selectable	= 0x00200000,
	Linked		= 0x00400000,
	Traversed	= 0x00800000,
	Multiselectable	= 0x01000000,
	Extselectable	= 0x02000000,
	AlertLow	= 0x04000000,
	AlertMedium	= 0x08000000,
	AlertHigh	= 0x10000000,
	Protected	= 0x20000000,
	Valid		= 0x3fffffff
    };

    enum Role {
	Client		= 0x01

    };
};

struct Q_EXPORT QAccessibleInterface : public QAccessible, public QUnknownInterface
{
    // navigation and hierarchy
    virtual int		hitTest( int x, int y ) const = 0;
    virtual QRect	location( int who ) const = 0;
    virtual bool	navigate( int dir, int start ) const = 0;
    virtual QAccessibleInterface* child( int who ) const = 0;
    virtual int		childCount() const = 0;
    virtual QAccessibleInterface* parent() const = 0;

    // descriptive properties and methods
    virtual void	doDefaultAction() = 0;
    virtual QString	defaultAction() const = 0;
    virtual QString	description() const = 0;
    virtual QString	help() const = 0;
    virtual QString	accelerator() const = 0;
    virtual QString	name() const = 0;
    virtual QString	value() const = 0;

    virtual Role	role() const = 0;
    virtual State	state() const = 0;

    // selection and focus
    virtual void	select( int how, int who ) = 0;
    virtual bool	hasFocus() const = 0;
    virtual int		selection() const = 0;

};

class Q_EXPORT QAccessibleObject : public QAccessibleInterface
{
public:
    QAccessibleObject( QObject * );

    void setState( State state );
    State state() const;

    void setName( const QString &name );
    QString name() const;

    void setDescription( const QString &description );
    QString description() const;

    void setHelp( const QString &help );
    QString help() const;

    void setValue( const QString &value );
    QString value() const;

    void setDefaultAction( const QString &def );
    QString defaultAction() const;

private:
    QObject *object;

    State state_;
    QString name_;
    QString descr_;
    QString help_;
    QString value_;
    QString default_;    
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLE_H
