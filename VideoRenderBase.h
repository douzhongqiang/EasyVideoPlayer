#ifndef VIDEORENDERBASE_H
#define VIDEORENDERBASE_H

#define USEDOPENFLRENDER 1
class VideoRenderBase
{
public:
	VideoRenderBase();
	virtual ~VideoRenderBase();

	// setRGBData
	virtual void setRGBData(const unsigned char* pImageData, int width, int height) = 0;
	// setYUVData
	virtual void setYUVData(unsigned char* const pImageData[], int width, int height) = 0;
	// rebind VBO
	virtual void rebindVBO(int width, int height) = 0;
};

#endif
