#if defined(Q_CC_GNU) && defined(__i386__)
enum CPUFeatures {
    None = 0,
    MMX = 0x1,
    SSE = 0x2,
    SSE2 = 0x4,
    CMOV = 0x8
};
static uint detectCPUFeatures() {
    uint result;
    /* see p. 118 of amd64 instruction set manual Vol3 */
    asm ("push %%ebx\n"
         "pushf\n"
         "pop %%eax\n"
         "mov %%eax, %%ebx\n"
         "xor $0x00200000, %%eax\n"
         "push %%eax\n"
         "popf\n"
         "pushf\n"
         "pop %%eax\n"
         "mov $0x0, %%edx\n"
         "xor %%ebx, %%eax\n"
         "jz 1f\n"

         "mov $0x00000001, %%eax\n"
         "cpuid\n"
         "1:\n"
         "pop %%ebx\n"
         "mov %%edx, %0\n"
        : "=r" (result)
        :
        : "%eax", "%ecx", "%edx"
        );

    uint features = 0;
    // result now contains the standard feature bits
    if (result & (1 << 15))
        features |= CMOV;
    if (result & (1 << 23))
        features |= MMX;
    if (result & (1 << 25))
        features |= SSE;
    if (result & (1 << 26))
        features |= SSE2;
    return features;
}

static void blend_color_sse(ARGB *target, const QSpan *span, ARGB color)
{
    if (!span->len)
        return;

    int alpha = qt_div_255(color.a * span->coverage);
    int pr = alpha * color.r;
    int pg = alpha * color.g;
    int pb = alpha * color.b;

    int rev_alpha = 255 - alpha;

    const ushort pm[4]
        = { (ushort)pb, (ushort)pg, (ushort)pr, 0 };
    const ushort mask[4]
        = { 0, 0, 0, 0xffff };
    if (span->len > 1 ) {
        /*
          registers:
          xmm0: premultiplied src
          xmm1: rev_alpha
          xmm2: *target
          xmm7: 0
          xmm6: alpha mask
        */
        asm("pxor %%xmm7, %%xmm7\n" // clear xmm7
            "movlps %2, %%xmm0\n" // src to xmm0
            "movlhps %%xmm0, %%xmm0\n"
            "movlps %5, %%xmm6\n" // src to xmm0
            "movlhps %%xmm6, %%xmm6\n"
            "movd %3, %%xmm1\n"  // rev_alpha to xmm1
            // #### should work without the line below
            "punpcklbw %%xmm7, %%xmm1\n"
            "pshuflw $0, %%xmm1, %%xmm1\n"
            "movlhps %%xmm1, %%xmm1\n" // spread rev_alpha over all channels
            "1:\n"
            "prefetchnta 128(%1)\n"
            "movlps (%1), %%xmm2\n" // target to xmm2
            "punpcklbw %%xmm7, %%xmm2\n" //  to xmm1
            "pmullw %%xmm1, %%xmm2\n" // target * ralpha
            "paddw %%xmm0, %%xmm2\n" // sum to xmm1
            "por %%xmm6, %%xmm2\n" // make sure alpha is set to 0xff
            "psrlw $8, %%xmm2\n" // shift right
            "packuswb %%xmm2, %%xmm2\n" // pack to 8 bits
            "movlps %%xmm2, (%1)\n"
            "add $8, %1\n"
            "dec %4\n"
            "jnz 1b\n"
            "mov %1, %0\n"
            : "=m" (target)
            : "r" (target),
              "m" (*pm),
              "r" (rev_alpha),
              "r" (span->len/2),
              "m" (*mask)
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
            );
    }
    if (span->len % 2) {
        qt_alpha_pixel_pm(pr, target->r, rev_alpha);
        qt_alpha_pixel_pm(pg, target->g, rev_alpha);
        qt_alpha_pixel_pm(pb, target->b, rev_alpha);
        target->a = 255;
    }
}

#define CMOV_PIX(pixel, out, mask, image_bits, offset) \
    asm ("mov %1, %%edx\n"                             \
         "and $"#mask",%%edx\n"                         \
         "cmovnz (%2,%3,0x4), %%edx\n"                  \
         "mov %%edx, %0\n"                             \
         : "=m" (pixel)                                \
         : "r" (out),                                  \
           "r" (image_bits),                           \
           "r" (offset)                                \
         : "%edx"                             \
        )

static void blend_transformed_bilinear_sse(ARGB *target, const QSpan *span,
                                           qreal ix, qreal iy, qreal dx, qreal dy,
                                           ARGB *image_bits, int image_width, int image_height)
{
    const int fixed_scale = 1 << 16;
    int x = int((ix + dx * span->x) * fixed_scale);
    int y = int((iy + dy * span->x) * fixed_scale);

    int fdx = (int)(dx * fixed_scale);
    int fdy = (int)(dy * fixed_scale);

    /*
      set up constant xmm registers:
      xmm7 : 0
      xmm6 : coverage
      xmm5 : alpha mask on xmm5
      xmm4 : 0x00ff...
    */
    {
        const ushort mask[4] = { 0, 0, 0, 0xffff };
        uint coverage = span->coverage;
        asm("pxor %%xmm7, %%xmm7\n" // clear xmm7
            "movd %0, %%xmm6\n"  // coverage to xmm6
            "punpcklbw %%xmm7, %%xmm6\n"
            "pshuflw $0, %%xmm6, %%xmm6\n" // spread over all channels
            "movlps %1, %%xmm5\n" // mask to xmm5
            "movlhps %%xmm5, %%xmm5\n"
            "movd %%xmm6, %0\n"
            "pcmpeqb %%xmm4, %%xmm4\n"
            "psrlw $8, %%xmm4\n" // 0x255 in xmm4
            :
            : "r" (coverage),
              "m" (*mask)
            : "%xmm5", "%xmm6", "%xmm7"
            );
    }
    for (int i = 0; i < span->len; ++i) {
        const int x1 = (x >> 16);
        const int y1 = (y >> 16);

        const int distx = ((x - (x1 << 16)) >> 8);
        const int disty = ((y - (y1 << 16)) >> 8);
        const int idistx = 256 - distx;
        const int idisty = 256 - disty;

        const long y1_offset = y1 * image_width;
        const long y2_offset = y1_offset + image_width;

        struct {
            uint tl;
            uint bl;
        } left;
        struct {
            uint tr;
            uint br;
        } right;

        {
            const int x2 = x1 + 1;
            const int y2 = y1 + 1;
            enum {
                X1Out = 0x1,
                X2Out = 0x2,
                Y1Out = 0x4,
                Y2Out = 0x8
            };
            register const uint out = (x1 >= 0 & x1 < image_width)
                                      | ((x2 >= 0 & x2 < image_width) << 1)
                                      | ((y1 >= 0 & y1 < image_height) << 2)
                                      | ((y2 >= 0 & y2 < image_height) << 3);
            CMOV_PIX(left.tl, out, 0x5, image_bits, y1_offset + x1); // X1Out|Y2Out
            CMOV_PIX(left.bl, out, 0x8, image_bits, y2_offset + x1); // X1Out|Y2Out
            CMOV_PIX(right.tr, out, 0x6, image_bits, y1_offset + x2); // X2Out|Y1Out
            CMOV_PIX(right.br, out, 0xa, image_bits, y2_offset + x2); // X2Out|Y2Out
        }
        /*
          tl, bl : xmm0
          tr: xmm1 low
          br : xmm1 high
          distx, idistx :  xmm2
          idistx, idisty: xmm3
          scratch: xmm6
          zero: xmm7
        */

        asm("movlps %0, %%xmm0\n" // left to xmm0
            "punpcklbw %%xmm7, %%xmm0\n"

            "movlps %1, %%xmm1\n"
            "punpcklbw %%xmm7, %%xmm1\n"

            "movd %2, %%xmm2\n"
            "pshuflw $0, %%xmm2, %%xmm2\n"
            "movlhps %%xmm2, %%xmm2\n" // spread distx
            "pmullw %%xmm2, %%xmm1\n"

            "movd %3, %%xmm2\n"
            "pshuflw $0, %%xmm2, %%xmm2\n"
            "movlhps %%xmm2, %%xmm2\n" // spread distx
            "pmullw %%xmm2, %%xmm0\n"

            "paddw %%xmm1, %%xmm0\n" // now contains xtop and xbottom
            "psrlw $8, %%xmm0\n"

            "movd %4, %%xmm2\n"
            "pshuflw $0, %%xmm2, %%xmm2\n"
            "movd %5, %%xmm3\n"
            "pshuflw $0, %%xmm3, %%xmm3\n"
            "movlhps %%xmm2, %%xmm3\n" // disty and idisty in mm2

            "pmullw %%xmm3, %%xmm0\n"

            "movhlps %%xmm0, %%xmm1\n"
            "paddw %%xmm1, %%xmm0\n"
            "psrlw $8, %%xmm0\n" // src is now in xmm0, ready for blend

            // blend operation follows
            /*
              src already in xmm0
              target in xmm1
              alpha of src in xmm2
              rev alpha in xmm5
            */
            "movd (%6), %%xmm1\n" // target to mm1
            "punpcklbw %%xmm7, %%xmm1\n"
            "pshuflw $255, %%xmm0, %%xmm2\n" // spread alpha over all channels
            "pmullw %%xmm6, %%xmm2\n" // alpha *= coverage
            "psrlw $8, %%xmm2\n" // shift right
            "pmullw %%xmm2, %%xmm0\n" // src *= alpha
            "movdqa %%xmm4, %%xmm3\n"
            "psubw %%xmm2, %%xmm3\n" // 0x255 - alpha in xmm3
            "pmullw %%xmm3, %%xmm1\n" // target *= ralpha
            "paddw %%xmm1, %%xmm0\n" // sum to xmm1
            "por %%xmm5, %%xmm0\n" // make sure alpha is set to 0xff
            "psrlw $8, %%xmm0\n" // shift right
            "packuswb %%xmm0, %%xmm0\n" // pack to 8 bits
            "movd %%xmm0, (%6)\n"
            :
            : "m" (left),
              "m" (right),
              "r" (distx),
              "r" (idistx),
              "r" (disty),
              "r" (idisty),
              "r" (target)
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
            );
//         qDebug("target = %x", *((uint *)target));
        x += fdx;
        y += fdy;
        ++target;
    }
}

#elif defined Q_CC_MSVC


#endif // Q_CC_GCC and Q_CC_MSVC

void qInitDrawhelperAsm()
{
    static uint features = 0;
    if (features)
        return;

#if defined (Q_CC_GNU) && defined (__i386__)
    features = detectCPUFeatures();

    if (features & SSE2) {
        dh[DrawHelper_RGB32]->blendColor = blend_color_sse;
        dh[DrawHelper_RGB32]->blendTransformedBilinear = blend_transformed_bilinear_sse;
    }
#else
    Q_UNUSED(dh)
#endif
}
