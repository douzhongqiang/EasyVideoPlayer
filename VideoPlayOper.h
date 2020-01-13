#ifndef VIDEOPLAYOPER_H
#define VIDEOPLAYOPER_H

#include <QObject>

class OpenGLRender;
class DecodecVideo;
class AudioPlayerThread;
class QTimer;
class VideoRenderBase;
class VideoPlayOper : public QObject
{
    Q_OBJECT
public:
    VideoPlayOper(QObject* parent = nullptr);
    ~VideoPlayOper();

    void setVideoRender(VideoRenderBase* render);
    void setVideoDecodec(DecodecVideo* codec);

    void play(const QString& name);
	void seek(qreal time);

private:
	VideoRenderBase* m_pRender = nullptr;
    DecodecVideo* m_pCodec = nullptr;
    AudioPlayerThread *m_pAudioPlayerThread = nullptr;

    QTimer* m_pTimer = nullptr;

private slots:
    void onTimeout(qreal time);
	void onTimeout2(void);
	
signals:
	void updateDisplayInfos(qreal time);
};
#endif
