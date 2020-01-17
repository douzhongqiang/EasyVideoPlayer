#ifndef DECODECVIDEO_H
#define DECODECVIDEO_H

#include <QString>
#include <QObject>
#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <atomic>
#include <QTime>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

class OpenGLRender;
class AudioPlayerThread;
class DecodecVideo : public QThread
{
    Q_OBJECT
public:
    struct DecodecVideInfo
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

    struct FrameData
    {
        qreal currentPts = 0.0;
        uchar* pImageData;
    };

	enum PlayStatus
	{
		Player_Normal,
		Player_Playing,
		Player_Pause,
		Player_Stop
	};

public:
    DecodecVideo(QObject* parent = nullptr);
    ~DecodecVideo();

    // open video file
    bool openVideoFile(const QString& fileName);
    // get frame data
    void getFrameData(uchar*& imageData);
	void getFrameData(AVFrame*& frame);
	void freeFrame(AVFrame* frame);
    // get current infos
    const DecodecVideInfo& getCurrentInfo(void);

    // updateToDisplay
    void updateToDisplay(qreal currentTime);
	void updateToDisplay2(qreal currentTime);

    // get Audio BufferData
    void getAudioBufferData(QByteArray& byte);
    // set Audio Player
    void setAudioPlayer(AudioPlayerThread* audioPlayer);
    // get Audio output Infos
    void getAudioOutputInfos(int& sampleRato, int& sampleSize, int& channelSize);

    void run(void) override;

	// seek Video
	void seekVideo(qreal time);

	// set Current Status
	void setCurrentPlayerStatus(PlayStatus status);
	PlayStatus getCurrentPlayerStatus(void);

	// for test
	void testCall(void);

private:
    AVFormatContext* m_pFormatContext = nullptr;
    AVCodecContext* m_pVideoCodecContext = nullptr;
    AVCodecContext* m_pAudioCodecContext = nullptr;
    AVFrame* m_pFrame = nullptr;
    AVFrame* m_pRGBFrame = nullptr;
	AVFrame* m_pTempFrame = nullptr;
    SwsContext* m_pSwsContext = nullptr;
    SwrContext* m_pSwrContext = nullptr;

    int m_nVideoIndex = -1;
    int m_nAudioIndex = -1;

    // Frame To RGB
    void frameToRgbImage(AVFrame* pDest, AVFrame* frame);
    // Frame To sample size 16bit
    int frameSameSampe(AVFrame* frame, uchar* pDest, int size);
    
    // Image Data Buffer
    FrameData m_pImageData[60];
	AVFrame* m_frameData[60];
    uchar* m_pDestBufferData = nullptr;

    // info
    DecodecVideInfo m_decodecInfo;

private:
    static int m_nFrameBufferSize;

    // for Video
    // sync thread
    QSemaphore m_semaphore;
    std::atomic<int> m_nHeadIndex;
    std::atomic<int> m_nEndIndex;
    std::atomic<int> m_nCurrentSize;
	std::atomic<int> m_nPlayerStatus;

    // open video decodec
    bool openVideoCodec(void);
    // decodec Video
    void decodecVideo(AVPacket* packet);

private:
    // for Audio
    uchar *m_pTempBuffer = nullptr;
    QByteArray m_audioArray;
    QMutex m_audioMutex;

    AudioPlayerThread* m_pAudioThread = nullptr;

    // open audio decodec
    bool openAudioCodec(void);

    // decodec Audio
    void decodecAudio(AVPacket* packet);

	// For Test
	QTime m_time;

signals:
    void updateDisplay(qreal time);
};
#endif
