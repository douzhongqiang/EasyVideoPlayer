#ifndef VIDEOPLAYERWIDDGET_H
#define VIDEOPLAYERWIDDGET_H

#include <QWidget>
#include "OpenGLRender.h"
#include "DecodecVideo.h"
#include "VideoPlayOper.h"

class VideoPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    VideoPlayerWidget();
    ~VideoPlayerWidget();

private:
    OpenGLRender* m_pOpenglRender = nullptr;
    DecodecVideo* m_pDecodecVideoThread = nullptr;
    VideoPlayOper* m_pVideoPlayOper = nullptr;

private slots:
    void onClickedLoadButton(void);
};
#endif
