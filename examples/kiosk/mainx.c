/*
 * main.c --
 *
 *      Main procedure
 *
 */

/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */
#include "video.h"
#include "proto.h"
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "util.h"
#include "dither.h"

/* Define buffer length. */

#define BUF_LENGTH 80000

/* Function return type declarations */
void usage(char *s);

/* External declaration of main decoding call. */

extern VidStream *mpegVidRsrc(TimeStamp time_stamp, VidStream *vid_stream);
extern VidStream *NewVidStream(unsigned int buffer_len);

#define EXT extern
#include "globals.h"


/*
 *--------------------------------------------------------------
 *
 * int_handler --
 *
 *        Handles Cntl-C interupts..
 *      (two different ones for different OSes)
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */
#define SIG_ONE_PARAM /* which platforms is this not true for? */
#ifndef SIG_ONE_PARAM
void
int_handler(void)
{
  if (!quietFlag) {
    fprintf(stderr, "Interrupted!\n");
  }
  if (curVidStream != NULL)
    DestroyVidStream(curVidStream);
  exit(1);
}
#else
void
int_handler(int signum)
{
  if (!quietFlag) {
    fprintf(stderr, "Interrupted!\n");
  }
  if (curVidStream != NULL)
    DestroyVidStream(curVidStream);
  exit(1);
}
#endif

/*
 *--------------------------------------------------------------
 *
 * main --
 *
 *        Parses command line, starts decoding and displaying.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */

#ifndef GUI_Qt
main(int argc, char **argv)
{
  char *name;
  static VidStream *theStream;
  int mark;
  int i;

  mark = 1;
  argc--;

  name = (char *) "";
  input = stdin;
  inputName = "stdin";
  ditherType = ORDERED2_DITHER;
  LUM_RANGE = 8;
  CR_RANGE = CB_RANGE = 4;
  noDisplayFlag = 0;


#ifdef SH_MEM
  shmemFlag = 1;
#endif

  while (argc) {
    if (strcmp(argv[mark], "-nop") == 0) {
      SetPFlag(TRUE);
      SetBFlag(TRUE);
      argc--; mark++;
    } else if (strcmp(argv[mark], "-nob") == 0) {
      SetBFlag(TRUE);
      argc--; mark++;
#ifdef GUI_X
    } else if (strcmp(argv[mark], "-display") == 0) {
      name = argv[++mark];
      argc -= 2; mark++;
#endif
    } else if (strcmp(argv[mark], "-start") == 0) {
      if (argc < 2) usage(argv[0]);
      startFrame = atoi(argv[++mark]);
      if (seekValue != 0) {
        partialFlag = TRUE;
      }
      argc -= 2; mark++;
    } else if (strcmp(argv[mark], "-seek") == 0) {
      if (argc < 2) usage(argv[0]);
      seekValue = atoi(argv[++mark]);
      if (startFrame != -1) startFrame = 0;
      argc -= 2; mark++;
    } else if (strcmp(argv[mark], "-end") == 0) {
      if (argc < 2) usage(argv[0]);
      endFrame = atoi(argv[++mark]);
      partialFlag = TRUE;
      argc -= 2; mark++;
    } else if (strcmp(argv[mark], "-gamma") == 0) {
      if (argc < 2) usage(argv[0]);
      sscanf(argv[++mark], "%lf", &gammaCorrect);
      if (gammaCorrect < 0) {
        fprintf(stderr, "ERROR: Gamma correction must be at least 0.\n");
      }
      if (!quietFlag) {
        printf("Gamma Correction set to %4.2f.\n",gammaCorrect);
      }

      gammaCorrectFlag = 1;
      argc -= 2; mark++;
#ifdef DCPREC
    } else if (strcmp(argv[mark], "-dc") == 0) {
      argc--; mark++;
      if (argc < 1) {
        perror("Must specify dc precision after -dc flag");
        usage(argv[0]);
      }
      dcprec = atoi(argv[mark]) - 8;
      if ((dcprec > 3) || (dcprec < 0)) {
        perror("DC precision must be at least 8 and at most 11");
        usage(argv[0]);
      }
      argc--; mark++;
#endif
    } else if (strcmp(argv[mark], "-quality") == 0) {
      argc--; mark++;
      if (argc < 1) {
        perror("Must specify on or off after -quality flag");
        usage(argv[0]);
      }
      if (strcmp(argv[mark], "on") == 0) {
        argc--; mark++;
        qualityFlag = 1;
      }
      else if (strcmp(argv[mark], "off") == 0) {
        argc--; mark++;
        qualityFlag = 0;
      }
      else {
        perror("Must specify on or off after -quality flag");
        usage(argv[0]);
      }
    } else if (strcmp(argv[mark], "-framerate") == 0) {
      argc--; mark++;
      if (argc < 1) {
        perror("Must specify framerate after -framerate flag");
        usage(argv[0]);
      }
      framerate = atoi(argv[mark]);
      argc--; mark++;
    } else if (strcmp(argv[mark], "-dither") == 0) {
      argc--; mark++;
      if (argc < 1) {
        perror("Must specify dither option after -dither flag");
        usage(argv[0]);
      }
      if (strcmp(argv[mark], "hybrid") == 0) {
        argc--; mark++;
        ditherType = HYBRID_DITHER;
      } else if (strcmp(argv[mark], "hybrid2") == 0) {
        argc--; mark++;
        ditherType = HYBRID2_DITHER;
      } else if (strcmp(argv[mark], "fs4") == 0) {
        argc--; mark++;
        ditherType = FS4_DITHER;
      } else if (strcmp(argv[mark], "fs2") == 0) {
        argc--; mark++;
        ditherType = FS2_DITHER;
      } else if (strcmp(argv[mark], "fs2fast") == 0) {
        argc--; mark++;
        ditherType = FS2FAST_DITHER;
      } else if (strcmp(argv[mark], "hybrid2") == 0) {
        argc--; mark++;
        ditherType = HYBRID2_DITHER;
      } else if (strcmp(argv[mark], "2x2") == 0) {
        argc--; mark++;
        ditherType = Twox2_DITHER;
      } else if ((strcmp(argv[mark], "gray256") == 0) ||
                 (strcmp(argv[mark], "grey256") == 0)) {
        argc--; mark++;
        ditherType = GRAY256_DITHER;
      } else if ((strcmp(argv[mark], "gray") == 0) ||
                 (strcmp(argv[mark], "grey") == 0)) {
        argc--; mark++;
        ditherType = GRAY_DITHER;
      } else if ((strcmp(argv[mark], "gray256x2") == 0) ||
                  (strcmp(argv[mark], "grey256x2") == 0)) {
        argc--; mark++;
        ditherType = GRAY2562_DITHER;
      } else if ((strcmp(argv[mark], "gray") == 0) ||
                   (strcmp(argv[mark], "grey") == 0)) {
        argc--; mark++;
        ditherType = GRAY_DITHER;
      } else if ((strcmp(argv[mark], "gray2") == 0) ||
                  (strcmp(argv[mark], "grey2") == 0)) {
        argc--; mark++;
        ditherType = GRAY2_DITHER;
      } else if (strcmp(argv[mark], "color") == 0 ||
                 strcmp(argv[mark], "colour") == 0) {
        argc--; mark++;
        ditherType = FULL_COLOR_DITHER;
      } else if (strcmp(argv[mark], "color2") == 0 ||
                 strcmp(argv[mark], "colour2") == 0) {
        argc--; mark++;
        ditherType = FULL_COLOR2_DITHER;
      } else if (strcmp(argv[mark], "none") == 0) {
        argc--; mark++;
        ditherType = NO_DITHER;
      } else if (strcmp(argv[mark], "ppm") == 0) {
        argc--; mark++;
        ditherType = PPM_DITHER;
      } else if (strcmp(argv[mark], "ordered") == 0) {
        argc--; mark++;
        ditherType = ORDERED_DITHER;
      } else if (strcmp(argv[mark], "ordered2") == 0) {
        argc--; mark++;
        ditherType = ORDERED2_DITHER;
      } else if (strcmp(argv[mark], "mbordered") == 0) {
        argc--; mark++;
        ditherType = MBORDERED_DITHER;
      } else if (strcmp(argv[mark], "mono") == 0) {
        argc--; mark++;
        ditherType = MONO_DITHER;
      } else if (strcmp(argv[mark], "threshold") == 0) {
        argc--; mark++;
        ditherType = MONO_THRESHOLD;
      } else {
        perror("Illegal dither option.");
        usage(argv[0]);
      }
    } 
    else if (strcmp(argv[mark], "-eachstat") == 0) {
      argc--; mark++;
#ifdef ANALYSIS
      showEachFlag = 1;
#else
      fprintf(stderr, "To use -eachstat, recompile with -DANALYSIS in CFLAGS\n");
      exit(1);
#endif
    }
    else if (strcmp(argv[mark], "-shmem_off") == 0) {
      argc--; mark++;
      shmemFlag = 0;
    }
#ifdef QUIET
    else if (strcmp(argv[mark], "-quiet") == 0) { 
    }
    else if (strcmp(argv[mark], "-noisy") == 0) {
#else
    else if (strcmp(argv[mark], "-noisy") == 0) {
    }
    else if (strcmp(argv[mark], "-quiet") == 0) { 
#endif
      argc--; mark++;
      quietFlag = !quietFlag;
    }
    else if (strcmp(argv[mark], "-owncm") == 0) {
      argc--; mark++;
      owncmFlag = 1;
    }
    else if (strcmp(argv[mark], "-step") == 0) {
      argc--; mark++;
      requireKeypressFlag = 1;
    }
    else if (strcmp(argv[mark], "-loop") == 0) {
      argc--; mark++;
      loopFlag = 1;
    }
    else if (strcmp(argv[mark], "-no_display") == 0) {
      argc--; mark++;
      noDisplayFlag = 1;
      shmemFlag=0;
    }
    else if (strcmp(argv[mark], "-l_range") == 0) {
      argc--; mark++;
      LUM_RANGE = atoi(argv[mark]);
      if (LUM_RANGE < 1) {
        fprintf(stderr, "Illegal luminance range value: %d\n", LUM_RANGE);
        exit(1);
      }
      argc--; mark++;
    }
    else if (strcmp(argv[mark], "-cr_range") == 0) {
      argc--; mark++;
      CR_RANGE = atoi(argv[mark]);
      if (CR_RANGE < 1) {
        fprintf(stderr, "Illegal cr range value: %d\n", CR_RANGE);
        exit(1);
      }
      argc--; mark++;
    }
    else if (strcmp(argv[mark], "-cb_range") == 0) {
      argc--; mark++;
      CB_RANGE = atoi(argv[mark]);
      if (CB_RANGE < 1) {
        fprintf(stderr, "Illegal cb range value: %d\n", CB_RANGE);
        exit(1);
      }
      argc--; mark++;
    } 
    else if ((strcmp(argv[mark], "-?") == 0) ||
               (strcmp(argv[mark], "-Help") == 0) ||
               (strcmp(argv[mark], "-help") == 0)) {
      usage(argv[0]);
    }
    else if (argv[mark][0] == '-') {
      fprintf(stderr, "Un-recognized flag %s\n",argv[mark]);
      usage(argv[0]);
    }
    else {
      input = fopen(argv[mark], "r");
      if (input == NULL) {
        fprintf(stderr, "Could not open file %s\n", argv[mark]);
        usage(argv[0]);
      }
      inputName = argv[mark];
      argc--; mark++;
    }
  }

  lum_values = (int *) malloc(LUM_RANGE*sizeof(int));
  cr_values = (int *) malloc(CR_RANGE*sizeof(int));
  cb_values = (int *) malloc(CB_RANGE*sizeof(int));

  signal(SIGINT, int_handler);

  if ((startFrame != -1) && (endFrame != -1) &&
      (endFrame < startFrame)) {
    usage(argv[0]);
  }

  init_tables();

  if (ditherType == MONO_DITHER || ditherType == MONO_THRESHOLD)
    matched_depth = 1;

  switch (ditherType) {
    
  case HYBRID_DITHER:
    InitColor();
    InitHybridDither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    break;
    
  case HYBRID2_DITHER:
    InitColor();
    InitHybridErrorDither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    break;
    
  case FS4_DITHER:
    InitColor();
    InitFS4Dither();
#if defined(GUI_X)
      InitDisplay(name);
#endif
    break;
    
  case FS2_DITHER:
    InitColor();
    InitFS2Dither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    break;
    
  case FS2FAST_DITHER:
    InitColor();
    InitFS2FastDither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    break;
    
  case Twox2_DITHER:
    InitColor();
    Init2x2Dither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    PostInit2x2Dither();
    break;

  case GRAY_DITHER:
  case GRAY2_DITHER:
#if defined(GUI_X)
    InitGrayDisplay(name);
#endif
    break;

  case GRAY256_DITHER:
  case GRAY2562_DITHER:
#if defined(GUI_X)
    InitGray256Display(name);
#endif
    break;

  case FULL_COLOR_DITHER:
  case FULL_COLOR2_DITHER:
#if defined(GUI_X)
    InitColorDisplay(name);
#endif
    InitColorDither(matched_depth == 32);
    break;

  case NO_DITHER:
    shmemFlag = 0;
    break;

  case PPM_DITHER:
    shmemFlag = 0;
    wpixel[0] = 0xff;
    wpixel[1] = 0xff00;
    wpixel[2] = 0xff0000;
    matched_depth = 24;
    InitColorDither(1);
    break;

  case ORDERED_DITHER:
    InitColor();
    InitOrderedDither();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    break;

  case MONO_DITHER:
  case MONO_THRESHOLD:
#if defined(GUI_X)
    InitMonoDisplay(name);
#endif
    break;

  case ORDERED2_DITHER:
    InitColor();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    InitOrdered2Dither();
    break;

  case MBORDERED_DITHER:
    InitColor();
#if defined(GUI_X)
    InitDisplay(name);
#endif
    InitMBOrderedDither();
    break;

  }

#if defined(GUI_X)
#ifdef SH_MEM
    if (shmemFlag && (display != NULL)) {
      if (!XShmQueryExtension(display)) {
        shmemFlag = 0;
        if (!quietFlag) {
          fprintf(stderr, "Shared memory not supported\n");
          fprintf(stderr, "Reverting to normal Xlib.\n");
        }
      }
    }
#endif
#endif

  if (setjmp(env) != 0) {

    DestroyVidStream(theStream);

    rewind(input);
    if (seekValue < 0) seekValue = 0 - seekValue;
    EOF_flag = 0;
    curBits = 0;
    bitOffset = 0;
    bufLength = 0;
    bitBuffer = NULL;
    totNumFrames = 0;
#ifdef ANALYSIS 
    init_stats();
#endif

  }

  theStream = NewVidStream((unsigned int) BUF_LENGTH);
#if defined(GUI_X)
  mpegVidRsrc(0, theStream);

  if (IS_2x2_DITHER(ditherType)) i = 2;
  else i = 1;  

  if (!noDisplayFlag) {
    ResizeDisplay((unsigned int) curVidStream->h_size*i, 
		  (unsigned int) curVidStream->v_size*i);
  }
  while (1) mpegVidRsrc(0, theStream);
}
 

/*
 *--------------------------------------------------------------
 *
 * usage --
 *
 *        Print mpeg_play usage
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        exits with a return value -1
 *
 *--------------------------------------------------------------
 */

void
usage(char *s)
                /* program name */
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "mpeg_play [options] [filename]\n");
    fprintf(stderr, "Options :\n");
    fprintf(stderr, "      [-display X_display]\n");
    fprintf(stderr, "      [-no_display]\n");
    fprintf(stderr, "      [-dither {ordered|ordered2|mbordered|fs4|fs2|fs2fast|hybrid|\n");
    fprintf(stderr, "                hybrid2|2x2|gray|gray256|color|color2|none|mono|threshold|ppm|\n");
    fprintf(stderr, "                gray2|gray256x2}]\n");
    fprintf(stderr, "      [-loop]\n");
    fprintf(stderr, "      [-start frame_num]\n");
    fprintf(stderr, "      [-end frame_num]\n");
    fprintf(stderr, "      [-seek file_offset]\n");
    fprintf(stderr, "      [-gamma gamma_correction_value]\n");
    fprintf(stderr, "      [-framerate num_frames_per_sec]  (0 means as fast as possible)\n");
#ifdef QUIET
    fprintf(stderr, "      [-noisy] (turns on all program output)\n");
#else
    fprintf(stderr, "      [-quiet] (turns off all program output)\n");
#endif
    fprintf(stderr, "      [-quality {on|off}] (current default set to ");
#ifdef QUALITY
    fprintf(stderr, "ON)\n");
#else
    fprintf(stderr, "OFF)\n");
#endif
    fprintf(stderr, "      [-?] [-help] for help\n");
    fprintf(stderr, "Rare options:\n");
    fprintf(stderr, "      [-eachstat]\n");
    fprintf(stderr, "      [-owncm]\n");
    fprintf(stderr, "      [-shmem_off]\n");
    fprintf(stderr, "      [-l_range num]\n");
    fprintf(stderr, "      [-cr_range num]     [-cb_range num]\n");
    fprintf(stderr, "      [-nob]              [-nop]\n");
#ifdef DCPREC
    fprintf(stderr, "      [-dc {8|9|10|11}] (defaults to 8)\n");
#endif
    exit (-1);
#endif
}
#endif



/*
 *--------------------------------------------------------------
 *
 * DoDitherImage --
 *
 *      Called when image needs to be dithered. Selects correct
 *      dither routine based on info in ditherType.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */

void
DoDitherImage(unsigned char *l, unsigned char *Cr, unsigned char *Cb, unsigned char *disp, int h, int w)
{
  switch(ditherType) {
#ifndef GUI_Qt
  case HYBRID_DITHER:
    HybridDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case HYBRID2_DITHER:
    HybridErrorDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS2FAST_DITHER:
    FS2FastDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS2_DITHER:
    FS2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS4_DITHER:
    FS4DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case Twox2_DITHER:
    Twox2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FULL_COLOR2_DITHER:
    if (matched_depth == 32)
      Twox2Color32DitherImage(l, Cr, Cb, disp, h, w);
    else
      Twox2Color16DitherImage(l, Cr, Cb, disp, h, w);
    break;
#endif
  case FULL_COLOR_DITHER:
    if (matched_depth == 32)
      Color32DitherImage(l, Cr, Cb, disp, h, w);
    else
      Color16DitherImage(l, Cr, Cb, disp, h, w);
    break;
#ifndef GUI_Qt
  case GRAY_DITHER:
  case GRAY256_DITHER:
    if (matched_depth == 8) 
      GrayDitherImage(l, Cr, Cb, disp, h, w);
    else if (matched_depth == 16) 
      Gray16DitherImage(l, Cr, Cb, disp, h, w);
    else if (matched_depth == 32 || matched_depth == 24)
      Gray32DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case GRAY2_DITHER:
  case GRAY2562_DITHER:
    if (matched_depth == 8) 
      Gray2DitherImage(l, Cr, Cb, disp, h, w);
    else if (matched_depth == 16) 
      Gray216DitherImage(l, Cr, Cb, disp, h, w);
    else if (matched_depth == 32 || matched_depth == 24)
      Gray232DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case NO_DITHER:
    break;

  case PPM_DITHER:
    Color32DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case ORDERED_DITHER:
    OrderedDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MONO_DITHER:
    MonoDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MONO_THRESHOLD:
    MonoThresholdImage(l, Cr, Cb, disp, h, w);
    break;

  case ORDERED2_DITHER:
    Ordered2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MBORDERED_DITHER:
    MBOrderedDitherImage(l, Cr, Cb, disp, h, w);
    break;
#endif
  }
}
