/*
 * Authors: Ryan Welling and James Lundgren
 * Date: 3/3/2015
 * This class decodes .mpff image files for the .mpff AV codec in ffmpeg
 * The structure of this decoder is based off of the BMP image format decoder
 *
 * BMP image format decoder
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


static int mpff_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;

    AVFrame *p;
    int width, height, depth, n;
    int i, ret, linesize;
    uint8_t *ptr;

 
    if (buf_size < 12) {
        av_log(avctx, AV_LOG_ERROR, "buf size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

    if (bytestream_get_byte(&buf) != 'M' ||
        bytestream_get_byte(&buf) != 'P' ||
	bytestream_get_byte(&buf) != 'F' ||
        bytestream_get_byte(&buf) != 'F') 

    {
      av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
      return AVERROR_INVALIDDATA;
    }


    depth = 8;

    avctx->width  = width;
    avctx->height = height > 0 ? height : -height;

    avctx->pix_fmt = AV_PIX_FMT_NONE;
    avctx->pix_fmt = AV_PIX_FMT_RGB8;


    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
      return ret;

    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;


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

    for (i = 0; i < avctx->height; i++) 
    {
      memcpy(ptr, buf, n);
      buf += n;
      ptr += linesize;
    }
          
    *got_frame = 1;

    return buf_size;
}

AVCodec ff_mpff_decoder = 
{
  .name           = "mpff",
  .long_name      = NULL_IF_CONFIG_SMALL("MPFF (Windows and OS/2 bitmap)"),
  .type           = AVMEDIA_TYPE_VIDEO,
  .id             = AV_CODEC_ID_BMP,
  .decode         = mpff_decode_frame,
  .capabilities   = CODEC_CAP_DR1,
};

