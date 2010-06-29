/* ooooooooooooooooooooooooooooooooooooo */
/* Copyright (C) 1991-03   P. Gburzynski */
/* ooooooooooooooooooooooooooooooooooooo */

/* --- */

/* ------------------------------------------------------------- */
/* SMURPH version of the C++ library (the minimum needed subset) */
/* ------------------------------------------------------------- */

#define	OBUFSIZE  256		// Size of an ostream buffer

#include        <sys/types.h>
#include	<sys/wait.h>
#include        <sys/time.h>
#include        <sys/times.h>
#include 	<sys/stat.h>
#include 	<sys/file.h>
#include        <setjmp.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <unistd.h>
#include        <stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#include	<stdarg.h>
#include	<time.h>
#include	<math.h>

#include	<iostream>
#include	<fstream>

/* --- END STANDARD INCLUDES --- */
