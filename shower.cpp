#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "vshow.h"
#include "recver.h"

/** 绑定 127.0.0.1:3020 udp 端口, 接收数据, 一个udp包, 总是完整帧, 交给 libavcodec 解码, 交给
 *  vshow 显示
 */

#define RECV_PORT 3020

int main(void)
{
	int ret;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		fprintf(stderr, "ERR: create sock err\n");
		exit(-1);
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(RECV_PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	if (bind(sock, (sockaddr*)&sin, sizeof(sin)) < 0)
	{
		fprintf(stderr, "ERR: bind %d\n", RECV_PORT);
		exit(-1);
	}

	uint8_t *buf = (uint8_t *)malloc(65536);
	if (!buf)
	{
		fprintf(stderr, "ERR: alloca 65536 err\n");
		exit(-1);
	}
	memset(buf, 0, 65536);

	avcodec_register_all();
	AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		printf("avcodec_find_decoder error\n");
		exit(-1);
	}

	AVCodecContext *dec = avcodec_alloc_context3(codec);
	if (!dec)
	{
		printf("avcodec_alloc_context3 error\n");
		exit(-1);
	}

	AVCodecParserContext *avParserContext = av_parser_init(codec->id);
	if (!avParserContext)
	{
		printf("av_parser_init error\n");
		exit(-1);
	}

	//初始化参数，下面的参数应该由具体的业务决定  
	//dec->time_base.num = 1;  
	//dec->frame_number = 1; //每包一个视频帧  
	//dec->codec_type = AVMEDIA_TYPE_VIDEO;  
	//dec->bit_rate = 0;  
	dec->time_base.den = 30;//帧率
	dec->codec_id = AV_CODEC_ID_H264;
	dec->width = 640;//视频宽  
	dec->height = 480;//视频高 
	dec->pix_fmt = AV_PIX_FMT_YUV422P;

	int num_bytes = avpicture_get_size(AV_PIX_FMT_YUV422P, 640, 480);
	printf("num_bytes:%d\n", num_bytes);
	uint8_t *avbuf = (uint8_t*)malloc(num_bytes);
	if (avbuf == NULL)
	{
		printf("malloc error\n");
		exit(-1);
	}

	AVFrame *frame = av_frame_alloc();
	if (frame == NULL)
	{
		printf("av_frame_alloc");
		exit(-1);
	}

	avpicture_fill((AVPicture *)frame, avbuf, AV_PIX_FMT_YUV422P, 680, 480);

	if(codec->capabilities & AV_CODEC_CAP_TRUNCATED)
		dec->flags |= AV_CODEC_FLAG_TRUNCATED;

	if (avcodec_open2(dec, codec, NULL) < 0)
	{
		fprintf(stderr, "ERR: open H264 decoder err\n");
		exit(-1);
	}

	void *shower = NULL;	// 成功解码第一帧后, 才知道大小
	int rc;
	fd_set net_rset;
	struct timeval tv;
	sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	uint8_t *decode_data = NULL;
	AVPacket *pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	for (; ; )
	{
		FD_ZERO(&net_rset);
		FD_SET(sock, &net_rset);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		ret = select(sock + 1, &net_rset, NULL, NULL, NULL);
		if (ret > 0 && FD_ISSET(sock, &net_rset) > 0)
		{
			//rc = recvfrom(sock, buf, 65536, 0, (sockaddr*)&from, &fromlen);
			rc = recv(sock, buf, 65536, MSG_DONTWAIT);
			printf("recv data len:%d\n", rc);
			
			decode_data = buf;
			while (rc > 0) 
			{
				// 解压
				ret = av_parser_parse2(avParserContext, dec, &pkt->data, &pkt->size,
						decode_data, rc, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
				if (ret < 0)
				{
					printf("av_parser_parse2 error\n");
					break;
				}
				printf("av_parser_parse2 succeed >>> pkt->size:%d\n", pkt->size);

				decode_data += ret;
				rc -= ret;

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
		}
		else
			printf("select error\n");

	}

	avcodec_close(dec);
	av_free(dec);
	av_free(frame);
	close(sock);
	vs_close(shower);
	return 0;
}

