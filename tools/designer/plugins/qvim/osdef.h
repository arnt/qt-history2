/*
 * osdef.h is automagically created from osdef?.h.in by osdef.sh -- DO NOT EDIT
 */
/* autoconf cannot fiddle out declarations. Use our homebrewn tools. (jw) */
/*
 * Declarations that may cause conflicts belong here so that osdef.sh
 * can clean out the forest. Everything else belongs in os_unix.h
 *
 * How this works:
 * - This file contains all unix prototypes that Vim might need.
 * - The shell script osdef.sh is executed at compile time to remove all the
 *   prototypes that are in an include file. This results in osdef.h.
 * - osdef.h is included in vim.h.
 *
 * sed cannot always handle so many commands, this is file 1 of 2
 */

#ifdef HAVE_FSEEKO
extern int	fseeko __ARGS((FILE *, off_t, int));
#endif
#ifdef HAVE_FTELLO
extern off_t	ftello __ARGS((FILE *));
#endif
#ifndef ferror	/* let me say it again: "macros should never have prototypes" */
#endif
#if defined(sun) || defined(_SEQUENT_)
/* used inside of stdio macros getc(), puts(), putchar()... */
extern int	_flsbuf __ARGS((int, FILE *));
extern int	_filbuf __ARGS((FILE *));
#endif

#if !defined(HAVE_SELECT)
struct pollfd;			/* for poll __ARGS */
extern int	poll __ARGS((struct pollfd *, long, int));
#endif

#ifdef HAVE_MEMSET
#endif
#ifdef HAVE_BCMP
#endif
#ifdef HAVE_MEMCMP
#endif
#ifdef HAVE_STRPBRK
#endif
#ifdef USEBCOPY
#else
# ifdef USEMEMCPY
# else
#  ifdef USEMEMMOVE
#  endif
# endif
#endif
/* used inside of FD_ZERO macro: */
#ifdef HAVE_SETSID
#endif
#ifdef HAVE_SETPGID
#endif
#ifdef HAVE_STRTOL
#endif
#ifdef HAVE_STRFTIME
#endif
#ifdef HAVE_STRCASECMP
#endif
#ifdef HAVE_STRNCASECMP
#endif

#ifndef USE_SYSTEM
#endif


#ifdef HAVE_SIGSET
extern RETSIGTYPE (*sigset __ARGS((int, RETSIGTYPE (*func) SIGPROTOARG))) __ARGS(SIGPROTOARG);
#endif


#if defined(HAVE_GETCWD) && !defined(sun)
#else
#endif
#ifndef __alpha	/* suggested by Campbell */
#endif
/*
 * osdef2.h.in - See osdef1.h.in for a description.
 */






#ifdef HAVE_TERMIOS_H
#endif

#ifdef HAVE_SYS_STATFS_H
#endif

#ifdef HAVE_GETTIMEOFDAY
#endif

#ifdef HAVE_GETPWNAM
#endif

#ifdef USE_TMPNAM
#else
#endif
