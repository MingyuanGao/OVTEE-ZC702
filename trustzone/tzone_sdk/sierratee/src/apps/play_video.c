/* 
 * OpenVirtualization: 
 * For additional details and support contact developer@sierraware.com.
 * Additional documentation can be found at www.openvirtualization.org
 * 
 * Copyright (C) 2011 SierraWare
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <stdlib.h>
#include <sw_lib_fb.h>
#include <sw_syscall.h>

#define MAX_NUM_OF_FRAMES 10
/* Definition from sw_fb.h */
#define RGB444_FORMAT 1 

void fb_display_video(uint8_t *displayBuffer[], int img_size)
{
	int fd,loop_repeat=0;
	struct sw_fb_info *temp;
	temp = malloc(sizeof(struct sw_fb_info));
	if(!temp) {
		printf("Cannot allocate space");
		return;
	}
	fd = __sw_open("fb0",0,0);
	if(fd<0)
	{
		free(temp);
		printf("Failed to open framebuffer\n");
		return;
	}
	__sw_ioctl(fd,SW_FB_IOCTL_GET_INFO,0,temp);
	temp->xres = 352;
	temp->yres = 288;
	temp->format = RGB444_FORMAT;
	__sw_ioctl(fd,SW_FB_IOCTL_SET_INFO,temp,0);
	
	while(loop_repeat<300)    /* Run loop 60 times */
	{
		__sw_ioctl(fd,SW_FB_IOCTL_WRITE_WINDOW,displayBuffer[(loop_repeat++)%10],(void*)img_size);
		__sw_usleep(33333.33333);   /*30 fps*/
    }
    free(temp);
    __sw_close(fd);
	
}

int play_video(char* filename) 
{
  AVFormatContext *pFormatCtx = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx = NULL;
  AVCodec         *pCodec = NULL;
  AVFrame         *pFrame = NULL; 
  AVFrame         *pFrameRGB = NULL;
  AVPacket        packet;
  int             frameFinished;
  int             numBytes;
  uint8_t         *buffer = NULL;
  
  int ret_val, loop_repeat=0;
  uint8_t *displayBuffer[MAX_NUM_OF_FRAMES];
  
  AVDictionary *optionsDict = NULL;
  struct SwsContext *sws_ctx = NULL;
  
  if(!filename) {
    printf("Please provide a movie file\n");
    return -1;
  }
  
  for(i=0;i<MAX_NUM_OF_FRAMES;i++) {
  	displayBuffer[i] = NULL;
  }
  /* Register required formats and codecs */
  av_register_all();
  /* Open video file */
  if(avformat_open_input(&pFormatCtx, filename, NULL, NULL)!=0) {
  	printf("Could not open media file\n");
    goto ret_path; /* Couldn't open file */
    }
    
  /* Find video stream */
  videoStream=1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1) {
    printf("Could not open media file\n");
    goto ret_path; /* Video stream not found */
    }
    
  pFormatCtx->streams[videoStream]->codec->width=352;
  pFormatCtx->streams[videoStream]->codec->height=288;
  pFormatCtx->streams[videoStream]->codec->pix_fmt=PIX_FMT_RGB444LE;
  /* Get stream information */
  if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
    printf("Could not open media file\n");
    goto ret_path; /* Stream information not found */
    }
  
  /* Dump information about file onto STDERR */
  av_dump_format(pFormatCtx, 0, filename, 0);
    
  /* Get a pointer to the video stream Codec Context */
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  /* Find the video stream decoder */
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    printf("Unsupported codec!\n");
    goto ret_path; /* Codec not found */
  }
  /* Open codec */
  if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0) {
    printf("Could not open codec\n");
    goto ret_path; /* Could not open codec */
    }
  
  /* Allocate a video frame */
  pFrame=avcodec_alloc_frame();
  if(pFrame==NULL) {
    printf("Could not allocate frame\n");
    goto ret_path;
    }
  
  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL) {
    printf("Could not allocate frame\n");
    goto ret_path;
    }
  
  /* Determine the required buffer size and allocate a buffer */
  numBytes=avpicture_get_size(PIX_FMT_RGB444, pCodecCtx->width,
			      pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  if(!buffer) {
  	printf("Could not allocate space for buffer\n");
  	goto ret_path;
  }

  sws_ctx =
    sws_getContext
    (
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        PIX_FMT_RGB444,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
  
  /* Assign appropriate parts of the buffer to image planes in pFrameRGB */
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB444,
		 pCodecCtx->width, pCodecCtx->height);
  i=0;
  
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    /* Check if packet is from the video stream */
    if(packet.stream_index==videoStream) {
      /* Decode the video frame */

      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
			   &packet);
      
      /* Check if the Video frame is acquired */
      if(frameFinished) {
      
	/* Convert the image from its native format to RGB */
        sws_scale
        (
            sws_ctx,
            (uint8_t const * const *)pFrame->data,
            pFrame->linesize,
            0,
            pCodecCtx->height,
            pFrameRGB->data,
            pFrameRGB->linesize
        );
	
	if(i<=MAX_NUM_OF_FRAMES) {
		displayBuffer[i] = malloc(pFrameRGB->linesize[0]*pCodecCtx->height);
		if(!displayBuffer[i]) {
			printf("Could not allocate space to store frame %d",i);
			goto ret_path;
	    }
      memcpy(displayBuffer[i++],pFrameRGB->data[0],pFrameRGB->linesize[0]*pCodecCtx->height);
	}
      }
    }
    	 
    /* Free the packet that was allocated by av_read_frame */
    av_free_packet(&packet);
  }
  
  fb_display_video(displayBuffer,pFrameRGB->linesize[0]*pCodecCtx->height);
  
ret_path:
  /* Free frames in the buffer */
	for(i=0;i<MAX_NUM_OF_FRAMES;i++) {
	    	if(displayBuffer[i])
		    	free(displayBuffer[i]);
	}
	if(sws_ctx)
		sws_freeContext(sws_ctx);
	if(buffer)				
		av_free(buffer);
	if(pFrameRGB)		/* Free the RGB image */
		av_free(pFrameRGB);
	if(pFrame)			  /* Free the original format frame */
		av_free(pFrame);
	if(pCodecCtx)			  /* Close the codec */
		avcodec_close(pCodecCtx);
	if(pFormatCtx)			  /* Close the video file */
		avformat_close_input(&pFormatCtx);
	return 0;
}
