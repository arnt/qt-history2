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

#include <qcursor.h>
#include <qbitmap.h>
#include <qgfx.h>
#include "qws.h"
#include "qws_cursor.h"

static QWSCursor systemCursorTable[14];


static unsigned char cur_arrow_bits[] = {
   0x07, 0x00, 0x39, 0x00, 0xc1, 0x01, 0x02, 0x02, 0x02, 0x01, 0x82, 0x00,
   0x04, 0x01, 0x24, 0x02, 0x54, 0x04, 0x88, 0x08, 0x00, 0x11, 0x00, 0x22,
   0x00, 0x14, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};
static unsigned char mcur_arrow_bits[] = {
   0x07, 0x00, 0x3f, 0x00, 0xff, 0x01, 0xfe, 0x03, 0xfe, 0x01, 0xfe, 0x00,
   0xfc, 0x01, 0xfc, 0x03, 0xdc, 0x07, 0x88, 0x0f, 0x00, 0x1f, 0x00, 0x3e,
   0x00, 0x1c, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};

static unsigned char cur_up_arrow_bits[] = {
   0x80, 0x00, 0x40, 0x01, 0x40, 0x01, 0x20, 0x02, 0x20, 0x02, 0x10, 0x04,
   0x10, 0x04, 0x08, 0x08, 0x78, 0x0f, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0, 0x01};
static unsigned char mcur_up_arrow_bits[] = {
   0x80, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0xe0, 0x03, 0xe0, 0x03, 0xf0, 0x07,
   0xf0, 0x07, 0xf8, 0x0f, 0xf8, 0x0f, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01};

static unsigned char cur_cross_bits[] = {
   0xc0, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x7f, 0x7f, 0x01, 0x40, 0x7f, 0x7f, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x40, 0x01, 0x40, 0x01, 0xc0, 0x01, 0x00, 0x00};
static unsigned char mcur_cross_bits[] = {
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0x00, 0x00};

static uchar cur_ibeam_bits[] = {
   0x00, 0x00, 0xe0, 0x03, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
   0x80, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00 };
static uchar mcur_ibeam_bits[] = {
   0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0x00, 0x00 };

static uchar cur_ver_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
    0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f,
    0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00 };
static uchar mcur_ver_bits[] = {
    0x00, 0x00, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f,
    0xfc, 0x7f, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x7f, 0xf8, 0x3f,
    0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03 };

static uchar cur_hor_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x30, 0x18,
    0x38, 0x38, 0xfc, 0x7f, 0xfc, 0x7f, 0x38, 0x38, 0x30, 0x18, 0x20, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar mcur_hor_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x60, 0x0c, 0x70, 0x1c, 0x78, 0x3c,
    0xfc, 0x7f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfc, 0x7f, 0x78, 0x3c,
    0x70, 0x1c, 0x60, 0x0c, 0x40, 0x04, 0x00, 0x00 };
static uchar cur_bdiag_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e,
    0x00, 0x37, 0x88, 0x23, 0xd8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xf8, 0x00,
    0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar mcur_bdiag_bits[] = {
    0x00, 0x00, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7f, 0x00, 0x7e, 0x04, 0x7f,
    0x8c, 0x7f, 0xdc, 0x77, 0xfc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01,
    0xfc, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00 };
static uchar cur_fdiag_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00,
    0xf8, 0x00, 0xd8, 0x01, 0x88, 0x23, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c,
    0x00, 0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00 };
static uchar mcur_fdiag_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00,
    0xfc, 0x41, 0xfc, 0x63, 0xdc, 0x77, 0x8c, 0x7f, 0x04, 0x7f, 0x00, 0x7e,
    0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x00 };
static uchar cur_blank_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };



static uchar wait_data_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0x04, 0x40, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x08, 0x20, 0x00,
   0x00, 0x08, 0x20, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x08, 0x20, 0x00,
   0x00, 0x50, 0x15, 0x00, 0x00, 0xa0, 0x0a, 0x00, 0x00, 0x40, 0x05, 0x00,
   0x00, 0x80, 0x02, 0x00, 0x00, 0x40, 0x04, 0x00, 0x00, 0x20, 0x08, 0x00,
   0x00, 0x10, 0x10, 0x00, 0x00, 0x08, 0x21, 0x00, 0x00, 0x88, 0x22, 0x00,
   0x00, 0x48, 0x25, 0x00, 0x00, 0xa8, 0x2a, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0x04, 0x40, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar wait_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf0, 0x1f, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xc0, 0x07, 0x00,
   0x00, 0x80, 0x03, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0x00,
   0x00, 0xf0, 0x1f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uchar vsplit_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
    0x00, 0x41, 0x82, 0x00, 0x80, 0x41, 0x82, 0x01, 0xc0, 0x7f, 0xfe, 0x03,
    0x80, 0x41, 0x82, 0x01, 0x00, 0x41, 0x82, 0x00, 0x00, 0x40, 0x02, 0x00,
    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar vsplitm_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe3, 0xc7, 0x00,
    0x80, 0xe3, 0xc7, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
    0xc0, 0xff, 0xff, 0x03, 0x80, 0xe3, 0xc7, 0x01, 0x00, 0xe3, 0xc7, 0x00,
    0x00, 0xe2, 0x47, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar hsplit_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
    0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar hsplitm_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
    0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
    0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
    0x80, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
    0x00, 0xc0, 0x01, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
    0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar phand_bits[] = {
    0x00, 0x00, 0x00, 0x00,	0xfe, 0x01, 0x00, 0x00,	0x01, 0x02, 0x00, 0x00,
    0x7e, 0x04, 0x00, 0x00,	0x08, 0x08, 0x00, 0x00,	0x70, 0x08, 0x00, 0x00,
    0x08, 0x08, 0x00, 0x00,	0x70, 0x14, 0x00, 0x00,	0x08, 0x22, 0x00, 0x00,
    0x30, 0x41, 0x00, 0x00,	0xc0, 0x20, 0x00, 0x00,	0x40, 0x12, 0x00, 0x00,
    0x80, 0x08, 0x00, 0x00,	0x00, 0x05, 0x00, 0x00,	0x00, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00 };
static uchar phandm_bits[] = {
    0xfe, 0x01, 0x00, 0x00,	0xff, 0x03, 0x00, 0x00,	0xff, 0x07, 0x00, 0x00,
    0xff, 0x0f, 0x00, 0x00,	0xfe, 0x1f, 0x00, 0x00,	0xf8, 0x1f, 0x00, 0x00,
    0xfc, 0x1f, 0x00, 0x00,	0xf8, 0x3f, 0x00, 0x00,	0xfc, 0x7f, 0x00, 0x00,
    0xf8, 0xff, 0x00, 0x00,	0xf0, 0x7f, 0x00, 0x00,	0xe0, 0x3f, 0x00, 0x00,
    0xc0, 0x1f, 0x00, 0x00,	0x80, 0x0f, 0x00, 0x00,	0x00, 0x07, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00 };

static uchar size_all_data_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x81, 0x40, 0x00, 0x80, 0x81, 0xc0, 0x00,
   0xc0, 0xff, 0xff, 0x01, 0x80, 0x81, 0xc0, 0x00, 0x00, 0x81, 0x40, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uchar size_all_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc2, 0x21, 0x00,
   0x00, 0xc3, 0x61, 0x00, 0x80, 0xc3, 0xe1, 0x00, 0xc0, 0xff, 0xff, 0x01,
   0xe0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x01, 0x80, 0xc3, 0xe1, 0x00,
   0x00, 0xc3, 0x61, 0x00, 0x00, 0xc2, 0x21, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


void QWSServer::initializeCursor()
{
    // setup system cursors

    // 16x16 cursors
    systemCursorTable[ArrowCursor].set(cur_arrow_bits,
				    mcur_arrow_bits, 16, 16, 0, 0);
    systemCursorTable[UpArrowCursor].set(cur_up_arrow_bits,
				    mcur_up_arrow_bits, 16, 16, 7, 0);
    systemCursorTable[CrossCursor].set(cur_cross_bits,
				    mcur_cross_bits, 16, 16, 7, 7);
    systemCursorTable[IBeamCursor].set(cur_ibeam_bits,
				    mcur_ibeam_bits, 16, 16, 7, 7);
    systemCursorTable[SizeVerCursor].set(cur_ver_bits,
				    mcur_ver_bits, 16, 16, 7, 7);
    systemCursorTable[SizeHorCursor].set(cur_hor_bits,
				    mcur_hor_bits, 16, 16, 7, 7);
    systemCursorTable[SizeBDiagCursor].set(cur_bdiag_bits,
				    mcur_bdiag_bits, 16, 16, 7, 7);
    systemCursorTable[SizeFDiagCursor].set(cur_fdiag_bits,
				    mcur_fdiag_bits, 16, 16, 7, 7);
    systemCursorTable[BlankCursor].set(cur_blank_bits,
				    cur_blank_bits, 16, 16, 0, 0);

    // 32x32 cursors
    systemCursorTable[WaitCursor].set(wait_data_bits,
				    wait_mask_bits, 32, 32, 15, 15);
    systemCursorTable[SplitVCursor].set(vsplit_bits,
				    vsplitm_bits, 32, 32, 15, 15);
    systemCursorTable[SplitHCursor].set(hsplit_bits,
				    hsplitm_bits, 32, 32, 15, 15);
    systemCursorTable[SizeAllCursor].set(size_all_data_bits,
				    size_all_mask_bits, 32, 32, 15, 15);
    systemCursorTable[PointingHandCursor].set(phand_bits,
				    phandm_bits, 32, 32, 0, 0);
}

QWSCursor *QWSCursor::systemCursor(int id)
{
    QWSCursor *cursor = 0;

    if (id >= 0 && id <= LastCursor) {
	cursor = &systemCursorTable[id];
    }

    if (cursor == 0) {
	cursor = &systemCursorTable[ArrowCursor];
    }

    return cursor;
}

void QWSCursor::set(const uchar *data, const uchar *mask,
		    int width, int height, int hx, int hy)
{
    hot.setX(hx);
    hot.setY(hy);

    cursor.create(width, height, 8, 3);
    cursor.setColor(0, 0xff000000);
    cursor.setColor(1, 0xffffffff);
    cursor.setColor(2, 0x00000000);

    int bytesPerLine = (width + 7) / 8;
    int p = 0;
    int d, m;

    int x = -1, w = 0;

    for (int i = 0; i < height; i++)
    {
	for (int j = 0; j < bytesPerLine; j++, data++, mask++)
	{
	    for (int b = 0; b < 8; b++)
	    {
		d = *data & (1 << b);
		m = *mask & (1 << b);
		if (d && m) p = 0;
		else if (!d && m) p = 1;
		else p = 2;
		cursor.setPixel(j*8+b, i, p);

		// calc region
		if (x < 0 && m)
		    x = j*8+b;
		else if (x >= 0 && !m) {
		    rgn = rgn.unite(QRect(x, i, w, 1));
		    x = -1;
		    w = 0;
		}
		if (m)
		    w++;
	    }
	}
	if (x >= 0) {
	    rgn = rgn.unite(QRect(x, i, w, 1));
	    x = -1;
	    w = 0;
	}
    }
}

