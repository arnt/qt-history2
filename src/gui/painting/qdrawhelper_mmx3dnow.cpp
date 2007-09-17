/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qdrawhelper_x86_p.h>

#ifdef QT_HAVE_3DNOW

#include <private/qdrawhelper_mmx_p.h>
#include <mm3dnow.h>

QT_BEGIN_NAMESPACE

struct QMMX3DNOWIntrinsics : public QMMXCommonIntrinsics
{
    static inline void end() {
        _m_femms();
    }
};

CompositionFunctionSolid qt_functionForModeSolid_MMX3DNOW[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QMMX3DNOWIntrinsics>,
    comp_func_solid_DestinationOver<QMMX3DNOWIntrinsics>,
    comp_func_solid_Clear<QMMX3DNOWIntrinsics>,
    comp_func_solid_Source<QMMX3DNOWIntrinsics>,
    0,
    comp_func_solid_SourceIn<QMMX3DNOWIntrinsics>,
    comp_func_solid_DestinationIn<QMMX3DNOWIntrinsics>,
    comp_func_solid_SourceOut<QMMX3DNOWIntrinsics>,
    comp_func_solid_DestinationOut<QMMX3DNOWIntrinsics>,
    comp_func_solid_SourceAtop<QMMX3DNOWIntrinsics>,
    comp_func_solid_DestinationAtop<QMMX3DNOWIntrinsics>,
    comp_func_solid_XOR<QMMX3DNOWIntrinsics>
};

CompositionFunction qt_functionForMode_MMX3DNOW[numCompositionFunctions] = {
    comp_func_SourceOver<QMMX3DNOWIntrinsics>,
    comp_func_DestinationOver<QMMX3DNOWIntrinsics>,
    comp_func_Clear<QMMX3DNOWIntrinsics>,
    comp_func_Source<QMMX3DNOWIntrinsics>,
    0,
    comp_func_SourceIn<QMMX3DNOWIntrinsics>,
    comp_func_DestinationIn<QMMX3DNOWIntrinsics>,
    comp_func_SourceOut<QMMX3DNOWIntrinsics>,
    comp_func_DestinationOut<QMMX3DNOWIntrinsics>,
    comp_func_SourceAtop<QMMX3DNOWIntrinsics>,
    comp_func_DestinationAtop<QMMX3DNOWIntrinsics>,
    comp_func_XOR<QMMX3DNOWIntrinsics>
};

void qt_blend_color_argb_mmx3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QMMX3DNOWIntrinsics>(count, spans, userData,
                                                 (CompositionFunctionSolid*)qt_functionForModeSolid_MMX3DNOW);
}

#endif // QT_HAVE_3DNOW

QT_END_NAMESPACE
