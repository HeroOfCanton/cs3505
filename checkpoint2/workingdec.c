/*
 * Authors: Ryan Welling and James Lundgren
 * Date: 3/3/2015
 * This class decodes .mpff image files for the .mpff AV codec in ffmpeg
 * The structure of this decoder is based off of the BMP image format decoder
 *
 * Copyright (c) 2005 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <inttypes.h>
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "msrledec.h"

static int mpff_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    unsigned int fsize, hsize;
    int width, height;
    unsigned int depth;

    unsigned int ihsize;
    int i, n, linesize, ret; // j is unused as is
  
    uint8_t *ptr;
    int dsize;
    const uint8_t *buf0 = buf;
  
    
    // need at least the file header, which is 12 bits
    if (buf_size < 12) 
    {
      av_log(avctx, AV_LOG_ERROR, "buf size too small (%d)\n", buf_size);
      return AVERROR_INVALIDDATA;
    }

    // Make sure the file is an .mpff file
    if (bytestream_get_byte(&buf) != 'M' ||
        bytestream_get_byte(&buf) != 'P' ||
        bytestream_get_byte(&buf) != 'F' ||
        bytestream_get_byte(&buf) != 'F')
    {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }

    // get the file size and ensure it is correct
    fsize = bytestream_get_le32(&buf);
    if (buf_size < fsize) 
    {
      av_log(avctx, AV_LOG_ERROR, "not enough data (%d < %u), trying to decode anyway\n",
	     buf_size, fsize);
      fsize = buf_size;
    }
    
    
    hsize  = bytestream_get_le32(&buf); /* header size */
    ihsize = bytestream_get_le32(&buf); /* more header size */
    
    // was 14LL... all 14's are now 12...might cause problem
    if (ihsize + 12LL > hsize) 
    {
      av_log(avctx, AV_LOG_ERROR, "invalid header size %u\n", hsize);
      return AVERROR_INVALIDDATA;
    }

    /* sometimes file size is set to some headers size, set a real size in that case */
    if (fsize == 12 || fsize == ihsize + 12)
        fsize = buf_size - 2;

    // The file size needs to be larger than the header
    if (fsize <= hsize) 
    {
      av_log(avctx, AV_LOG_ERROR,
	     "Declared file size is less than header size (%u < %u)\n",
	     fsize, hsize);
      return AVERROR_INVALIDDATA;
    }
    

    // get the width, height, and depth of the image
    width  = bytestream_get_le32(&buf);
    height = bytestream_get_le32(&buf);
    depth = bytestream_get_le16(&buf);

    avctx->width  = width;
    avctx->height = height > 0 ? height : -height;

    //avctx->pix_fmt = AV_PIX_FMT_NONE;

    // The only format we accept/encode is BGR24
    // should have the swtich statement still? 
    avctx->pix_fmt = AV_PIX_FMT_BGR24;

    
    if (avctx->pix_fmt == AV_PIX_FMT_NONE) {
        av_log(avctx, AV_LOG_ERROR, "unsupported pixel format\n");
        return AVERROR_INVALIDDATA;
    }

    // Get the buffer, if any errors occur, return out of the decoder
    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;

    // Set the picture type and key_frame as we did in the encoder
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;


    buf   = buf0 + hsize;
    dsize = buf_size - hsize;

    /* Line size in file multiple of 4 */
    n = ((avctx->width * depth + 31) / 8) & ~3;
    
   

    if (height > 0) 
    {
      ptr      = p->data[0] + (avctx->height - 1) * p->linesize[0];
      linesize = -p->linesize[0];
    } 
    else 
    {
      ptr      = p->data[0];
      linesize = p->linesize[0];
    }
   
    // Decode the image so that it can be displayed
    for (i = 0; i < avctx->height; i++) 
    {
      memcpy(ptr, buf, n);
      buf += n;
      ptr += linesize;
    }
	 

    *got_frame = 1;

    return buf_size;
}

AVCodec ff_mpff_decoder = {
    .name           = "mpff",
    .long_name      = NULL_IF_CONFIG_SMALL("mpff (Windows and OS/2 bitmap)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MPFF,
    .decode         = mpff_decode_frame,
    .capabilities   = CODEC_CAP_DR1,
};
