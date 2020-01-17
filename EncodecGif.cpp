#include "EncodecGif.h"

EncodecGif::EncodecGif()
{
	avcodec_register_all();
}

EncodecGif::~EncodecGif()
{

}

bool EncodecGif::start(int width, int height, int fps, const QString& filename)
{
	// 查找编码器
	AVCodecID coecId = AV_CODEC_ID_GIF;
	AVCodec* gifCodec = avcodec_find_encoder(coecId);
	if (gifCodec == nullptr)
		return false;

	// 创建编码器上下文
	m_pCodecContext = avcodec_alloc_context3(gifCodec);
	if (m_pCodecContext == nullptr)
		return false;

	// 设置编码器参数
	m_pCodecContext->width = width;
	m_pCodecContext->height = height;
	m_pCodecContext->time_base.num = 1;
	m_pCodecContext->time_base.den = fps;
	m_pCodecContext->framerate.num = fps;
	m_pCodecContext->framerate.den = 1;
	m_pCodecContext->pix_fmt = AV_PIX_FMT_RGB24;

	// 打开编码器
	int result = avcodec_open2(m_pCodecContext, gifCodec, nullptr);
	if (result < 0)
	{
		// 释放内存
		avcodec_free_context(&m_pCodecContext);
		return false;
	}

	// 创建文件封装上下文
	if (!createFormatContext(filename))
	{
		avcodec_free_context(&m_pCodecContext);
		return false;
	}

	m_nWidth = width;
	m_nHeight = height;
	m_nFrameRate = fps;
	m_nCurrentFrame = 0;
	
	return true;
}

bool EncodecGif::createFormatContext(const QString& filename)
{
	// 创建输出封装上下文
	int result = avformat_alloc_output_context2(&m_pFormatContext, nullptr, nullptr, filename.toLocal8Bit().data());
	if (result < 0)
		return false;

	// 创建视频流
	AVStream* stream = avformat_new_stream(m_pFormatContext, m_pCodecContext->codec);
	if (stream == nullptr)
	{
		avformat_free_context(m_pFormatContext);
		return false;
	}

	// 填写流内容信息
	stream->id = 0;
	stream->time_base = { 1, m_nFrameRate };
	// 拷贝参数
	result = avcodec_parameters_from_context(stream->codecpar, m_pCodecContext);
	if (result < 0)
	{
		avformat_free_context(m_pFormatContext);
		return false;
	}

	// 打开文件IO
	if (!(m_pFormatContext->oformat->flags & AVFMT_NOFILE))
	{
		result = avio_open(&m_pFormatContext->pb, filename.toLocal8Bit().data(), AVIO_FLAG_WRITE);
		if (result < 0)
		{
			avformat_free_context(m_pFormatContext);
			return false;
		}
	}

	// 写入文件头
	result = avformat_write_header(m_pFormatContext, nullptr);
	if (result < 0)
	{
		avformat_free_context(m_pFormatContext);
		return false;
	}

	return true;
}

bool EncodecGif::writeImageData(const unsigned char* pSrc)
{
	// 为Frame申请内存
	if (m_frame == nullptr) 
	{
		m_frame = av_frame_alloc();
		m_frame->format = AV_PIX_FMT_RGB24;
	}

	if (m_packet == nullptr)
		m_packet = av_packet_alloc();

	m_frame->width = m_nWidth;
	m_frame->height = m_nHeight;

	// fill frame data
	int result = av_frame_get_buffer(m_frame, 0);
	if (result < 0) 
		return false;

	// make sure the frame data is writeable
	result = av_frame_make_writable(m_frame);
	if (result < 0)
		return false;

	m_frame->pts = m_nCurrentFrame++;
	// 拷贝数据到Frame
	memcpy(m_frame->data[0], pSrc, m_nWidth * m_nHeight * 3);

	// 编码
	if (!encodeFunc(m_pCodecContext, m_frame, m_packet))
		return false;

	return true;
}

bool EncodecGif::encodeFunc(AVCodecContext* context, AVFrame* frame, AVPacket* packet)
{
	// encodec
	bool result = avcodec_send_frame(context, frame);
	if (result < 0)
		return false;

	while (result > 0)
	{
		result = avcodec_receive_packet(context, packet);
		if (result == AVERROR(EAGAIN) || result == AVERROR(AVERROR_EOF))
			return false;
		else if (result < 0)
			return false;

		// 时间戳转换
		av_packet_rescale_ts(packet, context->time_base, m_pFormatContext->streams[0]->time_base);
		packet->stream_index = 0;
		int res = av_interleaved_write_frame(m_pFormatContext, packet);
		if (res < 0)
			continue;

		// 释放内存
		av_packet_free(&packet);
	}
}

void EncodecGif::end(void)
{
	encodeFunc(m_pCodecContext, nullptr, m_packet);

	// 写入文件尾
	av_write_trailer(m_pFormatContext);
	// 关闭文件
	avio_closep(&m_pFormatContext->pb);
	av_frame_free(&m_frame);

	avcodec_free_context(&m_pCodecContext);
	avformat_free_context(m_pFormatContext);
}
