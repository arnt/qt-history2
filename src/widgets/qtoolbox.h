/****************************************************************************
**
** Definition of QToolBox widget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOOLBOX_H
#define QTOOLBOX_H

#ifndef QT_H
#include <qframe.h>
#include <qiconset.h>
#endif // QT_H

#ifndef QT_NO_TOOLBOX

class QToolBoxPrivate;

class Q_GUI_EXPORT QToolBox : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( int currentIndex READ currentIndex WRITE setCurrentIndex )
    Q_PROPERTY( int count READ count )

public:
    QToolBox( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~QToolBox();

    int addItem( QWidget *item, const QString &label );
    int addItem( QWidget *item, const QIconSet &iconSet, const QString &label );
    int insertItem( int index, QWidget *item, const QString &label );
    int insertItem( int index, QWidget *item, const QIconSet &iconSet, const QString &label );

    int removeItem( QWidget *item );

    void setItemEnabled( int index, bool enabled );
    bool isItemEnabled( int index ) const;

    void setItemLabel( int index, const QString &label );
    QString itemLabel( int index ) const;

    void setItemIconSet( int index, const QIconSet &iconSet );
    QIconSet itemIconSet( int index ) const;

    void setItemToolTip( int index, const QString &toolTip );
    QString itemToolTip( int index ) const;

    int currentIndex() const;
    QWidget *item( int index ) const;
    int indexOf( QWidget *item ) const;
    int count() const;

public slots:
    void setCurrentIndex( int index );

signals:
    void currentChanged( int index );

private slots:
    void buttonClicked();
    void itemDestroyed(QObject*);

protected:
    virtual void itemInserted( int index );
    virtual void itemRemoved( int index );
    void showEvent( QShowEvent *e );
    void frameChanged();
    void changeEvent( QEvent * );

private:
    void relayout();

private:
    QToolBoxPrivate *d;

#ifdef QT_COMPAT
public:
    QT_COMPAT QWidget *currentItem() const { return item(currentIndex()); }
    QT_COMPAT void setCurrentItem( QWidget *item ) { setCurrentIndex(indexOf(item)); }
#endif
};


inline int QToolBox::addItem( QWidget *item, const QString &label )
{ return insertItem( -1, item, QIconSet(), label ); }
inline int QToolBox::addItem( QWidget *item, const QIconSet &iconSet,
			      const QString &label )
{ return insertItem( -1, item, iconSet, label ); }
inline int QToolBox::insertItem( int index, QWidget *item, const QString &label )
{ return insertItem( index, item, QIconSet(), label ); }

#endif // QT_NO_TOOLBOX
#endif
