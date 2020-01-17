#include "DecodecVideoThread.h"

DecodecVideoThread::DecodecVideoThread(QObject* parent)
	:QThread(parent)
{

}

DecodecVideoThread::~DecodecVideoThread()
{

}

void DecodecVideoThread::run(void)
{
	while (this->isInterruptionRequested())
	{
		
		QThread::msleep(10);
	}
}
