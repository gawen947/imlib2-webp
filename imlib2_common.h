/* File: imlib2_common.h
   Time-stamp: <2011-10-10 00:41:40 gawen>

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

#ifndef _IMLIB2_COMMON_H_
#define _IMLIB2_COMMON_H_

/* most of this file comes directly from imlib2 source code */

#include <Imlib2.h>

#define UNSET_FLAGS(flags, f) ((flags) &= ~(f))
#define SET_FLAGS(flags, f) ((flags) |= (f))

typedef struct _imlibimage              ImlibImage;
# ifdef BUILD_X11
typedef struct _imlibimagepixmap        ImlibImagePixmap;
# endif
typedef struct _imlibborder             ImlibBorder;
typedef struct _imlibloader             ImlibLoader;
typedef struct _imlibimagetag           ImlibImageTag;

typedef int (*ImlibProgressFunction)(ImlibImage *im, char percent,
                                     int update_x, int update_y,
                                     int update_w, int update_h);
typedef void (*ImlibDataDestructorFunction)(ImlibImage *im, void *data);

enum _iflags
{
   F_NONE              = 0,
   F_HAS_ALPHA         = (1 << 0),
   F_UNLOADED          = (1 << 1),
   F_UNCACHEABLE       = (1 << 2),
   F_ALWAYS_CHECK_DISK = (1 << 3),
   F_INVALID           = (1 << 4),
   F_DONT_FREE_DATA    = (1 << 5),
   F_FORMAT_IRRELEVANT = (1 << 6),
   F_BORDER_IRRELEVANT = (1 << 7),
   F_ALPHA_IRRELEVANT  = (1 << 8)
};

typedef enum   _iflags                  ImlibImageFlags;

struct _imlibborder
{
   int left, right, top, bottom;
};

struct _imlibimagetag
{
   char           *key;
   int             val;
   void           *data;
   void          (*destructor)(ImlibImage *im, void *data);
   ImlibImageTag  *next;
};

struct _imlibimage
{
   char             *file;
   int               w, h;
   DATA32           *data;
   ImlibImageFlags   flags;
   time_t            moddate;
   ImlibBorder       border;
   int               references;
   ImlibLoader      *loader;
   char             *format;
   ImlibImage       *next;
   ImlibImageTag    *tags;
   char             *real_file;
   char             *key;
};

# ifdef BUILD_X11
struct _imlibimagepixmap
{
   int               w, h;
   Pixmap            pixmap, mask;
   Display          *display;
   Visual           *visual;
   int               depth;
   int               source_x, source_y, source_w, source_h;
   Colormap          colormap;
   char              antialias, hi_quality, dither_mask;
   ImlibBorder       border;
   ImlibImage       *image;
   char             *file;
   char              dirty;
   int               references;
   DATABIG           modification_count;
   ImlibImagePixmap *next;
};
# endif

struct _imlibloader
{
   char         *file;
   int           num_formats;
   char        **formats;
   void         *handle;
   char        (*load)(ImlibImage *im,
                       ImlibProgressFunction progress,
                       char progress_granularity, char immediate_load);
   char        (*save)(ImlibImage *im,
                       ImlibProgressFunction progress,
                       char progress_granularity);
   ImlibLoader  *next;
};

# define IMAGE_HAS_ALPHA(im) ((im)->flags & F_HAS_ALPHA)
# define IMAGE_IS_UNLOADED(im) ((im)->flags & F_UNLOADED)
# define IMAGE_IS_UNCACHEABLE(im) ((im)->flags & F_UNCACHEABLE)
# define IMAGE_ALWAYS_CHECK_DISK(im) ((im)->flags & F_ALWAYS_CHECK_DISK)
# define IMAGE_IS_VALID(im) (!((im)->flags & F_INVALID))
# define IMAGE_FREE_DATA(im) (!((im)->flags & F_DONT_FREE_DATA))

# define SET_FLAG(flags, f) ((flags) |= (f))
# define UNSET_FLAG(flags, f) ((flags) &= (~f))

# define IMAGE_DIMENSIONS_OK(w, h) \
   ( ((w) > 0) && ((h) > 0) && \
     ((unsigned long long)(w) * (unsigned long long)(h) <= (1ULL << 29) - 1) )

EAPI ImlibImageTag *__imlib_GetTag(ImlibImage *im, const char *key);

#endif /* _IMLIB2_COMMON_H_ */
