/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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
#ifndef __PROPERTYEDITORSQL_H__
#define __PROPERTYEDITORSQL_H__

class QDateEdit;
class QTimeEdit;
class QDateTimeEdit;

class PropertyDateItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyDateItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyDateItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QDateEdit *lined();
    QGuardedPtr<QDateEdit> lin;

};

class PropertyTimeItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyTimeItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyTimeItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QTimeEdit *lined();
    QGuardedPtr<QTimeEdit> lin;

};

class PropertyDateTimeItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyDateTimeItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyDateTimeItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QDateTimeEdit *lined();
    QGuardedPtr<QDateTimeEdit> lin;

};

#endif /* __PROPERTYEDITORSQL_H__ */
