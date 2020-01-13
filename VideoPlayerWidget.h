#ifndef VIDEOPLAYERWIDDGET_H
#define VIDEOPLAYERWIDDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include "OpenGLRender.h"
#include "DecodecVideo.h"
#include "VideoPlayOper.h"
#include "QtRenderWidget.h"
#include "VideoRenderBase.h"

class VideoPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    VideoPlayerWidget();
    ~VideoPlayerWidget();

private:
	VideoRenderBase* m_pRender = nullptr;
    DecodecVideo* m_pDecodecVideoThread = nullptr;
    VideoPlayOper* m_pVideoPlayOper = nullptr;

	QSlider* m_pSlider = nullptr;
	QLabel* m_pTimeTag = nullptr;
	QPushButton* m_pPlayButton = nullptr;

	void setCurrentDisplayTime(qreal time);

private slots:
    void onClickedLoadButton(void);
	void onSliderValueChanged(int value);
	void onPlayButtonClicked(void);

	void onUpdateDisplayInfos(qreal time);

protected:
	void closeEvent(QCloseEvent * event) override;
};
#endif
