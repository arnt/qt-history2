/****************************************************************************
 * **
 * ** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
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

#ifndef PATTERNIST_VARIABLELOADER_P_H
#define PATTERNIST_VARIABLELOADER_P_H

#include "qdynamiccontext_p.h"
#include "qqobjectnodemodel_p.h"
#include "qexternalvariableloader_p.h"

QT_BEGIN_HEADER

class VariableLoader : public Patternist::ExternalVariableLoader
{
public:
    typedef QHash<Patternist::QName, QVariant> BindingHash;
    typedef Patternist::PlainSharedPtr<VariableLoader> Ptr;

    inline VariableLoader(const BindingHash &bindings,
                          const Patternist::QObjectNodeModel::Ptr &nm) : m_bindingHash(bindings)
                                                                       , m_nodeModel(nm)
    {
        qDebug() << Q_FUNC_INFO << bindings.count();
    }

    virtual Patternist::SequenceType::Ptr announceExternalVariable(const Patternist::QName name,
                                                                   const Patternist::SequenceType::Ptr &declaredType);
    virtual Patternist::Item::Iterator::Ptr evaluateSequence(const Patternist::QName name,
                                                             const Patternist::DynamicContext::Ptr &);

    virtual Patternist::Item evaluateSingleton(const Patternist::QName name,
                                               const Patternist::DynamicContext::Ptr &);
private:
    const BindingHash m_bindingHash;
    const Patternist::QObjectNodeModel::Ptr m_nodeModel;
};

QT_END_HEADER
#endif
// vim: et:ts=4:sw=4:sts=4
