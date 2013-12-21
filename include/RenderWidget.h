#ifndef OVRRENDERWIDGET_H
#define OVRRENDERWIDGET_H

#include "GL\glew.h"
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QGLFramebufferObject>
#include <QTimer>
#include <QImage>

enum {
	FILTER_NEAREST,
	FILTER_BILINEAR,
	NUM_FILTER_MODES
};

enum {
	PATTERN_GRID,
	PATTERN_LINES,
	PATTERN_GRADIENT,
	NUM_PATTERNS
};

class RenderWidget : public QGLWidget
{
	Q_OBJECT

public:
	RenderWidget(QWidget *parent);
	~RenderWidget();

	QSize sizeHint() const;
	QImage saveScreenShot(void);
protected:
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();

	/*
	QSize RenderWidget::minimumSizeHint() const;
	QSize RenderWidget::maximumSizeHint() const;
	*/
private:
	void generateBuffers();
	void drawBackBuffer();
	void drawDistortion();
	void drawScreen();

	QGLShaderProgram patternShader[NUM_PATTERNS];
	QGLShaderProgram distortionShader;
	QGLShaderProgram screenShader;

	QGLFramebufferObject *backBuffer, *distortionBuffer, *screenBuffer;
	GLuint defaultFBO;
	unsigned int filterMode;
	unsigned int patternMode;
    QTimer timer;

	QSize screenResolution;
	float distortionToScreenRatio;
	float backToDistortionRatio;
	float distortionScale;

signals:
	void sizeScreenChanged(QSize newSize);
	void sizeDistortionChanged(QSize newSize);
	void sizeSourceChanged(QSize newSize);


	public slots:
		void setTextureFilter(unsigned int filter_mode);
		void setPattern(unsigned int pattern);
		void reloadShaders(void);
		void setDistortionShader(QString source);
		void setScreenShader(QString source);
};

#endif // OVRRENDERWIDGET_H
