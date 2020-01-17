#include "DecodecAudioThread.h"

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

void DecodecAudioThread::run(void)
{
	while (!this->isInterruptionRequested())
	{

		QThread::msleep(10);
	}
}
