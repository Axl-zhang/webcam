#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

#include "capture.h"
#include "vcompress.h"
#include "sender.h"
#include "vshow.h"
#include "recver.h"

/** webcam_server: 打开 /dev/video0, 获取图像, 压缩, 发送到 localhost:3020 端口
 *
 * 	使用 320x240, fps=10
 */
#define VIDEO_WIDTH 640 
#define VIDEO_HEIGHT 480 
#define VIDEO_FPS 7.0

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 3020




static void sighandler(int sig_no)
{
	printf("user kill\n");
	exit(0);
}



int main (void)
{
	int ret;

	signal(SIGUSR1, sighandler);

	void *capture = capture_open("/dev/video0", 
			VIDEO_WIDTH, VIDEO_HEIGHT,
			AV_PIX_FMT_YUV422P);
	if (!capture)
	{
		fprintf(stderr, "ERR: can't open '/dev/video0'\n");
		exit(-1);
	}

	void *encoder = vc_open(VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS);
	if (!encoder) 
	{
		fprintf(stderr, "ERR: can't open x264 encoder\n");
		exit(-1);
	}

	void *sender = sender_open(TARGET_IP, TARGET_PORT);
	if (!sender)
	{
		fprintf(stderr, "ERR: can't open sender for %s:%d\n",
			       	TARGET_IP, TARGET_PORT);
		exit(-1);
	}

	int tosleep = 1000000 / VIDEO_FPS;


	avcodec_register_all();
	AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		printf("avcodec_find_decoder error\n");
		return -1;
	}

	AVCodecContext *dec = avcodec_alloc_context3(codec);
	if (!dec)
	{
		printf("avcodec_alloc_context3 error\n");
		return -1;
	}

	AVCodecParserContext *avParserContext = av_parser_init(codec->id);
	if (!avParserContext)
	{
		printf("av_parser_init error\n");
		return -1;
	}

	//初始化参数，下面的参数应该由具体的业务决定  
	//dec->time_base.num = 1;  
	//dec->frame_number = 1; //每包一个视频帧  
	//dec->codec_type = AVMEDIA_TYPE_VIDEO;  
	//dec->bit_rate = 0;  
	//dec->time_base.den = 30;//帧率
	dec->codec_id = AV_CODEC_ID_H264;
	dec->width = 640;//视频宽  
	dec->height = 480;//视频高 
	dec->pix_fmt = AV_PIX_FMT_YUV422P;

	int num_bytes = avpicture_get_size(AV_PIX_FMT_YUV422P, 640, 480);
	uint8_t *avbuf = (uint8_t*)malloc(num_bytes);
	if (avbuf == NULL)
	{
		printf("malloc error\n");
		return -1;
	}

	AVFrame *frame = av_frame_alloc();
	if (frame == NULL)
	{
		printf("av_frame_alloc");
		return -1;
	}

	avpicture_fill((AVPicture *)frame, avbuf, AV_PIX_FMT_YUV422P, 680, 480);

	if(codec->capabilities & AV_CODEC_CAP_TRUNCATED)
		dec->flags |= AV_CODEC_FLAG_TRUNCATED;

	if (avcodec_open2(dec, codec, NULL) < 0)
	{
		fprintf(stderr, "ERR: open H264 decoder err\n");
		return -1;	
	}

	void *shower = 0;	// 成功解码第一帧后, 才知道大小
	AVPacket *pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	const void *outdata;
	int outlen, rc;
	uint8_t * codec_data = NULL;


//	av_opt_set(dec->priv_data, "preset", "superfast", 0);
	av_opt_set(dec->priv_data, "tune", "zerolatency", 0);

	for (; ; )
	{
		// 抓
		Picture pic;
		capture_get_picture(capture, &pic);

		// 压
		rc = vc_compress(encoder, pic.data, pic.stride, &outdata, &outlen);
		if (rc < 0)
			continue;

		printf("encode data len:%d\n", outlen);
		// 发
		sender_send(sender, outdata, outlen);
		codec_data = (uint8_t *)outdata;
#if 1 
		// 解压
		while (outlen > 0)
		{
			ret = av_parser_parse2(avParserContext, dec, &pkt->data, &pkt->size,
					codec_data, outlen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0)
			{
				printf("av_parser_parse2 error\n");
				continue;
			}
			printf("av_parser_parse2 succeed >>> pkt.size:%d\n", pkt->size);

			codec_data += ret;
			outlen -= ret;

			if (pkt->size > 0)
			{
				ret = avcodec_send_packet(dec, pkt);
				if (ret < 0)
				{
					printf("avcodec_send_packet error\n");
					continue;
				}

				printf("avcodec_send_packet succeed ret:%d\n", ret);

				while (ret >= 0)
				{
					ret = avcodec_receive_frame(dec, frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					{
						break;
					}
					else if (ret < 0)
					{
						printf("decode error\n");
						break;
					}

					// 解码成功
					printf("decode succeed\n");

					if (!shower)
						shower = vs_open(dec->width, dec->height);
					printf("++++++ dec->width:%d, dec->height:%d +++++++\n",
							       	dec->width, dec->height);
					if (!shower)
					{
						fprintf(stderr, "ERR: open shower window err!\n");
						break;
					}
						// 显示
					vs_show(shower, frame->data, frame->linesize);
				}
			}
		}
		// 等
#endif
		//usleep(tosleep);

	}

	sender_close(sender);
	vc_close(encoder);
	capture_close(capture);

	return 0;
}

