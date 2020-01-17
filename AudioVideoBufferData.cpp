#include "AudioVideoBufferData.h"
#include <QMutexLocker>

AudioVideoBufferData::AudioVideoBufferData(QObject* parent)
	: QObject(parent)
	, m_nVideoStartIndex(0)
	, m_nVideoEndIndex(0)
	, m_nVideoTotalCount(0)
	, m_nAudioStartIndex(0)
	, m_nAudioEndIndex(0)
	, m_nAudioTotalCount(0)
	, m_nVideoFrameStartIndex(0)
	, m_nVideoFrameEndIndex(0)
	, m_nVideoFrameTotalCount(0)
	, m_nAudioFrameStartIndex(0)
	, m_nAudioFrameEndIndex(0)
	, m_nAudioFrameTotalCount(0)
	, m_globalTimeClock(0)
{
	
}

AudioVideoBufferData::~AudioVideoBufferData()
{

}

// 添加一个视频视频数据到队列
bool AudioVideoBufferData::addVideoPacketToQueue(AVPacket* packet)
{
	if (m_nVideoTotalCount == MAX_VIDEO_PACKET_BUFFERSIZE)
		return false;

	m_videoPacketQueue[m_nVideoEndIndex++] = packet;
	if (m_nVideoEndIndex == MAX_VIDEO_PACKET_BUFFERSIZE)
		m_nVideoEndIndex = 0;
	m_nVideoTotalCount++;

	return true;
}

// 获取一个视频数据包
AVPacket* AudioVideoBufferData::takeVideoPacketFromQueue(void)
{
	if (m_nVideoTotalCount == 0)
		return nullptr;

	AVPacket* packet = m_videoPacketQueue[m_nAudioStartIndex++];
	if (m_nAudioStartIndex == MAX_VIDEO_PACKET_BUFFERSIZE)
		m_nAudioStartIndex = 0;
	m_nVideoTotalCount--;

	return packet;
}

// 添加一个音频数据到队列
bool AudioVideoBufferData::addAudioPacketToQueue(AVPacket* packet)
{
	if (m_nAudioTotalCount == MAX_AUDIO_PACKET_BUFFERSIZE)
		return false;

	m_audioPacketQueue[m_nAudioEndIndex++] = packet;
	if (m_nAudioEndIndex == MAX_AUDIO_PACKET_BUFFERSIZE)
		m_nAudioEndIndex = 0;
	m_nAudioTotalCount++;

	return true;
}

AVPacket* AudioVideoBufferData::takeAudioPacketFromQueue(void)
{
	if (m_nAudioTotalCount == 0)
		return nullptr;

	AVPacket* packet = m_audioPacketQueue[m_nAudioStartIndex++];
	if (m_nAudioStartIndex == MAX_AUDIO_PACKET_BUFFERSIZE)
		m_nAudioStartIndex = 0;
	m_nAudioTotalCount--;

	return packet;
}

void AudioVideoBufferData::setSyncTimeClock(qreal time)
{
	m_globalTimeClock = time;
}

qreal AudioVideoBufferData::getSyncTimeClock(void)
{
	return m_globalTimeClock;
}

AudioVideoBufferData* AudioVideoBufferData::getInstance(void)
{
	static AudioVideoBufferData instance;
	return &instance;
}

// ------------------------------------------------------------------------------
FFMpegOperator::FFMpegOperator()
{

}

FFMpegOperator::~FFMpegOperator()
{

}

// open video file
bool FFMpegOperator::openVideoFile(const QString& fileName)
{
	// open input format
	int result = avformat_open_input(&m_pFormatContext, fileName.toLocal8Bit().data(), nullptr, nullptr);
	if (result != 0 || m_pFormatContext == nullptr)
		return false;

	// fill stream info
	if (avformat_find_stream_info(m_pFormatContext, nullptr) < 0)
	{
		avformat_close_input(&m_pFormatContext);
		return false;
	}

	// get video and audio stream id
	int streamCount = m_pFormatContext->nb_streams;
	for (int i = 0; i < streamCount; ++i)
	{
		if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			m_nVideoIndex = i;
		else if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			m_nAudioIndex = i;
	}

	if (m_nVideoIndex < 0)
	{
		avformat_close_input(&m_pFormatContext);
		return false;
	}

	// open video codec
	if (!openVideoCodec())
		return false;
	// open audio codec
	openAudioCodec();

	return true;
}

void FFMpegOperator::closeVideoFile(void)
{

}

// open video decodec
bool FFMpegOperator::openVideoCodec(void)
{
	// find video codec
	m_pVideoCodecContext = m_pFormatContext->streams[m_nVideoIndex]->codec;
	AVCodec* videoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);
	if (videoCodec == nullptr)
	{
		avformat_close_input(&m_pFormatContext);
		return false;
	}

	// open video codec
	int result = avcodec_open2(m_pVideoCodecContext, videoCodec, nullptr);
	if (result != 0)
	{
		avformat_close_input(&m_pFormatContext);
		return false;
	}

	// fill info
	g_AudioVideoData->m_decodecInfo.width = m_pVideoCodecContext->width;
	g_AudioVideoData->m_decodecInfo.height = m_pVideoCodecContext->height;
#ifdef USEDOPENFLRENDER
	if (m_pVideoCodecContext->pix_fmt == AV_PIX_FMT_YUV420P)
		m_decodecInfo.isYUV420P = true;
	else
		m_decodecInfo.isYUV420P = false;
#else
	g_AudioVideoData->m_decodecInfo.isYUV420P = false;
#endif

	g_AudioVideoData->m_decodecInfo.totalTime = m_pFormatContext->duration * 1.0 / AV_TIME_BASE;

	return true;
}

// open audio decodec
bool FFMpegOperator::openAudioCodec(void)
{
	// find audio codec
	if (m_nAudioIndex < 0)
		return false;

	// find audio codec
	m_pAudioCodecContext = m_pFormatContext->streams[m_nAudioIndex]->codec;
	AVCodec* audioCodec = avcodec_find_decoder(m_pAudioCodecContext->codec_id);
	if (audioCodec == nullptr)
		return false;

	// open audio codec
	int result = avcodec_open2(m_pAudioCodecContext, audioCodec, nullptr);
	if (result != 0)
		return false;

	// fill audio info
	g_AudioVideoData->m_decodecInfo.sampleRate = m_pAudioCodecContext->sample_rate;
	g_AudioVideoData->m_decodecInfo.channelSize = m_pAudioCodecContext->channels;
	g_AudioVideoData->m_decodecInfo.needToConver = true;
	if (m_pAudioCodecContext->sample_fmt == AV_SAMPLE_FMT_S16)
	{
		g_AudioVideoData->m_decodecInfo.needToConver = false;
		g_AudioVideoData->m_decodecInfo.sampleSize = 16;
	}
	else if (m_pAudioCodecContext->sample_fmt == AV_SAMPLE_FMT_S32)
	{
		g_AudioVideoData->m_decodecInfo.needToConver = false;
		g_AudioVideoData->m_decodecInfo.sampleSize = 32;
	}
	else if (m_pAudioCodecContext->sample_fmt == AV_SAMPLE_FMT_U8)
	{
		g_AudioVideoData->m_decodecInfo.needToConver = false;
		g_AudioVideoData->m_decodecInfo.sampleSize = 8;
	}
	m_pAudioCodecContext->channel_layout = av_get_default_channel_layout(m_pAudioCodecContext->channels);

	if (m_pTempBuffer == nullptr)
	{
		m_pTempBuffer = new uchar[10000];
		memset(m_pTempBuffer, 0, 10000);
	}

	return true;
}

bool FFMpegOperator::deMuxing(AVPacket*& packet, bool& isEnd)
{
	isEnd = false;
	if (m_pFormatContext == nullptr)
		return false;

	if (packet == nullptr)
		packet = av_packet_alloc();

	// DeMuxing
	int result = av_read_frame(m_pFormatContext, packet);
	if (result == AVERROR_EOF)
	{
		isEnd = true;
		return false;
	}
	else if (result < 0)
		return false;

	return true;
}

void FFMpegOperator::decodecVideo(AVPacket* packet)
{
	if (m_pVideoFrame == nullptr)
		m_pVideoFrame = av_frame_alloc();

	// send packet
	int result = avcodec_send_packet(m_pVideoCodecContext, packet);
	if (result < 0)
		return;

	// recv frame
	while (result >= 0)
	{
		result = avcodec_receive_frame(m_pVideoCodecContext, m_pVideoFrame);
		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
			return;
		else if (result < 0)
			return;
	}

	// copy 
	AVFrame* tempFrame = av_frame_clone(m_pVideoFrame);

	// 添加到队列
	g_AudioVideoData->m_videoFrameData[g_AudioVideoData->m_nVideoFrameEndIndex++];
	if (g_AudioVideoData->m_nVideoFrameEndIndex == g_AudioVideoData->m_nVideoFrameTotalCount)
		g_AudioVideoData->m_nVideoFrameEndIndex = 0;
	g_AudioVideoData->m_nVideoFrameTotalCount++;

	av_packet_unref(packet);
	av_free_packet(packet);
}

// decodec Audio
void FFMpegOperator::decodecAudio(AVPacket* packet)
{
	if (m_pAudioFrame == nullptr)
		m_pAudioFrame = av_frame_alloc();

	//qDebug() << "Decodec Audio";
	// send packet
	int result = avcodec_send_packet(m_pAudioCodecContext, packet);
	if (result < 0)
		return;

	// recv frame
	// recv frame
	while (result >= 0)
	{
		result = avcodec_receive_frame(m_pAudioCodecContext, m_pAudioFrame);
		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
			return;
		else if (result < 0)
			return;
	}

	// add to buffer
	if (g_AudioVideoData->m_decodecInfo.needToConver)
	{
		QMutexLocker locker(&g_AudioVideoData->m_audioMutex);
		int len = frameSameSampe(m_pAudioFrame, m_pTempBuffer, 10000);
		if (len > 0)
			g_AudioVideoData->m_audioData.append((char*)m_pTempBuffer, len);
	}
	else
	{
		AVFrame* frame = av_frame_clone(m_pAudioFrame);
		g_AudioVideoData->m_audioFrameData[g_AudioVideoData->m_nAudioFrameEndIndex] = frame;
	}

	av_packet_unref(packet);
	av_free_packet(packet);
	//qDebug() << "Audio time is " << m_pFrame->pts * av_q2d(m_pFormatContext->streams[m_nAudioIndex]->time_base);
}

int FFMpegOperator::frameSameSampe(AVFrame* frame, uchar* pDest, int size)
{
	// get output audio info
	int sampleRato, sampleSize, channelSize;
	getAudioOutputInfos(sampleRato, sampleSize, channelSize);

	if (m_pSwrContext == nullptr)
	{
		m_pSwrContext = swr_alloc();

		int64_t outputChannelLayout = av_get_default_channel_layout(channelSize);

		swr_alloc_set_opts(m_pSwrContext, outputChannelLayout, AV_SAMPLE_FMT_S16, \
			m_pAudioCodecContext->sample_rate, m_pAudioCodecContext->channel_layout, m_pAudioCodecContext->sample_fmt, \
			m_pAudioCodecContext->sample_rate, 0, nullptr);
	}

	int result = swr_init(m_pSwrContext);

	uint8_t *array[1];
	array[0] = pDest;
	int len = swr_convert(m_pSwrContext, array, size, (const uint8_t**)frame->data, frame->nb_samples);
	len = len * channelSize * sampleSize / 8;
	return len;
}

void FFMpegOperator::getAudioOutputInfos(int& sampleRato, int& sampleSize, int& channelSize)
{
	sampleRato = g_AudioVideoData->m_decodecInfo.sampleRate;
	if (g_AudioVideoData->m_decodecInfo.needToConver)
	{
		sampleSize = 16;
		channelSize = 2;
	}
	else
	{
		sampleSize = g_AudioVideoData->m_decodecInfo.sampleSize;
		channelSize = g_AudioVideoData->m_decodecInfo.channelSize;
	}
}

FFMpegOperator* FFMpegOperator::getInstance(void)
{
	static FFMpegOperator instance;
	return &instance;
}
