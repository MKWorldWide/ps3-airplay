# AIR PS3 artwork

`artwork-source.png` contains the supplied artwork's decoded RGB pixels in a
lossless PNG with source metadata removed. The image pixels are unchanged from
the supplied JPEG. `ICON0.PNG` and `PIC1.PNG` are Lanczos-resized derivatives;
no crop, color treatment, or other visual edit is applied.

The release gate verifies both derivatives against the source pixels before
packaging. The standalone screen background is generated from the same source.
