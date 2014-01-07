#include "RenderWidget.h"


char emptyshader[] = 
	"#version 150\n"
	"void main() {}\n";

char texturepassthrough[] = 
	"#version 150\n"
	"uniform sampler2D Texture;\n"
	"in vec2 TexCoords;\n"
	"out vec4 outColor;\n"
	"void main(void)\n"
	"{\n"
	"	outColor =  texture(Texture,TexCoords);\n"
	"}\n";

char quad[] = 
	"#version 150\n"
	"layout(points) in;\n"
	"layout(triangle_strip, max_vertices = 4) out;\n"
	"out vec2 TexCoords;\n"
	"uniform vec2 lowerLeft = vec2(0.0,0.0);\n"
	"uniform vec2 upperRight = vec2(1.0,1.0);\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );\n"
	"	TexCoords = vec2( upperRight.x, upperRight.y );\n"
	"	EmitVertex();\n"
	"	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0 );\n"
	"	TexCoords = vec2( lowerLeft.x, upperRight.y );\n"
	"	EmitVertex();\n"
	"	gl_Position = vec4( 1.0,-1.0, 0.0, 1.0 );\n"
	"	TexCoords = vec2( upperRight.x, lowerLeft.y );\n"
	"	EmitVertex();\n"
	"	gl_Position = vec4(-1.0,-1.0, 0.0, 1.0 );\n"
	"	TexCoords = vec2( lowerLeft.x, lowerLeft.y );\n"
	"	EmitVertex();\n"
	"	EndPrimitive();\n"
	"}\n";

char barrel[] =
	"#version 150\n"
"layout(points) in;\n"
"layout(triangle_strip, max_vertices = 8) out;\n"
"out vec2 TexCoords;\n"
"invariant out vec2 ScreenCenter;\n"
"invariant out vec2 LensCenter;\n"
"uniform float DistortionOffset = 0.151976;\n"
"void main()\n"
"{\n"
"	ScreenCenter = vec2(0.25,0.5);\n"
"	LensCenter = vec2(0.25 + DistortionOffset * 0.25, 0.5);\n"
"	gl_Position = vec4( 0.0, 1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.5, 1.0);\n"
"	EmitVertex();\n"
"	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.0, 1.0 );\n"
"	EmitVertex();\n"
"	gl_Position = vec4( 0.0,-1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.5, 0.0 );\n"
"	EmitVertex();\n"
"	gl_Position = vec4(-1.0,-1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.0, 0.0 );\n"
"	EmitVertex();\n"
"	EndPrimitive();\n"
"	ScreenCenter = vec2(0.75,0.5);\n"
"	LensCenter = vec2(0.75 - DistortionOffset * 0.25, 0.5);\n"
"	gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 1.0, 1.0 );\n"
"	EmitVertex();\n"
"	gl_Position = vec4(0.0, 1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.5, 1.0 );\n"
"	EmitVertex();\n"
"	gl_Position = vec4( 1.0,-1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 1.0, 0.0 );\n"
"	EmitVertex();\n"
"	gl_Position = vec4(0.0,-1.0, 0.0, 1.0 );\n"
"	TexCoords = vec2( 0.5, 0.0 );\n"
"	EmitVertex();\n"
"	EndPrimitive();\n"
"}\n";

RenderWidget::RenderWidget(QWidget *parent)
	: QGLWidget(parent)
{
//	QWidget::setFixedSize(QSize(1280,800));
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
	msgBox = NULL;

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
	closeErrorBox(0);
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

void RenderWidget::errorBox(QString title, QString message)
{
	if (msgBox != NULL)
		closeErrorBox(0);

	msgBox = new QMessageBox( this );
	msgBox->setAttribute( Qt::WA_DeleteOnClose ); //makes sure the msgbox is deleted automatically when closed
	msgBox->setStandardButtons( QMessageBox::Ok );
	msgBox->setWindowTitle( title );
	msgBox->setText( message );
	msgBox->setIcon(QMessageBox::Critical);
	msgBox->setModal( false ); // if you want it non-modal
	msgBox->open( this, SLOT(closeErrorBox(int)) );
}

void RenderWidget::closeErrorBox(int result)
{
	if (msgBox != NULL)
		msgBox->close();

	msgBox = NULL;
}
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
	updateGL();
}

void RenderWidget::drawBackBuffer()
{
//	QSize size = screenResolution * distortionToScreenRatio * distortionScale

		backBuffer->bind();
		glViewport(0,0,backBuffer->width(),backBuffer->height());
		glClear(GL_COLOR_BUFFER_BIT);
		patternShader.bind();
		patternShader.setUniformValue("ScreenSize", screenResolution);
		patternShader.setUniformValue("TextureSize",sourceTexture.size());

		if (!sourceTexture.isNull())
		{
			bindTexture(sourceTexture);
		}

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
		distortionShader.setUniformValue("ScreenSize", screenResolution);

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
		screenShader.bind();
		screenShader.setUniformValue("TextureSize",distortionBuffer->size());
		screenShader.setUniformValue("ScreenSize", screenResolution);

		glDrawArrays(GL_POINTS, 0, 1);
		glBindTexture(GL_TEXTURE_2D,0);
		screenShader.release();
		screenBuffer->release();
	
}

void RenderWidget::paintGL()
{
	// draw the scene:
	bool linkSuccess = true;
	if (patternDirty)
	{
		patternDirty = false;
		distortionDirty = true;
		linkSuccess = linkSuccess && linkSource(patternSource);
		drawBackBuffer();
	}

	if (distortionDirty)
	{
		distortionDirty = false;
		screenDirty = true;
		linkSuccess = linkSuccess && linkDistortion(distortionSource);
		drawDistortion();
	}

	if (screenDirty)
	{
		screenDirty = false;
		linkSuccess = linkSuccess && linkScreen(screenSource);
		drawScreen();
	}

	if (linkSuccess)
		closeErrorBox(0);

	glViewport( 0, 0, this->width(), this->height() );
	screenShader.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,screenBuffer->texture());
	glDrawArrays(GL_POINTS, 0, 1);
	glBindTexture(GL_TEXTURE_2D,0);
	screenShader.release();
	
}

bool RenderWidget::linkSource(QString source)
{
	bool result = true;
	patternShader.removeAllShaders();
	if (!source.isEmpty())
	{
		patternShader.addShaderFromSourceCode (QGLShader::Geometry,quad);
		patternShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		if (patternShader.addShaderFromSourceCode (QGLShader::Fragment, source) 
			&& patternShader.link())
			return true;
		else {
			errorBox(QString("Error compiling source shader"), patternShader.log());
			patternShader.removeAllShaders();
			result = false;
		}
	} 

	patternShader.addShaderFromSourceCode (QGLShader::Geometry,quad);
	patternShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	patternShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	patternShader.link();
	return result;
}

void RenderWidget::setSourceShader(QString source)
{
	patternSource = source;
	patternDirty = true;
	updateGL();
}

bool RenderWidget::linkDistortion(QString source)
{
	bool result = true;
	distortionShader.removeAllShaders();
	if (!source.isEmpty())
	{
		distortionShader.addShaderFromSourceCode (QGLShader::Geometry,barrel);
		distortionShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		if (distortionShader.addShaderFromSourceCode (QGLShader::Fragment, source) 
			&& distortionShader.link())
			return true;
		else {
			errorBox(QString("Error compiling distortion shader"),distortionShader.log());
//			qDebug() << distortionShader.log();
			distortionShader.removeAllShaders();
			result = false;
		}
	} 

	distortionShader.addShaderFromSourceCode (QGLShader::Geometry,quad);
	distortionShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	distortionShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	distortionShader.link();
	return result;
}

void RenderWidget::setDistortionShader(QString source)
{
	distortionSource = source;
	distortionDirty = true;
	updateGL();
}

bool RenderWidget::linkScreen(QString source)
{
	bool result = true;
	screenShader.removeAllShaders();
	if (!source.isEmpty())
	{
		screenShader.addShaderFromSourceCode (QGLShader::Geometry,quad);
		screenShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
		
		if (screenShader.addShaderFromSourceCode (QGLShader::Fragment, source)
			&& screenShader.link())
			return true;
		else {
			errorBox(QString("Error compiling screen shader"),screenShader.log());
			screenShader.removeAllShaders();
			result = false;
		}
	} 

	screenShader.addShaderFromSourceCode (QGLShader::Geometry,quad);
	screenShader.addShaderFromSourceCode (QGLShader::Vertex,emptyshader);
	screenShader.addShaderFromSourceCode (QGLShader::Fragment, texturepassthrough);
	screenShader.link();
	return result;
}

void RenderWidget::setScreenShader(QString source)
{	
	screenSource = source;
	screenDirty = true;	
	updateGL();
}
void RenderWidget::setTextureFilter(unsigned int filter_mode)
{
	if (filter_mode < NUM_FILTER_MODES)
		filterMode = filter_mode;

	distortionDirty = true;
	updateGL();
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
	updateGL();
}

void RenderWidget::setRiftConfig(rift_t config)
{
	riftConfig = config;
	setScreenResolution(QSize(riftConfig.h_resolution,riftConfig.v_resolution));
	updateGL();
}

void RenderWidget::setSourceTexture(QImage texture)
{
	sourceTexture = texture;
	patternDirty = true;
	updateGL();
}
