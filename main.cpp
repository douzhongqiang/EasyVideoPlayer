#include "OpenGLRender.h"
#include "VideoPlayerWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QSurfaceFormat format;
//    format.setDepthBufferSize(24);
//    format.setStencilBufferSize(8);
//    format.setVersion(2, 0);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    QSurfaceFormat::setDefaultFormat(format);

    VideoPlayerWidget w;
    w.show();
    return a.exec();
}
