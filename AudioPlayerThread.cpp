#include "AudioPlayerThread.h"
#include "DecodecVideo.h"
#include <QMutexLocker>
#include <QDebug>

AudioPlayerThread::AudioPlayerThread(QObject* parent)
    :QThread(parent)
    , m_nStartPopIndex(0)
    , m_nEndPushIndex(0)
    , m_nTotalCanUsed(0)
    , m_nCurrentPlayIndex(0)
{
    m_audioData.clear();
    m_waitMutex.lock();
}

AudioPlayerThread::~AudioPlayerThread()
{
    this->requestInterruption();
    if (this->isRunning())
        this->wait();
}

void AudioPlayerThread::setSampleInfo(int sampleRate, int sampleSize, int channelCount)
{
    m_format.setSampleRate(sampleRate);
    m_format.setSampleSize(sampleSize);
    m_format.setChannelCount(channelCount);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    if (m_pAudioOutput)
        delete m_pAudioOutput;
    m_pAudioOutput = new QAudioOutput(m_format, this);
    m_pIODevice = m_pAudioOutput->start();
}

void AudioPlayerThread::setAudioData(char* audioData, int length)
{
    QMutexLocker locker(&m_mutex);

    m_audioData.append(audioData, length);
	m_nTotalCanUsed += length;
    //m_waitCondition.wakeAll();
}

// get current play time
qreal AudioPlayerThread::getCurrentPlayTime(void)
{
    //qDebug() << "Current Played Audio Size is " << m_nCurrentPlayIndex;
    qreal time = m_nCurrentPlayIndex / m_format.sampleRate() / m_format.channelCount() / (m_format.sampleSize() / 8);
    return time;
}

void AudioPlayerThread::setDecodec(DecodecVideo* decodec)
{
    m_pDecodecVideo = decodec;
}

void AudioPlayerThread::setCurrentTimeForce(qreal time)
{
	QMutexLocker locker(&m_mutex);
	m_audioData.clear();
	m_nTotalCanUsed = 0;

	m_nCurrentPlayIndex = time * m_format.sampleRate() * m_format.channelCount() * (m_format.sampleSize() / 8);
}

bool AudioPlayerThread::isAudioEmpty(void)
{
	if (m_nTotalCanUsed <= 0)
		return true;

    return false;
}

void AudioPlayerThread::run(void)
{
    while (!this->isInterruptionRequested())
    {
        if (isAudioEmpty())
        {
			//qDebug() << "Audio Is Empty!";
            QThread::msleep(10);
            continue;
        }

        if (m_pAudioOutput->bytesFree() >= m_audioData.size())
        {
            // fill all audio data
            QMutexLocker locker(&m_mutex);

            m_pIODevice->write(m_audioData);

            // update index
            m_nCurrentPlayIndex = m_nCurrentPlayIndex + m_audioData.size() * 1.0;
            if (m_pDecodecVideo)
            {
                // update To Display
                qreal time = getCurrentPlayTime();
                qDebug() << "Current Play Audio Time is " << time << "; Current Buffer Size: 0";
                m_pDecodecVideo->updateToDisplay(time);
            }

            m_audioData.clear();
			m_nTotalCanUsed = 0;
			//qDebug() << "Audio ByteFree >= Audio Size!";
            QThread::msleep(10);
            continue;
        }

        if (m_pAudioOutput->bytesFree() >= m_pAudioOutput->periodSize())
        {
            // fill period audio data
            QMutexLocker locker(&m_mutex);

            int size = m_pAudioOutput->bytesFree();
            m_pIODevice->write(m_audioData.data(), size);
            
            // update index
            m_nCurrentPlayIndex = m_nCurrentPlayIndex + size * 1.0;
            if (m_pDecodecVideo)
            {
                // update To Display
                qreal time = getCurrentPlayTime();
                //qDebug() << "Current Play Audio Time is " << time << "; Current Buffer Size: " << m_audioData.size() - size;
                m_pDecodecVideo->updateToDisplay(time);
            }

            m_audioData.remove(0, size);
			m_nTotalCanUsed -= size;

			//qDebug() << "Audio ByteFree >= Audio periodSize!";
            QThread::msleep(10);
            continue;
        }

		//qDebug() << "Audio Playing";
        QThread::msleep(15);
    }
}
