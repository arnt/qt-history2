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

struct QMMX3DNOWIntrinsics : public QMMXCommonIntrinsics
{
    static inline void end() {
        _m_femms();
    }
};

const CompositionFunctionSolid qt_functionForModeSolid_MMX3DNOW[] = {
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
    comp_func_solid_XOR<QMMX3DNOWIntrinsics>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_MMX3DNOW[] = {
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
    comp_func_XOR<QMMX3DNOWIntrinsics>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_mmx3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QMMX3DNOWIntrinsics>(count, spans, userData,
                                                 (CompositionFunctionSolid*)qt_functionForModeSolid_MMX3DNOW);
}

#endif // QT_HAVE_3DNOW
