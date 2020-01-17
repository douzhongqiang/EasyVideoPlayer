#include "AudioVideoBufferData.h"

int AudioVideoBufferData::m_nMaxQueueSize = 30;
AudioVideoBufferData::AudioVideoBufferData(QObject* parent)
{
	for (int i = 0; i < m_nMaxQueueSize; ++i)
	{

	}
}

AudioVideoBufferData::~AudioVideoBufferData()
{

}
