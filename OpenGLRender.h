#pragma once

#include <QtWidgets/QWidget>
#include <QOpenGLWidget>
#include <qopengl.h>
#include <QMatrix4x4>
#include "VideoRenderBase.h"
#include "DecodecVideo.h"

class QOpenGLFunctions;
class QOpenGLShader;
class QOpenGLShaderProgram;
class OpenGLRender : public QOpenGLWidget, public VideoRenderBase
{
    Q_OBJECT

public:
    OpenGLRender(QWidget *parent = Q_NULLPTR);
    ~OpenGLRender();

    // setRGBData
    void setRGBData(const uchar* pImageData, int width, int height) override;
    // setYUVData
    void setYUVData(uchar* const pImageData[], int width, int height) override;
    // rebind VBO
    void rebindVBO(int width, int height) override;

protected:
    void initializeGL(void) override;
    void paintGL(void) override;
    void resizeGL(int w, int h) override;

	virtual void customPainterEvent(QPainter* painter);

private:
    struct VertexInfo
    {
        float pos[3];
        float color[4];
        float coord[2];
    };

    QOpenGLFunctions* m_pFunc = nullptr;
    QOpenGLShader* m_pVertexShader = nullptr;
    QOpenGLShader* m_pFragmentShader = nullptr;
    QOpenGLShaderProgram *m_pShaderProgram = nullptr;

    // Uniform ID and Attribute ID
    GLint m_mLocalMat;
    GLint m_vLocalMat;
    GLint m_pLocalMat;
    GLint m_texture;
    GLint m_posVector;
    GLint m_colorVector;
    GLint m_coordVector;
    GLint m_yuvMat;
    GLint m_textureY;
    GLint m_textureU;
    GLint m_textureV;

    // VBO
    GLuint m_vboId;
    // IBO
    GLuint m_iboId;
    // Texture Id
    GLuint m_textureId;
    // Texture YUV Id
    GLuint m_textureYId;
    GLuint m_textureUId;
    GLuint m_textureVId;

    QMatrix4x4 mProjectionMatrix;
    QMatrix4x4 m_YUVTransFormMatrix;

    // sizeInfo
    int m_nWidth = 0;
    int m_nHeight = 0;

    bool m_isUsedRgbTexture = true;

private:
    // Create GPU Program
    GLuint createGPUProgram(const QString& vertexShaderFile, const QString& fragmentShaderFile);
    // Create Texture
    void createTexture(GLuint& textureId, const QString& imageFile);
	void createTexture(GLuint& textureId);
    // Create YUV Texture
    void createYUVTexture(void);
    // Load Texture
    void loadTexture(const uchar* pImageData, int width, int height);
    // Load YUVTexture
    void loadYUVTexture(uchar* const pImageData[], int width, int height);
};
