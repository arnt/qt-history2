/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifdef EXT
#define ARGS(x) 
#else
#define EXT
#define ARGS(x) x
#endif

/* Declaration of global variable to hold dither info. */
EXT int ditherType;
EXT int matched_depth ARGS(= 8);

#ifdef DCPREC
/* Declaration of global variable to hold DC precision */
EXT int dcprec ARGS(= 0);
#endif

/* Global file pointer to incoming data. */
EXT FILE *input;
EXT char *inputName;

/* End of File flag. */
EXT int EOF_flag ARGS(= 0);

/* Loop flag. */
EXT int loopFlag ARGS(= 0);

/* Shared memory flag. */
EXT int shmemFlag ARGS(= 0);

/* Quiet flag. */
#ifdef QUIET
EXT int quietFlag ARGS(= 1);
#else
EXT int quietFlag ARGS(= 0);
#endif

/* Own Color Map flag. */
EXT int owncmFlag ARGS(= 0);

/* "Press return" flag, requires return for each new frame */
EXT int requireKeypressFlag ARGS(=0);

/* Display image on screen? */
EXT int noDisplayFlag ARGS(= 0);

/* Seek Value. 
   0 means do not seek.
   N (N>0) means seek to N after the header is parsed
   N (N<0) means the seek has beeen done to offset N
*/
EXT long seekValue ARGS(= 0);

/* Framerate, -1: specified in stream (default)
               0: as fast as possible
               N (N>0): N frames/sec  
               */
EXT int framerate ARGS(= -1);

/* Flags/values to control Arbitrary start/stop frames. */
EXT int partialFlag ARGS(= 0), startFrame ARGS(= -1), endFrame ARGS(= -1);

/* Flag for gamma correction */
EXT int gammaCorrectFlag ARGS(=0);
EXT double gammaCorrect ARGS(=1.0);

/* Setjmp/Longjmp env. */
EXT jmp_buf env;

/* Flag for high quality at the expense of speed */
#ifdef QUALITY
EXT int qualityFlag ARGS(= 1);
#else
EXT int qualityFlag ARGS(= 0);
#endif

