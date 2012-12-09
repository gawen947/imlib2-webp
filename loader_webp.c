/* File: loader_webp.c
   Time-stamp: <2012-12-09 21:19:30 gawen>

   Copyright (c) 2011 David Hauweele <david@hauweele.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE. */

#define _BSD_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <webp/decode.h>
#include <webp/encode.h>
#include <Imlib2.h>

#include "imlib2_common.h"
#include "loader.h"

static uint8_t * read_file(const char *filename, size_t *size,
                           ImlibProgressFunction progress)
{
  struct stat buf;
  uint8_t *data;
  int fd;

#ifndef __EMX__
  if((fd = open(filename, O_RDONLY)) < 0)
#else
  if((fd = open(filename, O_RDONLY | O_BINARY)) < 0)
#endif
    return NULL;

  if(fstat(fd, &buf) < 0 ||
     !(data = malloc(buf.st_size)))
    return NULL;

  *size = read(fd, data, buf.st_size);

  return data;
}

char load(ImlibImage * im, ImlibProgressFunction progress,
          char progress_granularity, char immediate_load)
{
  uint8_t *data;
  size_t size;
  int w,h;
  int has_alpha;
#if (WEBP_DECODER_ABI_VERSION >= 0x200)
  WebPBitstreamFeatures features;
#endif
  char ret = 0;

  if(im->data)
    return 0;

  if(!(data = read_file(im->real_file, &size, progress)))
    return 0;

#if (WEBP_DECODER_ABI_VERSION >= 0x200)
  if(WebPGetFeatures(data, size, &features) != VP8_STATUS_OK)
    goto EXIT;
  w = features.width;
  h = features.height;
  has_alpha = features.has_alpha;
#else /* compatibility with versions <= 0.1.3 */
  if (!WebPGetInfo(data, size, &w, &h))
    goto EXIT;
  has_alpha = 0;
#endif

  if(!im->loader && !im->data) {
    im->w = w;
    im->h = h;

    if(!IMAGE_DIMENSIONS_OK(w, h))
      goto EXIT;

    if(!has_alpha)
      UNSET_FLAGS(im->flags, F_HAS_ALPHA);
    else
      SET_FLAGS(im->flags, F_HAS_ALPHA);
    im->format = strdup("webp");
  }

  if((!im->data && im->loader) || immediate_load || progress)
     im->data = (DATA32*)WebPDecodeBGRA(data, size, &w, &h);

  if(progress)
    progress(im, 100, 0, 0, 0, 0);

  ret = 1;

EXIT:
  free(data);
  return ret;
}

char save(ImlibImage *im, ImlibProgressFunction progress,
          char progress_granularity)
{
  ImlibImageTag *tag;
  uint8_t *data;
  float fqual;
  size_t size;
  int fd;
  int quality = 75;
  char ret = 0;

  if(!im->data)
    return 0;

#ifndef __EMX__
  if((fd = open(im->real_file, O_WRONLY | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
#else
  if((fd = open(im->real_file, O_WRONLY | O_CREAT | O_BINARY,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
#endif
    return 0;

  /* look for tags attached to image to get extra parameters like quality
     settings etc. - this is the "api" to hint for extra information for
     saver modules */
  tag = __imlib_GetTag(im, "compression");
  if(tag) {
    int compression = tag->val;

    if(compression < 0)
      compression = 0;
    else if(compression > 9)
      compression = 9;

    quality = (9 - compression) * 10;
    quality = quality * 10 / 9;
  }
  tag = __imlib_GetTag(im, "quality");
  if(tag) {
    quality = tag->val;

    if(quality < 1)
      quality = 1;
    else if(quality > 100)
      quality = 100;
  }

  fqual = (float)quality;

  if(!(size = WebPEncodeBGRA((const uint8_t *)im->data, im->w, im->h,
                             im->w << 2, fqual, &data)))
    goto EXIT;

  if(write(fd, data, size) != size)
    goto EXIT;

  if(progress)
    progress(im, 100, 0, 0, 0, 0);

  ret = 1;

EXIT:
  close(fd);
  if(data)
    free(data);
  return ret;
}

void formats(ImlibLoader *l)
{
  int i;
  char *list_formats[] = { "webp" };

  l->num_formats = (sizeof(list_formats) / sizeof(char *));
  l->formats     = malloc(sizeof(char *) * l->num_formats);
  for(i = 0 ; i < l->num_formats ; i++)
    l->formats[i] = strdup(list_formats[i]);
}
