/* $XConsortium: TabB.h /main/5 1995/07/15 20:42:07 drk $ */
/*
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
 */
/*
 * HISTORY
 */

/*******************************************************************************
 *
 * TabB.h: The widget public header file for the ExmTabButton demonstration
 *         widget.
 *
 ******************************************************************************/


/* Ensure that the file be included only once. */
#ifndef _ExmTabB_h
#define _ExmTabB_h


/* Allow for C++ compilation. */
#ifdef __cplusplus
extern "C" {
#endif


/* Include appropriate files. */
#include <Exm/CommandB.h>  /* public header file for ExmCommandButton */
#include <Xm/JoinSideT.h> /* contains defs. needed by ExmNopenSide resource */


/* Define the widget class and widget record. */
externalref WidgetClass exmTabButtonWidgetClass;

typedef struct _ExmTabButtonClassRec *ExmTabButtonWidgetClass;
typedef struct _ExmTabButtonRec *ExmTabButtonWidget;


/* Define an IsSubclass macro. */
#ifndef ExmIsTabButton
#define ExmIsTabButton(w) XtIsSubclass(w, exmTabButtonWidgetClass)
#endif


/* Define string equivalents of new resource names. */
#define ExmNopenSide  "openSide"
#define ExmCOpenSide  "OpenSide"
#define ExmROpenSide  "ExmOpenSide"


/* Specify the API for this widget. */
extern Widget ExmCreateTabButton(Widget    parent,
                                 char     *name,
                                 Arg      *arglist,
                                 Cardinal  argCount);


/* Allow for C++ compilation. */
#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


/* Ensure that the file be included only once. */
#endif /* _ExmTabB_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
