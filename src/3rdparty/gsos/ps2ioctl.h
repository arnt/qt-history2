/*
 *              Copyright (C) 2000  Sony Computer Entertainment Inc.
 *                              All Rights Reserved.
 */

/*
 *  ps2ioctl.h
 *
 */

#ifndef _PS2IOCTL_H_
#define _PS2IOCTL_H_

#include <linux/ioctl.h>

/***************************************************************************/

struct ps2_packet {
    void *ptr;			/* pointer to GIF/VIF packet */
    int len;			/* length of GIF/VIF packet */
};

struct ps2_packetc {
    void *ptr;			/* pointer to DMA tag */
};

struct ps2_image {
    void *ptr;			/* pointer to image buffer */
    int fbp;			/* framebuffer base pointer */
    int fbw;			/* framebuffer width */
    int psm;			/* pixel format */
    int x, y;			/* coordinate of region */
    int w, h;			/* width and height of region */
};

struct ps2_gssreg {
    int reg;			/* register number */
    unsigned long long val;	/* register value */
};

struct ps2_crtmode {
    int mode;			/* 2: NTSC / 3: PAL / 0: VESA */
    int res;	/* 0: interlace / 1: non-interlace */
		/* 0: 640x480 / 1: 800x600 / 2: 1024x768 / 3: 1280x1024 */
		/*   b8-b15: frame rate (VESA only) 0=higher */
		/*   b16: interlace mode (0: FIELD / 1: FRAME) */
};

struct ps2_display {
    int ch;			/* display channel */
    int w, h;			/* width, height */
    int dx, dy;			/* pixel position */
};

struct ps2_dispfb {
    int ch;			/* display channel */
    int fbp;			/* framebuffer base pointer */
    int fbw;			/* framebuffer width */
    int psm;			/* pixel format */
    int dbx, dby;		/* pixel position */
};

struct ps2_pmode {
    int sw;			/* channel switch  0: off / 1: on */
    int aout, dout;		/* output select  0: out1 / 1: out2 / 2: off */
    int mmod, amod, slbg;
    int alp;			/* alpha value */
    int bgcolor;		/* background color */
};

struct ps2_screeninfo {
    int mode;			/* CRT mode */
    int res;			/* resolution mode */
    int w, h;			/* width, height */
    int fbp;			/* frame buffer base pointer */
    int psm;			/* pixel format */
    int ch;			/* display channel */
    int ctx;			/* drawenv context */

    int init;			/* 0: CRT / 1: dispfb / 2: no */
};

/***************************************************************************/

#define PS2EV_N_MAX		31
#define PS2EV_N_VBSTART		0
#define PS2EV_N_VBEND		1
#define PS2EV_N_SIGNAL		2
#define PS2EV_N_FINISH		3
#define PS2EV_N_HSYNC		4
#define PS2EV_N_VSYNC		5
#define PS2EV_N_EDW		6
#define PS2EV_N_EXHSYNC		7
#define PS2EV_N_EXVSYNC		8
#define PS2EV_N_GIFPACKET	9
#define PS2EV_N_VIF0PACKET	10
#define PS2EV_N_VIF1PACKET	11

#define PS2EV_VBSTART		(1 << PS2EV_N_VBSTART)
#define PS2EV_VBEND		(1 << PS2EV_N_VBEND)
#define PS2EV_SIGNAL		(1 << PS2EV_N_SIGNAL)
#define PS2EV_FINISH		(1 << PS2EV_N_FINISH)
#define PS2EV_HSYNC		(1 << PS2EV_N_HSYNC)
#define PS2EV_VSYNC		(1 << PS2EV_N_VSYNC)
#define PS2EV_EDW		(1 << PS2EV_N_EDW)
#define PS2EV_EXHSYNC		(1 << PS2EV_N_EXHSYNC)
#define PS2EV_EXVSYNC		(1 << PS2EV_N_EXVSYNC)
#define PS2EV_GIFPACKET		(1 << PS2EV_N_GIFPACKET)
#define PS2EV_VIF0PACKET	(1 << PS2EV_N_VIF0PACKET)
#define PS2EV_VIF1PACKET	(1 << PS2EV_N_VIF1PACKET)


#define PS2IOC_MAGIC		0xee
#define PS2IOC_EV_BASE		0
#define PS2IOC_GS_BASE		32
#define PS2IOC_VU_BASE		64
#define PS2IOC_IPU_BASE		128

/* interrupt functions */

#define PS2IOC_ENABLEEVENT	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 1)
#define PS2IOC_GETEVENT		_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 2)
#define PS2IOC_WAITEVENT	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 3)
#define PS2IOC_MARKEVENT	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 4)
#define PS2IOC_EVENTCOUNT	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 5)
#define PS2IOC_HSYNCACT		_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 6)
#define PS2IOC_EXHSYNCACT	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 7)
#define PS2IOC_GETHSYNC		_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 8)
#define PS2IOC_GETEXHSYNC	_IO(PS2IOC_MAGIC, PS2IOC_EV_BASE + 9)

/* GS/GIF functions */

#define PS2IOC_GIFPACKET	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 1, \
				     struct ps2_packet)
#define PS2IOC_GIFPACKETA	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 2, \
				     struct ps2_packet)
#define PS2IOC_STOREIMAGE	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 3, \
				     struct ps2_image)
#define PS2IOC_STOREIMAGEA	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 4, \
				     struct ps2_image)
#define PS2IOC_LOADIMAGE	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 5, \
				     struct ps2_image)
#define PS2IOC_LOADIMAGEA	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 6, \
				     struct ps2_image)
#define PS2IOC_SGSSREG		_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 7, \
				     struct ps2_gssreg)
#define PS2IOC_GGSSREG		_IOWR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 8, \
				      struct ps2_gssreg)
#define PS2IOC_GSRESET		_IO(PS2IOC_MAGIC, PS2IOC_GS_BASE + 9)
#define PS2IOC_SCRTMODE		_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 10, \
				     struct ps2_crtmode)
#define PS2IOC_GCRTMODE		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 11, \
				     struct ps2_crtmode)
#define PS2IOC_SDISPLAY		_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 12, \
				     struct ps2_display)
#define PS2IOC_GDISPLAY		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 13, \
				     struct ps2_display)
#define PS2IOC_SDISPFB		_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 14, \
				     struct ps2_dispfb)
#define PS2IOC_GDISPFB		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 15, \
				     struct ps2_dispfb)
#define PS2IOC_SPMODE		_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 16, \
				     struct ps2_pmode)
#define PS2IOC_GPMODE		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 17, \
				     struct ps2_pmode)
#define PS2IOC_SSCREENINFO	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 18, \
				     struct ps2_screeninfo)
#define PS2IOC_GSCREENINFO	_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 19, \
				     struct ps2_screeninfo)

#define PS2IOC_SIMT		_IO(PS2IOC_MAGIC, PS2IOC_GS_BASE + 20)
#define PS2IOC_GIMT		_IO(PS2IOC_MAGIC, PS2IOC_GS_BASE + 21)
#define PS2IOC_SDPMS		_IO(PS2IOC_MAGIC, PS2IOC_GS_BASE + 22)
#define PS2IOC_GSIGID		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 23, \
				     unsigned long)
#define PS2IOC_GLBLID		_IOR(PS2IOC_MAGIC, PS2IOC_GS_BASE + 24, \
				     unsigned long)

#define PS2IOC_GIFPACKETC	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 25, \
				     struct ps2_packet)
#define PS2IOC_GIFPACKETCA	_IOW(PS2IOC_MAGIC, PS2IOC_GS_BASE + 26, \
				     struct ps2_packet)

/* VU/VIF functions */

#define PS2IOC_VIF0PACKET	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 1, \
				     struct ps2_packet)
#define PS2IOC_VIF0PACKETA	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 2, \
				     struct ps2_packet)
#define PS2IOC_VIF1PACKET	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 3, \
				     struct ps2_packet)
#define PS2IOC_VIF1PACKETA	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 4, \
				     struct ps2_packet)
#define PS2IOC_VIF0PACKETC	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 5, \
				     struct ps2_packet)
#define PS2IOC_VIF0PACKETCA	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 6, \
				     struct ps2_packet)
#define PS2IOC_VIF1PACKETC	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 7, \
				     struct ps2_packet)
#define PS2IOC_VIF1PACKETCA	_IOW(PS2IOC_MAGIC, PS2IOC_VU_BASE + 8, \
				     struct ps2_packet)




/***************************************************************************/

#endif /* _PS2IOCTL_H_ */
