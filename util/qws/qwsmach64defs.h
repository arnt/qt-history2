/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#define GEN_TEST_CNTL 0x00d0
#define FIFO_STAT 0x0310
#define BUS_CNTL 0x00a0
#define GUI_STAT 0x0338
#define MEM_VGA_WP_SEL 0x00b4
#define MEM_VGA_RP_SEL 0x00b8
#define CONTEXT_MASK 0x0320
#define DST_OFF_PITCH 0x0100
#define DST_Y_X 0x010c
#define DST_HEIGHT 0x0114
#define DST_BRES_ERR 0x0124
#define DST_BRES_INC 0x0128
#define DST_BRES_DEC 0x012c
#define SRC_OFF_PITCH 0x0180
#define SRC_Y_X 0x018c
#define SRC_HEIGHT1_WIDTH1 0x0198
#define SRC_Y_X_START 0x01a4
#define SRC_HEIGHT2_WIDTH2 0x01b0
#define SRC_CNTL 0x01b4
#define SRC_LINE_X_LEFT_TO_RIGHT 0x10
#define HOST_CNTL 0x0240
#define PAT_REG0 0x0280
#define PAT_REG1 0x0284
#define PAT_CNTL 0x0288
#define SC_LEFT 0x02a0
#define SC_TOP 0x02ac
#define SC_BOTTOM 0x02b0
#define SC_RIGHT 0x02a4
#define DP_BKGD_CLR 0x02c0
#define DP_FRGD_CLR 0x02c4
#define DP_WRITE_MASK 0x02c8
#define DP_MIX 0x02d4
#define FRGD_MIX_S 0x70000
#define BKGD_MIX_D 3
#define DP_SRC 0x02d8
#define FRGD_SRC_FRGD_CLR 0x100
#define CLR_CMP_CLR 0x0300
#define CLR_CMP_MASK 0x0304
#define CLR_CMP_CNTL 0x0308
#define DP_PIX_WIDTH 0x02d0
#define HOST_16BPP 0x40000
#define SRC_16BPP 0x400
#define DST_16BPP 4
#define BYTE_ORDER_LSB_TO_MSB 0x1000000
#define DP_CHAIN_MASK 0x02cc
#define GUI_ENGINE_ENABLE 0x100
#define BUS_HOST_ERR_ACK 0x00800000
#define BUS_FIFO_ERR_ACK 0x00200000
#define DP_FRGD_CLR 0x02c4
#define DP_SRC 0x02d8
#define BKGD_SRC_BKGD_CLR 0
#define FRGD_SRC_FRGD_CLR 0x100
#define FRGD_MIX_AVERAGE 0x170000
#define BKGD_MIX_AVERAGE 0x0000
#define MONO_SRC_ONE 0
#define DST_X 0x0104
#define DST_Y 0x0108
#define DST_HEIGHT 0x0114
#define DST_WIDTH 0x0110
#define CONFIG_CNTL 0x00dc
#define SRC_WIDTH1 0x0190
#define DST_CNTL 0x0130
#define DST_HEIGHT_WIDTH 0x0118
#define CUR_CLR0 0x0060
#define CUR_CLR1 0x0064
#define CUR_OFFSET 0x0068
#define CUR_HORZ_VERT_OFF 0x0070
#define CUR_HORZ_VERT_POSN 0x006c

#define HOST_BYTE_ALIGN 1
#define GUI_TRAJ_CNTL (0xcc*4)
#define MIX_SRC 0x0007
#define MIX_DST 0x0003

