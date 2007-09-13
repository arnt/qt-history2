/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Patternist_FunctionArgument_H
#define Patternist_FunctionArgument_H

#include <QList>
#include <QSharedData>

#include "qqname_p.h"
#include "qsequencetype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
