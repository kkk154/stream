#include <iostream>
#include <opencv2/opencv.hpp>
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

#ifdef __cplusplus
};
#endif


int main() {

    AVFormatContext *pFormatCtx;
    int             i, videoIndex;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    AVDictionary* options = nullptr;
    int ret, got_picture;
    bool isDec = true;

    struct SwsContext *img_convert_ctx;
    // 改成你自己的 URL
    char filepath[] = "rtsp://admin:iec123456@192.168.1.71:554/h264/ch1/main/av_stream";
    av_register_all();
    avformat_network_init();
    av_dict_set(&options, "buffer_size", "10240000", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, filepath, nullptr, &options) != 0)////打开网络流或文件流
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr)<0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoIndex = -1;
    for (i = 0; i<pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            break;
        }
    if (videoIndex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr)
    {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, nullptr)<0)
    {
        printf("Could not open codec.\n");
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_NV21, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_NV21, pCodecCtx->width, pCodecCtx->height);

    //Output Info---输出一些文件（RTSP）信息
    printf("---------------- File Information ---------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("-------------------------------------------------\n");

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_NV21, SWS_BICUBIC, nullptr, nullptr, nullptr);

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

//    FILE *fpSave;
//    if ((fpSave = fopen("geth264.h264", "ab")) == NULL) //h264保存的文件名
//        return 0;
    while(isDec){
        //------------------------------
        if (av_read_frame(pFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoIndex)
            {
//                fwrite(packet->data, 1, packet->size, fpSave);//写数据到文件中
                avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
                if(got_picture){
                    //YUV to RGB
                    sws_scale(img_convert_ctx,(const uint8_t* const*)pFrame->data,pFrame->linesize,0,pCodecCtx->height,pFrameYUV->data,pFrameYUV->linesize);
                    cv::Mat frame = cv::Mat(pCodecCtx->height*3 / 2, pCodecCtx->width, CV_8UC1, (unsigned char *)pFrameYUV->data[0]);
                    cv::imshow("frame", frame);
                    cv::waitKey(40);
                }
            }
            av_packet_unref(packet);
        }
    }
    //--------------
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
