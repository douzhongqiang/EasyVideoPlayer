#ifndef DECODECAUIDOTHREAD_H
#define DECODECAUIDOTHREAD_H

#include <QThread>
#include <atomic>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class DecodecAudioThread : public QThread
{
	Q_OBJECT

public:
	DecodecAudioThread(QObject* parent = nullptr);
	~DecodecAudioThread();

	void setAudioCodecContext(AVCodecContext* codecContext);

	void run(void);

private:
	uchar* m_pAudioData = nullptr;
	static int m_nMaxAudioSize;
	std::atomic<int> m_nHeadIndex;
	std::atomic<int> m_nTailIndex;
	std::atomic<int> m_nTotalCount;

	AVCodecContext* m_pAudioCodecContext = nullptr;
};
#endif
