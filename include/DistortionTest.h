#ifndef OVRIMAGEQUALITY_H
#define OVRIMAGEQUALITY_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QActionGroup>
#include <QFileSystemWatcher>
#include "ui_distortiontest.h"
#include "RenderWidget.h"

#define DIRECTORY_SHADER_SOURCE "shaders/source/"
#define DIRECTORY_TEXTURE_SOURCE "textures/"
#define DIRECTORY_SHADER_DISTORTION "shaders/distortion/"

class DistortionTest : public QMainWindow
{
	Q_OBJECT

public:
	DistortionTest(QWidget *parent = 0);
	~DistortionTest();

protected:
	RenderWidget *glwidget;
private:
	Ui::DistortionTestClass ui;
	QActionGroup *distortionGroup;
	QActionGroup *filteringGroup;
	QActionGroup *patternGroup;
	QActionGroup *textureGroup;
	QActionGroup *hmdGroup;
	QMap<QString, QAction *> distortionMenuActions;
	QMap<QString, QAction *> sourceMenuActions;
	QMap<QString, QAction *> textureMenuActions;

	QFileSystemWatcher currentShaders;
	QFileSystemWatcher directoryWatcher;
	QString distortionPath;
	QString sourceShaderPath;
	QString sourceTexturePath;

	public slots:
		void about();
		void triggeredDistortion(QAction *action);
		void triggeredFiltering(QAction *action);
		void triggeredPattern(QAction *action);
		void triggeredHMD(QAction *action);
		void triggeredTexture(QAction *action);
		void saveScreenShot();
		void enumerateDistortionMenu();
		void enumerateSourceMenu();
		void enumerateTextureMenu();
		void handleDirectoryChange(QString path);

signals:
		void changeTextureFilter(unsigned int newMode);
		void changeSourceShader(QString shader);
		void changeDistortionShader(QString shader);
		void changeScreenShader(QString shader);
		void changeRiftConfig(rift_t config);
		void changeTexture(QImage texture);
};

#endif // OVRIMAGEQUALITY_H
