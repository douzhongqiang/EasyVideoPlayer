#include "VideoPlayerWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>

VideoPlayerWidget::VideoPlayerWidget()
{
    m_pOpenglRender = new OpenGLRender;
    m_pOpenglRender->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_pOpenglRender);

    QWidget* pW = new QWidget;
    mainLayout->addWidget(pW);
    QPushButton* loadVideoButton = new QPushButton("Load");
    QObject::connect(loadVideoButton, &QPushButton::clicked, \
        this, &VideoPlayerWidget::onClickedLoadButton);
    QHBoxLayout* buttonLayout = new QHBoxLayout(pW);
    buttonLayout->addWidget(loadVideoButton);
    buttonLayout->addStretch();

    m_pDecodecVideoThread = new DecodecVideo(this);
    
    // create player
    m_pVideoPlayOper = new VideoPlayOper(this);
    m_pVideoPlayOper->setVideoRender(m_pOpenglRender);
    m_pVideoPlayOper->setVideoDecodec(m_pDecodecVideoThread);
}

VideoPlayerWidget::~VideoPlayerWidget()
{
    
}

void VideoPlayerWidget::onClickedLoadButton(void)
{
    QString videoFileName = QFileDialog::getOpenFileName(this, "OpenVideoFile", "./");
    m_pVideoPlayOper->play(videoFileName);
}
