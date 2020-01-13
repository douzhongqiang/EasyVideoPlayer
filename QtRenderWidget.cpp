#include "QtRenderWidget.h"
#include <QTime>
#include <QPainter>
#include <QDebug>

QtRenderWidget::QtRenderWidget(QWidget* parent)
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QtRenderWidget::~QtRenderWidget()
{

}

// setRGBData
void QtRenderWidget::setRGBData(const uchar* pImageData, int width, int height)
{
	QImage image(pImageData, width, height, QImage::Format_RGB888);
	m_pixmap = QPixmap::fromImage(image);
	this->update();
}

// setYUVData
void QtRenderWidget::setYUVData(uchar* const pImageData[], int width, int height)
{
	
}

void QtRenderWidget::rebindVBO(int width, int height)
{
	
}

void QtRenderWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect rect = this->rect();

	painter.fillRect(rect, QBrush(QColor(0, 0, 0)));

	if (m_pixmap.isNull())
		return QWidget::paintEvent(event);

	float rx = this->width() * 1.0 / m_pixmap.width();
	float ry = this->height() * 1.0 / m_pixmap.height();
	float minr = qMin(rx, ry);
	int width = m_pixmap.width() * minr;
	int height = m_pixmap.height() * minr;

	QRect pixRect((rect.width() - width) / 2, (rect.height() - height) / 2, width, height);
	
	painter.drawPixmap(pixRect, m_pixmap);
}

//void QtRenderWidget::resizeEvent(QResizeEvent* event)
//{
//	qreal widthRato = this->rect().width()
//}
