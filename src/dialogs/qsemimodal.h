/****************************************************************************
**
** Definition of QSemiModal class for source compatibility.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSEMIMODAL_H
#define QSEMIMODAL_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_COMPAT
#ifndef QT_NO_SEMIMODAL
class Q_GUI_EXPORT QSemiModal : public QDialog
{
    Q_OBJECT
public:
    QSemiModal( QWidget* parent=0, const char* name=0, bool modal=FALSE, WFlags f=0 )
	: QDialog( parent, name, modal, f ) { }

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSemiModal( const QSemiModal & );
    QSemiModal &operator=( const QSemiModal & );
#endif
};
#endif
#endif

#endif // QSEMIMODAL_H
