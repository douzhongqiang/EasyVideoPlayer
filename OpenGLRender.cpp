#include "OpenGLRender.h"
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QPainter>
#include <QDebug>

OpenGLRender::OpenGLRender(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_YUVTransFormMatrix(1.0f, 1.0f, 1.0f, 0.0f, \
                           0.0f, -0.39465f, 2.03211, 0.0f, \
                           1.13983f, -0.58060f, 0.0f, 0.0f, \
                           0.0f, 0.0f, 0.0f, 0.0f)
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

OpenGLRender::~OpenGLRender()
{

}

GLuint OpenGLRender::createGPUProgram(const QString& vertexShaderFile, const QString& fragmentShaderFile)
{
    // load vertex shader
    m_pVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    bool result = m_pVertexShader->compileSourceFile(vertexShaderFile);
    if (!result)
    {
        //qDebug() << m_pVertexShader->log();
        delete m_pVertexShader;
        m_pVertexShader = nullptr;
    }

    // load fragment shader
    m_pFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    result = m_pFragmentShader->compileSourceFile(fragmentShaderFile);
    if (!result)
    {
        //qDebug() << m_pFragmentShader->log();
        delete m_pVertexShader;
        m_pVertexShader = nullptr;
        delete m_pFragmentShader;
        m_pFragmentShader = nullptr;
    }

    // create shader program
    m_pShaderProgram = new QOpenGLShaderProgram(this);
    m_pShaderProgram->addShader(m_pVertexShader);
    m_pShaderProgram->addShader(m_pFragmentShader);
    m_pShaderProgram->link();

    return m_pShaderProgram->programId();
}

void OpenGLRender::initializeGL(void)
{
    m_pFunc = QOpenGLContext::currentContext()->functions();
    m_pFunc->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    GLuint programId = createGPUProgram(":/Resources/VertexShader.vert", ":/Resources/FragmentShader.frag");

    // 获取Uniform ID & Attribute ID
    m_mLocalMat = m_pFunc->glGetUniformLocation(programId, "M");
    m_vLocalMat = m_pFunc->glGetUniformLocation(programId, "V");
    m_pLocalMat = m_pFunc->glGetUniformLocation(programId, "P");
    m_texture = m_pFunc->glGetUniformLocation(programId, "U_MainTexture");
    m_posVector = m_pFunc->glGetAttribLocation(programId, "pos");
    m_colorVector = m_pFunc->glGetAttribLocation(programId, "color");
    m_coordVector = m_pFunc->glGetAttribLocation(programId, "coord");
    // YUV About
    m_textureY = m_pFunc->glGetUniformLocation(programId, "Tex_y");
    m_textureU = m_pFunc->glGetUniformLocation(programId, "Tex_u");
    m_textureV = m_pFunc->glGetUniformLocation(programId, "Tex_v");
    m_yuvMat = m_pFunc->glGetUniformLocation(programId, "YUVMat");

    // 创建VBO
    VertexInfo nVertexInfo[4];
    nVertexInfo[0].pos[0] = -1.0f;
    nVertexInfo[0].pos[1] = 1.0f;
    nVertexInfo[0].pos[2] = 0.0f;
    nVertexInfo[0].color[0] = 0.0f;
    nVertexInfo[0].color[1] = 0.0f;
    nVertexInfo[0].color[2] = 0.0f;
    nVertexInfo[0].color[3] = 0.0f;
    nVertexInfo[0].coord[0] = 0.0f;
    nVertexInfo[0].coord[1] = 0.0f;

    nVertexInfo[1].pos[0] = -1.0f;
    nVertexInfo[1].pos[1] = -1.0f;
    nVertexInfo[1].pos[2] = 0.0f;
    nVertexInfo[1].color[0] = 0.0f;
    nVertexInfo[1].color[1] = 0.0f;
    nVertexInfo[1].color[2] = 0.0f;
    nVertexInfo[1].color[3] = 0.0f;
    nVertexInfo[1].coord[0] = 0.0f;
    nVertexInfo[1].coord[1] = 1.0f;

    nVertexInfo[2].pos[0] = 1.0f;
    nVertexInfo[2].pos[1] = 1.0f;
    nVertexInfo[2].pos[2] = 0.0f;
    nVertexInfo[2].color[0] = 0.0f;
    nVertexInfo[2].color[1] = 0.0f;
    nVertexInfo[2].color[2] = 0.0f;
    nVertexInfo[2].color[3] = 0.0f;
    nVertexInfo[2].coord[0] = 1.0f;
    nVertexInfo[2].coord[1] = 0.0f;

    nVertexInfo[3].pos[0] = 1.0f;
    nVertexInfo[3].pos[1] = -1.0f;
    nVertexInfo[3].pos[2] = -1.0f;
    nVertexInfo[3].color[0] = 0.0f;
    nVertexInfo[3].color[1] = 0.0f;
    nVertexInfo[3].color[2] = 0.0f;
    nVertexInfo[3].color[3] = 0.0f;
    nVertexInfo[3].coord[0] = 1.0f;
    nVertexInfo[3].coord[1] = 1.0f;

    // Create VBO
    m_pFunc->glGenBuffers(1, &m_vboId);
    m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    m_pFunc->glBufferData(GL_ARRAY_BUFFER, sizeof(VertexInfo) * 4, nVertexInfo, GL_STATIC_DRAW);
    m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create IBO
    unsigned int index[6] = { 0, 1, 2, 1, 3, 2};
    m_pFunc->glGenBuffers(1, &m_iboId);
    m_pFunc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboId);
    m_pFunc->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
    m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

    // bind texture
    //createTexture(m_textureId, ":/Resources/25.jpg");
	createTexture(m_textureId, ":/Resources/25.jpg");
    // bind YUV texture
    createYUVTexture();

	// bind VBO
	m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, m_vboId);

	// 使用DrawArrays绘制
	m_pFunc->glEnableVertexAttribArray(m_posVector);
	m_pFunc->glVertexAttribPointer(m_posVector, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)0);
	m_pFunc->glEnableVertexAttribArray(m_colorVector);
	m_pFunc->glVertexAttribPointer(m_colorVector, 4, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)(sizeof(float) * 3));
	m_pFunc->glEnableVertexAttribArray(m_coordVector);
	m_pFunc->glVertexAttribPointer(m_coordVector, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)(sizeof(float) * 7));
	//OpenGLCore->glDrawArrays(GL_TRIANGLES, 0, 3);
	m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_pShaderProgram->bind();
}

void OpenGLRender::paintGL(void)
{
    /*bool result = m_pShaderProgram->bind();
    if (!result)
        return;*/

	QPainter painter(this);
	painter.beginNativePainting();

	m_pShaderProgram->bind();
    QMatrix4x4 nMormalMat;
	
	m_pFunc->glClearColor(41.0f / 255.0f, 71.0f / 255.0f, 121.0f / 255.0f, 1.0f);
	m_pFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pFunc->glUniformMatrix4fv(m_mLocalMat, 1, GL_FALSE, nMormalMat.data());
    m_pFunc->glUniformMatrix4fv(m_vLocalMat, 1, GL_FALSE, nMormalMat.data());
    m_pFunc->glUniformMatrix4fv(m_pLocalMat, 1, GL_FALSE, nMormalMat.data());
    m_pFunc->glUniformMatrix4fv(m_yuvMat, 1, GL_FALSE, m_YUVTransFormMatrix.data());

    // for rgb 0Texture
    if (m_isUsedRgbTexture)
    {
        m_pFunc->glActiveTexture(GL_TEXTURE0);
        m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureId);
        m_pFunc->glUniform1i(m_texture, 0);
    }
    else
    {
        // For YUV Texture
        m_pFunc->glActiveTexture(GL_TEXTURE0);
        m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureYId);
        m_pFunc->glUniform1i(m_textureY, 0);

        m_pFunc->glActiveTexture(GL_TEXTURE1);
        m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureUId);
        m_pFunc->glUniform1i(m_textureU, 1);

        m_pFunc->glActiveTexture(GL_TEXTURE2);
        m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureVId);
        m_pFunc->glUniform1i(m_textureV, 2);
    }

	// bind VBO
	m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, m_vboId);

	// 使用DrawArrays绘制
	m_pFunc->glEnableVertexAttribArray(m_posVector);
	m_pFunc->glVertexAttribPointer(m_posVector, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)0);
	m_pFunc->glEnableVertexAttribArray(m_colorVector);
	m_pFunc->glVertexAttribPointer(m_colorVector, 4, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)(sizeof(float) * 3));
	m_pFunc->glEnableVertexAttribArray(m_coordVector);
	m_pFunc->glVertexAttribPointer(m_coordVector, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)(sizeof(float) * 7));
	//OpenGLCore->glDrawArrays(GL_TRIANGLES, 0, 3);
	m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 使用DeawElement绘制
    m_pFunc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboId);
    m_pFunc->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_pFunc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_pShaderProgram->release();
	
	painter.endNativePainting();

	/*QPen nPen;
	nPen.setColor(QColor(255, 0, 0));
	QFont font = this->font();
	font.setPixelSize(30);
	painter.setFont(font);
	painter.setPen(nPen);
	painter.drawText(this->rect(), "Test Text");*/

	customPainterEvent(&painter);

    //m_pShaderProgram->release();
}

void OpenGLRender::resizeGL(int w, int h)
{
    m_pFunc->glViewport(0, 0, w, h);

    if (m_nWidth > 0 && m_nHeight > 0)
        this->rebindVBO(m_nWidth, m_nHeight);

    /*mProjectionMatrix.setToIdentity();
    mProjectionMatrix.perspective(45.0f, w * 1.0f / h, 0.1f, 500.0f);*/
    update();
}

void OpenGLRender::customPainterEvent(QPainter* painter)
{
	Q_UNUSED(painter)
}

void OpenGLRender::createTexture(GLuint& textureId, const QString& imageFile)
{
    QImage image(imageFile);
    if (image.isNull())
        return;

    image = image.convertToFormat(QImage::Format_RGB888);
    //image = image.mirrored();

    m_pFunc->glGenTextures(1, &textureId);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, textureId);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, \
        GL_RGB, GL_UNSIGNED_BYTE, image.bits());
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);

    // rebind vbo
    m_nWidth = image.width();
    m_nHeight = image.height();
    rebindVBO(m_nWidth, m_nHeight);
}

void OpenGLRender::createTexture(GLuint& textureId)
{
	m_pFunc->glGenTextures(1, &textureId);
	m_pFunc->glBindTexture(GL_TEXTURE_2D, textureId);
	m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRender::createYUVTexture(void)
{
    // Create Y Texture
    m_pFunc->glGenTextures(1, &m_textureYId);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureYId);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 768, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);

    // Create U Texture
    m_pFunc->glGenTextures(1, &m_textureUId);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureUId);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 768, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);

    // Create V Texture
    m_pFunc->glGenTextures(1, &m_textureVId);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureVId);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 768, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRender::loadTexture(const uchar* pImageData, int width, int height)
{
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, \
        GL_RGB, GL_UNSIGNED_BYTE, pImageData);
    /*m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRender::loadYUVTexture(uchar* const pImageData[], int width, int height)
{
    // Y
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureYId);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, \
        GL_RED, GL_UNSIGNED_BYTE, pImageData[0]);
    /*m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);

    // U
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureUId);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, \
        GL_RED, GL_UNSIGNED_BYTE, pImageData[1]);
    /*m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);

    // V
    m_pFunc->glBindTexture(GL_TEXTURE_2D, m_textureVId);
    m_pFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, \
        GL_RED, GL_UNSIGNED_BYTE, pImageData[2]);
    /*m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_pFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
    m_pFunc->glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRender::setRGBData(const uchar* pImageData, int width, int height)
{
    // ReBind vbo
    if (m_nWidth != width || m_nHeight != height)
    {
        m_nWidth = width;
        m_nHeight = height;
        rebindVBO(m_nWidth, m_nHeight);
    }

    // Rebind texture
    loadTexture(pImageData, width, height);
    m_isUsedRgbTexture = true;

    this->update();
}

void OpenGLRender::setYUVData(uchar* const pImageData[], int width, int height)
{
    // ReBind vbo
    if (m_nWidth != width || m_nHeight != height)
    {
        m_nWidth = width;
        m_nHeight = height;
        rebindVBO(m_nWidth, m_nHeight);
    }

    loadYUVTexture(pImageData, width, height);

    // changed YUV row 3 w value for YUV Transform
    QVector4D vec4 = m_YUVTransFormMatrix.column(3);
    vec4.setW(1.0);
    m_YUVTransFormMatrix.setColumn(3, vec4);
    m_isUsedRgbTexture = false;

    this->update();
}

void OpenGLRender::rebindVBO(int width, int height)
{
    float rx = this->width() * 1.0 / width;
    float ry = this->height() * 1.0 / height;
    float minr = qMin(rx, ry);
    float x = width * minr / this->width();
    float y = height * minr / this->height();

    // 创建VBO
    VertexInfo nVertexInfo[4];
    nVertexInfo[0].pos[0] = -x;
    nVertexInfo[0].pos[1] = y;
    nVertexInfo[0].pos[2] = 0.0f;
    nVertexInfo[0].color[0] = 1.0f;
    nVertexInfo[0].color[1] = 0.0f;
    nVertexInfo[0].color[2] = 0.0f;
    nVertexInfo[0].color[3] = 1.0f;
    nVertexInfo[0].coord[0] = 0.0f;
    nVertexInfo[0].coord[1] = 0.0f;

    nVertexInfo[1].pos[0] = -x;
    nVertexInfo[1].pos[1] = -y;
    nVertexInfo[1].pos[2] = 0.0f;
    nVertexInfo[1].color[0] = 0.0f;
    nVertexInfo[1].color[1] = 1.0f;
    nVertexInfo[1].color[2] = 0.0f;
    nVertexInfo[1].color[3] = 1.0f;
    nVertexInfo[1].coord[0] = 0.0f;
    nVertexInfo[1].coord[1] = 1.0f;

    nVertexInfo[2].pos[0] = x;
    nVertexInfo[2].pos[1] = y;
    nVertexInfo[2].pos[2] = 0.0f;
    nVertexInfo[2].color[0] = 0.0f;
    nVertexInfo[2].color[1] = 0.0f;
    nVertexInfo[2].color[2] = 1.0f;
    nVertexInfo[2].color[3] = 1.0f;
    nVertexInfo[2].coord[0] = 1.0f;
    nVertexInfo[2].coord[1] = 0.0f;

    nVertexInfo[3].pos[0] = x;
    nVertexInfo[3].pos[1] = -y;
    nVertexInfo[3].pos[2] = 0.0f;
    nVertexInfo[3].color[0] = 1.0f;
    nVertexInfo[3].color[1] = 0.0f;
    nVertexInfo[3].color[2] = 1.0f;
    nVertexInfo[3].color[3] = 1.0f;
    nVertexInfo[3].coord[0] = 1.0f;
    nVertexInfo[3].coord[1] = 1.0f;

    // Create VBO
    m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    m_pFunc->glBufferData(GL_ARRAY_BUFFER, sizeof(VertexInfo) * 4, nVertexInfo, GL_STATIC_DRAW);
    m_pFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);
}
