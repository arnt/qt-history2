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

#ifdef QT_HAVE_IWMMXT

#include <mmintrin.h>
#include <private/qdrawhelper_sse_p.h>

#ifndef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) \
 (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))
#endif

struct QIWMMXTIntrinsics : public QMMXCommonIntrinsics
{
    static inline m64 alpha(m64 x) {
        return _mm_shuffle_pi16 (x, _MM_SHUFFLE(3, 3, 3, 3));
    }

    static inline m64 _load_alpha(uint x, const m64 &mmx_0x0000) {
        m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
        return _mm_shuffle_pi16(t, _MM_SHUFFLE(0, 0, 0, 0));
    }

    static inline m64 load_alpha(uint x) {
        return _load_alpha(x, _mm_setzero_si64());
    }

    static inline void end() {
    }
};

const CompositionFunctionSolid qt_functionForModeSolid_IWMMXT[] = {
    comp_func_solid_SourceOver<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationOver<QIWMMXTIntrinsics>,
    comp_func_solid_Clear<QIWMMXTIntrinsics>,
    comp_func_solid_Source<QIWMMXTIntrinsics>,
    0,
    comp_func_solid_SourceIn<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationIn<QIWMMXTIntrinsics>,
    comp_func_solid_SourceOut<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationOut<QIWMMXTIntrinsics>,
    comp_func_solid_SourceAtop<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationAtop<QIWMMXTIntrinsics>,
    comp_func_solid_XOR<QIWMMXTIntrinsics>,
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

const CompositionFunction qt_functionForMode_IWMMXT[] = {
    comp_func_SourceOver<QIWMMXTIntrinsics>,
    comp_func_DestinationOver<QIWMMXTIntrinsics>,
    comp_func_Clear<QIWMMXTIntrinsics>,
    comp_func_Source<QIWMMXTIntrinsics>,
    0,
    comp_func_SourceIn<QIWMMXTIntrinsics>,
    comp_func_DestinationIn<QIWMMXTIntrinsics>,
    comp_func_SourceOut<QIWMMXTIntrinsics>,
    comp_func_DestinationOut<QIWMMXTIntrinsics>,
    comp_func_SourceAtop<QIWMMXTIntrinsics>,
    comp_func_DestinationAtop<QIWMMXTIntrinsics>,
    comp_func_XOR<QIWMMXTIntrinsics>,
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

void qt_blend_color_argb_iwmmxt(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QIWMMXTIntrinsics>(count, spans, userData,
                                               (CompositionFunctionSolid*)qt_functionForModeSolid_IWMMXT);
}

#endif // QT_HAVE_IWMMXT
