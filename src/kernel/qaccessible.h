#ifndef QACCESSIBLE_H
#define QACCESSIBLE_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QObject;

class Q_EXPORT QAccessible
{
public:
    enum Reason {
	StateChange,
	ValueChange,
	NameChange,
	DescriptionChange,
	Selection,
	SelectionAdd,
	SelectionRemove,
	SelectionWithin,
	Focus,

	MenuStart,
	MenuEnd,
	PopupMenuStart,
	PopupMenuEnd,
	DragDropStart,
	DragDropEnd,
	DialogStart,
	DialogEnd,
	ObjectShow,
	ObjectHide,
	ObjectReorder
    };

    enum State {
	Normal		= 0x00000000,
	Unavailable	= 0x00000001,
	Selected	= 0x00000001,
	Focused		= 0x00000001,
	Pressed		= 0x00000001,
	Checked		= 0x00000001,
	Mixed		= 0x00000001,
	Readonly	= 0x00000001,
	Hottracked	= 0x00000001,
	Default		= 0x00000001,
	Expanded	= 0x00000001,
	Collapsed	= 0x00000001,
	Busy		= 0x00000001,
	Floating	= 0x00000001,
	Marqueed	= 0x00000001,
	Animated	= 0x00000001,
	Invisible	= 0x00000001,
	Offscreen	= 0x00000001,
	Sizeable	= 0x00000001,
	Moveable	= 0x00000001,
	Selfvoicing	= 0x00000001,
	Focusable	= 0x00000001,
	Selectable	= 0x00000001,
	Linked		= 0x00000001,
	Traversed	= 0x00000001,
	Multiselectable	= 0x00000001,
	Extselectable	= 0x02000000,
	Protected	= 0x20000000
    };

    QAccessible();

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
    bool notify( QObject *o, Reason reason = Focus );

    State state_;
    QString name_;
    QString descr_;
    QString help_;
    QString value_;
    QString default_;

    friend class QObject;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLE_H
