/*
 *              Copyright (C) 2000  Sony Computer Entertainment Inc.
 *                              All Rights Reserved.
 */
#ifndef _include_gsosregs_h_
#define _include_gsosregs_h_


/*****************************************************************
 *  This is a definition file for GPU2 hardware register ports
 * and values. This file is put by a documet 'GPU2 Functional
 * Specification Rev 1.4  Chap-4. Register set'.
 *
 *		update for Rev 1.4 at Mon Jul 14 18:15:33 JST 1997
 *              take@krc.sony.co.jp
 *****************************************************************
 */
 
/*****************************************************************
 *  This is a definition file for GPU2 hardware register ports
 * and values. This file is put by a documet 'GPU2 Functional
 * Specification Rev 1.1  Chap-4. Register set'.
 *
 *		update for Rev 1.1 at Wed Nov 13 16:59:47 JST 1996
 *
 *  Defined types and values are as follow. All register name are
 * same as the spec. document, bus added 'Gpu2' or 'GPU2_'.
 *
 *    Gpu2RegAddr:    type of register address.
 *    Gpu2XXXData:    macro for making full reg. data value.
 *    Gpu2XXXYYY:     macro for getting particular data fron full reg. data.
 *    Gpu2RegZZZ:     data value define.
 *
 *                    XXX: reg. name
 *                    YYY: particular data name on the reg.
 */

/*****************************************************************
 *  GPU2 Register Address Definition
 *****************************************************************
 */
/*---- Register Address Map --------------------------------*/
enum GsosRegAddr {

	/*-- vertex info. reg --*/
    GSOS_PRIM			= 0x000,
	GSOS_RGBAQ			= 0x001,
	GSOS_ST				= 0x002,
	GSOS_UV				= 0x003,
	GSOS_XYZF2			= 0x004,
	GSOS_XYZ2           = 0x005,
	GSOS_XYZF			= 0x00a,
	GSOS_XYZF3			= 0x00c,
	GSOS_XYZ3			= 0x00d,
	GSOS_RGBAQ2			= 0x011,
	GSOS_ST2			= 0x012,
	GSOS_UV2			= 0x013,
	GSOS_XYOFFSET_1		= 0x018,
	GSOS_XYOFFSET_2		= 0x019,
	GSOS_PRMODECONT		= 0x01a,

	/*-- drawing attribute reg. --*/
	GSOS_PRMODE			= 0x01b,
	GSOS_TEX0_1			= 0x006,
	GSOS_TEX0_2			= 0x007,
	GSOS_TEX1_1			= 0x014,
	GSOS_TEX1_2			= 0x015,
	GSOS_TEX2_1			= 0x016,
	GSOS_TEX2_2			= 0x017,
	GSOS_TEXCLUT        = 0x01c,
	GSOS_SCANMSK		= 0x022,
	GSOS_MIPTBP1_1		= 0x034,
	GSOS_MIPTBP1_2		= 0x035,
	GSOS_MIPTBP2_1		= 0x036,
	GSOS_MIPTBP2_2		= 0x037,
	GSOS_CLAMP_1		= 0x008,
	GSOS_CLAMP_2		= 0x009,
	GSOS_TEXA			= 0x03b,
	GSOS_FOGCOL			= 0x03d,
	GSOS_CACHEINVLD		= 0x03f,

	/*-- pixel operation reg. --*/
	GSOS_SCISSOR_1		= 0x040,
	GSOS_SCISSOR_2		= 0x041,
	GSOS_ALPHA_1		= 0x042,
	GSOS_ALPHA_2		= 0x043,
	GSOS_DIMX			= 0x044,
	GSOS_DTHE			= 0x045,
	GSOS_COLCLAMP		= 0x046,
	GSOS_TEST_1			= 0x047,
	GSOS_TEST_2			= 0x048,
	GSOS_PABE			= 0x049,
	GSOS_FBA_1			= 0x04a,
	GSOS_FBA_2			= 0x04b,

	/*-- buffer reg. --*/
	GSOS_FRAME_1		= 0x04c,
	GSOS_FRAME_2		= 0x04d,
	GSOS_ZBUF_1			= 0x04e,
	GSOS_ZBUF_2			= 0x04f,

	/*-- inter-buffer transfer reg. --*/
	GSOS_BITBLTBUF      = 0x050,
	GSOS_TRXPOS			= 0x051,
	GSOS_TRXREG			= 0x052,
	GSOS_TRXDIR			= 0x053,
	GSOS_HWREG			= 0x054,

	/*-- other reg. --*/
	GSOS_SIGNAL			= 0x060,
	GSOS_FINISH			= 0x061,

	/*-- PCRTC reg. (privilege port) --*/
	GSOS_PMODE			= 0x080,
	GSOS_SMODE1			= 0x081,
        GSOS_SMODE2                     = 0x082,    
        GSOS_RFSH                       = 0x083,
	GSOS_SYNCH1			= 0x084,
	GSOS_SYNCH2			= 0x085,
	GSOS_SYNCV			= 0x086,
	GSOS_DISPFB1			= 0x087,
	GSOS_DISPLAY1			= 0x088,
	GSOS_DISPFB2			= 0x089,
	GSOS_DISPLAY2			= 0x08a,
	GSOS_EXTBUF			= 0x08b,
	GSOS_EXTDATA			= 0x08c,
	GSOS_EXTWRITE			= 0x08d,
	GSOS_BGCOLOR			= 0x08e,

	/*-- host I/F reg. (privilege port) --*/
	GSOS_CSR			= 0x0c0,
	GSOS_IMR			= 0x0c1,
	GSOS_BUSDIR			= 0x0c4,
	GSOS_SIGID			= 0x0c8,
        GSOS_LABELID                    = 0x0c9,
        GSOS_SYSCNT                     = 0x0cf,
        /*-- pseudo register for simulation */
        GSOS_SIMU_DISPLAY = 0x100, 
        GSOS_SIMU_PCRTC   = 0x101 
} ;

 
/*****************************************************************
 */

#define GsM(l) (unsigned)( ((GSOSbit64)1 << (l)) - 1)
#define GsLLong(d,s,m) (GSOSbit64)((GSOSbit64)((d)&GsM((m)))<<(s))
#define GsLL2I(d,s,m) (unsigned)(((unsigned)((GSOSbit64)(d)>>(s)))&GsM((m)))

/*****************************************************************
 */

#define GsosPrimData( prim, iip, tme, fge, abe, aa1, fst, ctxt, fix ) \
    GsLLong((prim),0,3) | GsLLong((iip),3,1) | GsLLong((tme),4,1) | \
    GsLLong((fge),5,3) | GsLLong((abe),6,1) | GsLLong((aa1),7,1) | \
    GsLLong((fst),8,3) | GsLLong((ctxt),9,1) | GsLLong((fix),10,1)
#define GsosPrimPrim( d ) (unsigned)(GsLL2I((d),0,3))
#define GsosPrimIip( d ) (unsigned)(GsLL2I((d),3,1))
#define GsosPrimTme( d ) (unsigned)(GsLL2I((d),4,1))
#define GsosPrimFge( d ) (unsigned)(GsLL2I((d),5,1))
#define GsosPrimAbe( d ) (unsigned)(GsLL2I((d),6,1))
#define GsosPrimAa1( d ) (unsigned)(GsLL2I((d),7,1))
#define GsosPrimFst( d ) (unsigned)(GsLL2I((d),8,1))
#define GsosPrimCtxt( d ) (unsigned)(GsLL2I((d),9,1))
#define GsosPrimFix( d ) (unsigned)(GsLL2I((d),10,1))

#define GsosXyzfData( x, y, z, f ) \
    GsLLong((x),0,16) | GsLLong((y),16,16) | \
    GsLLong((z),32,24) | GsLLong((f),56,8)
#define GsosXyzfX( d ) (unsigned)(GsLL2I((d),0,16))
#define GsosXyzfY( d ) (unsigned)(GsLL2I((d),16,16))
#define GsosXyzfZ( d ) (unsigned)(GsLL2I((d),32,24))
#define GsosXyzfF( d ) (unsigned)(GsLL2I((d),56,8))

#define GsosRgbaqData( r, g, b, a, q ) \
    GsLLong((r),0,8) | GsLLong((g),8,8) | GsLLong((b),16,8) | \
    GsLLong((a),24,8) | GsLLong((q),32,32)
#define GsosRgbaqR( d ) (unsigned)(GsLL2I((d),0,8))
#define GsosRgbaqG( d ) (unsigned)(GsLL2I((d),8,8))
#define GsosRgbaqB( d ) (unsigned)(GsLL2I((d),16,8))
#define GsosRgbaqA( d ) (unsigned)(GsLL2I((d),24,8))
#define GsosRgbaqQ( d ) (unsigned)(GsLL2I((d),32,32))

#define GsosStData( s, t ) \
    GsLLong((s),0,32) | GsLLong((t),32,32)
#define GsosStS( d ) (unsigned)(GsLL2I((d),0,32))
#define GsosStT( d ) (unsigned)(GsLL2I((d),32,32))

#define GsosUvData( u, v ) \
    GsLLong((u),0,14) | GsLLong((v),16,14)
#define GsosUvU( d ) (unsigned)(GsLL2I((d),0,14))
#define GsosUvV( d ) (unsigned)(GsLL2I((d),16,14))

#define GsosXyoffsetData( xoffset, yoffset ) \
    GsLLong((xoffset),0,16) | GsLLong((yoffset),32,16) 
#define GsosXyoffsetX( d ) (unsigned)(GsLL2I((d),0,16))
#define GsosXyoffsetY( d ) (unsigned)(GsLL2I((d),32,16))

#define GsosXyzData( x, y, z ) \
    GsLLong((x),0,16) | GsLLong((y),16,16) | GsLLong((z),32,32)
#define GsosXyzX( d ) (unsigned)(GsLL2I((d),0,16))
#define GsosXyzY( d ) (unsigned)(GsLL2I((d),16,16))
#define GsosXyzZ( d ) (unsigned)(GsLL2I((d),32,32))

#define GsosPrmodecontData( ac ) GsLLong((ac),0,1) 
#define GsosPrmodecontAc( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosPrmodeData( iip, tme, fge, abe, aa1, fst, ctxt, fix ) \
    GsLLong((iip),3,1) | GsLLong((tme),4,1) | GsLLong((fge),5,1) | \
    GsLLong((abe),6,1) | GsLLong((aa1),7,1) | GsLLong((fst),8,1) | \
    GsLLong((ctxt),9,1) | GsLLong((fix),10,1)
#define GsosPrmodeIip( d ) (unsigned)(GsLL2I(d,3,1))
#define GsosPrmodeTme( d ) (unsigned)(GsLL2I(d,4,1))
#define GsosPrmodeFge( d ) (unsigned)(GsLL2I(d,5,1))
#define GsosPrmodeAbe( d ) (unsigned)(GsLL2I(d,6,1))
#define GsosPrmodeAa1( d ) (unsigned)(GsLL2I(d,7,1))
#define GsosPrmodeFst( d ) (unsigned)(GsLL2I(d,8,1))
#define GsosPrmodeCtxt( d ) (unsigned)(GsLL2I(d,9,1))
#define GsosPrmodeFix( d ) (unsigned)(GsLL2I(d,10,1))

#define GsosScanmskData( msk ) GsLLong((msk),0,2) 
#define GsosScanmskMsk( d ) (unsigned)(GsLL2I(d,0,2))

#define GsosTex0Data( tbp0,tbw,psm,tw,th,tcc,tfx,cbp,cpsm,csm,csa,cld ) \
    GsLLong((tbp0),0,14) | GsLLong((tbw),14,6) | GsLLong((psm),20,6) | \
    GsLLong((tw),26,4) | GsLLong((th),30,4) | GsLLong((tcc),34,1) | \
    GsLLong((tfx),35,2) | GsLLong((cbp),37,14) | GsLLong((cpsm),51,4) | \
    GsLLong((csm),55,1) | GsLLong((csa),56,5) | GsLLong((cld),61,3) 
#define GsosTex0Tbp0( d ) (unsigned)(GsLL2I(d,0,14))
#define GsosTex0Tbw( d ) (unsigned)(GsLL2I(d,14,6))
#define GsosTex0Psm( d ) (unsigned)(GsLL2I(d,20,6))
#define GsosTex0Tw( d ) (unsigned)(GsLL2I(d,26,4))
#define GsosTex0Th( d ) (unsigned)(GsLL2I(d,30,4))
#define GsosTex0Tcc( d ) (unsigned)(GsLL2I(d,34,1))
#define GsosTex0Tfx( d ) (unsigned)(GsLL2I(d,35,2))
#define GsosTex0Cbp( d ) (unsigned)(GsLL2I(d,37,14))
#define GsosTex0Cpsm( d ) (unsigned)(GsLL2I(d,51,4))
#define GsosTex0Csm( d ) (unsigned)(GsLL2I(d,55,1))
#define GsosTex0Csa( d ) (unsigned)(GsLL2I(d,56,5))
#define GsosTex0Cld( d ) (unsigned)(GsLL2I(d,61,3))

#define GsosTex1Data( lcm, mxl, mmag, mmin, mtba, l, k ) \
    GsLLong((lcm),0,1) | GsLLong((mxl),2,3) | \
    GsLLong((mmag),5,1) | GsLLong((mmin),6,3) | \
    GsLLong((mtba),9,1) | GsLLong((l),19,2) | GsLLong( (k), 32, 12 ) 
#define GsosTex1Lcm( d ) (unsigned)(GsLL2I(d,0,1))
#define GsosTex1Mxl( d ) (unsigned)(GsLL2I(d,2,3))
#define GsosTex1Mmag( d ) (unsigned)(GsLL2I(d,5,1))
#define GsosTex1Mmin( d ) (unsigned)(GsLL2I(d,6,3))
#define GsosTex1Mtba( d ) (unsigned)(GsLL2I(d,9,1))
#define GsosTex1L( d ) (unsigned)(GsLL2I(d,19,2))
#define GsosTex1K( d ) (unsigned)(GsLL2I(d,32,12))

#define GsosTex2Data( psm, cbp, cpsm, csm, csa, cld ) \
    GsLLong((psm),20,6) | GsLLong((cbp),37,14) | GsLLong((cpsm),51,4) | \
    GsLLong((csm),55,1) | GsLLong((csa),56,5) | GsLLong((cld),61,3)
#define GsosTex2Psm( d ) (unsigned)(GsLL2I(d,20,6))
#define GsosTex2Cbp( d ) (unsigned)(GsLL2I(d,37,14))
#define GsosTex2Cpsm( d ) (unsigned)(GsLL2I(d,51,4))
#define GsosTex2Csm( d ) (unsigned)(GsLL2I(d,55,1))
#define GsosTex2Csa( d ) (unsigned)(GsLL2I(d,56,5))
#define GsosTex2Cld( d ) (unsigned)(GsLL2I(d,61,3))

#define GsosTexclutData( cbw, cou, cov ) \
    GsLLong((cbw),0,6) | GsLLong((cou),6,6) | GsLLong((cov),12,10)
#define GsosTexclutCbw( d ) (unsigned)(GsLL2I(d,0,6))
#define GsosTexclutCou( d ) (unsigned)(GsLL2I(d,6,6))
#define GsosTexclutCov( d ) (unsigned)(GsLL2I(d,12,10))

#define GsosMiptbp1Data( tbp1, tbw1, tbp2, tbw2, tbp3, tbw3 ) \
    GsLLong((tbp1),0,14) | GsLLong((tbw1),14,6) | \
    GsLLong((tbp2),20,14) | GsLLong((tbw2),34,6) | \
    GsLLong((tbp3),40,14) | GsLLong((tbw3),54,6)
#define GsosMiptbp1Tbp1( d ) (unsigned)(GsLL2I(d,0,14))
#define GsosMiptbp1Tbw1( d ) (unsigned)(GsLL2I(d,14,6))
#define GsosMiptbp1Tbp2( d ) (unsigned)(GsLL2I(d,20,14))
#define GsosMiptbp1Tbw2( d ) (unsigned)(GsLL2I(d,34,6))
#define GsosMiptbp1Tbp3( d ) (unsigned)(GsLL2I(d,40,14))
#define GsosMiptbp1Tbw3( d ) (unsigned)(GsLL2I(d,54,6))

#define GsosMiptbp2Data( tbp4, tbw4, tbp5, tbw5, tbp6, tbw6 ) \
    GsLLong((tbp4),0,14) | GsLLong((tbw4),14,6) | \
    GsLLong((tbp5),20,14) | GsLLong((tbw5),34,6) | \
    GsLLong((tbp6),40,14) | GsLLong((tbw6),54,6)
#define GsosMiptbp2Tbp4( d ) (unsigned)(GsLL2I(d,0,14))
#define GsosMiptbp2Tbw4( d ) (unsigned)(GsLL2I(d,14,6))
#define GsosMiptbp2Tbp5( d ) (unsigned)(GsLL2I(d,20,14))
#define GsosMiptbp2Tbw5( d ) (unsigned)(GsLL2I(d,34,6))
#define GsosMiptbp2Tbp6( d ) (unsigned)(GsLL2I(d,40,14))
#define GsosMiptbp2Tbw6( d ) (unsigned)(GsLL2I(d,54,6))

#define GsosTexaData( ta0, aem, ta1 ) \
    GsLLong((ta0),0,8) | GsLLong((aem),15,1) | GsLLong((ta1),32,8) 
#define GsosTexaTa0( d ) (unsigned)(GsLL2I(d,0,8))
#define GsosTexaAem( d ) (unsigned)(GsLL2I(d,15,1))
#define GsosTexaTa1( d ) (unsigned)(GsLL2I(d,32,8))

#define GsosClampData( wms, wmt, minu, maxu, minv, maxv ) \
    GsLLong((wms),0,2) | GsLLong((wmt),2,2) | GsLLong((minu),4,10) | \
    GsLLong((maxu),14,10) | GsLLong((minv),24,10) | GsLLong((maxv),34,10)
#define GsosClampWms( d ) (unsigned)(GsLL2I(d,0,2))
#define GsosClampWmt( d ) (unsigned)(GsLL2I(d,2,2))
#define GsosClampMinu( d ) (unsigned)(GsLL2I(d,4,10))
#define GsosClampMaxu( d ) (unsigned)(GsLL2I(d,14,10))
#define GsosClampMinv( d ) (unsigned)(GsLL2I(d,24,10))
#define GsosClampMaxv( d ) (unsigned)(GsLL2I(d,34,10))
#define GsosClampUmsk( d ) (unsigned)(GsLL2I(d,4,10))
#define GsosClampUfix( d ) (unsigned)(GsLL2I(d,14,10))
#define GsosClampVmsk( d ) (unsigned)(GsLL2I(d,24,10))
#define GsosClampVfix( d ) (unsigned)(GsLL2I(d,34,10))

#define GsosFogcolData( fcr, fcg, fcb ) \
    GsLLong((fcr),0,8) | GsLLong((fcg),8,8) | GsLLong((fcb),16,8) 
#define GsosFogcolFcr( d ) (unsigned)(GsLL2I(d,0,8))
#define GsosFogcolFcg( d ) (unsigned)(GsLL2I(d,8,8))
#define GsosFogcolFcb( d ) (unsigned)(GsLL2I(d,16,8))

#define GsosScissorData( scax0, scax1, scay0, scay1 ) \
    GsLLong((scax0),0,11) | GsLLong((scax1),16,11) | \
    GsLLong((scay0),32,11) | GsLLong((scay1),48,11)
#define GsosScissorScax0( d ) (unsigned)(GsLL2I(d,0,11))
#define GsosScissorScax1( d ) (unsigned)(GsLL2I(d,16,11))
#define GsosScissorScay0( d ) (unsigned)(GsLL2I(d,32,11))
#define GsosScissorScay1( d ) (unsigned)(GsLL2I(d,48,11))

#define GsosTestData( ate, atst, aref, afail, date, datm, zte, ztst ) \
    GsLLong((ate),0,1) | GsLLong((atst),1,3) | GsLLong((aref),4,8) | \
    GsLLong((afail),12,2) | GsLLong((date),14,1) | GsLLong((datm),15,1) | \
    GsLLong((zte),16,1) | GsLLong((ztst),17,2)
#define GsosTestAte( d ) (unsigned)(GsLL2I(d,0,1))
#define GsosTestAtst( d ) (unsigned)(GsLL2I(d,1,3))
#define GsosTestAref( d ) (unsigned)(GsLL2I(d,4,8))
#define GsosTestAfail( d ) (unsigned)(GsLL2I(d,12,2))
#define GsosTestDate( d ) (unsigned)(GsLL2I(d,14,1))
#define GsosTestDatm( d ) (unsigned)(GsLL2I(d,15,1))
#define GsosTestZte( d ) (unsigned)(GsLL2I(d,16,1))
#define GsosTestZtst( d ) (unsigned)(GsLL2I(d,17,2))

#define GsosAlphaData( a, b, c, d, fix ) \
    GsLLong((a),0,2) | GsLLong((b),2,2) | GsLLong((c),4,2) | \
    GsLLong((d),6,2) | GsLLong((fix),32,8)
#define GsosAlphaA( d ) (unsigned)(GsLL2I(d,0,2))
#define GsosAlphaB( d ) (unsigned)(GsLL2I(d,2,2))
#define GsosAlphaC( d ) (unsigned)(GsLL2I(d,4,2))
#define GsosAlphaD( d ) (unsigned)(GsLL2I(d,6,2))
#define GsosAlphaFix( d ) (unsigned)(GsLL2I(d,32,8))

#define GsosPabeData( pabe ) GsLLong((pabe),0,1)
#define GsosPabePabe( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosDimxData( dimx00, dimx01, dimx02, dimx03, dimx10, dimx11, dimx12, dimx13, dimx20, dimx21, dimx22, dimx23, dimx30, dimx31, dimx32, dimx33 ) \
    GsLLong((dimx00),0,3) | GsLLong((dimx01),4,3) | GsLLong((dimx02),8,3) | \
    GsLLong((dimx03),12,3) | GsLLong((dimx10),16,3) | GsLLong((dimx11),20,3) | \
    GsLLong((dimx12),24,3) | GsLLong((dimx13),28,3) | GsLLong((dimx20),32,3) | \
    GsLLong((dimx21),36,3) | GsLLong((dimx22),40,3) | GsLLong((dimx23),44,3) | \
    GsLLong((dimx30),48,3) | GsLLong((dimx31),52,3) | GsLLong((dimx32),56,3) | \
    GsLLong((dimx33),60,3) 
#define GsosDimxDimx00( d ) (unsigned)(GsLL2I(d,0,3))
#define GsosDimxDimx01( d ) (unsigned)(GsLL2I(d,4,3))
#define GsosDimxDimx02( d ) (unsigned)(GsLL2I(d,8,3))
#define GsosDimxDimx03( d ) (unsigned)(GsLL2I(d,12,3))
#define GsosDimxDimx10( d ) (unsigned)(GsLL2I(d,16,3))
#define GsosDimxDimx11( d ) (unsigned)(GsLL2I(d,20,3))
#define GsosDimxDimx12( d ) (unsigned)(GsLL2I(d,24,3))
#define GsosDimxDimx13( d ) (unsigned)(GsLL2I(d,28,3))
#define GsosDimxDimx20( d ) (unsigned)(GsLL2I(d,32,3))
#define GsosDimxDimx21( d ) (unsigned)(GsLL2I(d,36,3))
#define GsosDimxDimx22( d ) (unsigned)(GsLL2I(d,40,3))
#define GsosDimxDimx23( d ) (unsigned)(GsLL2I(d,44,3))
#define GsosDimxDimx30( d ) (unsigned)(GsLL2I(d,48,3))
#define GsosDimxDimx31( d ) (unsigned)(GsLL2I(d,52,3))
#define GsosDimxDimx32( d ) (unsigned)(GsLL2I(d,56,3))
#define GsosDimxDimx33( d ) (unsigned)(GsLL2I(d,60,3))

#define GsosDtheData( dthe ) GsLLong((dthe),0,1)
#define GsosDtheDthe( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosColclampData( clamp ) GsLLong((clamp),0,1)
#define GsosColclampClamp( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosFbaData( fba ) GsLLong((fba),0,1) 
#define GsosFbaFba( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosFrameData( fbp, fbw, psm, fbmsk ) \
    GsLLong((fbp),0,9) | GsLLong((fbw),16,6) | \
    GsLLong((psm),24,6) | GsLLong((fbmsk),32,32) 
#define GsosFrameFbp( d ) (unsigned)(GsLL2I(d,0,9))
#define GsosFrameFbw( d ) (unsigned)(GsLL2I(d,16,6))
#define GsosFramePsm( d ) (unsigned)(GsLL2I(d,24,6))
#define GsosFrameFbmsk( d ) (unsigned)(GsLL2I(d,32,32))

#define GsosZbufData( zbp, psm, zmsk ) \
    GsLLong((zbp),0,8) | GsLLong((psm),24,4) | GsLLong((zmsk),32,1) 
#define GsosZbufZbp( d ) (unsigned)(GsLL2I(d,0,8))
#define GsosZbufPsm( d ) (unsigned)(GsLL2I(d,24,4))
#define GsosZbufZmsk( d ) (unsigned)(GsLL2I(d,32,1))

#define GsosBitbltbufData( sbp, sbw, spsm, dbp, dbw, dpsm ) \
    GsLLong((sbp),0,14) | GsLLong((sbw),16,6) | GsLLong((spsm),24,6) | \
    GsLLong((dbp),32,14) | GsLLong((dbw),48,6) | GsLLong((dpsm),56,6)
#define GsosBitbltbufSbp( d ) (unsigned)(GsLL2I(d,0,14))
#define GsosBitbltbufSbw( d ) (unsigned)(GsLL2I(d,16,6))
#define GsosBitbltbufSpsm( d ) (unsigned)(GsLL2I(d,24,6))
#define GsosBitbltbufDbp( d ) (unsigned)(GsLL2I(d,32,14))
#define GsosBitbltbufDbw( d ) (unsigned)(GsLL2I(d,48,6))
#define GsosBitbltbufDpsm( d ) (unsigned)(GsLL2I(d,56,6))

#define GsosTrxposData( ssax, ssay, dsax, dsay, dir ) \
    GsLLong((ssax),0,11) | GsLLong((ssay),16,11) | \
    GsLLong((dsax),32,11) | GsLLong((dsay),48,11) | GsLLong((dir),59,2)
#define GsosTrxposSsax( d ) (unsigned)(GsLL2I(d,0,11))
#define GsosTrxposSsay( d ) (unsigned)(GsLL2I(d,16,11))
#define GsosTrxposDsax( d ) (unsigned)(GsLL2I(d,32,11))
#define GsosTrxposDsay( d ) (unsigned)(GsLL2I(d,48,11))
#define GsosTrxposDir( d ) (unsigned)(GsLL2I(d,59,2))

#define GsosTrxregData( rrw, rrh ) \
    GsLLong((rrw),0,12) | GsLLong((rrh),32,12)
#define GsosTrxregRrw( d ) (unsigned)(GsLL2I(d,0,12))
#define GsosTrxregRrh( d ) (unsigned)(GsLL2I(d,32,12))

#define GsosTrxdirData( xdir ) GsLLong((xdir),0,2)
#define GsosTrxdirXdir( d ) (unsigned)(GsLL2I(d,0,2))

#define GsosPmodeData( en1, en2, crtmd, mmod, amod, slbg, alp, nfld, exvwins, exvwine, exsyncmd ) \
    GsLLong((en1),0,1) | GsLLong((en2),1,1) | GsLLong((crtmd),2,3) | \
    GsLLong((mmod),5,1) | GsLLong((amod),6,1) | GsLLong((slbg),7,1) | \
    GsLLong((alp),8,8) | GsLLong((nfld),16,1) | GsLLong((exvwins),32,10) | \
    GsLLong((exvwine),42,10) | GsLLong((exsyncmd),52,1)
#define GsosPmodeEn1( d ) (unsigned)(GsLL2I(d,0,1))
#define GsosPmodeEn2( d ) (unsigned)(GsLL2I(d,1,1))
#define GsosPmodeCrtmd( d ) (unsigned)(GsLL2I(d,2,3))
#define GsosPmodeMmod( d ) (unsigned)(GsLL2I(d,5,1))
#define GsosPmodeAmod( d ) (unsigned)(GsLL2I(d,6,1))
#define GsosPmodeSlbg( d ) (unsigned)(GsLL2I(d,7,1))
#define GsosPmodeAlp( d ) (unsigned)(GsLL2I(d,8,8))
#define GsosPmodeNfld( d ) (unsigned)(GsLL2I(d,16,1))
#define GsosPmodeExvwins( d ) (unsigned)(GsLL2I(d,32,10))
#define GsosPmodeExvwine( d ) (unsigned)(GsLL2I(d,42,10))
#define GsosPmodeExsyncmd( d ) (unsigned)(GsLL2I(d,52,1))

#define GsosSmode1Data( rc, lc, t1248, slck, cmod, ex, vhp, prst, sint, xpck, pck2, spml, gcont, phs, pvs, pehs, pevs ) \
    GsLLong((rc),0,3) | GsLLong((lc),3,7) | GsLLong((t1248),10,2) | \
    GsLLong((slck),12,1) | GsLLong((cmod),13,2) | GsLLong((ex),15,1) | \
    GsLLong((vhp),16,1) | GsLLong((prst),17,1) | GsLLong((sint),18,1) | \
    GsLLong((xpck),19,1) | GsLLong((pck2),20,2) | GsLLong((spml),22,4) | \
    GsLLong((gcont),26,1) | GsLLong((phs),27,1) | GsLLong((pvs),28,1) | \
    GsLLong((pehs),29,1) | GsLLong((pevs),30,1)
#define GsosSmode1Rc( d ) (unsigned)(GsLL2I(d,0,3))
#define GsosSmode1Lc( d ) (unsigned)(GsLL2I(d,3,7))
#define GsosSmode1T1248( d ) (unsigned)(GsLL2I(d,10,2))
#define GsosSmode1Slck( d ) (unsigned)(GsLL2I(d,12,1))
#define GsosSmode1Cmod( d ) (unsigned)(GsLL2I(d,13,2))
#define GsosSmode1Ex( d ) (unsigned)(GsLL2I(d,15,1))
#define GsosSmode1Vhp( d ) (unsigned)(GsLL2I(d,16,1))
#define GsosSmode1Prst( d ) (unsigned)(GsLL2I(d,17,1))
#define GsosSmode1Sint( d ) (unsigned)(GsLL2I(d,18,1))
#define GsosSmode1Xpck( d ) (unsigned)(GsLL2I(d,19,1))
#define GsosSmode1Pck2( d ) (unsigned)(GsLL2I(d,20,2))
#define GsosSmode1Spml( d ) (unsigned)(GsLL2I(d,22,4))
#define GsosSmode1Gcont( d ) (unsigned)(GsLL2I(d,26,1))
#define GsosSmode1Phs( d ) (unsigned)(GsLL2I(d,27,1))
#define GsosSmode1Pvs( d ) (unsigned)(GsLL2I(d,28,1))
#define GsosSmode1Pehs( d ) (unsigned)(GsLL2I(d,29,1))
#define GsosSmode1Pevs( d ) (unsigned)(GsLL2I(d,30,1))

#define GsosSmode2Data( iint, ffmd, dpms ) \
    GsLLong((iint),0,1) | GsLLong((ffmd),1,1) | GsLLong((dpms),2,2)
#define GsosSmode2Int( d ) (unsigned)(GsLL2I(d,0,1))
#define GsosSmode2Ffmd( d ) (unsigned)(GsLL2I(d,1,1))
#define GsosSmode2Dpms( d ) (unsigned)(GsLL2I(d,2,2))

#define GsosSrfshData( rfsh ) GsLLong((xdir),0,5)
#define GsosSrfshRfsh( d ) (unsigned)(GsLL2I(d,0,5))

#define GsosSynch1Data( hfp, hbp, hseq, hsvs, hs ) \
    GsLLong((hfp),0,11) | GsLLong((hbp),11,11) | GsLLong((hseq),22,10) | \
    GsLLong((hsvs),32,11) | GsLLong((hs),43,10)
#define GsosSynch1Hfp( d ) (unsigned)(GsLL2I(d,0,11))
#define GsosSynch1Hbp( d ) (unsigned)(GsLL2I(d,11,11))
#define GsosSynch1Hseq( d ) (unsigned)(GsLL2I(d,22,10))
#define GsosSynch1Hsvs( d ) (unsigned)(GsLL2I(d,32,11))
#define GsosSynch1Hs( d ) (unsigned)(GsLL2I(d,42,10))

#define GsosSynch2Data( hf, hb ) \
    GsLLong((hf),0,11) | GsLLong((hb),11,11)
#define GsosSynch2Hf( d ) (unsigned)(GsLL2I(d,0,11))
#define GsosSynch2Hb( d ) (unsigned)(GsLL2I(d,11,11))

#define GsosSyncvData( vfp, vfpe, vbp, vbpe, vdp, vs ) \
    GsLLong((vfp),0,10) | GsLLong((vfpe),10,10) | GsLLong((vbp),20,10) | \
    GsLLong((vbpe),32,10) | GsLLong((vdp),42,10) | GsLLong((vs),52,10)
#define GsosSyncvVfp( d ) (unsigned)(GsLL2I(d,0,10))
#define GsosSyncvVfpe( d ) (unsigned)(GsLL2I(d,10,10))
#define GsosSyncvVbp( d ) (unsigned)(GsLL2I(d,20,10))
#define GsosSyncvVbpe( d ) (unsigned)(GsLL2I(d,32,10))
#define GsosSyncvVdp( d ) (unsigned)(GsLL2I(d,42,10))
#define GsosSyncvVs( d ) (unsigned)(GsLL2I(d,52,10))

#define GsosDispfbData( fbp, fbw, psm, dbx, dby ) \
    GsLLong((fbp),0,9) | GsLLong((fbw),9,6) | GsLLong((psm),15,5) | \
    GsLLong((dbx),32,11) | GsLLong((dby),43,11)
#define GsosDispfbFbp( d ) (unsigned)(GsLL2I(d,0,9))
#define GsosDispfbFbw( d ) (unsigned)(GsLL2I(d,9,6))
#define GsosDispfbPsm( d ) (unsigned)(GsLL2I(d,15,6))
#define GsosDispfbDbx( d ) (unsigned)(GsLL2I(d,32,11))
#define GsosDispfbDby( d ) (unsigned)(GsLL2I(d,43,11))

#define GsosDisplayData( dx, dy, magh, magv, dw, dh ) \
    GsLLong((dx),0,12) | GsLLong((dy),12,11) | GsLLong((magh),23,4) | \
    GsLLong((magv),27,2) | GsLLong((dw),32,12) | GsLLong((dh),44,11)
#define GsosDisplayDx( d ) (unsigned)(GsLL2I(d,0,12))
#define GsosDisplayDy( d ) (unsigned)(GsLL2I(d,12,11))
#define GsosDisplayMagh( d ) (unsigned)(GsLL2I(d,23,4))
#define GsosDisplayMagv( d ) (unsigned)(GsLL2I(d,27,2))
#define GsosDisplayDw( d ) (unsigned)(GsLL2I(d,32,12))
#define GsosDisplayDh( d ) (unsigned)(GsLL2I(d,44,11))

#define GsosExtbufData( exbp, exbw, fbin, wffmd, emoda, emodc, wdx, wdy ) \
    GsLLong((exbp),0,14) | GsLLong((exbw),14,6) | GsLLong((fbin),20,2) | \
    GsLLong((wffmd),22,1) | GsLLong((emoda),23,2) | GsLLong((emodc),25,2) | \
    GsLLong((magv),32,11) | GsLLong((dh),43,11)
#define GsosExtbufExbp( d ) (unsigned)(GsLL2I(d,0,14))
#define GsosExtbufExbw( d ) (unsigned)(GsLL2I(d,14,6))
#define GsosExtbufFbin( d ) (unsigned)(GsLL2I(d,20,2))
#define GsosExtbufWffmd( d ) (unsigned)(GsLL2I(d,22,1))
#define GsosExtbufEmoda( d ) (unsigned)(GsLL2I(d,23,2))
#define GsosExtbufEmodc( d ) (unsigned)(GsLL2I(d,25,2))
#define GsosExtbufWdx( d ) (unsigned)(GsLL2I(d,32,11))
#define GsosExtbufWdy( d ) (unsigned)(GsLL2I(d,43,11))

#define GsosExtdataData( sx, dy, smph, smpv, ww, wh ) \
    GsLLong((sx),0,12) | GsLLong((sy),12,11) | GsLLong((smph),23,4) | \
    GsLLong((smpw),27,2) | GsLLong((ww),32,12) | GsLLong((wh),44,11)
#define GsosExtdataSx( d ) (unsigned)(GsLL2I(d,0,12))
#define GsosExtdataSy( d ) (unsigned)(GsLL2I(d,12,11))
#define GsosExtdataSmph( d ) (unsigned)(GsLL2I(d,23,4))
#define GsosExtdataSmpv( d ) (unsigned)(GsLL2I(d,27,2))
#define GsosExtdataWw( d ) (unsigned)(GsLL2I(d,32,12))
#define GsosExtdataWh( d ) (unsigned)(GsLL2I(d,44,11))

#define GsosExtwriteData( rfsh ) GsLLong((write),0,1)
#define GsosExtwriteWrite( d ) (unsigned)(GsLL2I(d,0,1))

#define GsosBgcolorData( r, g, b ) \
    GsLLong((r),0,8) | GsLLong((g),8,8) | GsLLong((b),16,8)
#define GsosBgcolorR( d ) (unsigned)(GsLL2I(d,0,8))
#define GsosBgcolorG( d ) (unsigned)(GsLL2I(d,8,8))
#define GsosBgcolorB( d ) (unsigned)(GsLL2I(d,16,8))

/*****************************************************************
 *  utilities
 *****************************************************************
 */
#define GSOS_XYOFFSET    1024
#define GSOS_SUBPIX_OFST(x) (((GSOSbit64)(x) + GSOS_XYOFFSET)<<4)
#define GSOS_SUBTEX_OFST(x) (((GSOSbit64)(x)<<4)+0x8)

#endif
