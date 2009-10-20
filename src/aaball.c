/*  This file is a part of gtkboard, a board games system.
    Copyright (C) 2003, Arvind Narayanan <arvindn@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

*/
#include <stdio.h>
#include <math.h>
#include <glib.h>
#include <string.h>

#include "aaball.h"


/** \file aaball.c
  \brief routines for generating antialiased images, primarily balls.
  */

static char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8' ,
					'9', 'A', 'B', 'C', 'D', 'E', 'F'};


static int pixmap_get_color(int fg, int bg, float ratio)
{
	int f1, f2, f3, b1, b2, b3, w1, w2, w3;
	float lw, lf, lb;
	if (ratio < 0) ratio = 0;
	if (ratio > 1) ratio = 1;
	// FIXME: could we have problems with equality of floats?
	if (ratio == 0)
	{
		w1 = fg >> 16;
		w2 = (fg >> 8) & 0xff;
		w3 = fg & 0xff;
	}
	else if (ratio == 1)
	{
		w1 = bg >> 16;
		w2 = (bg >> 8) & 0xff;
		w3 = bg & 0xff;
	}
	else
	{
		f1 = fg >> 16;
		f2 = (fg >> 8) & 0xff;
		f3 = fg & 0xff;
		b1 = bg >> 16;
		b2 = (bg >> 8) & 0xff;
		b3 = bg & 0xff;
		w1 =  (1 - ratio) * f1 +  (ratio) * b1;
		w2 =  (1 - ratio) * f2 +  (ratio) * b2;
		w3 =  (1 - ratio) * f3 +  (ratio) * b3;
		lf = sqrt(f1 * f1 + f2 * f2 + f3 * f3);
		lb = sqrt(b1 * b1 + b2 * b2 + b3 * b3);
		lw = sqrt(w1 * w1 + w2 * w2 + w3 * w3);
		lw /= ((1 - ratio) * lf + ratio * lb);
		w1 /= lw; if (w1 > 0xff) w1 = 0xff;
		w2 /= lw; if (w2 > 0xff) w2 = 0xff;
		w3 /= lw; if (w3 > 0xff) w3 = 0xff;
	}
	return (w1 << 16) + (w2 << 8) + w3;
}

static char *pixmap_get_hex_color(int fg, int bg, float ratio)
{
	int red, green, blue, val;
	static char color[7] = { 0 };
	val = pixmap_get_color (fg, bg, ratio);
	red = val >> 16;
	green = (val >> 8) & 0xff;
	blue = val & 0xff;
	color[0] = hex[red/16];
	color[1] = hex[red%16];
	color[2] = hex[green/16];
	color[3] = hex[green%16];
	color[4] = hex[blue/16];
	color[5] = hex[blue%16];
	return color;
}
	
static char *pixmap_map [256];

//! Generates a ball.
/** 
 @param len = size of ball
 @param pixbuf = buffer to store the pixmap in
 @param fg = foreground color (color of the ball)
 @param bg = background color (color of the square)
 @param rad = radius of the ball
 @param grad = gradient with which the ball merges into the backgound (larger gradient indicates sharper boundary)*/
char ** pixmap_ball_gen(int len, char *pixbuf, int fg, int bg, float rad, float grad)
{
	char **map = pixmap_map;
	char *buf = pixbuf;
	static char colbuf[18][20];
	int i, j;
	for(i=0; i<18; i++) map[i] = colbuf[i];
	g_snprintf(map[0], 20, "%d %d 16 1", len, len);
	for(i=0; i<16; i++)
	{
		g_snprintf (map[i+1], 20, "%c c #%s", hex[i],
				pixmap_get_hex_color(fg, bg, i / 15.0));
	}
	for (i=0; i<len; i++)
	{
		char *ptr = map[i+17] = buf + i * (len+1);
		for (j=0; j<len; j++)
		{
			int mid = len/2, x = i - mid, y = j - mid;
			float q = (x * x + y * y) / rad / rad;
			int  k;
			if (q<1) *ptr++ = '0';else
			for(k=1; k<16; k++)
				if ((q >= 1 + (k-1) / grad && q < 1 + k/grad) || k == 15)
				{ *ptr++ = hex[k]; break; }
		}
		*ptr++ = 0;
	}
	map[17+len] = NULL;
	return map;
}

static void rgbmap_ball_gen_real (int len, unsigned char *rgbbuf, int fg, float rad, float grad, float midx, float midy)
{
	int i, j;
	unsigned char *bufp = rgbbuf;
	for (i=0; i<len; i++)
	for (j=0; j<len; j++)
	{
		float x = i - midx, y = j - midy;
		float q = (x * x + y * y) / rad / rad;
		int bg = (bufp[0] << 16) + (bufp[1] << 8) + bufp[2];
		int color = q < 1 ? fg : pixmap_get_color (fg, bg, (q - 1) * grad / 16);
		*bufp++ = color >> 16;
		*bufp++ = (color >> 8) & 0xFF;
		*bufp++ = color & 0xFF;
	}
}

void rgbmap_square_gen (int len, unsigned char *rgbbuf, int fg, int bg, float side)
{
	int i, j;
	unsigned char *bufp = rgbbuf;
	for (i=0; i<len; i++)
	for (j=0; j<len; j++)
	{
		float x = 2*i - len, y = 2*j - len;
		int color;
		if (x < 0) x = -x;
		if (y < 0) y = -y;
		color =  x < side && y < side ? fg : bg;
		*bufp++ = color >> 16;
		*bufp++ = (color >> 8) & 0xFF;
		*bufp++ = color & 0xFF;
	}
}

void rgbmap_ball_gen (int len, unsigned char *rgbbuf, int fg, int bg, float rad, float grad)
{
	int i, j;
	unsigned char *bufp = rgbbuf;
	for (i=0; i<len; i++)
	for (j=0; j<len; j++)
	{
		*bufp++ = bg >> 16;
		*bufp++ = (bg >> 8) & 0xFF;
		*bufp++ = bg & 0xFF;
	}
	rgbmap_ball_gen_real (len, rgbbuf, fg, rad, grad, len/2, len/2);
}

//! Same as rgbmap_ball_gen but don't generate the background - i.e, overlay the ball on the existing image
void rgbmap_ball_gen_nobg (int len, unsigned char *rgbbuf, int fg, int bg, float rad, float grad)
{
	rgbmap_ball_gen_real (len, rgbbuf, fg, rad, grad, len/2, len/2);
}

void rgbmap_ball_shadow_gen (int len, unsigned char *rgbbuf, int fg, int bg, float rad, float grad, int shadowlen)
{
	int i, j;
	unsigned char *bufp = rgbbuf;
	int red, green, blue, lighting_color, shadow_color;
	float shadow_factor = 0.25;
	red   = (fg >> 16) / 4 + 192;
	green = ((fg >>  8) & 0xff) / 4 + 192;
	blue  = (fg & 0xff) / 4 + 192;
	lighting_color = (red << 16) + (green << 8) + blue;
	red   = (fg >> 16) * shadow_factor;
	green = ((fg >>  8) & 0xff) * shadow_factor;
	blue  = (fg & 0xff) * shadow_factor;
	shadow_color = (red << 16) + (green << 8) + blue;
	for (i=0; i<len; i++)
	for (j=0; j<len; j++)
	{
		*bufp++ = bg >> 16;
		*bufp++ = (bg >> 8) & 0xFF;
		*bufp++ = bg & 0xFF;
	}
	rgbmap_ball_gen_real (len, rgbbuf, shadow_color, rad, grad, 
			(len + shadowlen)/2, (len + shadowlen)/2);
	rgbmap_ball_gen_real (len, rgbbuf, lighting_color, rad, grad, 
			(len - 1.5 * shadowlen)/2, (len - 1.5 * shadowlen)/2);
	rgbmap_ball_gen_real (len, rgbbuf, fg, rad, grad, 
			(len - shadowlen)/2, (len - shadowlen)/2);
}

	
//! Used if you already have the pixmap and only want to generate the header (i.e, same shape, different color)
char ** pixmap_header_gen(int len, char *pixbuf, int fg, int bg)
{
	char **map = pixmap_map;
	char *buf = pixbuf;
	static char colbuf[18][20];
	int i, j;
	for(i=0; i<18; i++) map[i] = colbuf[i];
	g_snprintf(map[0], 20, "%d %d 16 1", len, len);
	for(i=0; i<16; i++)
	{
		g_snprintf (map[i+1], 20, "%c c #%s", hex[i],
				pixmap_get_hex_color(fg, bg, i / 15.0));
	}
	for (i=0; i<len; i++)
		map[i+17] = buf + i * (len+1);
	map[17+len] = NULL;
	return map;
}


//! Generates dice.
/** Args same as pixmap_ball_gen() except num which is the number on the die */
char ** pixmap_die_gen(int len, char *pixbuf, int fg, int bg, float rad, float grad, int num)
{
	char **map = pixmap_map;
	char *buf = pixbuf;
	static char colbuf[18][20];
	int i, j, k;
	float cenx[6][6] = 
	{
		{ 0.5 },
		{ 0.25, 0.75},
		{ 0.2, 0.5, 0.8 },
		{ 0.25, 0.25, 0.75, 0.75 },
		{ 0.2, 0.2, 0.5, 0.8, 0.8 },
		{ 0.25, 0.25, 0.25, 0.75, 0.75, 0.75 }
	};
	float ceny[6][6] = 
	{
		{ 0.5 },
		{ 0.25, 0.75},
		{ 0.2, 0.5, 0.8 },
		{ 0.25, 0.75, 0.25, 0.75 },
		{ 0.2, 0.8, 0.5, 0.2, 0.8 },
		{ 0.2, 0.5, 0.8, 0.2, 0.5, 0.8 }
	};
	for(i=0; i<18; i++) map[i] = colbuf[i];
	g_snprintf(map[0], 20, "%d %d 16 1", len, len);
	for(i=0; i<16; i++)
	{
		g_snprintf (map[i+1], 20, "%c c #%s", hex[i],
				pixmap_get_hex_color(fg, bg, i / 15.0));
	}
	for (i=0; i<len; i++)
	{
		char *ptr = map[i+17] = buf + i * (len+1);
		memset (ptr, 'F', len);
		ptr[len] = 0;
		for (k=0; k<num; k++)
		{
			float midx = cenx[num-1][k] * len;
			float midy = (1 - ceny[num-1][k]) * len;
			for (j=0; j<len; j++)
			{
				float x = i - midx, y = j - midy;
				float q = (x * x + y * y) / rad / rad;
				int  k;
				if (q < 1) {ptr[j] = '0';}
				else
				for(k=1; k<16; k++)
					if ((q >= 1 + (k-1) / grad && q < 1 + k/grad))
					{ ptr[j] = hex[k]; break; }
			}
		}
	}
	map[17+len] = NULL;
	return map;
}

