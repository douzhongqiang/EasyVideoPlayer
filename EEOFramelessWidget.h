#ifndef EEOFRAMELESSWIDGET_H
#define EEOFRAMELESSWIDGET_H

#include <QWidget>
#include <QMargins>
#include <QVBoxLayout>
#include <functional>
#include "eeowidgets_global.h"

class Movable;
class EEOWIDGETSSHARED_EXPORT EEOFramelessWidget : public QWidget
{
    Q_OBJECT

public:
    EEOFramelessWidget(QWidget* parent = nullptr);
    ~EEOFramelessWidget();

public:
    void setResizeable(bool resizeable = true);
    bool isResizeable(){ return m_bResizeable; }
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;

    void addDragWidget(QWidget* widget);
    void addDragRect(QRect rect);
	void addDragHeight(int height);

	void setDragFunction(std::function<bool(int xpt, int yPt)> func);

#ifdef Q_OS_MAC
    // show Minumuzed can used in MacOS, minimize window by oc code
    void showCustomMinimized();
#endif

private:
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized = false;
    bool m_bResizeable = true;

    QList<QWidget*> m_dragWidgets;
    QList<QRect> m_dragRects;
	QList<int> m_dragHeights;

	std::function<bool(int xpt, int yPt)> m_canDragFunc;

protected:
    static const int m_nBorderWidth;
    QVBoxLayout* m_mainLayout = nullptr;
    void paintEvent(QPaintEvent* event) final;
	virtual bool isDragAtWidget(int xPt, int yPt);

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray & eventType, void * message, long * result) final;
#else
    Movable* m_pMovable = nullptr;
#endif
};

#endif
