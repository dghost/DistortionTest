#ifndef OVRRENDERWIDGET_H
#define OVRRENDERWIDGET_H

#include "GL\glew.h"
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QGLFramebufferObject>
#include <QTimer>

enum {
	DISTORTION_NONE,
	DISTORTION_BARREL,
	NUM_DISTORTION_SHADERS
};

enum {
	FILTER_NEAREST,
	FILTER_BILINEAR,
	NUM_FILTER_MODES
};

enum {
	PATTERN_GRID,
	PATTERN_GRADIENT,
	NUM_PATTERNS
};

class RenderWidget : public QGLWidget
{
	Q_OBJECT

public:
	RenderWidget(QWidget *parent);
	~RenderWidget();

protected:
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	/*
	QSize sizeHint() const;
	QSize RenderWidget::minimumSizeHint() const;
	QSize RenderWidget::maximumSizeHint() const;
	*/
private:
	QGLShaderProgram patternShader[NUM_PATTERNS];
	QGLShaderProgram distortionShader[NUM_DISTORTION_SHADERS];

	QGLFramebufferObject *backBuffer;
	GLuint defaultFBO;
	unsigned int shaderNum;
	unsigned int filterMode;
	unsigned int patternMode;
    QTimer timer;

signals:
	void sizeChanged(QSize newSize);

	public slots:
		void setShader(unsigned int shader_number);
		void setTextureFilter(unsigned int filter_mode);
		void setPattern(unsigned int pattern);
};

#endif // OVRRENDERWIDGET_H
