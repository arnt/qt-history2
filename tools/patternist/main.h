/****************************************************************************
 * ** * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.  * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_main_h
#define Patternist_main_h

#include <QCoreApplication>

QT_BEGIN_HEADER

class PatternistCLI
{
public:
        Q_DECLARE_TR_FUNCTIONS(PatternistCLI)
private:
    PatternistCLI();
    Q_DISABLE_COPY(PatternistCLI)
};

QT_END_HEADER 

#endif
