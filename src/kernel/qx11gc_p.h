#ifndef QX11GC_P_H
#define QX11GC_P_H

static const short ropCodes[] = {                     // ROP translation table
    GXcopy, // CopyROP
    GXor, // OrROP
    GXxor, // XorROP
    GXandInverted, // NotAndROP EraseROP
    GXcopyInverted, // NotCopyROP
    GXorInverted, // NotOrROP
    GXequiv, // NotXorROP
    GXand, // AndROP
    GXinvert, // NotROP
    GXclear, // ClearROP
    GXset, // SetROP
    GXnoop, // NopROP
    GXandReverse, // AndNotROP
    GXorReverse, // OrNotROP
    GXnand, // NandROP
    GXnor // NorROP
};

struct QX11GCPrivate {
    QX11GCPrivate()
	{
	    dpy = 0;
	    scrn = -1;
	    hd = 0;
	    rendhd = 0;
//  	    flags = Qt::IsStartingUp;
	    bg_col = Qt::white;                             // default background color
 	    bg_mode = Qt::TransparentMode;                  // default background mode
	    rop = Qt::CopyROP;                                // default ROP
	    tabstops = 0;                               // default tabbing
	    tabarray = 0;
	    tabarraylen = 0;
	    ps_stack = 0;
	    wm_stack = 0;
	    gc = gc_brush = 0;
	    pdev = 0;
	    dpy  = 0;
// 	    txop = txinv = 0;
	    penRef = brushRef = 0;
	    clip_serial = 0;
// 	    pfont = 0;
// 	    block_ext = false;
	}
    Display *dpy;
    int scrn;
    Qt::HANDLE hd;
    Qt::HANDLE rendhd;
    GC gc;
    GC gc_brush;

    QColor bg_col;
    uchar bg_mode;
    Qt::RasterOp rop;
//     uchar pu;
//     QPoint bro;
//     QFont cfont;
//     QFont *pfont; 	// font used for metrics (might be different for printers)
    QPen cpen;
    QBrush cbrush;
    QBrush bg_brush;
    QRegion crgn;
    int tabstops;
    int *tabarray;
    int tabarraylen;

    void *penRef;
    void *brushRef;
    void *ps_stack;
    void *wm_stack;
    uint clip_serial;
    QPaintDevice *pdev; // tmp - Q4PaintDevice
};

#endif // QX11GC_P_H
