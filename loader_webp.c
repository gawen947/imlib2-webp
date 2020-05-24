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
#define _DEFAULT_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <webp/decode.h>
#include <webp/encode.h>
#include <webp/demux.h>
#include <Imlib2.h>

#include "imlib2_common.h"
#include "loader.h"

/* Store version and commit as unused string constant. */
#ifdef VERSION
const char loader_webp_version[] = VERSION;
#endif
#ifdef COMMIT
const char loader_webp_commit[] = COMMIT;
#endif

DATA32 * __imlib_AllocateData(ImlibImage *im);

static uint8_t * read_file(const char *filename, size_t *size,
                           ImlibProgressFunction progress)
{
  struct stat buf;
  uint8_t *data = NULL;
  int fd;

#ifndef __EMX__
  if((fd = open(filename, O_RDONLY)) < 0)
#else
  if((fd = open(filename, O_RDONLY | O_BINARY)) < 0)
#endif
    return NULL;

  if(fstat(fd, &buf) < 0 ||
     !(data = malloc(buf.st_size)))
    goto EXIT;

  *size = read(fd, data, buf.st_size);

EXIT:
  close(fd);
  return data;
}

char load(ImlibImage * im, ImlibProgressFunction progress,
          char progress_granularity, char immediate_load)
{
  uint8_t *data;
  size_t size;
  int has_alpha;
  int has_animation;
  char ret = 0;
#if (WEBP_DECODER_ABI_VERSION >= 0x200)
  WebPBitstreamFeatures features;
#endif

  /* Load the WebP file into one single buffer. */
  if(!(data = read_file(im->real_file, &size, progress)))
    return 0;

  /* Extract WebP features. */
#if (WEBP_DECODER_ABI_VERSION >= 0x200)
  if(WebPGetFeatures(data, size, &features) != VP8_STATUS_OK)
    goto EXIT;
  im->w = features.width;
  im->h = features.height;
  has_alpha     = features.has_alpha;
  has_animation = features.has_animation;
#else /* compatibility with versions <= 0.1.3 */
  if (!WebPGetInfo(data, size, &im->w, &im->h))
    goto EXIT;
  has_alpha     = 0;
  has_animation = 0;
#endif

  if(!IMAGE_DIMENSIONS_OK(im->w, im->h))
    goto EXIT;

  if(!has_alpha)
    UNSET_FLAGS(im->flags, F_HAS_ALPHA);
  else
    SET_FLAGS(im->flags, F_HAS_ALPHA);

  im->format = strdup("webp");

  /* If this was not true, then we are only interested
     in the image size and format so we stop here. */
  if(im->loader || immediate_load || progress) {
    /* Now we are commited to load the image.
       Note that Imlib2 will call __imlib_FreeData(im)
       if we return with an error code. */
    __imlib_AllocateData(im);

    if(has_animation) {
      /* Unfortunately Imlib2 does not support animation.
         So we only decode the first frame for animated WebP. */
      struct WebPData webp_data = {
        .bytes = (uint8_t*)data,
        .size  = size
      };

      WebPDemuxer* demux = WebPDemux(&webp_data);
      WebPIterator iter;
      WebPDecoderConfig config;

      WebPInitDecoderConfig(&config);

      config.options.use_threads = 1;
      config.output.colorspace   = MODE_BGRA;

      config.output.u.RGBA.rgba        = (uint8_t *)im->data;
      config.output.u.RGBA.stride      = im->w * sizeof(DATA32);
      config.output.u.RGBA.size        = config.output.u.RGBA.stride * im->h;
      config.output.is_external_memory = 1;

      if(WebPDemuxGetFrame(demux, 1, &iter) &&
         WebPDecode(iter.fragment.bytes, iter.fragment.size, &config) == VP8_STATUS_OK)
        ret = 1;

      WebPDemuxReleaseIterator(&iter);
      WebPDemuxDelete(demux);

      if(!ret)
        goto EXIT;
    }
    else if (!WebPDecodeBGRAInto(data, size, (uint8_t *)im->data, im->w * im->h * sizeof(DATA32), im->w * sizeof(DATA32)))
      goto EXIT;
  }

  if(progress)
    progress(im, 100, 0, 0, im->w, im->h);

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

  /* Open the destination file. */
#ifndef __EMX__
  if((fd = open(im->real_file, O_WRONLY | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
#else
  if((fd = open(im->real_file, O_WRONLY | O_CREAT | O_BINARY,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
#endif
    return 0;

  /* Look for tags attached to image to get extra parameters like quality
     settings etc. - this is the "api" to hint for extra information for
     saver modules */
  tag = __imlib_GetTag(im, "compression");
  if(tag) {
    int compression = tag->val;

    if(compression < 0)
      compression = 0;
    else if(compression > 9)
      compression = 9;

    quality = ((9 - compression) * 100) / 9;
  }

  tag = __imlib_GetTag(im, "quality");
  if(tag) {
    quality = tag->val;

    if(quality < 0)
      quality = 0;
    else if(quality > 100)
      quality = 100;
  }

  fqual = (float)quality;

  if(!(size = WebPEncodeBGRA((const uint8_t *)im->data, im->w, im->h,
                             im->w << 2, fqual, &data)))
    goto EXIT;

  if(write(fd, data, size) != (ssize_t)size)
    goto EXIT;

  if(progress)
    progress(im, 100, 0, 0, im->w, im->h);

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
