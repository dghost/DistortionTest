#include "RenderWidget.h"
#include <QMessageBox>

char emptyshader[] = "#version 150\n"
	"void main() {}\n";

char texturepassthrough[] = "#version 150\n"
"uniform sampler2D Texture;\n"
"in vec2 texCoords;\n"
"out vec4 outColor;\n"
"void main(void)\n"
"{\n"
"	outColor =  texture(Texture,texCoords);\n"
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
	distortionToScreenRatio = 1.0;
	backToDistortionRatio = 1.0;
	distortionScale = 1.0;
	filterMode = FILTER_NEAREST;
	patternSource = "";
	patternDirty = true;
	distortionSource = "";
	distortionDirty = true;
	screenSource = "";
	screenDirty = true;

	rift_t temp = {1280, 800, 
		0.14976f, 0.0936f,
		0.064f,
		0.0635f,
		0.041f,
		{ 1.0f, 0.22f, 0.24f, 0.0f },
	{ 0.996f, -0.004f, 1.014f, 0.0f }};
	riftConfig = temp;
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
	return QSize(riftConfig.h_resolution, riftConfig.v_resolution);
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

	setRiftConfig(riftConfig);
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
		patternShader.setUniformValue("ScreenSize", screenResolution);

		if (!sourceTexture.isNull())
		{
			bindTexture(sourceTexture);
			patternShader.setUniformValue("TextureSize",sourceTexture.size());
		}

		glDrawArrays(GL_POINTS, 0, 1);
		glBindTexture(GL_TEXTURE_2D,0);
		patternShader.release();
		backBuffer->release();
}

void RenderWidget::drawDistortion()
{
		linkDistortion(distortionSource);
		distortionBuffer->bind();
		glViewport(0,0,distortionBuffer->width(),distortionBuffer->height());
		glClear(GL_COLOR_BUFFER_BIT);
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
		float aspect = riftConfig.h_resolution / (2.0f * riftConfig.v_resolution);
		distortionShader.bind();
		distortionShader.setUniformValue("HmdWarpParam",riftConfig.distortion_k[0],riftConfig.distortion_k[1],riftConfig.distortion_k[2],riftConfig.distortion_k[3]);
		distortionShader.setUniformValue("DistortionOffset", 1.0f - (2.0f * riftConfig.lens_separation_distance) / riftConfig.h_screen_size);
		distortionShader.setUniformValue("ScaleIn",4.0, 2.0/aspect);
		distortionShader.setUniformValue("Scale",0.25, 0.5 * aspect);
		distortionShader.setUniformValue("TextureSize",backBuffer->size());
		glDrawArrays(GL_POINTS, 0, 1);
		glBindTexture(GL_TEXTURE_2D,0);
		distortionShader.release();
		distortionBuffer->release();
}

void RenderWidget::drawScreen()
{
		linkScreen(screenSource);
		screenBuffer->bind();
		glViewport(0,0,screenBuffer->width(),screenBuffer->height());
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D,distortionBuffer->texture());
		screenShader.bind();

		glDrawArrays(GL_POINTS, 0, 1);
		glBindTexture(GL_TEXTURE_2D,0);
		screenShader.release();
		screenBuffer->release();
	
}

void RenderWidget::paintGL()
{
	// draw the scene:

	if (patternDirty)
	{
		linkSource(patternSource);
		drawBackBuffer();
		patternDirty = false;
		distortionDirty = true;
	}

	if (distortionDirty)
	{
		linkDistortion(distortionSource);
		drawDistortion();
		distortionDirty = false;
		screenDirty = true;
	}

	if (screenDirty)
	{
		linkScreen(screenSource);
		drawScreen();
		screenDirty = false;
	}

	glViewport( 0, 0, this->width(), this->height() );
	screenShader.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,screenBuffer->texture());
	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	screenShader.release();
	
}

void RenderWidget::linkSource(QString source)
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
	patternShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	patternShader.link();
}

void RenderWidget::setSourceShader(QString source)
{
	patternSource = source;
	patternDirty = true;
}

void RenderWidget::linkDistortion(QString source)
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
//			qDebug() << distortionShader.log();
			distortionShader.removeAllShaders();
		}
	} 

	distortionShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	distortionShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	distortionShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	distortionShader.link();
}

void RenderWidget::setDistortionShader(QString source)
{
	distortionSource = source;
	distortionDirty = true;
}

void RenderWidget::linkScreen(QString source)
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

void RenderWidget::setScreenShader(QString source)
{	
	screenSource = source;
	screenDirty = true;	
}
void RenderWidget::setTextureFilter(unsigned int filter_mode)
{
	if (filter_mode < NUM_FILTER_MODES)
		filterMode = filter_mode;

	distortionDirty = true;

}

QImage RenderWidget::saveScreenShot(void)
{
	makeCurrent();
	QImage temp = screenBuffer->toImage();
	doneCurrent();
	return temp;
}

void RenderWidget::setScreenResolution(QSize resolution)
{
	makeCurrent();
	screenResolution = resolution;
	generateBuffers();
	patternDirty = true;
	distortionDirty = true;
	screenDirty = true;
	doneCurrent();
}

void RenderWidget::setRiftConfig(rift_t config)
{
	riftConfig = config;
	setScreenResolution(QSize(riftConfig.h_resolution,riftConfig.v_resolution));
}

void RenderWidget::setSourceTexture(QImage texture)
{
	sourceTexture = texture;
	patternDirty = true;
}
