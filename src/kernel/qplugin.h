/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qplugin.h#1 $
**
** Definition of QPlugIn class
**
** Created : 2000-01-01
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#ifndef QPLUGIN_H
#define QPLUGIN_H

#ifndef QT_H
#include "qplugininterface.h"
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class Q_EXPORT QPlugIn : public QPlugInInterface
{
public:
    enum LibraryPolicy
    { 
	Default,
	OptimizeSpeed,
	Manual
    };

    QPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );
    ~QPlugIn();

    bool load();
    bool unload( bool force = FALSE );
    bool loaded() const;

    void setPolicy( LibraryPolicy pol );
    LibraryPolicy policy() const;

    QString library() const;

    QString name();
    QString description();
    QString author();

    QStringList featureList();

protected:
    bool use();
    QPlugInInterface* plugInterface() { return ifc; }

private:
    bool loadInterface();
    QPlugInInterface* ifc;

    typedef QPlugInInterface* (*LoadInterfaceProc)();

#ifdef _WS_WIN_
    HINSTANCE pHnd;
#else
    void* pHnd;
#endif
    QString libfile;
    LibraryPolicy libPol;
    QCString function;
};

#if 1
class Q_EXPORT QPlugInManagerSignalEmitter : public QObject
{
    Q_OBJECT
public:
    QPlugInManagerSignalEmitter();

    void emitFeatureAdded( const QString& feature );
    void emitFeatureRemoved( const QString& feature );

signals:
    void featureAdded( const QString& );
    void featureRemoved( const QString& );
};
#endif

#endif

#endif // QPLUGIN_H

