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
	QActionGroup *filteringGroup;
	QActionGroup *hmdGroup;

	QFileSystemWatcher currentFiles;
	QFileSystemWatcher directoryWatcher;

	QActionGroup *distortionGroup;
	QMap<QString, QAction *> distortionMenuActions;
	QString distortionPath;
	QString currentDistortionShader;

	QActionGroup *patternGroup;
	QMap<QString, QAction *> sourceMenuActions;
	QString sourceShaderPath;
	QString currentSourceShader;

	QActionGroup *textureGroup;
	QMap<QString, QAction *> textureMenuActions;
	QString sourceTexturePath;
	QString currentSourceTexture;

	public slots:
		void about();
		void saveScreenShot();
		void triggeredFiltering(QAction *action);
		void triggeredHMD(QAction *action);

		void setDistortionShader(QString filePath);
		void triggeredDistortion(QAction *action);
		void enumerateDistortionMenu();

		void setSourceTexture(QString filePath);
		void triggeredTexture(QAction *action);
		void enumerateTextureMenu();

		void setSourceShader(QString filePath);
		void triggeredPattern(QAction *action);
		void enumerateSourceMenu();

		void handleDirectoryChange(QString path);
		void handleFileChange(QString path);
signals:
		void changeTextureFilter(unsigned int newMode);
		void changeSourceShader(QString shader);
		void changeDistortionShader(QString shader);
		void changeScreenShader(QString shader);
		void changeRiftConfig(rift_t config);
		void changeTexture(QImage texture);
};

#endif // OVRIMAGEQUALITY_H
