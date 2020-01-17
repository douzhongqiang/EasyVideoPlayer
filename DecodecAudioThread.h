#ifndef DECODECAUIDOTHREAD_H
#define DECODECAUIDOTHREAD_H

#include <QThread>
#include <atomic>

class DecodecAudioThread : public QThread
{
	Q_OBJECT

public:
	DecodecAudioThread(QObject* parent = nullptr);
	~DecodecAudioThread();

	void run(void);

private:
	uchar* m_pAudioData = nullptr;
	static int m_nMaxAudioSize;
	std::atomic<int> m_nHeadIndex;
	std::atomic<int> m_nTailIndex;
	std::atomic<int> m_nTotalCount;
};
#endif
