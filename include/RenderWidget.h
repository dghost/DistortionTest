#ifndef OVRRENDERWIDGET_H
#define OVRRENDERWIDGET_H

#include "GL\glew.h"
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QGLFramebufferObject>
#include <QTimer>
#include <QImage>
#include <QMessageBox>

enum {
	FILTER_NEAREST,
	FILTER_BILINEAR,
	NUM_FILTER_MODES
};

typedef struct _riftSettings_t 
{
		unsigned int h_resolution;
		unsigned int v_resolution;
		float h_screen_size;
		float v_screen_size;
		float interpupillary_distance;
		float lens_separation_distance;
		float eye_to_screen_distance;
		float distortion_k[4];
		float chrom_abr[4];
} rift_t;

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
	bool linkSource(QString source);
	bool linkDistortion(QString source);
	bool linkScreen(QString source);

	QMessageBox *msgBox;

	QString patternSource;
	QGLShaderProgram patternShader;
	QImage sourceTexture;

	QString distortionSource;
	QGLShaderProgram distortionShader;

	QString screenSource;
	QGLShaderProgram screenShader;

	QGLFramebufferObject *backBuffer, *distortionBuffer, *screenBuffer;
	GLuint defaultFBO;
	unsigned int filterMode;
    QTimer timer;

	bool patternDirty;
	bool distortionDirty;
	bool screenDirty;

	QSize screenResolution;
	float distortionToScreenRatio;
	float backToDistortionRatio;
	float distortionScale;

	rift_t riftConfig;

signals:
	void sizeScreenChanged(QSize newSize);
	void sizeDistortionChanged(QSize newSize);
	void sizeSourceChanged(QSize newSize);


	public slots:
		void setTextureFilter(unsigned int filter_mode);
		void setSourceShader(QString source);
		void setSourceTexture(QImage texture);
		void setDistortionShader(QString source);
		void setScreenShader(QString source);
		void setRiftConfig(rift_t config);
		void setScreenResolution(QSize resolution);
		void errorBox(QString title, QString message);
		void closeErrorBox(int result);
};

#endif // OVRRENDERWIDGET_H
