#include "DecodecAudioThread.h"
#include "AudioVideoBufferData.h"

int DecodecAudioThread::m_nMaxAudioSize = 1024 * 4 * 20;
DecodecAudioThread::DecodecAudioThread(QObject* parent)
	:QThread(parent)
	, m_nHeadIndex(0)
	, m_nTailIndex(0)
	, m_nTotalCount(0)
{
	m_pAudioData = new uchar[m_nMaxAudioSize];
}

DecodecAudioThread::~DecodecAudioThread()
{
	delete[] m_pAudioData;
}

void DecodecAudioThread::setAudioCodecContext(AVCodecContext* codecContext)
{
	m_pAudioCodecContext = codecContext;
}

void DecodecAudioThread::run(void)
{
	while (!this->isInterruptionRequested())
	{
		AVPacket* packet = g_AudioVideoData->takeAudioPacketFromQueue();
		// 数据包为空
		if (packet == nullptr)
		{
			QThread::msleep(10);
			continue;
		}
	}
}
