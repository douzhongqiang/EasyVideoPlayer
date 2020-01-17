#ifndef DECODECVIDEOTHREAD_H
#define DECODECVIDEOTHREAD_H

#include <QThread>

class DecodecVideoThread : public QThread
{
	Q_OBJECT

public:
	DecodecVideoThread(QObject* parent);
	~DecodecVideoThread();
};
#endif
