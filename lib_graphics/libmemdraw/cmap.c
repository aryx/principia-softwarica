/*s: lib_graphics/libmemdraw/cmap.c */
/*
 * generated by mkcmap.c
 */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: global [[def]] */
static Memcmap def = {
/* cmap2rgb */ {
    0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x00,0xcc,0x00,0x44,0x00,0x00,
    0x44,0x44,0x00,0x44,0x88,0x00,0x44,0xcc,0x00,0x88,0x00,0x00,0x88,0x44,0x00,0x88,
    0x88,0x00,0x88,0xcc,0x00,0xcc,0x00,0x00,0xcc,0x44,0x00,0xcc,0x88,0x00,0xcc,0xcc,
    0x00,0xdd,0xdd,0x11,0x11,0x11,0x00,0x00,0x55,0x00,0x00,0x99,0x00,0x00,0xdd,0x00,
    0x55,0x00,0x00,0x55,0x55,0x00,0x4c,0x99,0x00,0x49,0xdd,0x00,0x99,0x00,0x00,0x99,
    0x4c,0x00,0x99,0x99,0x00,0x93,0xdd,0x00,0xdd,0x00,0x00,0xdd,0x49,0x00,0xdd,0x93,
    0x00,0xee,0x9e,0x00,0xee,0xee,0x22,0x22,0x22,0x00,0x00,0x66,0x00,0x00,0xaa,0x00,
    0x00,0xee,0x00,0x66,0x00,0x00,0x66,0x66,0x00,0x55,0xaa,0x00,0x4f,0xee,0x00,0xaa,
    0x00,0x00,0xaa,0x55,0x00,0xaa,0xaa,0x00,0x9e,0xee,0x00,0xee,0x00,0x00,0xee,0x4f,
    0x00,0xff,0x55,0x00,0xff,0xaa,0x00,0xff,0xff,0x33,0x33,0x33,0x00,0x00,0x77,0x00,
    0x00,0xbb,0x00,0x00,0xff,0x00,0x77,0x00,0x00,0x77,0x77,0x00,0x5d,0xbb,0x00,0x55,
    0xff,0x00,0xbb,0x00,0x00,0xbb,0x5d,0x00,0xbb,0xbb,0x00,0xaa,0xff,0x00,0xff,0x00,
    0x44,0x00,0x44,0x44,0x00,0x88,0x44,0x00,0xcc,0x44,0x44,0x00,0x44,0x44,0x44,0x44,
    0x44,0x88,0x44,0x44,0xcc,0x44,0x88,0x00,0x44,0x88,0x44,0x44,0x88,0x88,0x44,0x88,
    0xcc,0x44,0xcc,0x00,0x44,0xcc,0x44,0x44,0xcc,0x88,0x44,0xcc,0xcc,0x44,0x00,0x00,
    0x55,0x00,0x00,0x55,0x00,0x55,0x4c,0x00,0x99,0x49,0x00,0xdd,0x55,0x55,0x00,0x55,
    0x55,0x55,0x4c,0x4c,0x99,0x49,0x49,0xdd,0x4c,0x99,0x00,0x4c,0x99,0x4c,0x4c,0x99,
    0x99,0x49,0x93,0xdd,0x49,0xdd,0x00,0x49,0xdd,0x49,0x49,0xdd,0x93,0x49,0xdd,0xdd,
    0x4f,0xee,0xee,0x66,0x00,0x00,0x66,0x00,0x66,0x55,0x00,0xaa,0x4f,0x00,0xee,0x66,
    0x66,0x00,0x66,0x66,0x66,0x55,0x55,0xaa,0x4f,0x4f,0xee,0x55,0xaa,0x00,0x55,0xaa,
    0x55,0x55,0xaa,0xaa,0x4f,0x9e,0xee,0x4f,0xee,0x00,0x4f,0xee,0x4f,0x4f,0xee,0x9e,
    0x55,0xff,0xaa,0x55,0xff,0xff,0x77,0x00,0x00,0x77,0x00,0x77,0x5d,0x00,0xbb,0x55,
    0x00,0xff,0x77,0x77,0x00,0x77,0x77,0x77,0x5d,0x5d,0xbb,0x55,0x55,0xff,0x5d,0xbb,
    0x00,0x5d,0xbb,0x5d,0x5d,0xbb,0xbb,0x55,0xaa,0xff,0x55,0xff,0x00,0x55,0xff,0x55,
    0x88,0x00,0x88,0x88,0x00,0xcc,0x88,0x44,0x00,0x88,0x44,0x44,0x88,0x44,0x88,0x88,
    0x44,0xcc,0x88,0x88,0x00,0x88,0x88,0x44,0x88,0x88,0x88,0x88,0x88,0xcc,0x88,0xcc,
    0x00,0x88,0xcc,0x44,0x88,0xcc,0x88,0x88,0xcc,0xcc,0x88,0x00,0x00,0x88,0x00,0x44,
    0x99,0x00,0x4c,0x99,0x00,0x99,0x93,0x00,0xdd,0x99,0x4c,0x00,0x99,0x4c,0x4c,0x99,
    0x4c,0x99,0x93,0x49,0xdd,0x99,0x99,0x00,0x99,0x99,0x4c,0x99,0x99,0x99,0x93,0x93,
    0xdd,0x93,0xdd,0x00,0x93,0xdd,0x49,0x93,0xdd,0x93,0x93,0xdd,0xdd,0x99,0x00,0x00,
    0xaa,0x00,0x00,0xaa,0x00,0x55,0xaa,0x00,0xaa,0x9e,0x00,0xee,0xaa,0x55,0x00,0xaa,
    0x55,0x55,0xaa,0x55,0xaa,0x9e,0x4f,0xee,0xaa,0xaa,0x00,0xaa,0xaa,0x55,0xaa,0xaa,
    0xaa,0x9e,0x9e,0xee,0x9e,0xee,0x00,0x9e,0xee,0x4f,0x9e,0xee,0x9e,0x9e,0xee,0xee,
    0xaa,0xff,0xff,0xbb,0x00,0x00,0xbb,0x00,0x5d,0xbb,0x00,0xbb,0xaa,0x00,0xff,0xbb,
    0x5d,0x00,0xbb,0x5d,0x5d,0xbb,0x5d,0xbb,0xaa,0x55,0xff,0xbb,0xbb,0x00,0xbb,0xbb,
    0x5d,0xbb,0xbb,0xbb,0xaa,0xaa,0xff,0xaa,0xff,0x00,0xaa,0xff,0x55,0xaa,0xff,0xaa,
    0xcc,0x00,0xcc,0xcc,0x44,0x00,0xcc,0x44,0x44,0xcc,0x44,0x88,0xcc,0x44,0xcc,0xcc,
    0x88,0x00,0xcc,0x88,0x44,0xcc,0x88,0x88,0xcc,0x88,0xcc,0xcc,0xcc,0x00,0xcc,0xcc,
    0x44,0xcc,0xcc,0x88,0xcc,0xcc,0xcc,0xcc,0x00,0x00,0xcc,0x00,0x44,0xcc,0x00,0x88,
    0xdd,0x00,0x93,0xdd,0x00,0xdd,0xdd,0x49,0x00,0xdd,0x49,0x49,0xdd,0x49,0x93,0xdd,
    0x49,0xdd,0xdd,0x93,0x00,0xdd,0x93,0x49,0xdd,0x93,0x93,0xdd,0x93,0xdd,0xdd,0xdd,
    0x00,0xdd,0xdd,0x49,0xdd,0xdd,0x93,0xdd,0xdd,0xdd,0xdd,0x00,0x00,0xdd,0x00,0x49,
    0xee,0x00,0x4f,0xee,0x00,0x9e,0xee,0x00,0xee,0xee,0x4f,0x00,0xee,0x4f,0x4f,0xee,
    0x4f,0x9e,0xee,0x4f,0xee,0xee,0x9e,0x00,0xee,0x9e,0x4f,0xee,0x9e,0x9e,0xee,0x9e,
    0xee,0xee,0xee,0x00,0xee,0xee,0x4f,0xee,0xee,0x9e,0xee,0xee,0xee,0xee,0x00,0x00,
    0xff,0x00,0x00,0xff,0x00,0x55,0xff,0x00,0xaa,0xff,0x00,0xff,0xff,0x55,0x00,0xff,
    0x55,0x55,0xff,0x55,0xaa,0xff,0x55,0xff,0xff,0xaa,0x00,0xff,0xaa,0x55,0xff,0xaa,
    0xaa,0xff,0xaa,0xff,0xff,0xff,0x00,0xff,0xff,0x55,0xff,0xff,0xaa,0xff,0xff,0xff,
},
/* rgb2cmap */ {
    0x00,0x00,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x00,0x11,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x11,0x11,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x17,0x07,0x07,0x18,0x18,0x29,
    0x04,0x04,0x04,0x05,0x05,0x05,0x16,0x06,0x06,0x17,0x28,0x07,0x07,0x18,0x29,0x3a,
    0x15,0x15,0x15,0x05,0x05,0x16,0x16,0x06,0x06,0x17,0x28,0x39,0x07,0x18,0x29,0x3a,
    0x26,0x26,0x26,0x05,0x16,0x16,0x27,0x27,0x38,0x28,0x28,0x39,0x39,0x29,0x29,0x3a,
    0x37,0x37,0x37,0x09,0x09,0x09,0x27,0x38,0x0a,0x0a,0x39,0x0b,0x0b,0x0b,0x1c,0x3a,
    0x08,0x08,0x08,0x09,0x09,0x09,0x38,0x0a,0x0a,0x0a,0x1b,0x0b,0x0b,0x1c,0x1c,0x2d,
    0x19,0x19,0x19,0x09,0x1a,0x1a,0x2b,0x0a,0x0a,0x1b,0x1b,0x0b,0x0b,0x1c,0x2d,0x3e,
    0x2a,0x2a,0x2a,0x1a,0x2b,0x2b,0x2b,0x3c,0x1b,0x1b,0x2c,0x2c,0x3d,0x2d,0x2d,0x3e,
    0x3b,0x3b,0x3b,0x0d,0x0d,0x3c,0x3c,0x0e,0x0e,0x0e,0x2c,0x3d,0x0f,0x0f,0x3e,0x3e,
    0x0c,0x0c,0x0c,0x0d,0x0d,0x0d,0x3c,0x0e,0x0e,0x0e,0x3d,0x0f,0x0f,0x0f,0x10,0x3e,
    0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x2f,0x0e,0x1f,0x1f,0x20,0x0f,0x0f,0x10,0x10,0x21,
    0x2e,0x2e,0x2e,0x1e,0x2f,0x2f,0x2f,0x1f,0x1f,0x20,0x20,0x31,0x10,0x10,0x21,0x21,
    0x3f,0x3f,0x3f,0x2f,0x30,0x30,0x30,0x30,0x20,0x31,0x31,0x31,0x31,0x21,0x21,0x32,
    0x00,0x11,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x11,0x11,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x11,0x11,0x22,0x22,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x04,0x04,0x22,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x17,0x07,0x07,0x18,0x18,0x29,
    0x04,0x04,0x04,0x05,0x05,0x05,0x16,0x06,0x06,0x17,0x28,0x07,0x07,0x18,0x29,0x3a,
    0x15,0x15,0x15,0x05,0x05,0x16,0x16,0x06,0x06,0x17,0x28,0x39,0x07,0x18,0x29,0x3a,
    0x26,0x26,0x26,0x05,0x16,0x16,0x27,0x27,0x38,0x28,0x28,0x39,0x39,0x29,0x29,0x3a,
    0x37,0x37,0x37,0x09,0x09,0x09,0x27,0x38,0x0a,0x0a,0x39,0x0b,0x0b,0x0b,0x1c,0x3a,
    0x08,0x08,0x08,0x09,0x09,0x09,0x38,0x0a,0x0a,0x0a,0x1b,0x0b,0x0b,0x1c,0x1c,0x2d,
    0x19,0x19,0x19,0x09,0x1a,0x1a,0x2b,0x0a,0x0a,0x1b,0x1b,0x0b,0x0b,0x1c,0x2d,0x3e,
    0x2a,0x2a,0x2a,0x1a,0x2b,0x2b,0x2b,0x3c,0x1b,0x1b,0x2c,0x2c,0x3d,0x2d,0x2d,0x3e,
    0x3b,0x3b,0x3b,0x0d,0x0d,0x3c,0x3c,0x0e,0x0e,0x0e,0x2c,0x3d,0x0f,0x0f,0x3e,0x3e,
    0x0c,0x0c,0x0c,0x0d,0x0d,0x0d,0x3c,0x0e,0x0e,0x0e,0x3d,0x0f,0x0f,0x0f,0x10,0x3e,
    0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x2f,0x0e,0x1f,0x1f,0x20,0x0f,0x0f,0x10,0x10,0x21,
    0x2e,0x2e,0x2e,0x1e,0x2f,0x2f,0x2f,0x1f,0x1f,0x20,0x20,0x31,0x10,0x10,0x21,0x21,
    0x3f,0x3f,0x3f,0x2f,0x30,0x30,0x30,0x30,0x20,0x31,0x31,0x31,0x31,0x21,0x21,0x32,
    0x11,0x11,0x11,0x01,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x11,0x11,0x22,0x22,0x01,0x12,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x11,0x22,0x22,0x22,0x33,0x33,0x23,0x34,0x02,0x13,0x24,0x35,0x03,0x14,0x25,0x36,
    0x04,0x22,0x22,0x33,0x33,0x33,0x05,0x06,0x06,0x06,0x17,0x07,0x07,0x18,0x18,0x29,
    0x04,0x04,0x33,0x33,0x33,0x05,0x16,0x06,0x06,0x17,0x28,0x07,0x07,0x18,0x29,0x3a,
    0x15,0x15,0x33,0x33,0x05,0x16,0x16,0x06,0x06,0x17,0x28,0x39,0x07,0x18,0x29,0x3a,
    0x26,0x26,0x26,0x05,0x16,0x16,0x27,0x27,0x38,0x28,0x28,0x39,0x39,0x29,0x29,0x3a,
    0x37,0x37,0x37,0x09,0x09,0x09,0x27,0x38,0x0a,0x0a,0x39,0x0b,0x0b,0x0b,0x1c,0x3a,
    0x08,0x08,0x08,0x09,0x09,0x09,0x38,0x0a,0x0a,0x0a,0x1b,0x0b,0x0b,0x1c,0x1c,0x2d,
    0x19,0x19,0x19,0x09,0x1a,0x1a,0x2b,0x0a,0x0a,0x1b,0x1b,0x0b,0x0b,0x1c,0x2d,0x3e,
    0x2a,0x2a,0x2a,0x1a,0x2b,0x2b,0x2b,0x3c,0x1b,0x1b,0x2c,0x2c,0x3d,0x2d,0x2d,0x3e,
    0x3b,0x3b,0x3b,0x0d,0x0d,0x3c,0x3c,0x0e,0x0e,0x0e,0x2c,0x3d,0x0f,0x0f,0x3e,0x3e,
    0x0c,0x0c,0x0c,0x0d,0x0d,0x0d,0x3c,0x0e,0x0e,0x0e,0x3d,0x0f,0x0f,0x0f,0x10,0x3e,
    0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x2f,0x0e,0x1f,0x1f,0x20,0x0f,0x0f,0x10,0x10,0x21,
    0x2e,0x2e,0x2e,0x1e,0x2f,0x2f,0x2f,0x1f,0x1f,0x20,0x20,0x31,0x10,0x10,0x21,0x21,
    0x3f,0x3f,0x3f,0x2f,0x30,0x30,0x30,0x30,0x20,0x31,0x31,0x31,0x31,0x21,0x21,0x32,
    0x4f,0x4f,0x22,0x40,0x40,0x40,0x40,0x41,0x41,0x41,0x52,0x42,0x42,0x53,0x53,0x64,
    0x4f,0x22,0x22,0x22,0x40,0x40,0x40,0x41,0x41,0x41,0x52,0x42,0x42,0x53,0x53,0x64,
    0x22,0x22,0x22,0x33,0x33,0x33,0x40,0x41,0x41,0x41,0x52,0x42,0x42,0x53,0x53,0x64,
    0x43,0x22,0x33,0x33,0x33,0x44,0x44,0x45,0x45,0x45,0x56,0x46,0x46,0x46,0x57,0x68,
    0x43,0x43,0x33,0x33,0x44,0x44,0x44,0x45,0x45,0x45,0x56,0x46,0x46,0x57,0x57,0x68,
    0x43,0x43,0x33,0x44,0x44,0x44,0x55,0x45,0x45,0x56,0x56,0x46,0x46,0x57,0x68,0x68,
    0x43,0x43,0x43,0x44,0x44,0x55,0x55,0x45,0x45,0x56,0x67,0x46,0x46,0x57,0x68,0x79,
    0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,0x4a,0x4a,0x4a,0x5b,0x79,
    0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x5a,0x4a,0x4a,0x4a,0x5b,0x6c,
    0x47,0x47,0x47,0x48,0x48,0x59,0x59,0x49,0x49,0x5a,0x5a,0x4a,0x4a,0x5b,0x5b,0x6c,
    0x58,0x58,0x58,0x59,0x59,0x59,0x6a,0x49,0x5a,0x5a,0x6b,0x6b,0x5b,0x5b,0x6c,0x7d,
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x6b,0x4e,0x4e,0x4e,0x6c,0x7d,
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x5e,0x4e,0x4e,0x4e,0x5f,0x5f,
    0x5c,0x5c,0x5c,0x4c,0x5d,0x5d,0x5d,0x4d,0x4d,0x5e,0x5e,0x4e,0x4e,0x5f,0x5f,0x60,
    0x5c,0x5c,0x5c,0x5d,0x5d,0x6e,0x6e,0x5e,0x5e,0x5e,0x6f,0x6f,0x5f,0x5f,0x60,0x60,
    0x6d,0x6d,0x6d,0x6e,0x6e,0x6e,0x7f,0x7f,0x6f,0x6f,0x70,0x70,0x5f,0x60,0x60,0x71,
    0x4f,0x4f,0x40,0x40,0x40,0x40,0x51,0x41,0x41,0x52,0x63,0x42,0x42,0x53,0x64,0x75,
    0x4f,0x4f,0x22,0x40,0x40,0x40,0x51,0x41,0x41,0x52,0x63,0x42,0x42,0x53,0x64,0x75,
    0x43,0x22,0x33,0x33,0x33,0x40,0x51,0x41,0x41,0x52,0x63,0x42,0x42,0x53,0x64,0x75,
    0x43,0x43,0x33,0x33,0x44,0x44,0x44,0x45,0x45,0x45,0x56,0x46,0x46,0x57,0x57,0x68,
    0x43,0x43,0x33,0x44,0x44,0x44,0x55,0x45,0x45,0x56,0x56,0x46,0x46,0x57,0x68,0x68,
    0x43,0x43,0x43,0x44,0x44,0x55,0x55,0x45,0x45,0x56,0x67,0x46,0x46,0x57,0x68,0x79,
    0x54,0x54,0x54,0x44,0x55,0x55,0x55,0x45,0x56,0x56,0x67,0x78,0x78,0x57,0x68,0x79,
    0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,0x4a,0x4a,0x4a,0x5b,0x79,
    0x47,0x47,0x47,0x48,0x48,0x48,0x59,0x49,0x49,0x49,0x5a,0x4a,0x4a,0x5b,0x5b,0x6c,
    0x58,0x58,0x58,0x48,0x59,0x59,0x59,0x49,0x49,0x5a,0x5a,0x4a,0x4a,0x5b,0x6c,0x6c,
    0x69,0x69,0x69,0x59,0x59,0x6a,0x6a,0x49,0x5a,0x5a,0x6b,0x6b,0x5b,0x5b,0x6c,0x7d,
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x7b,0x4d,0x4d,0x4d,0x6b,0x4e,0x4e,0x4e,0x7d,0x7d,
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x7b,0x4d,0x4d,0x4d,0x5e,0x4e,0x4e,0x4e,0x5f,0x7d,
    0x5c,0x5c,0x5c,0x5d,0x5d,0x5d,0x5d,0x4d,0x5e,0x5e,0x5e,0x4e,0x4e,0x5f,0x5f,0x60,
    0x6d,0x6d,0x6d,0x5d,0x6e,0x6e,0x6e,0x5e,0x5e,0x6f,0x6f,0x70,0x5f,0x5f,0x60,0x60,
    0x7e,0x7e,0x7e,0x6e,0x6e,0x7f,0x7f,0x7f,0x6f,0x6f,0x70,0x70,0x70,0x60,0x60,0x71,
    0x50,0x50,0x50,0x40,0x40,0x51,0x51,0x41,0x41,0x52,0x63,0x74,0x42,0x53,0x64,0x75,
    0x50,0x50,0x50,0x40,0x40,0x51,0x51,0x41,0x41,0x52,0x63,0x74,0x42,0x53,0x64,0x75,
    0x50,0x50,0x33,0x33,0x40,0x51,0x51,0x41,0x41,0x52,0x63,0x74,0x42,0x53,0x64,0x75,
    0x43,0x43,0x33,0x44,0x44,0x44,0x55,0x45,0x45,0x56,0x56,0x46,0x46,0x57,0x68,0x68,
    0x43,0x43,0x43,0x44,0x44,0x55,0x55,0x45,0x45,0x56,0x67,0x46,0x46,0x57,0x68,0x79,
    0x54,0x54,0x54,0x44,0x55,0x55,0x55,0x45,0x56,0x56,0x67,0x78,0x78,0x57,0x68,0x79,
    0x54,0x54,0x54,0x55,0x55,0x55,0x66,0x66,0x56,0x67,0x67,0x78,0x78,0x68,0x68,0x79,
    0x47,0x47,0x47,0x48,0x48,0x48,0x66,0x49,0x49,0x49,0x78,0x78,0x4a,0x4a,0x5b,0x79,
    0x47,0x47,0x47,0x48,0x48,0x59,0x59,0x49,0x49,0x5a,0x5a,0x4a,0x4a,0x5b,0x6c,0x6c,
    0x58,0x58,0x58,0x59,0x59,0x59,0x6a,0x49,0x5a,0x5a,0x6b,0x6b,0x5b,0x5b,0x6c,0x7d,
    0x69,0x69,0x69,0x59,0x6a,0x6a,0x6a,0x7b,0x5a,0x6b,0x6b,0x6b,0x7c,0x6c,0x6c,0x7d,
    0x7a,0x7a,0x7a,0x4c,0x4c,0x7b,0x7b,0x7b,0x4d,0x6b,0x6b,0x7c,0x7c,0x4e,0x7d,0x7d,
    0x4b,0x4b,0x4b,0x4c,0x4c,0x7b,0x7b,0x4d,0x4d,0x5e,0x7c,0x7c,0x4e,0x5f,0x5f,0x7d,
    0x5c,0x5c,0x5c,0x5d,0x5d,0x5d,0x6e,0x4d,0x5e,0x5e,0x6f,0x4e,0x5f,0x5f,0x60,0x60,
    0x6d,0x6d,0x6d,0x6e,0x6e,0x6e,0x6e,0x5e,0x6f,0x6f,0x6f,0x70,0x5f,0x60,0x60,0x71,
    0x7e,0x7e,0x7e,0x6e,0x7f,0x7f,0x7f,0x7f,0x6f,0x70,0x70,0x70,0x70,0x60,0x71,0x71,
    0x61,0x61,0x61,0x40,0x51,0x51,0x62,0x62,0x73,0x63,0x63,0x74,0x74,0x64,0x64,0x75,
    0x61,0x61,0x61,0x40,0x51,0x51,0x62,0x62,0x73,0x63,0x63,0x74,0x74,0x64,0x64,0x75,
    0x61,0x61,0x61,0x40,0x51,0x51,0x62,0x62,0x73,0x63,0x63,0x74,0x74,0x64,0x64,0x75,
    0x43,0x43,0x43,0x44,0x44,0x55,0x55,0x45,0x45,0x56,0x67,0x46,0x46,0x57,0x68,0x79,
    0x54,0x54,0x54,0x44,0x55,0x55,0x55,0x45,0x56,0x56,0x67,0x78,0x78,0x57,0x68,0x79,
    0x54,0x54,0x54,0x55,0x55,0x55,0x66,0x66,0x56,0x67,0x67,0x78,0x78,0x68,0x68,0x79,
    0x65,0x65,0x65,0x55,0x55,0x66,0x66,0x66,0x77,0x67,0x78,0x78,0x78,0x78,0x79,0x79,
    0x65,0x65,0x65,0x48,0x48,0x66,0x66,0x77,0x77,0x77,0x78,0x78,0x78,0x5b,0x79,0x79,
    0x76,0x76,0x76,0x48,0x59,0x59,0x77,0x77,0x77,0x5a,0x5a,0x4a,0x4a,0x5b,0x6c,0x6c,
    0x69,0x69,0x69,0x59,0x59,0x6a,0x6a,0x77,0x5a,0x5a,0x6b,0x6b,0x5b,0x6c,0x6c,0x7d,
    0x69,0x69,0x69,0x6a,0x6a,0x6a,0x7b,0x7b,0x5a,0x6b,0x6b,0x7c,0x7c,0x6c,0x7d,0x7d,
    0x7a,0x7a,0x7a,0x4c,0x7b,0x7b,0x7b,0x7b,0x4d,0x6b,0x7c,0x7c,0x7c,0x7c,0x7d,0x7d,
    0x7a,0x7a,0x7a,0x4c,0x7b,0x7b,0x7b,0x7b,0x4d,0x5e,0x7c,0x7c,0x7c,0x5f,0x5f,0x7d,
    0x6d,0x6d,0x6d,0x5d,0x5d,0x6e,0x7b,0x5e,0x5e,0x6f,0x6f,0x7c,0x5f,0x5f,0x60,0x60,
    0x6d,0x6d,0x6d,0x6e,0x6e,0x6e,0x7f,0x7f,0x6f,0x6f,0x70,0x70,0x5f,0x60,0x60,0x71,
    0x7e,0x7e,0x7e,0x7f,0x7f,0x7f,0x7f,0x7f,0x6f,0x70,0x70,0x70,0x70,0x60,0x71,0x71,
    0x72,0x72,0x72,0x8f,0x8f,0x62,0x62,0x73,0x73,0x80,0x74,0x81,0x81,0x81,0x92,0x75,
    0x72,0x72,0x72,0x8f,0x8f,0x62,0x62,0x73,0x73,0x80,0x74,0x81,0x81,0x81,0x92,0x75,
    0x72,0x72,0x72,0x83,0x83,0x62,0x62,0x73,0x73,0x80,0x74,0x81,0x81,0x81,0x92,0x75,
    0x82,0x82,0x82,0x83,0x83,0x83,0x83,0x84,0x84,0x84,0x84,0x85,0x85,0x85,0x96,0x79,
    0x82,0x82,0x82,0x83,0x83,0x83,0x66,0x84,0x84,0x84,0x67,0x85,0x85,0x85,0x96,0x79,
    0x65,0x65,0x65,0x83,0x83,0x66,0x66,0x66,0x84,0x84,0x78,0x78,0x85,0x85,0x96,0x79,
    0x65,0x65,0x65,0x83,0x66,0x66,0x66,0x77,0x77,0x77,0x78,0x78,0x78,0x96,0x79,0x79,
    0x76,0x76,0x76,0x87,0x87,0x66,0x77,0x77,0x77,0x88,0x78,0x89,0x89,0x89,0x89,0x79,
    0x76,0x76,0x76,0x87,0x87,0x87,0x77,0x77,0x88,0x88,0x88,0x89,0x89,0x89,0x9a,0x9a,
    0x86,0x86,0x86,0x87,0x87,0x87,0x77,0x88,0x88,0x88,0x6b,0x89,0x89,0x9a,0x9a,0x7d,
    0x7a,0x7a,0x7a,0x87,0x6a,0x7b,0x7b,0x7b,0x88,0x6b,0x6b,0x7c,0x7c,0x9a,0x7d,0x7d,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x7b,0x7b,0x8c,0x8c,0x8c,0x7c,0x7c,0x8d,0x8d,0x7d,0x7d,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x8b,0x7b,0x8c,0x8c,0x8c,0x7c,0x8d,0x8d,0x8d,0x9e,0x9e,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x8b,0x9c,0x8c,0x8c,0x9d,0x9d,0x8d,0x8d,0x9e,0x9e,0x9e,
    0x9b,0x9b,0x9b,0x9c,0x9c,0x9c,0x7f,0x8c,0x9d,0x9d,0x70,0x70,0x9e,0x9e,0x9e,0x71,
    0x7e,0x7e,0x7e,0x7f,0x7f,0x7f,0x7f,0x7f,0x9d,0x70,0x70,0x70,0x9e,0x9e,0x71,0x71,
    0x8e,0x8e,0x8e,0x8f,0x8f,0x8f,0x73,0x73,0x80,0x80,0x91,0x81,0x81,0x92,0x92,0xa3,
    0x8e,0x8e,0x8e,0x8f,0x8f,0x8f,0x73,0x73,0x80,0x80,0x91,0x81,0x81,0x92,0x92,0xa3,
    0x82,0x82,0x82,0x83,0x83,0x83,0x73,0x73,0x80,0x80,0x91,0x81,0x81,0x92,0x92,0xa3,
    0x82,0x82,0x82,0x83,0x83,0x83,0x83,0x84,0x84,0x84,0x95,0x85,0x85,0x85,0x96,0xa7,
    0x82,0x82,0x82,0x83,0x83,0x83,0x94,0x84,0x84,0x84,0x95,0x85,0x85,0x96,0x96,0xa7,
    0x82,0x82,0x82,0x83,0x83,0x94,0x94,0x84,0x84,0x95,0x95,0x85,0x85,0x96,0xa7,0xa7,
    0x76,0x76,0x76,0x83,0x94,0x94,0x77,0x77,0x77,0x95,0x95,0x85,0x85,0x96,0xa7,0xa7,
    0x76,0x76,0x76,0x87,0x87,0x87,0x77,0x77,0x88,0x88,0x88,0x89,0x89,0x89,0x9a,0x9a,
    0x86,0x86,0x86,0x87,0x87,0x87,0x77,0x88,0x88,0x88,0x99,0x89,0x89,0x9a,0x9a,0xab,
    0x86,0x86,0x86,0x87,0x87,0x98,0x98,0x88,0x88,0x99,0x99,0x89,0x89,0x9a,0x9a,0xab,
    0x97,0x97,0x97,0x98,0x98,0x98,0x98,0x88,0x99,0x99,0x99,0x89,0x9a,0x9a,0xab,0xab,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x8b,0x8b,0x8c,0x8c,0x8c,0x8c,0x8d,0x8d,0x8d,0xab,0xbc,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x8b,0x8b,0x8c,0x8c,0x8c,0x9d,0x8d,0x8d,0x8d,0x9e,0x9e,
    0x9b,0x9b,0x9b,0x8b,0x9c,0x9c,0x9c,0x8c,0x9d,0x9d,0x9d,0x8d,0x8d,0x9e,0x9e,0xaf,
    0x9b,0x9b,0x9b,0x9c,0x9c,0xad,0xad,0x9d,0x9d,0x9d,0xae,0xae,0x9e,0x9e,0xaf,0xaf,
    0xac,0xac,0xac,0xad,0xad,0xad,0xad,0x9d,0xae,0xae,0xae,0xbf,0x9e,0xaf,0xaf,0xaf,
    0x9f,0x9f,0x9f,0x8f,0x90,0x90,0xa1,0x80,0x80,0x91,0x91,0x81,0x81,0x92,0xa3,0xb4,
    0x9f,0x9f,0x9f,0x8f,0x90,0x90,0xa1,0x80,0x80,0x91,0x91,0x81,0x81,0x92,0xa3,0xb4,
    0x9f,0x9f,0x9f,0x83,0x90,0x90,0xa1,0x80,0x80,0x91,0x91,0x81,0x81,0x92,0xa3,0xb4,
    0x82,0x82,0x82,0x83,0x83,0x94,0x94,0x84,0x84,0x95,0x95,0x85,0x85,0x96,0x96,0xa7,
    0x93,0x93,0x93,0x83,0x94,0x94,0x94,0x84,0x84,0x95,0x95,0x85,0x85,0x96,0xa7,0xa7,
    0x93,0x93,0x93,0x94,0x94,0x94,0xa5,0x84,0x95,0x95,0xa6,0xa6,0x96,0x96,0xa7,0xb8,
    0xa4,0xa4,0xa4,0x94,0x94,0xa5,0xa5,0x77,0x95,0x95,0xa6,0xa6,0x96,0xa7,0xa7,0xb8,
    0x86,0x86,0x86,0x87,0x87,0x87,0x77,0x88,0x88,0x88,0x99,0x89,0x89,0x9a,0x9a,0xb8,
    0x86,0x86,0x86,0x87,0x87,0x98,0x98,0x88,0x88,0x99,0x99,0x89,0x89,0x9a,0x9a,0xab,
    0x97,0x97,0x97,0x98,0x98,0x98,0x98,0x88,0x99,0x99,0x99,0x89,0x9a,0x9a,0xab,0xab,
    0x97,0x97,0x97,0x98,0x98,0xa9,0xa9,0x99,0x99,0x99,0xaa,0xaa,0x9a,0xab,0xab,0xbc,
    0x8a,0x8a,0x8a,0x8b,0x8b,0xa9,0xa9,0x8c,0x8c,0x8c,0xaa,0x8d,0x8d,0x8d,0xab,0xbc,
    0x8a,0x8a,0x8a,0x8b,0x8b,0x9c,0x9c,0x8c,0x8c,0x9d,0x9d,0x8d,0x8d,0x9e,0x9e,0xbc,
    0x9b,0x9b,0x9b,0x9c,0x9c,0x9c,0xad,0x9d,0x9d,0x9d,0xae,0x8d,0x9e,0x9e,0xaf,0xaf,
    0xac,0xac,0xac,0x9c,0xad,0xad,0xad,0x9d,0x9d,0xae,0xae,0xae,0x9e,0xaf,0xaf,0xaf,
    0xbd,0xbd,0xbd,0xad,0xad,0xbe,0xbe,0xbe,0xae,0xae,0xbf,0xbf,0xbf,0xaf,0xaf,0xb0,
    0xa0,0xa0,0xa0,0x90,0xa1,0xa1,0xa1,0xb2,0x91,0x91,0xa2,0xa2,0xb3,0xa3,0xa3,0xb4,
    0xa0,0xa0,0xa0,0x90,0xa1,0xa1,0xa1,0xb2,0x91,0x91,0xa2,0xa2,0xb3,0xa3,0xa3,0xb4,
    0xa0,0xa0,0xa0,0x90,0xa1,0xa1,0xa1,0xb2,0x91,0x91,0xa2,0xa2,0xb3,0xa3,0xa3,0xb4,
    0x93,0x93,0x93,0x94,0x94,0x94,0xa5,0x84,0x95,0x95,0xa6,0xa6,0x96,0x96,0xa7,0xb8,
    0xa4,0xa4,0xa4,0x94,0x94,0xa5,0xa5,0x84,0x95,0x95,0xa6,0xa6,0x96,0x96,0xa7,0xb8,
    0xa4,0xa4,0xa4,0x94,0xa5,0xa5,0xa5,0xb6,0x95,0xa6,0xa6,0xa6,0xb7,0xa7,0xa7,0xb8,
    0xa4,0xa4,0xa4,0xa5,0xa5,0xa5,0xb6,0xb6,0x95,0xa6,0xa6,0xb7,0xb7,0xa7,0xb8,0xb8,
    0xb5,0xb5,0xb5,0x87,0x87,0xb6,0xb6,0xb6,0x88,0x99,0xa6,0xb7,0xb7,0x9a,0xb8,0xb8,
    0x97,0x97,0x97,0x98,0x98,0x98,0x98,0x88,0x99,0x99,0x99,0x89,0x9a,0x9a,0xab,0xab,
    0x97,0x97,0x97,0x98,0x98,0xa9,0xa9,0x99,0x99,0x99,0xaa,0xaa,0x9a,0xab,0xab,0xbc,
    0xa8,0xa8,0xa8,0xa9,0xa9,0xa9,0xa9,0xa9,0x99,0xaa,0xaa,0xaa,0xbb,0xab,0xab,0xbc,
    0xa8,0xa8,0xa8,0xa9,0xa9,0xa9,0xba,0xba,0x8c,0xaa,0xaa,0xbb,0xbb,0xab,0xbc,0xbc,
    0xb9,0xb9,0xb9,0x9c,0x9c,0xba,0xba,0xba,0x9d,0x9d,0xbb,0xbb,0xbb,0x9e,0x9e,0xbc,
    0xac,0xac,0xac,0x9c,0x9c,0xad,0xad,0x9d,0x9d,0xae,0xae,0xae,0x9e,0x9e,0xaf,0xaf,
    0xac,0xac,0xac,0xad,0xad,0xad,0xbe,0xbe,0xae,0xae,0xae,0xbf,0x9e,0xaf,0xaf,0xb0,
    0xbd,0xbd,0xbd,0xbe,0xbe,0xbe,0xbe,0xbe,0xae,0xbf,0xbf,0xbf,0xbf,0xaf,0xb0,0xb0,
    0xb1,0xb1,0xb1,0xce,0xce,0xb2,0xb2,0xcf,0xcf,0xa2,0xa2,0xb3,0xb3,0xc0,0xb4,0xb4,
    0xb1,0xb1,0xb1,0xce,0xce,0xb2,0xb2,0xcf,0xcf,0xa2,0xa2,0xb3,0xb3,0xc0,0xb4,0xb4,
    0xb1,0xb1,0xb1,0xc2,0xc2,0xb2,0xb2,0xc3,0xc3,0xa2,0xa2,0xb3,0xb3,0xc0,0xb4,0xb4,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xc2,0xa5,0xc3,0xc3,0xc3,0xa6,0xc4,0xc4,0xc4,0xa7,0xb8,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xa5,0xb6,0xc3,0xc3,0xc3,0xa6,0xc4,0xc4,0xc4,0xb8,0xb8,
    0xb5,0xb5,0xb5,0xc2,0xa5,0xb6,0xb6,0xb6,0xc3,0xa6,0xa6,0xb7,0xb7,0xc4,0xb8,0xb8,
    0xb5,0xb5,0xb5,0xa5,0xb6,0xb6,0xb6,0xb6,0xc3,0xa6,0xb7,0xb7,0xb7,0xb7,0xb8,0xb8,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xb6,0xb6,0xc7,0xc7,0xc7,0xb7,0xb7,0xc8,0xc8,0xb8,0xb8,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xc6,0xc6,0xc7,0xc7,0xc7,0xaa,0xc8,0xc8,0xc8,0xab,0xbc,
    0xa8,0xa8,0xa8,0xc6,0xc6,0xa9,0xa9,0xc7,0xc7,0xaa,0xaa,0xaa,0xc8,0xc8,0xab,0xbc,
    0xa8,0xa8,0xa8,0xa9,0xa9,0xa9,0xba,0xba,0xaa,0xaa,0xaa,0xbb,0xbb,0xab,0xbc,0xbc,
    0xb9,0xb9,0xb9,0xca,0xca,0xba,0xba,0xba,0xcb,0xaa,0xbb,0xbb,0xbb,0xcc,0xbc,0xbc,
    0xb9,0xb9,0xb9,0xca,0xca,0xba,0xba,0xcb,0xcb,0xcb,0xbb,0xbb,0xcc,0xcc,0xcc,0xbc,
    0xc9,0xc9,0xc9,0xca,0xca,0xca,0xba,0xcb,0xcb,0xcb,0xae,0xcc,0xcc,0xcc,0xaf,0xaf,
    0xbd,0xbd,0xbd,0xad,0xbe,0xbe,0xbe,0xbe,0xae,0xae,0xbf,0xbf,0xcc,0xaf,0xaf,0xb0,
    0xbd,0xbd,0xbd,0xbe,0xbe,0xbe,0xbe,0xbe,0xbf,0xbf,0xbf,0xbf,0xbf,0xaf,0xb0,0xb0,
    0xcd,0xcd,0xcd,0xce,0xce,0xce,0xb2,0xcf,0xcf,0xcf,0xb3,0xb3,0xc0,0xc0,0xd1,0xb4,
    0xcd,0xcd,0xcd,0xce,0xce,0xce,0xb2,0xcf,0xcf,0xcf,0xb3,0xb3,0xc0,0xc0,0xd1,0xb4,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xc2,0xb2,0xc3,0xc3,0xc3,0xb3,0xb3,0xc0,0xc0,0xd1,0xb4,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xc2,0xc2,0xc3,0xc3,0xc3,0xd4,0xc4,0xc4,0xc4,0xd5,0xd5,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xc2,0xb6,0xc3,0xc3,0xc3,0xd4,0xc4,0xc4,0xc4,0xd5,0xb8,
    0xc1,0xc1,0xc1,0xc2,0xc2,0xb6,0xb6,0xc3,0xc3,0xd4,0xb7,0xb7,0xc4,0xd5,0xd5,0xb8,
    0xb5,0xb5,0xb5,0xc2,0xb6,0xb6,0xb6,0xb6,0xc3,0xd4,0xb7,0xb7,0xb7,0xd5,0xd5,0xb8,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xc6,0xb6,0xc7,0xc7,0xc7,0xb7,0xc8,0xc8,0xc8,0xd9,0xd9,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xc6,0xc6,0xc7,0xc7,0xc7,0xd8,0xc8,0xc8,0xc8,0xd9,0xd9,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xd7,0xd7,0xc7,0xc7,0xd8,0xd8,0xc8,0xc8,0xd9,0xd9,0xbc,
    0xb9,0xb9,0xb9,0xd7,0xd7,0xba,0xba,0xba,0xd8,0xd8,0xbb,0xbb,0xbb,0xd9,0xd9,0xbc,
    0xb9,0xb9,0xb9,0xca,0xca,0xba,0xba,0xcb,0xcb,0xcb,0xbb,0xbb,0xcc,0xcc,0xcc,0xbc,
    0xc9,0xc9,0xc9,0xca,0xca,0xca,0xba,0xcb,0xcb,0xcb,0xbb,0xcc,0xcc,0xcc,0xdd,0xdd,
    0xc9,0xc9,0xc9,0xca,0xca,0xdb,0xdb,0xcb,0xcb,0xdc,0xdc,0xcc,0xcc,0xdd,0xdd,0xdd,
    0xda,0xda,0xda,0xdb,0xdb,0xdb,0xdb,0xdc,0xdc,0xdc,0xdc,0xcc,0xdd,0xdd,0xdd,0xb0,
    0xbd,0xbd,0xbd,0xdb,0xbe,0xbe,0xbe,0xdc,0xdc,0xbf,0xbf,0xbf,0xdd,0xdd,0xb0,0xb0,
    0xde,0xde,0xde,0xdf,0xdf,0xdf,0xe0,0xcf,0xd0,0xd0,0xe1,0xc0,0xc0,0xd1,0xd1,0xe2,
    0xde,0xde,0xde,0xdf,0xdf,0xdf,0xe0,0xcf,0xd0,0xd0,0xe1,0xc0,0xc0,0xd1,0xd1,0xe2,
    0xde,0xde,0xde,0xdf,0xdf,0xdf,0xe0,0xc3,0xd0,0xd0,0xe1,0xc0,0xc0,0xd1,0xd1,0xe2,
    0xd2,0xd2,0xd2,0xc2,0xd3,0xd3,0xd3,0xc3,0xc3,0xd4,0xd4,0xc4,0xc4,0xd5,0xd5,0xe6,
    0xd2,0xd2,0xd2,0xd3,0xd3,0xd3,0xd3,0xc3,0xd4,0xd4,0xd4,0xc4,0xc4,0xd5,0xd5,0xe6,
    0xd2,0xd2,0xd2,0xd3,0xd3,0xd3,0xe4,0xc3,0xd4,0xd4,0xe5,0xc4,0xd5,0xd5,0xe6,0xe6,
    0xe3,0xe3,0xe3,0xd3,0xd3,0xe4,0xb6,0xd4,0xd4,0xe5,0xe5,0xb7,0xd5,0xd5,0xe6,0xe6,
    0xc5,0xc5,0xc5,0xc6,0xc6,0xc6,0xd7,0xc7,0xc7,0xd8,0xd8,0xc8,0xc8,0xd9,0xd9,0xd9,
    0xd6,0xd6,0xd6,0xc6,0xd7,0xd7,0xd7,0xc7,0xd8,0xd8,0xd8,0xc8,0xc8,0xd9,0xd9,0xea,
    0xd6,0xd6,0xd6,0xd7,0xd7,0xd7,0xe8,0xd8,0xd8,0xd8,0xe9,0xc8,0xd9,0xd9,0xea,0xea,
    0xe7,0xe7,0xe7,0xd7,0xd7,0xe8,0xe8,0xd8,0xd8,0xe9,0xe9,0xe9,0xd9,0xd9,0xea,0xea,
    0xc9,0xc9,0xc9,0xca,0xca,0xca,0xba,0xcb,0xcb,0xcb,0xe9,0xcc,0xcc,0xcc,0xea,0xea,
    0xc9,0xc9,0xc9,0xca,0xca,0xdb,0xdb,0xcb,0xcb,0xdc,0xdc,0xcc,0xcc,0xdd,0xdd,0xdd,
    0xda,0xda,0xda,0xdb,0xdb,0xdb,0xdb,0xdc,0xdc,0xdc,0xdc,0xcc,0xdd,0xdd,0xdd,0xee,
    0xda,0xda,0xda,0xdb,0xdb,0xec,0xec,0xdc,0xdc,0xed,0xed,0xed,0xdd,0xdd,0xee,0xee,
    0xeb,0xeb,0xeb,0xec,0xec,0xec,0xec,0xdc,0xed,0xed,0xed,0xed,0xdd,0xee,0xee,0xee,
    0xef,0xef,0xef,0xdf,0xe0,0xe0,0xe0,0xd0,0xd0,0xe1,0xe1,0xf2,0xd1,0xd1,0xe2,0xe2,
    0xef,0xef,0xef,0xdf,0xe0,0xe0,0xe0,0xd0,0xd0,0xe1,0xe1,0xf2,0xd1,0xd1,0xe2,0xe2,
    0xef,0xef,0xef,0xdf,0xe0,0xe0,0xe0,0xd0,0xd0,0xe1,0xe1,0xf2,0xd1,0xd1,0xe2,0xe2,
    0xd2,0xd2,0xd2,0xd3,0xd3,0xe4,0xe4,0xd4,0xd4,0xd4,0xe5,0xe5,0xd5,0xd5,0xe6,0xe6,
    0xe3,0xe3,0xe3,0xd3,0xe4,0xe4,0xe4,0xd4,0xd4,0xe5,0xe5,0xf6,0xd5,0xd5,0xe6,0xe6,
    0xe3,0xe3,0xe3,0xe4,0xe4,0xe4,0xe4,0xd4,0xe5,0xe5,0xe5,0xf6,0xd5,0xe6,0xe6,0xf7,
    0xe3,0xe3,0xe3,0xe4,0xe4,0xe4,0xf5,0xf5,0xe5,0xe5,0xf6,0xf6,0xd5,0xe6,0xe6,0xf7,
    0xd6,0xd6,0xd6,0xd7,0xd7,0xd7,0xf5,0xc7,0xd8,0xd8,0xf6,0xc8,0xd9,0xd9,0xd9,0xf7,
    0xd6,0xd6,0xd6,0xd7,0xd7,0xe8,0xe8,0xd8,0xd8,0xd8,0xe9,0xe9,0xd9,0xd9,0xea,0xea,
    0xe7,0xe7,0xe7,0xd7,0xe8,0xe8,0xe8,0xd8,0xd8,0xe9,0xe9,0xe9,0xd9,0xea,0xea,0xea,
    0xe7,0xe7,0xe7,0xe8,0xe8,0xe8,0xf9,0xf9,0xe9,0xe9,0xe9,0xfa,0xd9,0xea,0xea,0xfb,
    0xf8,0xf8,0xf8,0xe8,0xf9,0xf9,0xf9,0xcb,0xe9,0xe9,0xfa,0xfa,0xcc,0xea,0xea,0xfb,
    0xda,0xda,0xda,0xdb,0xdb,0xdb,0xdb,0xdc,0xdc,0xdc,0xdc,0xcc,0xdd,0xdd,0xdd,0xee,
    0xda,0xda,0xda,0xdb,0xdb,0xec,0xec,0xdc,0xdc,0xed,0xed,0xed,0xdd,0xdd,0xee,0xee,
    0xeb,0xeb,0xeb,0xec,0xec,0xec,0xec,0xdc,0xed,0xed,0xed,0xed,0xdd,0xee,0xee,0xee,
    0xeb,0xeb,0xeb,0xec,0xec,0xfd,0xfd,0xfd,0xed,0xed,0xfe,0xfe,0xee,0xee,0xee,0xff,
    0xf0,0xf0,0xf0,0xe0,0xf1,0xf1,0xf1,0xf1,0xe1,0xf2,0xf2,0xf2,0xf2,0xe2,0xe2,0xf3,
    0xf0,0xf0,0xf0,0xe0,0xf1,0xf1,0xf1,0xf1,0xe1,0xf2,0xf2,0xf2,0xf2,0xe2,0xe2,0xf3,
    0xf0,0xf0,0xf0,0xe0,0xf1,0xf1,0xf1,0xf1,0xe1,0xf2,0xf2,0xf2,0xf2,0xe2,0xe2,0xf3,
    0xe3,0xe3,0xe3,0xe4,0xe4,0xe4,0xf5,0xf5,0xe5,0xe5,0xf6,0xf6,0xd5,0xe6,0xe6,0xf7,
    0xf4,0xf4,0xf4,0xe4,0xe4,0xf5,0xf5,0xf5,0xe5,0xe5,0xf6,0xf6,0xf6,0xe6,0xe6,0xf7,
    0xf4,0xf4,0xf4,0xe4,0xf5,0xf5,0xf5,0xf5,0xe5,0xf6,0xf6,0xf6,0xf6,0xe6,0xf7,0xf7,
    0xf4,0xf4,0xf4,0xf5,0xf5,0xf5,0xf5,0xf5,0xe5,0xf6,0xf6,0xf6,0xf6,0xe6,0xf7,0xf7,
    0xf4,0xf4,0xf4,0xf5,0xf5,0xf5,0xf5,0xf5,0xd8,0xf6,0xf6,0xf6,0xd9,0xd9,0xf7,0xf7,
    0xe7,0xe7,0xe7,0xe8,0xe8,0xe8,0xe8,0xd8,0xe9,0xe9,0xe9,0xfa,0xd9,0xea,0xea,0xea,
    0xf8,0xf8,0xf8,0xe8,0xe8,0xf9,0xf9,0xf9,0xe9,0xe9,0xfa,0xfa,0xfa,0xea,0xea,0xfb,
    0xf8,0xf8,0xf8,0xf9,0xf9,0xf9,0xf9,0xf9,0xe9,0xfa,0xfa,0xfa,0xfa,0xea,0xfb,0xfb,
    0xf8,0xf8,0xf8,0xf9,0xf9,0xf9,0xf9,0xf9,0xfa,0xfa,0xfa,0xfa,0xfa,0xea,0xfb,0xfb,
    0xf8,0xf8,0xf8,0xdb,0xf9,0xf9,0xf9,0xdc,0xdc,0xfa,0xfa,0xfa,0xdd,0xdd,0xee,0xfb,
    0xeb,0xeb,0xeb,0xec,0xec,0xec,0xec,0xdc,0xed,0xed,0xed,0xed,0xdd,0xee,0xee,0xee,
    0xeb,0xeb,0xeb,0xec,0xec,0xfd,0xfd,0xfd,0xed,0xed,0xfe,0xfe,0xee,0xee,0xee,0xff,
    0xfc,0xfc,0xfc,0xfd,0xfd,0xfd,0xfd,0xfd,0xed,0xfe,0xfe,0xfe,0xfe,0xee,0xff,0xff,
}
};
/*e: global [[def]] */
/*s: global [[memdefcmap]] */
Memcmap *memdefcmap = &def;
/*e: global [[memdefcmap]] */
/*s: function [[_memmkcmap]] */
void _memmkcmap(void){}
/*e: function [[_memmkcmap]] */
/*e: lib_graphics/libmemdraw/cmap.c */
