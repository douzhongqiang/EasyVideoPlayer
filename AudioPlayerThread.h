#ifndef AUDIOPLAYERTHREAD_H
#define AUDIOPLAYERTHREAD_H

#include <QThread>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <QTime>

class DecodecVideo;
class AudioPlayerThread : public QThread
{
    Q_OBJECT

public:
    AudioPlayerThread(QObject* parent = nullptr);
    ~AudioPlayerThread();

    void run(void) override;

    // set sample infos
    void setSampleInfo(int sampleRate, int sampleSize, int channelCount);
    // set Audio Data
    void setAudioData(char* audioData, int length);

    // get current play time
    qreal getCurrentPlayTime(void);
    // set decodec 
    void setDecodec(DecodecVideo* decodec);

	// set current time
	void setCurrentTimeForce(qreal time);

private:
    QAudioFormat m_format;
    QAudioOutput* m_pAudioOutput = nullptr;
    QByteArray m_audioData;
    QIODevice* m_pIODevice = nullptr;
    DecodecVideo* m_pDecodecVideo = nullptr;

    QMutex m_mutex;
    QMutex m_waitMutex;
    QWaitCondition m_waitCondition;

    bool isAudioEmpty(void);

    std::atomic<int> m_nStartPopIndex;
    std::atomic<int> m_nEndPushIndex;

    std::atomic<int> m_nTotalCanUsed;
    std::atomic<qreal> m_nCurrentPlayIndex;

	// for test
	QTime m_time;
};
#endif
