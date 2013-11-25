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
}

RenderWidget::~RenderWidget()
{
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

QSize RenderWidget::sizeHint() const
{
    return QSize(1280, 800);
}
*/


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

	for (int i = 0; i < NUM_DISTORTION_SHADERS ; i++)
		distortionShader[i].removeAllShaders();

	distortionShader[DISTORTION_NONE].addShaderFromSourceFile (QGLShader::Geometry,"shaders\\quad_full.geom");
	distortionShader[DISTORTION_NONE].addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
	distortionShader[DISTORTION_NONE].addShaderFromSourceFile (QGLShader::Fragment, "shaders\\passthrough.frag");
	distortionShader[DISTORTION_NONE].link();

	distortionShader[DISTORTION_BARREL].addShaderFromSourceFile (QGLShader::Geometry,"shaders\\barrel.geom");
	distortionShader[DISTORTION_BARREL].addShaderFromSourceFile (QGLShader::Vertex,"shaders\\empty.shdr");
	distortionShader[DISTORTION_BARREL].addShaderFromSourceFile (QGLShader::Fragment, "shaders\\barrel.frag");
	distortionShader[DISTORTION_BARREL].link();

	doneCurrent();
}

void RenderWidget::initializeGL()
{
	// Set up the rendering context, define display lists etc.:
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glEnable(GL_DEPTH_TEST);


	shaderNum = DISTORTION_NONE;
	filterMode = FILTER_NEAREST;
	patternMode = PATTERN_GRID;

	backBuffer = new QGLFramebufferObject(1280, 800, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D);
	reloadShaders();
}

void RenderWidget::resizeGL( int w, int h )
{

	glViewport( 0, 0, (GLint)w, (GLint)h );
	emit(sizeChanged(QSize(w,h)));

}

void RenderWidget::paintGL()
{
	// draw the scene:
	QGLShaderProgram *currentDistortion = &distortionShader[shaderNum];
	QGLShaderProgram *currentPattern = &patternShader[patternMode];

	backBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT);
	currentPattern->bind();
	currentPattern->setUniformValue("in_ScreenSize", backBuffer->size());
	glDrawArrays(GL_POINTS, 0, 1);
	currentPattern->release();

	backBuffer->release();


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,backBuffer->texture());
	currentDistortion->bind();
//	currentDistortion->setUniformValue("Texture",0);
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

	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	currentDistortion->release();
}


void RenderWidget::setShader(unsigned int shader_number)
{
	if (shader_number >= 0 && shader_number < NUM_DISTORTION_SHADERS)
		shaderNum = shader_number;
}

void RenderWidget::setTextureFilter(unsigned int filter_mode)
{
	if (filter_mode >= 0 && filter_mode < NUM_FILTER_MODES)
		filterMode = filter_mode;
}

void RenderWidget::setPattern(unsigned int pattern)
{
	if (pattern >= 0 && pattern < NUM_PATTERNS)
		patternMode = pattern;
}
