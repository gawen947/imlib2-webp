/* File: loader_webp.c
   Time-stamp: <2011-10-10 02:00:19 gawen>

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
#include <string.h>
#include <fcntl.h>

#include <webp/decode.h>
#include <webp/encode.h>
#include <Imlib2.h>

#include "imlib2_common.h"
#include "loader.h"

#define BLK_SZ 524288

static uint8_t * read_file(const char *filename, size_t *size,
                           ImlibProgressFunction progress)
{
  size_t i      = 0;
  size_t s      = 0;
  uint8_t *data = NULL;
  int fd;

#ifndef __EMX__
  if((fd = open(filename, O_RDONLY)) < 0)
#else
  if((fd = open(filename, O_RDONLY | O_BINARY)) < 0)
#endif
    return NULL;

  while(1) {
    size_t n;

    if(s - i == 0) {
      s += BLK_SZ;
      data = realloc(data, s);

      if(!data)
        return NULL;
    }

    n = read(fd, data + i, s - i);

    i += n;

    if(n <= 0)
      break;
  }

  close(fd);

  *size = i;

  return data;
}

char load(ImlibImage * im, ImlibProgressFunction progress,
          char progress_granularity, char immediate_load)
{
  uint8_t *data;
  size_t size;
  int w,h;
  char ret = 0;

  if(im->data)
    return 0;

  if(!(data = read_file(im->real_file, &size, progress)))
    return 0;

  if(!WebPGetInfo(data, size, &w, &h))
    goto EXIT;

  if(!im->loader && !im->data) {
    im->w = w;
    im->h = h;

    if(!IMAGE_DIMENSIONS_OK(w, h))
      goto EXIT;

    UNSET_FLAGS(im->flags, F_HAS_ALPHA);
    im->format = strdup("webp");
  }

  if((!im->data && im->loader) || immediate_load || progress)
    im->data = (DATA32*)WebPDecodeRGBA(data, size, &w, &h);

  ret = 1;

EXIT:
  free(data);
  return ret;
}

char save(ImlibImage *im, ImlibProgressFunction progress,
          char progress_granularity)
{
  return 0;
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
