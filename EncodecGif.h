#ifndef ENCODECGIF_H
#define ENCODECGIF_H

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#include <QString>
class EncodecGif
{
public:
	EncodecGif();
	~EncodecGif();

	bool start(int width, int height, int fps, const QString& filename);
	void end(void);
	// 输入 RGB888数据
	bool writeImageData(const unsigned char* pSrc);

private:
	AVFormatContext* m_pFormatContext = nullptr;
	AVCodecContext* m_pCodecContext = nullptr;
	AVFrame* m_frame = nullptr;
	AVPacket* m_packet = nullptr;

	int m_nWidth = 0;
	int m_nHeight = 0;
	int m_nFrameRate = 0;
	int m_nCurrentFrame = 0;

	// 编码
	bool encodeFunc(AVCodecContext* context, AVFrame* frame, AVPacket* packet);
	// 创建封装上下文
	bool createFormatContext(const QString& filename);
};
#endif
