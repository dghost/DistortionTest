#include "RenderWidget.h"
#include <QMessageBox>

char solidgreen[] = "#version 150\n"
	"out vec4 outColor;\n"
	"void main(void)\n"
	"{\n"
	"outColor = vec4(0.0,0.75,0.0,1.0);\n"
	"}\n";

char emptyshader[] = "#version 150\n"
	"void main() {}\n";

char texturepassthrough[] = "#version 150\n"
"uniform sampler2D Texture;\n"
"in vec2 texCoords;\n"
"out vec4 out_Color;\n"
"void main(void)\n"
"{\n"
"	out_Color =  texture(Texture,texCoords);\n"
"}\n";

QGLFormat desiredFormat()
{
    QGLFormat fmt;
    fmt.setSwapInterval(1);
    return fmt;
}

RenderWidget::RenderWidget(QWidget *parent)
	: QGLWidget(desiredFormat(),parent)
{
//	QWidget::setFixedSize(QSize(1280,800));
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    if(format().swapInterval() == -1)
    {
        // V_blank synchronization not available (tearing likely to happen)
        qDebug("Swap Buffers at v_blank not available: refresh at approx 60fps.");
        timer.setInterval(17);
    }
    else
    {
        // V_blank synchronization available
        timer.setInterval(0);
    }
    timer.start();
	backBuffer = NULL;
	distortionBuffer = NULL;
	screenBuffer = NULL;
	screenResolution = QSize(1280,800);
	distortionToScreenRatio = 1.0;
	backToDistortionRatio = 1.0;
	distortionScale = 1.0;
	filterMode = FILTER_NEAREST;
}

RenderWidget::~RenderWidget()
{
	if (screenBuffer)
		delete screenBuffer;
	if (distortionBuffer)
		delete distortionBuffer;
	if (backBuffer)
		delete backBuffer;
}

/*
QSize RenderWidget::minimumSizeHint() const
{
    return QSize(1280, 800);
}

QSize RenderWidget::maximumSizeHint() const
{
    return QSize(1280, 800);
}
*/

QSize RenderWidget::sizeHint() const
{
    return QSize(1280, 800);
}

void RenderWidget::generateBuffers()
{
	if (screenBuffer)
		delete screenBuffer;

	screenBuffer = new QGLFramebufferObject(screenResolution, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,GL_RGBA8);
	glBindTexture(GL_TEXTURE_2D,screenBuffer->texture());
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D,0);

	if (distortionBuffer)
		delete distortionBuffer;

	QSize distortionSize = screenResolution * distortionToScreenRatio;	
	distortionBuffer = new QGLFramebufferObject(distortionSize, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D,distortionBuffer->texture());
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D,0);

	if (backBuffer)
		delete backBuffer;


	QSize backSize = distortionSize * backToDistortionRatio * distortionScale;
	backBuffer = new QGLFramebufferObject(backSize, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,GL_RGBA32F);
	setTextureFilter(filterMode);

	emit(sizeDistortionChanged(distortionSize));
	emit(sizeSourceChanged(backSize));

	emit(sizeScreenChanged(screenResolution));



}
void RenderWidget::initializeGL()
{
	// Set up the rendering context, define display lists etc.:
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glEnable(GL_DEPTH_TEST);



	setScreenShader("");
	setDistortionShader("");
	setSourceShader("");
	generateBuffers();
}

void RenderWidget::resizeGL( int w, int h )
{

//	glViewport( 0, 0, (GLint)w, (GLint)h );
//	emit(sizeChanged(QSize(w,h)));

}

void RenderWidget::drawBackBuffer()
{
//	QSize size = screenResolution * distortionToScreenRatio * distortionScale
	backBuffer->bind();
	glViewport(0,0,backBuffer->width(),backBuffer->height());
	glClear(GL_COLOR_BUFFER_BIT);
	patternShader.bind();
	patternShader.setUniformValue("in_ScreenSize", screenResolution);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	patternShader.release();
	backBuffer->release();
}

void RenderWidget::drawDistortion()
{
	distortionBuffer->bind();
	glViewport(0,0,distortionBuffer->width(),distortionBuffer->height());
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,backBuffer->texture());
	distortionShader.bind();

	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	distortionShader.release();
	distortionBuffer->release();
}

void RenderWidget::drawScreen()
{
	screenBuffer->bind();
	glViewport(0,0,screenBuffer->width(),screenBuffer->height());
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,distortionBuffer->texture());
	distortionShader.bind();

	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	distortionShader.release();
	screenBuffer->release();
}

void RenderWidget::paintGL()
{
	// draw the scene:

	drawBackBuffer();
	drawDistortion();
	drawScreen();
	
	glViewport( 0, 0, this->width(), this->height() );
	screenShader.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,screenBuffer->texture());
	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	screenShader.release();
	
}

void RenderWidget::setSourceShader(QString source)
{
	patternShader.removeAllShaders();
	if (!source.isEmpty())
	{
		patternShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
		patternShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		patternShader.addShaderFromSourceCode (QGLShader::Fragment, source);
		if (patternShader.link())
			return;
		else {
			QMessageBox::information(0, "error", patternShader.log());
			patternShader.removeAllShaders();
		}
	} 

	patternShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	patternShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	patternShader.addShaderFromSourceCode (QGLShader::Fragment, solidgreen);
	patternShader.link();
}

void RenderWidget::setDistortionShader(QString source)
{
	distortionShader.removeAllShaders();
	if (!source.isEmpty())
	{
		distortionShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\barrel.geom");
		distortionShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		distortionShader.addShaderFromSourceCode (QGLShader::Fragment, source);
		if (distortionShader.link())
			return;
		else {
			QMessageBox::information(0, "error", distortionShader.log());
			distortionShader.removeAllShaders();
		}
	} 

	distortionShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	distortionShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	distortionShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	distortionShader.link();

}


void RenderWidget::setScreenShader(QString source)
{
	screenShader.removeAllShaders();
	if (!source.isEmpty())
	{
		screenShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
		screenShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		screenShader.addShaderFromSourceCode (QGLShader::Fragment, source);
		if (screenShader.link())
			return;
		else {
			QMessageBox::information(0, "error", screenShader.log());
			screenShader.removeAllShaders();
		}
	} 

	screenShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	screenShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	screenShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	screenShader.link();
}
void RenderWidget::setTextureFilter(unsigned int filter_mode)
{
	if (filter_mode < NUM_FILTER_MODES)
		filterMode = filter_mode;

	glBindTexture(GL_TEXTURE_2D,backBuffer->texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (filterMode == FILTER_BILINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D,0);

}

QImage RenderWidget::saveScreenShot(void)
{
	makeCurrent();
	QImage temp = screenBuffer->toImage();
	doneCurrent();
	return temp;
}