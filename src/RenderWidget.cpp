#include "RenderWidget.h"

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
	patternMode = PATTERN_GRID;
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


void RenderWidget::reloadShaders(void)
{
	makeCurrent();
	for (int i = 0; i < NUM_PATTERNS ; i++)
		patternShader[i].removeAllShaders();

	patternShader[PATTERN_GRID].addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	patternShader[PATTERN_GRID].addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
	patternShader[PATTERN_GRID].addShaderFromSourceFile (QGLShader::Fragment, "shaders\\checker.frag");
	patternShader[PATTERN_GRID].link();

	patternShader[PATTERN_LINES].addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	patternShader[PATTERN_LINES].addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
	patternShader[PATTERN_LINES].addShaderFromSourceFile (QGLShader::Fragment, "shaders\\lines.frag");
	patternShader[PATTERN_LINES].link();

	patternShader[PATTERN_GRADIENT].addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	patternShader[PATTERN_GRADIENT].addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
	patternShader[PATTERN_GRADIENT].addShaderFromSourceFile (QGLShader::Fragment, "shaders\\gradient.frag");
	patternShader[PATTERN_GRADIENT].link();

	doneCurrent();
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
	generateBuffers();
	reloadShaders();
}

void RenderWidget::resizeGL( int w, int h )
{

//	glViewport( 0, 0, (GLint)w, (GLint)h );
//	emit(sizeChanged(QSize(w,h)));

}

void RenderWidget::drawBackBuffer()
{
//	QSize size = screenResolution * distortionToScreenRatio * distortionScale
	QGLShaderProgram *currentPattern = &patternShader[patternMode];
	backBuffer->bind();
	glViewport(0,0,backBuffer->width(),backBuffer->height());
	glClear(GL_COLOR_BUFFER_BIT);
	currentPattern->bind();
	currentPattern->setUniformValue("in_ScreenSize", screenResolution);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	currentPattern->release();
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

void RenderWidget::setDistortionShader(QString source)
{
	distortionShader.removeAllShaders();
	if (source.isEmpty())
	{
		distortionShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
		distortionShader.addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
		distortionShader.addShaderFromSourceFile (QGLShader::Fragment, "shaders\\passthrough.frag");
		distortionShader.link();
	} else {
		distortionShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\barrel.geom");
		distortionShader.addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
		distortionShader.addShaderFromSourceCode (QGLShader::Fragment, source);
		distortionShader.link();
	}
}


void RenderWidget::setScreenShader(QString source)
{
	screenShader.removeAllShaders();
	if (source.isEmpty())
	{
		screenShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
		screenShader.addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
		screenShader.addShaderFromSourceFile (QGLShader::Fragment, "shaders\\passthrough.frag");
		screenShader.link();
	} else {
		screenShader.addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
		screenShader.addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
		screenShader.addShaderFromSourceCode (QGLShader::Fragment, source);
		screenShader.link();
	}
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

void RenderWidget::setPattern(unsigned int pattern)
{
	if (pattern < NUM_PATTERNS)
		patternMode = pattern;
}

QImage RenderWidget::saveScreenShot(void)
{
	makeCurrent();
	QImage temp = screenBuffer->toImage();
	doneCurrent();
	return temp;
}