/*
 *              Copyright (C) 2000  Sony Computer Entertainment Inc.
 *                              All Rights Reserved.
 */

#ifndef GSOS_H
#define GSOS_H

#include "gsostypes.h"
#include "gsosregs.h"

#ifdef __cplusplus
extern "C" {
#endif


#define GSOS_GIF_EOP_CONTINUE  0
#define GSOS_GIF_EOP_TERMINATE 1
#define GSOS_GIF_PRE_IGNORE    0
#define GSOS_GIF_PRE_ENABLE    1
#define GSOS_GIF_PRI_IIP       (1<<3)
#define GSOS_GIF_PRI_TME       (1<<4)
#define GSOS_GIF_PRI_FGE       (1<<5)
#define GSOS_GIF_PRI_ABE       (1<<6)
#define GSOS_GIF_PRI_AA1       (1<<7)
#define GSOS_GIF_PRI_FST       (1<<8)
#define GSOS_GIF_PRI_CTXT      (1<<9)
#define GSOS_GIF_PRI_FIX       (1<<10)
#define GSOS_GIF_FLG_PACKED    0
#define GSOS_GIF_FLG_REGLIST   1
#define GSOS_GIF_FLG_IMAGE     2

#define GSOS_GIF_REG_PRIM      0x0
#define GSOS_GIF_REG_RGBAQ     0x1
#define GSOS_GIF_REG_ST        0x2
#define GSOS_GIF_REG_UV        0x3
#define GSOS_GIF_REG_XYZF2     0x4
#define GSOS_GIF_REG_XYZ2      0x5
#define GSOS_GIF_REG_TEX0_1    0x6
#define GSOS_GIF_REG_TEX0_2    0x7
#define GSOS_GIF_REG_CLAMP_1   0x8
#define GSOS_GIF_REG_CLAMP_2   0x9
#define GSOS_GIF_REG_FOG       0xa
#define GSOS_GIF_REG_XYZF3     0xc
#define GSOS_GIF_REG_XYZ3      0xd
#define GSOS_GIF_REG_AD        0xe
#define GSOS_GIF_REG_NOP       0xf

#define GSOS_PRIM_POINT    0x0
#define GSOS_PRIM_LINE     0x1
#define GSOS_PRIM_LSTRIP   0x2
#define GSOS_PRIM_TRIANGLE 0x3
#define GSOS_PRIM_TSTRIP   0x4
#define GSOS_PRIM_TFAN     0x5
#define GSOS_PRIM_SPRITE   0x6

int gsosGetScreenSize(int *w, int *h);
int gsosSave();
int gsosRestore();

int gsosOpen();
int gsosClose();
void gsosSetScreen(int mode, int res, int w, int h, int fbp, int psm, int ch, int ctx, int init);
int gsosMakeGiftag( GSOSbit64 nloop, GSOSbit64 eop, GSOSbit64 pre, GSOSbit64 prim, GSOSbit64 flg, GSOSbit64 nreg, GSOSbit64 regs );
void gsosSetPacketAddrData(GSOSbit64 addr, GSOSbit64 data );
void gsosSetPacket2(GSOSbit64 data1, GSOSbit64 data2 );
void gsosSetPacket4(GSOSbit64 data1, GSOSbit64 data2, GSOSbit64 data3, GSOSbit64 data4 );
void gsosSetPacketAddrData4(int addr, int data1, int data2, int data3, int data4 );
void gsosExec();
int gsosReadImage( int x, int y, int w, int h, unsigned int bp, int bw, int psm, GSOSuchar *pPix );
int gsosWriteImage( int x, int y, int w, int h, unsigned int bp, int bw, int psm, GSOSuchar *pPix );
void gsosFlush(void) ;

#ifdef __cplusplus
}
#endif

#endif /* GSOS_H */
