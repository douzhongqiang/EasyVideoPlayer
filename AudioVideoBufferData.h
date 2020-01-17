#ifndef AUDIOVIDEOBUFFERDATA_H
#define AUDIOVIDEOBUFFERDATA_H

#include <QObject>
#include <atomic>
#include <QMutex>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

#define MAX_VIDEO_PACKET_BUFFERSIZE 30
#define MAX_AUDIO_PACKET_BUFFERSIZE 30
#define MAX_VIDEO_FRAME_BUFFERSIZE 10
#define MAX_AUDIO_FRAME_BUFFERSIZE 10
#define g_AudioVideoData AudioVideoBufferData::getInstance()
#define g_AudioVideoOper FFMpegOperator::getInstance()

class FFMpegOperator;
class AudioVideoBufferData : public QObject
{
public:
	struct DecodecVideoInfo
	{
		// Video Info
		int width = 0;
		int height = 0;
		bool isYUV420P = false;

		// Audio Info
		int sampleRate = 44100;
		int sampleSize = 16;
		int channelSize = 2;
		bool needToConver = true;

		// global infos
		qreal totalTime = 0;
	};

public:
	AudioVideoBufferData(QObject* parent = nullptr);
	~AudioVideoBufferData();

	// 添加一个视频视频数据到队列
	bool addVideoPacketToQueue(AVPacket* packet);
	// 获取一个视频数据包
	AVPacket* takeVideoPacketFromQueue(void);

	// 添加一个音频数据到队列
	bool addAudioPacketToQueue(AVPacket* packet);
	// 获取一个音频数据包
	AVPacket* takeAudioPacketFromQueue(void);

	// 设置同步时钟
	void setSyncTimeClock(qreal time);
	// 获取同步时钟
	qreal getSyncTimeClock(void);

	static AudioVideoBufferData* getInstance(void);
	friend class FFMpegOperator;

private:
	DecodecVideoInfo m_decodecInfo;

	// 视频数据队列
	AVPacket* m_videoPacketQueue[MAX_VIDEO_PACKET_BUFFERSIZE];
	std::atomic<int> m_nVideoStartIndex;
	std::atomic<int> m_nVideoEndIndex;
	std::atomic<int> m_nVideoTotalCount;

	// 音频数据队列
	AVPacket* m_audioPacketQueue[MAX_AUDIO_PACKET_BUFFERSIZE];
	std::atomic<int> m_nAudioStartIndex;
	std::atomic<int> m_nAudioEndIndex;
	std::atomic<int> m_nAudioTotalCount;

	// 视频解码后数据缓存
	AVFrame* m_videoFrameData[MAX_VIDEO_FRAME_BUFFERSIZE];
	std::atomic<int> m_nVideoFrameStartIndex;
	std::atomic<int> m_nVideoFrameEndIndex;
	std::atomic<int> m_nVideoFrameTotalCount;

	// 音频解码后数据缓存
	AVFrame* m_audioFrameData[MAX_AUDIO_FRAME_BUFFERSIZE];
	std::atomic<int> m_nAudioFrameStartIndex;
	std::atomic<int> m_nAudioFrameEndIndex;
	std::atomic<int> m_nAudioFrameTotalCount;
	QByteArray m_audioData;
	QMutex m_audioMutex;

	// 同步时钟
	std::atomic<qreal> m_globalTimeClock;
};

class FFMpegOperator
{
public:
	FFMpegOperator();
	virtual ~FFMpegOperator();

	// open video file
	bool openVideoFile(const QString& fileName);
	// close video file
	void closeVideoFile(void);
	
	// 解封装
	bool deMuxing(AVPacket*& packet, bool& isEnd);
	// 解码视频
	void decodecVideo(AVPacket* packet);
	// 解码音频
	void decodecAudio(AVPacket* packet);

	static FFMpegOperator* getInstance(void);

private:
	AVFormatContext* m_pFormatContext = nullptr;
	AVCodecContext* m_pVideoCodecContext = nullptr;
	AVCodecContext* m_pAudioCodecContext = nullptr;

	int m_nVideoIndex = -1;
	int m_nAudioIndex = -1;
	uchar *m_pTempBuffer = nullptr;

	// 打开视频解码器
	bool openVideoCodec(void);
	// 打开音频解码器
	bool openAudioCodec(void);
	// 音频重采样
	int frameSameSampe(AVFrame* frame, uchar* pDest, int size);
	// get Audio output Infos
	void getAudioOutputInfos(int& sampleRato, int& sampleSize, int& channelSize);

	AVFrame* m_pVideoFrame = nullptr;
	AVFrame* m_pAudioFrame = nullptr;

	SwsContext* m_pSwsContext = nullptr;
	SwrContext* m_pSwrContext = nullptr;
};
#endif
