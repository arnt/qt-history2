/****************************************************************************
**
** Definition of the QAccessibleObject and QAccessibleApplication classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#ifndef QT_H
#include "qaccessible.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleObjectPrivate;
class QObject;

class Q_EXPORT QAccessibleObject : public Qt, public QAccessibleInterface
{
public:
    QAccessibleObject(QObject *object);
    virtual ~QAccessibleObject();

    QRESULT	queryInterface( const QUuid &, QUnknownInterface** );
    Q_REFCOUNT

    bool	isValid() const;
    QObject	*object() const;

private:
    QAccessibleObjectPrivate *d;
};

class Q_EXPORT QAccessibleApplication : public QAccessibleObject
{
public:
    QAccessibleApplication(QApplication *object);

    // hierarchy
    int		childCount() const;
    int		indexOfChild(const QAccessibleInterface*) const;
    bool	queryChild( int control, QAccessibleInterface** ) const;
    bool	queryParent( QAccessibleInterface** ) const;

    // navigation
    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;

    // properties and state
    QString	text( Text t, int control ) const;
    void	setText( Text t, int control, const QString &text );
    Role	role( int control ) const;
    State	state( int control ) const;
    QMemArray<int> selection() const;

    // methods
    bool	doDefaultAction( int control );
    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEOBJECT_H
