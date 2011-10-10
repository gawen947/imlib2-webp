/* File: thumbnailer.c
   Time-stamp: <2011-10-10 21:07:35 gawen>

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

#include <string.h>

#include <Imlib2.h>

static const char * get_selfname(const char *argv0) {
  const char *progname = (const char *)strrchr(argv0, '/');
  return progname ? (progname + 1) : argv0;
}

int main(int argc, char **argv)
{
  const char *tmp;
  int size, w, h;
  Imlib_Image src_img;
  Imlib_Image dst_img;

  if(argc != 4)
    errx(1, "invalid usage:\n"
         "  %s <source> <destination> <size>", get_selfname(argv[0]));

  size  = atoi(argv[3]);
  src_img = imlib_load_image(argv[1]);

  if(!src_img)
    errx(2, "cannot load source image");

  imlib_context_set_image(src_img);

  w = imlib_image_get_width();
  h = imlib_image_get_height();

  dst_img = imlib_create_cropped_scaled_image(0, 0,
                                              w, h,
                                              size * w / h, size);
  imlib_free_image();

  imlib_context_set_image(dst_img);

  tmp = strrchr(argv[2], '.');
  if(tmp)
    imlib_image_set_format(tmp + 1);

  imlib_save_image(argv[2]);

  imlib_free_image();

  return 0;
}

