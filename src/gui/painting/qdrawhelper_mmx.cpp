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

#if defined(QT_HAVE_MMX)

#include <private/qdrawhelper_mmx_p.h>

QT_BEGIN_NAMESPACE

CompositionFunctionSolid qt_functionForModeSolid_MMX[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QMMXIntrinsics>,
    comp_func_solid_DestinationOver<QMMXIntrinsics>,
    comp_func_solid_Clear<QMMXIntrinsics>,
    comp_func_solid_Source<QMMXIntrinsics>,
    0,
    comp_func_solid_SourceIn<QMMXIntrinsics>,
    comp_func_solid_DestinationIn<QMMXIntrinsics>,
    comp_func_solid_SourceOut<QMMXIntrinsics>,
    comp_func_solid_DestinationOut<QMMXIntrinsics>,
    comp_func_solid_SourceAtop<QMMXIntrinsics>,
    comp_func_solid_DestinationAtop<QMMXIntrinsics>,
    comp_func_solid_XOR<QMMXIntrinsics>
};

CompositionFunction qt_functionForMode_MMX[numCompositionFunctions] = {
    comp_func_SourceOver<QMMXIntrinsics>,
    comp_func_DestinationOver<QMMXIntrinsics>,
    comp_func_Clear<QMMXIntrinsics>,
    comp_func_Source<QMMXIntrinsics>,
    0,
    comp_func_SourceIn<QMMXIntrinsics>,
    comp_func_DestinationIn<QMMXIntrinsics>,
    comp_func_SourceOut<QMMXIntrinsics>,
    comp_func_DestinationOut<QMMXIntrinsics>,
    comp_func_SourceAtop<QMMXIntrinsics>,
    comp_func_DestinationAtop<QMMXIntrinsics>,
    comp_func_XOR<QMMXIntrinsics>
};

void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QMMXIntrinsics>(count, spans, userData,
                                            (CompositionFunctionSolid*)qt_functionForModeSolid_MMX);
}

#endif // QT_HAVE_MMX

QT_END_NAMESPACE
