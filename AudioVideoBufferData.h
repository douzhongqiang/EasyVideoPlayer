#ifndef AUDIOVIDEOBUFFERDATA_H
#define AUDIOVIDEOBUFFERDATA_H

#include <QObject>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
class AudioVideoBufferData : public QObject
{
	Q_OBJECT

public:
	AudioVideoBufferData(QObject* parent = nullptr);
	~AudioVideoBufferData();

	static int m_nMaxQueueSize;
private:
	AVPacket* m_audioPacketQueue[];
};
#endif
