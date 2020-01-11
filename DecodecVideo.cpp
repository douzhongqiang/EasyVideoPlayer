#include "DecodecVideo.h"
#include "OpenGLRender.h"
#include <QMutexLocker>
#include "AudioPlayerThread.h"

int DecodecVideo::m_nFrameBufferSize = 60;

DecodecVideo::DecodecVideo(QObject* parent)
    :QThread(parent)
    , m_semaphore(m_nFrameBufferSize)
    , m_nHeadIndex(0)
    , m_nEndIndex(0)
    , m_nCurrentSize(0)
{
    avcodec_register_all();
    m_pFrame = av_frame_alloc();
}

DecodecVideo::~DecodecVideo()
{
    avformat_close_input(&m_pFormatContext);
    av_frame_free(&m_pFrame);
    av_frame_free(&m_pRGBFrame);
    sws_freeContext(m_pSwsContext);

    // wait for thread
    this->requestInterruption();
    if (this->isRunning())
        this->wait();

    delete[] m_pDestBufferData;
    m_pDestBufferData = nullptr;
}

bool DecodecVideo::openVideoFile(const QString& fileName)
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

    // create Buffer Data
    for (int i = 0; i < m_nFrameBufferSize; ++i)
    {
        int pixSize = 0;
        if (m_decodecInfo.isYUV420P)
            pixSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pVideoCodecContext->width, m_pVideoCodecContext->height);
        else
            pixSize = avpicture_get_size(AV_PIX_FMT_RGB24, m_pVideoCodecContext->width, m_pVideoCodecContext->height);
        m_pImageData[i].pImageData = new uchar[pixSize];
        memset(m_pImageData[i].pImageData, 0, pixSize);
    }
    return true;
}

void DecodecVideo::getFrameData(uchar*& imageData)
{
    if (m_nCurrentSize <= 0)
    {
        imageData = nullptr;
        return;
    }

    // create temp buffer data
    int pixSize = avpicture_get_size(AV_PIX_FMT_RGB24, m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    if (m_pDestBufferData == nullptr)
        m_pDestBufferData = new uchar[pixSize];

    if (m_decodecInfo.isYUV420P)
        pixSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pVideoCodecContext->width, m_pVideoCodecContext->height);

    // copy data
    memcpy(m_pDestBufferData, m_pImageData[m_nHeadIndex++].pImageData, pixSize);
    imageData = m_pDestBufferData;
    
    if (m_nHeadIndex == m_nFrameBufferSize)
        m_nHeadIndex = 0;
    m_nCurrentSize--;
    m_semaphore.release();
}

const DecodecVideo::DecodecVideInfo& DecodecVideo::getCurrentInfo(void)
{
    return m_decodecInfo;
}

void DecodecVideo::getAudioBufferData(QByteArray& byte)
{
    QMutexLocker locker(&m_audioMutex);

    byte = m_audioArray;
    m_audioArray.clear();
}

// set Audio Player
void DecodecVideo::setAudioPlayer(AudioPlayerThread* audioPlayer)
{
    m_pAudioThread = audioPlayer;
}

void DecodecVideo::getAudioOutputInfos(int& sampleRato, int& sampleSize, int& channelSize)
{
    sampleRato = m_decodecInfo.sampleRate;
    sampleSize = 16;
    channelSize = 2;
}

void DecodecVideo::run(void)
{
    while (!this->isInterruptionRequested())
    {
        AVPacket pkt;
        int result = av_read_frame(m_pFormatContext, &pkt);
        if (result)
            continue;
        
        // decode video and audio
        if (pkt.stream_index == m_nVideoIndex)
            decodecVideo(&pkt);
        else if (pkt.stream_index == m_nAudioIndex)
        {
            decodecAudio(&pkt);

            // add audio data
            QByteArray audioArray;
            this->getAudioBufferData(audioArray);
            if (m_pAudioThread && audioArray.size() > 0)
                m_pAudioThread->setAudioData(audioArray.data(), audioArray.size());
            qDebug() << "Audio Data Size is " << audioArray.size();
        }

        av_packet_unref(&pkt);
    }
}

// open video decodec
bool DecodecVideo::openVideoCodec(void)
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
    m_decodecInfo.width = m_pVideoCodecContext->width;
    m_decodecInfo.height = m_pVideoCodecContext->height;
    if (m_pVideoCodecContext->pix_fmt == AV_PIX_FMT_YUV420P)
        m_decodecInfo.isYUV420P = true;
    else
        m_decodecInfo.isYUV420P = false;

    return true;
}

// open audio decodec
bool DecodecVideo::openAudioCodec(void)
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
    m_decodecInfo.sampleRate = m_pAudioCodecContext->sample_rate;
    m_decodecInfo.channelSize = m_pAudioCodecContext->channels;
    m_pAudioCodecContext->channel_layout = av_get_default_channel_layout(m_pAudioCodecContext->channels);

    if (m_pTempBuffer == nullptr)
    {
        m_pTempBuffer = new uchar[10000];
        memset(m_pTempBuffer, 0, 10000);
    }
    return true;
}

// decodec Video
void DecodecVideo::decodecVideo(AVPacket* packet)
{ 
    // send packet
    if (avcodec_send_packet(m_pVideoCodecContext, packet))
        return;

    // recv frame
    if (avcodec_receive_frame(m_pVideoCodecContext, m_pFrame))
        return;

    // acquire 1 
    m_semaphore.acquire();

    // fill rgb data
    if (!m_decodecInfo.isYUV420P)
    {
        // create RGB Frame
        if (m_pRGBFrame == nullptr)
            m_pRGBFrame = av_frame_alloc();

        avpicture_fill((AVPicture *)m_pRGBFrame, m_pImageData[m_nEndIndex].pImageData, AV_PIX_FMT_RGB24, m_pFrame->width, m_pFrame->height);
        frameToRgbImage(m_pRGBFrame, m_pFrame);
    }
    else
    {
        int len = 0;
        memcpy(m_pImageData[m_nEndIndex].pImageData + len, m_pFrame->data[0], m_pFrame->width * m_pFrame->height);

        len += m_pFrame->width * m_pFrame->height;
        memcpy(m_pImageData[m_nEndIndex].pImageData + len, m_pFrame->data[1], m_pFrame->width / 2 * m_pFrame->height / 2);

        len += m_pFrame->width / 2 * m_pFrame->height / 2;
        memcpy(m_pImageData[m_nEndIndex].pImageData + len, m_pFrame->data[2], m_pFrame->width / 2 * m_pFrame->height / 2);
    }

    // get pts
    int64_t pts = 0;
    if (packet->dts == AV_NOPTS_VALUE && packet->pts && packet->pts != AV_NOPTS_VALUE)
        pts = packet->pts;
    else if (packet->dts != AV_NOPTS_VALUE)
        pts = packet->dts;
    // calc video time
    qreal time = pts * av_q2d(m_pFormatContext->streams[m_nVideoIndex]->time_base);
    m_pImageData[m_nEndIndex].currentPts = time;
    qDebug() << "Decodec Video is " << time << "; pts is " << m_pFrame->pts << "; calc time is " << \
        m_pFrame->pts * av_q2d(m_pFormatContext->streams[m_nVideoIndex]->time_base);

    // update values
    if (++m_nEndIndex == m_nFrameBufferSize)
        m_nEndIndex = 0;
    ++m_nCurrentSize;

    //emit updateDisplay();
}

// decodec Audio
void DecodecVideo::decodecAudio(AVPacket* packet)
{
    qDebug() << "Decodec Audio";
    // send packet
    if (avcodec_send_packet(m_pAudioCodecContext, packet))
        return;

    // recv frame
    if (avcodec_receive_frame(m_pAudioCodecContext, m_pFrame))
        return;

    QMutexLocker locker(&m_audioMutex);
    // add to buffer
    int len = frameSameSampe(m_pFrame, m_pTempBuffer, 10000);
    if (len > 0)
        m_audioArray.append((char*)m_pTempBuffer, len);

    qDebug() << "Audio time is " << m_pFrame->pts * av_q2d(m_pFormatContext->streams[m_nAudioIndex]->time_base);
}

void DecodecVideo::frameToRgbImage(AVFrame* pDest, AVFrame* frame)
{
    if (m_pSwsContext == nullptr)
    {
        m_pSwsContext = sws_getContext(frame->width, frame->height, (AVPixelFormat)(frame->format), \
            frame->width, frame->height, AV_PIX_FMT_RGB24, \
            SWS_BILINEAR, nullptr, nullptr, nullptr);
    }

    //avpicture_fill( )
    sws_scale(m_pSwsContext, frame->data, frame->linesize, 0, frame->height, \
        pDest->data, pDest->linesize);
}

int DecodecVideo::frameSameSampe(AVFrame* frame, uchar* pDest, int size)
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

void DecodecVideo::updateToDisplay(qreal currentTime)
{
    //return;
    if (m_nCurrentSize <= 0 || currentTime < m_pImageData[m_nHeadIndex].currentPts)
        return;   

    while (1)
    {
        int index = m_nHeadIndex + 1;
        if (index == m_nFrameBufferSize)
            index = 0;

        if (index == m_nEndIndex)
            break;

        qreal tempPts = m_pImageData[index].currentPts;
        if (currentTime > tempPts && m_nCurrentSize > 0)
        {
            m_nHeadIndex = index;
            m_nCurrentSize--;
            m_semaphore.release();
        }
        else
            break;
    }

    qDebug() << "Current Play Video is " << m_pImageData[m_nHeadIndex].currentPts;
    emit updateDisplay();
}
