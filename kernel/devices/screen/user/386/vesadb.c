/* this file was automatically generated from vesa.txt */

#include <u.h>
#include <libc.h>
#include <bio.h>
#include "pci.h"
#include "vga.h"

/*
 * VESA Monitor Timing Standard mode definitions as per
 * VESA and Industry Standards and Guidelines for Computer
 * Display Monitor Timing, Version 1.0, Revision 0.8, 17 September 1998.
 *
 * See /public/doc/vesa/dmtv1r08.pdf.
 *
 * This might go back into vgadb at some point. It's here mainly
 * so that people don't change it, and so that we can run without vgadb.
 */

static Mode vesa640x480x60 = {
    .name = "640x480@60Hz",
    .x = 640,
    .y = 480,

    .ht = 800,
    .shb = 648,
    .ehb = 648+144,
    .shs = 656,
    .ehs = 656+96,

    .vt = 525,
    .vrs = 490,
    .vre = 490+2,
    .vbs = 488,
    .vbe = 488+29,

    .frequency = 25175000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa640x480x72 = {
    .name = "640x480@72Hz",
    .x = 640,
    .y = 480,

    .ht = 832,
    .shb = 648,
    .ehb = 648+176,
    .shs = 664,
    .ehs = 664+40,

    .vt = 520,
    .vrs = 489,
    .vre = 489+3,
    .vbs = 488,
    .vbe = 488+24,

    .frequency = 31500000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa640x480x75 = {
    .name = "640x480@75Hz",
    .x = 640,
    .y = 480,

    .ht = 840,
    .shb = 640,
    .ehb = 640+200,
    .shs = 656,
    .ehs = 656+64,

    .vt = 500,
    .vrs = 481,
    .vre = 481+3,
    .vbs = 480,
    .vbe = 480+20,

    .frequency = 31500000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa640x480x85 = {
    .name = "640x480@85Hz",
    .x = 640,
    .y = 480,

    .ht = 832,
    .shb = 640,
    .ehb = 640+192,
    .shs = 696,
    .ehs = 696+56,

    .vt = 509,
    .vrs = 481,
    .vre = 481+3,
    .vbs = 480,
    .vbe = 480+29,

    .frequency = 36000000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa800x600x56 = {
    .name = "800x600@56Hz",
    .x = 800,
    .y = 600,

    .ht = 1024,
    .shb = 800,
    .ehb = 800+224,
    .shs = 824,
    .ehs = 824+72,

    .vt = 625,
    .vrs = 601,
    .vre = 601+2,
    .vbs = 600,
    .vbe = 600+25,

    .frequency = 36000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa800x600x60 = {
    .name = "800x600@60Hz",
    .x = 800,
    .y = 600,

    .ht = 1056,
    .shb = 800,
    .ehb = 800+256,
    .shs = 840,
    .ehs = 840+128,

    .vt = 628,
    .vrs = 601,
    .vre = 601+4,
    .vbs = 600,
    .vbe = 600+28,

    .frequency = 40000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa800x600x72 = {
    .name = "800x600@72Hz",
    .x = 800,
    .y = 600,

    .ht = 1040,
    .shb = 800,
    .ehb = 800+240,
    .shs = 856,
    .ehs = 856+120,

    .vt = 666,
    .vrs = 637,
    .vre = 637+6,
    .vbs = 600,
    .vbe = 600+66,

    .frequency = 50000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa800x600x75 = {
    .name = "800x600@75Hz",
    .x = 800,
    .y = 600,

    .ht = 1056,
    .shb = 800,
    .ehb = 800+256,
    .shs = 816,
    .ehs = 816+80,

    .vt = 625,
    .vrs = 601,
    .vre = 601+3,
    .vbs = 600,
    .vbe = 600+25,

    .frequency = 49500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa800x600x85 = {
    .name = "800x600@85Hz",
    .x = 800,
    .y = 600,

    .ht = 1048,
    .shb = 800,
    .ehb = 800+248,
    .shs = 832,
    .ehs = 832+64,

    .vt = 631,
    .vrs = 601,
    .vre = 601+3,
    .vbs = 600,
    .vbe = 600+31,

    .frequency = 56250000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1024x768x60 = {
    .name = "1024x768@60Hz",
    .x = 1024,
    .y = 768,

    .ht = 1344,
    .shb = 1024,
    .ehb = 1024+320,
    .shs = 1048,
    .ehs = 1048+136,

    .vt = 806,
    .vrs = 771,
    .vre = 771+6,
    .vbs = 768,
    .vbe = 768+38,

    .frequency = 65000000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa1024x768x70 = {
    .name = "1024x768@70Hz",
    .x = 1024,
    .y = 768,

    .ht = 1328,
    .shb = 1024,
    .ehb = 1024+304,
    .shs = 1048,
    .ehs = 1048+136,

    .vt = 806,
    .vrs = 771,
    .vre = 771+6,
    .vbs = 768,
    .vbe = 768+38,

    .frequency = 75000000,

    .hsync = '-',
    .vsync = '-',
    .interlace = '\0',
};

static Mode vesa1024x768x75 = {
    .name = "1024x768@75Hz",
    .x = 1024,
    .y = 768,

    .ht = 1312,
    .shb = 1024,
    .ehb = 1024+288,
    .shs = 1040,
    .ehs = 1040+96,

    .vt = 800,
    .vrs = 769,
    .vre = 769+3,
    .vbs = 768,
    .vbe = 768+32,

    .frequency = 78750000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1024x768x85 = {
    .name = "1024x768@85Hz",
    .x = 1024,
    .y = 768,

    .ht = 1376,
    .shb = 1024,
    .ehb = 1024+352,
    .shs = 1072,
    .ehs = 1072+96,

    .vt = 808,
    .vrs = 769,
    .vre = 769+3,
    .vbs = 768,
    .vbe = 768+40,

    .frequency = 94500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1152x864x75 = {
    .name = "1152x864@75Hz",
    .x = 1152,
    .y = 864,

    .ht = 1600,
    .shb = 1152,
    .ehb = 1152+448,
    .shs = 1216,
    .ehs = 1216+128,

    .vt = 900,
    .vrs = 865,
    .vre = 865+3,
    .vbs = 864,
    .vbe = 864+36,

    .frequency = 108000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1280x960x60 = {
    .name = "1280x960@60Hz",
    .x = 1280,
    .y = 960,

    .ht = 1800,
    .shb = 1280,
    .ehb = 1280+520,
    .shs = 1376,
    .ehs = 1376+112,

    .vt = 1000,
    .vrs = 961,
    .vre = 961+3,
    .vbs = 960,
    .vbe = 960+40,

    .frequency = 108000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1280x960x85 = {
    .name = "1280x960@85Hz",
    .x = 1280,
    .y = 960,

    .ht = 1728,
    .shb = 1280,
    .ehb = 1280+448,
    .shs = 1344,
    .ehs = 1344+160,

    .vt = 1011,
    .vrs = 961,
    .vre = 961+3,
    .vbs = 960,
    .vbe = 960+51,

    .frequency = 148500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1280x1024x60 = {
    .name = "1280x1024@60Hz",
    .x = 1280,
    .y = 1024,

    .ht = 1688,
    .shb = 1280,
    .ehb = 1280+408,
    .shs = 1328,
    .ehs = 1328+112,

    .vt = 1066,
    .vrs = 1025,
    .vre = 1025+3,
    .vbs = 1024,
    .vbe = 1024+42,

    .frequency = 108000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1280x1024x75 = {
    .name = "1280x1024@75Hz",
    .x = 1280,
    .y = 1024,

    .ht = 1688,
    .shb = 1280,
    .ehb = 1280+408,
    .shs = 1296,
    .ehs = 1296+144,

    .vt = 1066,
    .vrs = 1025,
    .vre = 1025+3,
    .vbs = 1024,
    .vbe = 1024+42,

    .frequency = 135000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1280x1024x85 = {
    .name = "1280x1024@85Hz",
    .x = 1280,
    .y = 1024,

    .ht = 1728,
    .shb = 1280,
    .ehb = 1280+448,
    .shs = 1344,
    .ehs = 1344+160,

    .vt = 1072,
    .vrs = 1025,
    .vre = 1025+3,
    .vbs = 1024,
    .vbe = 1024+48,

    .frequency = 157500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1600x1200x60 = {
    .name = "1600x1200@60Hz",
    .x = 1600,
    .y = 1200,

    .ht = 2160,
    .shb = 1600,
    .ehb = 1600+560,
    .shs = 1664,
    .ehs = 1664+192,

    .vt = 1250,
    .vrs = 1201,
    .vre = 1201+3,
    .vbs = 1200,
    .vbe = 1200+50,

    .frequency = 162000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1600x1200x65 = {
    .name = "1600x1200@65Hz",
    .x = 1600,
    .y = 1200,

    .ht = 2160,
    .shb = 1600,
    .ehb = 1600+560,
    .shs = 1664,
    .ehs = 1664+192,

    .vt = 1250,
    .vrs = 1201,
    .vre = 1201+3,
    .vbs = 1200,
    .vbe = 1200+50,

    .frequency = 175500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1600x1200x70 = {
    .name = "1600x1200@70Hz",
    .x = 1600,
    .y = 1200,

    .ht = 2160,
    .shb = 1600,
    .ehb = 1600+560,
    .shs = 1664,
    .ehs = 1664+192,

    .vt = 1250,
    .vrs = 1201,
    .vre = 1201+3,
    .vbs = 1200,
    .vbe = 1200+50,

    .frequency = 189000000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1600x1200x75 = {
    .name = "1600x1200@75Hz",
    .x = 1600,
    .y = 1200,

    .ht = 2160,
    .shb = 1600,
    .ehb = 1600+560,
    .shs = 1664,
    .ehs = 1664+192,

    .vt = 1250,
    .vrs = 1201,
    .vre = 1201+3,
    .vbs = 1200,
    .vbe = 1200+50,

    .frequency = 202500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1600x1200x85 = {
    .name = "1600x1200@85Hz",
    .x = 1600,
    .y = 1200,

    .ht = 2160,
    .shb = 1600,
    .ehb = 1600+560,
    .shs = 1664,
    .ehs = 1664+192,

    .vt = 1250,
    .vrs = 1201,
    .vre = 1201+3,
    .vbs = 1200,
    .vbe = 1200+50,

    .frequency = 229500000,

    .hsync = '+',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1792x1344x60 = {
    .name = "1792x1344@60Hz",
    .x = 1792,
    .y = 1344,

    .ht = 2448,
    .shb = 1792,
    .ehb = 1792+656,
    .shs = 1920,
    .ehs = 1920+200,

    .vt = 1394,
    .vrs = 1345,
    .vre = 1345+3,
    .vbs = 1344,
    .vbe = 1344+50,

    .frequency = 204750000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1792x1344x75 = {
    .name = "1792x1344@75Hz",
    .x = 1792,
    .y = 1344,

    .ht = 2456,
    .shb = 1792,
    .ehb = 1792+664,
    .shs = 1888,
    .ehs = 1888+216,

    .vt = 1417,
    .vrs = 1345,
    .vre = 1345+3,
    .vbs = 1344,
    .vbe = 1344+73,

    .frequency = 261000000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1856x1392x60 = {
    .name = "1856x1392@60Hz",
    .x = 1856,
    .y = 1392,

    .ht = 2528,
    .shb = 1856,
    .ehb = 1856+672,
    .shs = 1952,
    .ehs = 1952+224,

    .vt = 1439,
    .vrs = 1393,
    .vre = 1393+3,
    .vbs = 1392,
    .vbe = 1392+47,

    .frequency = 218250000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1856x1392x75 = {
    .name = "1856x1392@75Hz",
    .x = 1856,
    .y = 1392,

    .ht = 2560,
    .shb = 1856,
    .ehb = 1856+704,
    .shs = 1984,
    .ehs = 1984+224,

    .vt = 1500,
    .vrs = 1393,
    .vre = 1393+3,
    .vbs = 1392,
    .vbe = 1392+108,

    .frequency = 288000000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1920x1440x60 = {
    .name = "1920x1440@60Hz",
    .x = 1920,
    .y = 1440,

    .ht = 2600,
    .shb = 1920,
    .ehb = 1920+680,
    .shs = 2048,
    .ehs = 2048+208,

    .vt = 1500,
    .vrs = 1441,
    .vre = 1441+3,
    .vbs = 1440,
    .vbe = 1440+60,

    .frequency = 234000000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

static Mode vesa1920x1440x75 = {
    .name = "1920x1440@75Hz",
    .x = 1920,
    .y = 1440,

    .ht = 2640,
    .shb = 1920,
    .ehb = 1920+720,
    .shs = 2064,
    .ehs = 2064+224,

    .vt = 1500,
    .vrs = 1441,
    .vre = 1441+3,
    .vbs = 1440,
    .vbe = 1440+60,

    .frequency = 297000000,

    .hsync = '-',
    .vsync = '+',
    .interlace = '\0',
};

Mode *vesamodes[] = {
    &vesa640x480x60,
    &vesa640x480x72,
    &vesa640x480x75,
    &vesa640x480x85,
    &vesa800x600x56,
    &vesa800x600x60,
    &vesa800x600x72,
    &vesa800x600x75,
    &vesa800x600x85,
    &vesa1024x768x60,
    &vesa1024x768x70,
    &vesa1024x768x75,
    &vesa1024x768x85,
    &vesa1152x864x75,
    &vesa1280x960x60,
    &vesa1280x960x85,
    &vesa1280x1024x60,
    &vesa1280x1024x75,
    &vesa1280x1024x85,
    &vesa1600x1200x60,
    &vesa1600x1200x65,
    &vesa1600x1200x70,
    &vesa1600x1200x75,
    &vesa1600x1200x85,
    &vesa1792x1344x60,
    &vesa1792x1344x75,
    &vesa1856x1392x60,
    &vesa1856x1392x75,
    &vesa1920x1440x60,
    &vesa1920x1440x75,
    0
};
