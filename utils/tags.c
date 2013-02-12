/**
 * @file  tags.c
 * @brief utils for adding tags (meta info) to mgz/h files
 *
 */
/*
 * Original Author: Bruce Fischl
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2011/06/17 02:37:48 $
 *    $Revision: 1.15 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fio.h"
#include "error.h"
#include "tags.h"

int
TAGskip(FILE *fp, int tag, long long len)
{
#if 1
  unsigned char *buf ;
  int ret ;

  buf = (unsigned char *)calloc(len, sizeof(unsigned char)) ;
  if (buf==NULL)
    ErrorExit(ERROR_NOMEMORY,
              "TAGskip: tag=%d, failed to calloc %u bytes!\n",tag, len);
  ret = fread(buf, sizeof(unsigned char), len, fp) ;
  free(buf) ;
  return(ret) ;
#else
  return(fseek(fp, len, SEEK_CUR)) ;  // doesn't work for gzipped files
#endif
}

int TAGreadStart(FILE *fp, long long *plen)
{
  int  tag ;

  tag = freadInt(fp) ;
  if (feof(fp))
    return(0) ;
  switch (tag)
  {
  case TAG_OLD_MGH_XFORM:
    *plen = (long long)freadInt(fp) ;  /* sorry - backwards compatibility
                                          with Tosa's stuff */
    *plen = *plen -1 ; // doesn't include null
    break ;
  case TAG_OLD_SURF_GEOM:    // these don't take lengths at all
  case TAG_OLD_USEREALRAS:
  case TAG_OLD_COLORTABLE:
    *plen = 0 ;
    break ;
  default:
    *plen = freadLong(fp) ;
  }

  return(tag) ;
}

int
TAGwriteStart(FILE *fp, int tag, long long *phere, long long len)
{
  long here ;

  fwriteInt(tag, fp) ;

  here = ftell(fp) ;
  *phere = (long long)here ;
  fwriteLong(len, fp) ;

  return(NO_ERROR) ;
}

int TAGwrite(FILE *fp, int tag, void *buf, long long len)
{
  long long here ;

  TAGwriteStart(fp, tag, &here, len) ;
  fwrite(buf, sizeof(char), len, fp) ;
  TAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

int
TAGwriteEnd(FILE *fp, long long there)
{
  long long here ;

  here = ftell(fp) ;
#if 0
  fseek(fp, there, SEEK_SET) ;
  fwriteLong((long long)(here-(there+sizeof(long long))), fp) ;
  fseek(fp, here, SEEK_SET) ;
#endif

  return(NO_ERROR) ;
}

int
TAGmakeCommandLineString(int argc, char **argv, char *cmd_line)
{
  int i ;

  cmd_line[0] = 0 ;
  for (i = 0 ; i < argc ; i++)
  {
    strcat(cmd_line, argv[i]) ;
    strcat(cmd_line, " ") ;
  }
  return(NO_ERROR) ;
}

int
TAGwriteCommandLine(FILE *fp, char *cmd_line)
{
  long long here ;

  TAGwriteStart(fp, TAG_CMDLINE, &here, strlen(cmd_line)+1) ;
  fprintf(fp, "%s", cmd_line) ;
  TAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

int TAGwriteAutoAlign(FILE *fp, MATRIX *M)
{
  long long here ;
  char buf[16*100];
  long long len;

  bzero(buf,16*100);
  sprintf(buf,"AutoAlign %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf",
	  M->rptr[1][1],M->rptr[1][2],M->rptr[1][3],M->rptr[1][4],
	  M->rptr[2][1],M->rptr[2][2],M->rptr[2][3],M->rptr[2][4],
	  M->rptr[3][1],M->rptr[3][2],M->rptr[3][3],M->rptr[3][4],
	  M->rptr[4][1],M->rptr[4][2],M->rptr[4][3],M->rptr[4][4]);
  len = strlen(buf);
  TAGwriteStart(fp, TAG_AUTO_ALIGN, &here, len) ;
  fwrite(buf, sizeof(char), len, fp) ;

  TAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

MATRIX *TAGreadAutoAlign(FILE *fp)
{
  int c,r;
  char buf[1000];
  MATRIX *M;

  M = MatrixAlloc(4,4,MATRIX_REAL);
  fscanf(fp,"%s",buf); // get past "AutoAlign" string
  for(r=1; r<=4; r++){
    for(c=1; c<=4; c++){
      fscanf(fp,"%f",&(M->rptr[r][c]));
    }
  }
  return(M);
}


/* zlib support */

int
znzTAGskip(znzFile fp, int tag, long long len)
{
#if 1
  unsigned char *buf ;
  int ret ;

  buf = (unsigned char *)calloc(len, sizeof(unsigned char)) ;
  if (buf==NULL)
    ErrorExit(ERROR_NOMEMORY,
              "znzTAGskip: tag=%d, failed to calloc %u bytes!\n",tag, len);
  ret = znzread(buf, sizeof(unsigned char), len, fp) ;
  free(buf) ;
  return(ret) ;
#else
  return(znzseek(fp, len, SEEK_CUR)) ;  // doesn't work for gzipped files
#endif
}

int znzTAGreadStart(znzFile fp, long long *plen)
{
  int  tag ;

  tag = znzreadInt(fp);
  if (znzeof(fp))
    return(0) ;

  switch (tag)
  {
  case TAG_OLD_MGH_XFORM:
    *plen = (long long)znzreadInt(fp) ;  /* sorry - backwards compatibility
                                            with Tosa's stuff */
    *plen = *plen -1 ; // doesn't include null
    break ;
  case TAG_OLD_SURF_GEOM:    // these don't take lengths at all
  case TAG_OLD_USEREALRAS:
  case TAG_OLD_COLORTABLE:
    *plen = 0 ;
    break ;
  default:
    *plen = znzreadLong(fp) ;
  }

  return(tag) ;
}

int
znzTAGwriteStart(znzFile fp, int tag, long long *phere, long long len)
{
  long here ;

  znzwriteInt(tag, fp) ;

  here = znztell(fp) ;
  *phere = (long long)here ;
  znzwriteLong(len, fp) ;

  return(NO_ERROR) ;
}

int znzTAGwrite(znzFile fp, int tag, void *buf, long long len)
{
  long long here ;

  znzTAGwriteStart(fp, tag, &here, len) ;
  znzwrite(buf, sizeof(char), len, fp) ;
  znzTAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

int
znzTAGwriteEnd(znzFile fp, long long there)
{
  long long here ;

  here = znztell(fp) ;
#if 0
  fseek(fp, there, SEEK_SET) ;
  fwriteLong((long long)(here-(there+sizeof(long long))), fp) ;
  fseek(fp, here, SEEK_SET) ;
#endif

  return(NO_ERROR) ;
}

int
znzTAGwriteCommandLine(znzFile fp, char *cmd_line)
{
  long long here ;

  znzTAGwriteStart(fp, TAG_CMDLINE, &here, strlen(cmd_line)+1) ;
  znzprintf(fp, "%s", cmd_line) ;
  znzTAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

#define MATRIX_STRLEN  (4*4*100)
int znzWriteMatrix(znzFile fp, MATRIX *M)
{
  long long here, len ;
  char buf[MATRIX_STRLEN];

  bzero(buf,MATRIX_STRLEN);
  sprintf(buf,"AutoAlign %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf",
    M->rptr[1][1],M->rptr[1][2],M->rptr[1][3],M->rptr[1][4],
    M->rptr[2][1],M->rptr[2][2],M->rptr[2][3],M->rptr[2][4],
    M->rptr[3][1],M->rptr[3][2],M->rptr[3][3],M->rptr[3][4],
    M->rptr[4][1],M->rptr[4][2],M->rptr[4][3],M->rptr[4][4]);
  znzTAGwriteStart(fp, TAG_AUTO_ALIGN, &len, MATRIX_STRLEN) ;
  here = znztell(fp) ;
  znzwrite(buf, sizeof(char), MATRIX_STRLEN, fp) ;
  here = znztell(fp) ;

  znzTAGwriteEnd(fp, len) ;
  return(NO_ERROR) ;
}

MATRIX *znzReadMatrix(znzFile fp)
{
  /* no fscanf equivalent in zlib!! have to hack it */
  char buf[MATRIX_STRLEN];
  MATRIX *M;
  char ch[100];
  int  ret ;
  long long here, len ;
  
  znzTAGreadStart(fp, &len) ;
  M = MatrixAlloc(4,4,MATRIX_REAL);
  here = znztell(fp) ;
  ret = znzread(buf, sizeof(unsigned char), MATRIX_STRLEN, fp) ;
  here = znztell(fp) ;
  sscanf(buf,"%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", ch, 
         &(M->rptr[1][1]),
         &(M->rptr[1][2]),
         &(M->rptr[1][3]),
         &(M->rptr[1][4]),
         &(M->rptr[2][1]),
         &(M->rptr[2][2]),
         &(M->rptr[2][3]),
         &(M->rptr[2][4]),
         &(M->rptr[3][1]),
         &(M->rptr[3][2]),
         &(M->rptr[3][3]),
         &(M->rptr[3][4]),
         &(M->rptr[4][1]),
         &(M->rptr[4][2]),
         &(M->rptr[4][3]),
         &(M->rptr[4][4])
        ); 
  
  return(M);
}

int znzWriteAutoAlignMatrix(znzFile fp, MATRIX *M)
{
  long long here ;
  char buf[16*100];
  long long len;

  bzero(buf,16*100);
  sprintf(buf,"AutoAlign %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf %10lf",
    M->rptr[1][1],M->rptr[1][2],M->rptr[1][3],M->rptr[1][4],
    M->rptr[2][1],M->rptr[2][2],M->rptr[2][3],M->rptr[2][4],
    M->rptr[3][1],M->rptr[3][2],M->rptr[3][3],M->rptr[3][4],
    M->rptr[4][1],M->rptr[4][2],M->rptr[4][3],M->rptr[4][4]);
  len = strlen(buf);
  znzTAGwriteStart(fp, TAG_AUTO_ALIGN, &here, len) ;
  znzwrite(buf, sizeof(char), len, fp) ;

  znzTAGwriteEnd(fp, here) ;
  return(NO_ERROR) ;
}

MATRIX *znzReadAutoAlignMatrix(znzFile fp)
{
  /* no fscanf equivalent in zlib!! have to hack it */
  char buf[16*100];
  bzero(buf,16*100);
  
  int cnt = 0;
  int c = 0;
  while ( (c = znzgetc(fp)) != '\0' )
  {
    buf[cnt] = c;
    cnt++;
  }
  znzseek(fp, -1, SEEK_CUR);
  
  MATRIX *M;
  M = MatrixAlloc(4,4,MATRIX_REAL);
  char ch[100];
  sscanf(buf,"%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", ch, 
         &(M->rptr[1][1]),
         &(M->rptr[1][2]),
         &(M->rptr[1][3]),
         &(M->rptr[1][4]),
         &(M->rptr[2][1]),
         &(M->rptr[2][2]),
         &(M->rptr[2][3]),
         &(M->rptr[2][4]),
         &(M->rptr[3][1]),
         &(M->rptr[3][2]),
         &(M->rptr[3][3]),
         &(M->rptr[3][4]),
         &(M->rptr[4][1]),
         &(M->rptr[4][2]),
         &(M->rptr[4][3]),
         &(M->rptr[4][4])
        ); 
  
  return(M);
}
