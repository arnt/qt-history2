/****************************************************************************
** $Id: $
**
** Definition of QCategoryBar widget class
**
** Created : 961105
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCATEGORYBAR_H
#define QCATEGORYBAR_H

#include <qwidget.h>
#include <qiconset.h>

class QCategoryButton;
class QCategoryBarPrivate;
class QWidgetList;

class Q_EXPORT QCategoryBar : public QWidget
{
    Q_OBJECT

public:
    QCategoryBar( QWidget *parent = 0, const char *name = 0 );
    ~QCategoryBar();

    virtual void addCategory( const QString &label, QWidget *page );
    virtual void addCategory( const QString &label, const QIconSet &iconSet,
			      QWidget *page );
    virtual void insertCategory( const QString &label, QWidget *page, int index = -1 );
    virtual void insertCategory( const QString &label, const QIconSet &iconSet,
			 QWidget *page, int index = -1 );

    bool isCategoryEnabled( QWidget *page ) const;

    QString categoryLabel( QWidget *page ) const;

    QIconSet categoryIconSet( QWidget *page ) const;

    QString categoryToolTip( QWidget *page ) const;

    QWidget *currentPage() const;
    int currentIndex() const;
    QWidget *page( int index ) const;
    int pageIndex( QWidget *page ) const;

    int count() const;

public slots:
    virtual void setCurrentPage( int index );
    virtual void setCurrentPage( QWidget *page );
    virtual void setCategoryEnabled( QWidget *page, bool enabled );
    virtual void removeCategory( QWidget *page );
    virtual void setCategoryLabel( QWidget *page, const QString &label );
    virtual void setCategoryIconSet( QWidget *page, const QIconSet &iconSet );
    virtual void setCategoryToolTip( QWidget *page, const QString &toolTip );

signals:
    void currentChanged( QWidget *page );

private slots:
    void buttonClicked();

private:
    void updateTabs();
    void relayout();
    void activateClosestPage( QWidget *page );

private:
    QCategoryBarPrivate *d;

};

#endif
