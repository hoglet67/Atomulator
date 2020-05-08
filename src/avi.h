/*
**  Oricutron
**  Copyright (C) 2009-2014 Peter Gordon
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation, version 2
**  of the License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>

#define 	AVIF_HASINDEX         0x00000010
#define 	AVIF_MUSTUSEINDEX     0x00000020
#define 	AVIF_ISINTERLEAVED    0x00000100
#define 	AVIF_TRUSTCKTYPE      0x00000800
#define 	AVIF_WASCAPTUREFILE   0x00010000
#define 	AVIF_COPYRIGHTED      0x00020000

#define X_SIZE 512
#define Y_SIZE 384

struct avi_handle
{
   FILE       *f;
   uint32_t   csize;
   uint32_t   frames, frameadjust;
   uint32_t   hdrlsize;
   uint32_t   movisize;
   uint32_t   audiolen;
   uint32_t   offs_riffsize;
   uint32_t   offs_hdrlsize;
   uint32_t   offs_frames;
   uint32_t   offs_frames2;
   uint32_t   offs_movisize;
   uint32_t   offs_audiolen;
   uint32_t   offs_usperfrm;
   uint32_t   offs_frmrate;
   struct timeval time_start;

   bool       dosnd;
   bool       lastframevalid;
   uint8_t    rledata[X_SIZE*Y_SIZE*4];
   uint8_t    lastframe[X_SIZE*Y_SIZE];

   // This is an int instead of bool because we use bit 1 for optimisation purposes
   // (matches the oric 50hz bit)
   int is50hz;
};

struct avi_handle *avi_open( char *filename, uint8_t *pal, bool dosound, int is50hz );
bool avi_addframe( struct avi_handle **ah, uint8_t *srcdata );
bool avi_addaudio( struct avi_handle **ah, int16_t *audiodata, uint32_t audiosize );
void avi_close( struct avi_handle **ah );
