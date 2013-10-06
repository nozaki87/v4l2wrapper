#ifndef JPEGHELPER_H
#define JPEGHELPER_H

extern unsigned char  *load_jpg(char const *filename);
extern unsigned char  *load_memory_jpg(unsigned char *memory, int size);
extern int mjpeg2jpeg(unsigned char * dst, const unsigned char * src, int srcsize);

#endif // JPEGHELPER_H
