#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#ifndef QT_H
#include "qaccessible.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QButton;

class Q_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    QAccessibleWidget( QWidget *w, Role r = Client, QString name = QString::null, 
	QString description = QString::null, QString value = QString::null, 
	QString help = QString::null, QString defAction = QString::null,
	QString accelerator = QString::null );

    QAccessibleInterface* hitTest( int x, int y, int *who ) const;
    QRect	location( int who ) const;
    QAccessibleInterface* navigate( NavDirection direction, int *target ) const;
    int		childCount() const;
    QAccessibleInterface *child( int who ) const;
    QAccessibleInterface *parent() const;

    bool	doDefaultAction( int who );
    QString	defaultAction( int who ) const;
    QString	description( int who ) const;
    QString	help( int who ) const;
    QString	accelerator( int who ) const;
    QString	name( int who ) const;
    QString	value( int who ) const;
    Role	role( int who ) const;
    State	state( int who ) const;

    QAccessibleInterface *hasFocus( int *who ) const;

protected:
    QWidget *widget() const;

private:
    QWidget *widget_;

    Role role_;
    QString name_;
    QString description_;
    QString value_;
    QString help_;
    QString defAction_;
    QString accelerator_;
};

class Q_EXPORT QAccessibleButton : public QAccessibleWidget
{
public:
    QAccessibleButton( QButton *b, Role r, QString description = QString::null,
	QString help = QString::null );

    bool	doDefaultAction( int who );
    QString	accelerator( int who ) const;
    QString	name( int who ) const;
    State	state( int who ) const;
};

class Q_EXPORT QAccessibleRangeControl : public QAccessibleWidget
{
public:
    QAccessibleRangeControl( QWidget *w, Role role, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString value( int who ) const;
};

class Q_EXPORT QAccessibleText : public QAccessibleWidget
{
public:
    QAccessibleText( QWidget *w, Role role, QString name = QString::null, 
	QString description = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString value( int who ) const;
};

class Q_EXPORT QAccessibleDisplay : public QAccessibleWidget
{
public:
    QAccessibleDisplay( QWidget *w, Role role, QString description = QString::null, 
	QString value = QString::null, QString help = QString::null, 
	QString defAction = QString::null, QString accelerator = QString::null );

    QString name( int who ) const;
    Role    role( int who ) const;
};

#endif // QT_ACCESSIBILITY_SUPPORT

#endif // Q_ACESSIBLEWIDGET_H
