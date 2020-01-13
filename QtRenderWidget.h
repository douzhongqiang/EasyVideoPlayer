#ifndef QTRENDERWIDGET_H
#define QTRENDERWIDGET_H

#include <QWidget>
#include "VideoRenderBase.h"
class QtRenderWidget : public QWidget, public VideoRenderBase
{
	Q_OBJECT

public:
	QtRenderWidget(QWidget* parent = nullptr);
	~QtRenderWidget();

	// setRGBData
	void setRGBData(const uchar* pImageData, int width, int height) override;
	// setYUVData
	void setYUVData(uchar* const pImageData[], int width, int height) override;
	// rebind VBO
	void rebindVBO(int width, int height) override;

private:
	QPixmap m_pixmap;
	qreal m_rato;

protected:
	void paintEvent(QPaintEvent* event) override;
	//void resizeEvent(QResizeEvent* event) override;
};
#endif
