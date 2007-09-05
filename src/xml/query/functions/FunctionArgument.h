/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_FunctionArgument_H
#define Patternist_FunctionArgument_H

#include <QList>
#include <QSharedData>

#include "QName.h"
#include "SequenceType.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @todo Docs
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FunctionArgument : public QSharedData
    {
    public:
        typedef PlainSharedPtr<FunctionArgument> Ptr;
        typedef QList<FunctionArgument::Ptr> List;

        FunctionArgument(const QName name,
                         const SequenceType::Ptr &type);

        QName name() const;
        SequenceType::Ptr type() const;

    private:
        Q_DISABLE_COPY(FunctionArgument)
        const QName m_name;
        const SequenceType::Ptr m_type;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
