# Imlib2-WebP

A WebP loader for Imlib2. Published under the BSD license.

## About WebP

WebP is a new image format that provides lossy compression for photographic 
images. In a large scale study of 900,000 web images, WebP images were 39.8%
smaller than jpeg images of similar quality.

A WebP file consists of VP8 image data, and a container based on RIFF. The 
standalone libwebp library serves as a reference implementation for the 
WebP specification and is available at this git repository and as a tarball. 
Webmasters and web developers can use the WebP image format to create smaller 
and better looking images that can help make the web faster.

You may find more informations at http://code.google.com/speed/webp

## About Imlib2

Imlib2 is an advanced replacement for libraries like libXpm. Imlib2 provides
many more features with much greater flexibility and speed than standard
libraries, including font rasterization, rotation, RGBA space rendering and 
blending, dynamic binary filters, scripting, and more.

You may find more informations at http://docs.enlightenment.org/api/imlib2/html

## INSTALLATION

You will need a C compiler like gcc, make and the following dependencies :

* libimlib2-dev
* libwebp-dev
* pkg-config

On debian use the following command as root to install the required packages :

    # aptitude install libimlib2-dev libwebp-dev pkg-config

Then enter the following command :

    $ make
    $ sudo make install

This will install the webp loader for Imlib2.

Please take advice that BSD make probably won't work, you need to use gmake
instead.
