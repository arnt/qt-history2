/****************************************************************************
**
** Definition of the QAccessibleWidget class.
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

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#ifndef QT_H
#include "qaccessibleobject.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleWidgetPrivate;

class Q_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    QAccessibleWidget( QObject *o, Role r = Client, QString name = QString(), 
	QString description = QString(), QString value = QString(), 
	QString help = QString(), QString defAction = QString(),
	QString accelerator = QString(), State s = Normal );

    ~QAccessibleWidget();

    int		controlAt( int x, int y ) const;
    QRect	rect( int control ) const;
    int		navigate( NavDirection direction, int startControl ) const;
    int		childCount() const;
    bool	queryChild( int control, QAccessibleInterface ** ) const;
    bool	queryParent( QAccessibleInterface ** ) const;

    QString	text( Text t, int control ) const;
    void	setText( Text t, int control, const QString &text );
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setFocus( int control );
    bool	setSelected( int control, bool on, bool extend );
    void	clearSelection();
    QMemArray<int> selection() const;

protected:
    QWidget *widget() const;

private:
    QAccessibleWidgetPrivate *d;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEWIDGET_H
