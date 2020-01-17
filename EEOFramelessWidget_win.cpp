#include "EEOFramelessWidget.h"

#ifdef Q_OS_WIN
#include <QStyleOption>
#include <QPainter>
#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
#include <gdiplus.h>
#include <GdiPlusColor.h>
#include "CoreEngine/GlobalStyle.h"
#include <qdrawutil.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

const int EEOFramelessWidget::m_nBorderWidth = 10;
EEOFramelessWidget::EEOFramelessWidget(QWidget* parent)
	:QWidget(parent)
{
	setWindowFlags(this->windowFlags() | Qt::Window | Qt::FramelessWindowHint/* | Qt::NoDropShadowWindowHint*/);
	setAttribute(Qt::WA_TranslucentBackground, true);
	g_GlobalStyleInstance->applyCurrentStyle(this, "QWidget_BorderImage_WhiteBg");

	setResizeable(true);

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(m_nBorderWidth);
	m_mainLayout->setSpacing(0);
}

EEOFramelessWidget::~EEOFramelessWidget()
{

}

void EEOFramelessWidget::setResizeable(bool resizeable)
{
	/*Q_UNUSED(resizeable)
	return;*/

	bool visible = isVisible();
	m_bResizeable = resizeable;
	if (m_bResizeable) {
		setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
		//        setWindowFlag(Qt::WindowMaximizeButtonHint);

		//this line will get titlebar/thick frame/Aero back, which is exactly what we want
		//we will get rid of titlebar and thick frame again in nativeEvent() later
		HWND hwnd = (HWND)this->winId();
		DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
		::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
	}
	else {
		setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
		//        setWindowFlag(Qt::WindowMaximizeButtonHint,false);

		HWND hwnd = (HWND)this->winId();
		DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
		::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
	}

	//we better left 1 piexl width of border untouch, so OS can draw nice shadow around it
	/*const MARGINS shadow = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);*/

	setVisible(visible);
}

void EEOFramelessWidget::setContentsMargins(const QMargins &margins)
{
	QWidget::setContentsMargins(margins + m_frames);
	m_margins = margins;
}

void EEOFramelessWidget::setContentsMargins(int left, int top, int right, int bottom)
{
	QWidget::setContentsMargins(left + m_frames.left(), \
		top + m_frames.top(), \
		right + m_frames.right(), \
		bottom + m_frames.bottom());
	m_margins.setLeft(left);
	m_margins.setTop(top);
	m_margins.setRight(right);
	m_margins.setBottom(bottom);
}

QMargins EEOFramelessWidget::contentsMargins() const
{
	QMargins margins = QWidget::contentsMargins();
	margins -= m_frames;
	return margins;
}

QRect EEOFramelessWidget::contentsRect() const
{
	QRect rect = QWidget::contentsRect();
	int width = rect.width();
	int height = rect.height();
	rect.setLeft(rect.left() - m_frames.left());
	rect.setTop(rect.top() - m_frames.top());
	rect.setWidth(width);
	rect.setHeight(height);
	return rect;
}

void EEOFramelessWidget::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
	QWidget::getContentsMargins(left, top, right, bottom);
	if (!(left&&top&&right&&bottom)) return;
	if (isMaximized())
	{
		*left -= m_frames.left();
		*top -= m_frames.top();
		*right -= m_frames.right();
		*bottom -= m_frames.bottom();
	}
}

void EEOFramelessWidget::addDragWidget(QWidget* widget)
{
	m_dragWidgets.append(widget);
}

void EEOFramelessWidget::addDragHeight(int height)
{
	m_dragHeights.append(height);
}

bool EEOFramelessWidget::isDragAtWidget(int xPt, int yPt)
{
	if (m_canDragFunc)
		return m_canDragFunc(xPt, yPt);

	QWidget* pWidget = this->childAt(xPt, yPt);
	if (pWidget == nullptr)
		return false;

	for (auto iter = m_dragWidgets.begin(); iter != m_dragWidgets.end(); ++iter)
	{
		if (pWidget == *iter)
			return true;
	}

	for (auto iter = m_dragRects.begin(); iter != m_dragRects.end(); ++iter)
	{
		if (iter->contains(xPt, yPt))
			return true;
	}

	for (auto iter = m_dragHeights.begin(); iter != m_dragHeights.end(); ++iter)
	{
		QRect rect(m_nBorderWidth, m_nBorderWidth, this->width() - m_nBorderWidth * 2, *iter);
		if (rect.contains(xPt, yPt))
			return true;
	}

	return false;
}

void EEOFramelessWidget::addDragRect(QRect rect)
{
	m_dragRects << rect;
}

void EEOFramelessWidget::setDragFunction(std::function<bool(int xpt, int yPt)> func)
{
	m_canDragFunc = func;
}

void EEOFramelessWidget::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	/*p.fillRect(this->rect().adjusted(m_nBorderWidth, m_nBorderWidth, -m_nBorderWidth, -m_nBorderWidth),\
	QBrush(QColor(255, 255, 255)));

	qDrawBorderPixmap(&p, rect(), \
	QMargins(m_nBorderWidth, m_nBorderWidth, m_nBorderWidth, m_nBorderWidth), \
	QPixmap(":/images/other_bg.png"));*/

	return QWidget::paintEvent(event);
}

bool EEOFramelessWidget::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
	MSG *param = static_cast<MSG *>(message);

	switch (param->message)
	{
	case WM_NCCALCSIZE: {
		/*if (this->parentWidget() == nullptr)
		{
			NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(param->lParam);
			if (params.rgrc[0].top != 0)
			params.rgrc[0].top -= 1;
		}*/

		//this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
		*result = WVR_REDRAW;
		return true;
	}
	case WM_NCHITTEST:
	{
		int nX = GET_X_LPARAM(param->lParam) - this->geometry().x();
		int nY = GET_Y_LPARAM(param->lParam) - this->geometry().y();

		// dispose resize postion
		QRect nRect = this->rect().adjusted(m_nBorderWidth, m_nBorderWidth, -m_nBorderWidth, -m_nBorderWidth);
		bool needDispose = this->rect().contains(nX, nY) && !nRect.contains(nX, nY);
		if (!needDispose)
		{
			// When Mouse on controls, don't dispose
			if (isDragAtWidget(nX, nY))
			{
				*result = HTCAPTION;
				return true;
			}

			return QWidget::nativeEvent(eventType, message, result);
		}

		*result = HTCAPTION;

		// dispose window frame
		if ((nX > 0) && (nX < m_nBorderWidth))
			*result = HTLEFT;

		if ((nX > this->width() - m_nBorderWidth) && (nX < this->width()))
			*result = HTRIGHT;

		if ((nY > 0) && (nY < m_nBorderWidth))
			*result = HTTOP;

		if ((nY > this->height() - m_nBorderWidth) && (nY < this->height()))
			*result = HTBOTTOM;

		if ((nX > 0) && (nX < m_nBorderWidth) && (nY > 0)
			&& (nY < m_nBorderWidth))
			*result = HTTOPLEFT;

		if ((nX > this->width() - m_nBorderWidth) && (nX < this->width())
			&& (nY > 0) && (nY < m_nBorderWidth))
			*result = HTTOPRIGHT;

		if ((nX > 0) && (nX < m_nBorderWidth)
			&& (nY > this->height() - m_nBorderWidth) && (nY < this->height()))
			*result = HTBOTTOMLEFT;

		if ((nX > this->width() - m_nBorderWidth) && (nX < this->width())
			&& (nY > this->height() - m_nBorderWidth) && (nY < this->height()))
			*result = HTBOTTOMRIGHT;

		return true;
	}
	case WM_GETMINMAXINFO:
	{
		if (::IsZoomed(param->hwnd))
		{
			RECT frame = { 0, 0, 0, 0 };
			AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

			//record frame area data
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
			double dpr = this->devicePixelRatioF();
#else
			double dpr = 1.0;
#endif

			m_frames.setLeft(abs(frame.left) / dpr + 0.5);
			m_frames.setTop(abs(frame.bottom) / dpr + 0.5);
			m_frames.setRight(abs(frame.right) / dpr + 0.5);
			m_frames.setBottom(abs(frame.bottom) / dpr + 0.5);

			QWidget::setContentsMargins(m_frames.left() + m_margins.left(), \
				m_frames.top() + m_margins.top(), \
				m_frames.right() + m_margins.right(), \
				m_frames.bottom() + m_margins.bottom());
			m_bJustMaximized = true;
			m_mainLayout->setMargin(0);
			g_GlobalStyleInstance->applyCurrentStyle(this, "QWidget_NormalWhiteBg", true);
		}
		else
		{
			if (m_bJustMaximized)
			{
				QWidget::setContentsMargins(m_margins);
				m_frames = QMargins();
				m_bJustMaximized = false;
				m_mainLayout->setMargin(m_nBorderWidth);
				g_GlobalStyleInstance->applyCurrentStyle(this, "QWidget_BorderImage_WhiteBg", true);
			}
		}
		return false;
	}
	}

	return QWidget::nativeEvent(eventType, message, result);
}

#endif
