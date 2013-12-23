#ifndef OVRIMAGEQUALITY_H
#define OVRIMAGEQUALITY_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QActionGroup>

#include "ui_distortiontest.h"
#include "RenderWidget.h"

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
	QActionGroup *hmdGroup;

	public slots:
		void about();
		void triggeredDistortion(QAction *action);
		void triggeredFiltering(QAction *action);
		void triggeredPattern(QAction *action);
		void triggeredHMD(QAction *action);
		void saveScreenShot();

signals:
		void changeTextureFilter(unsigned int newMode);
		void changeSourceShader(QString shader);
		void changeDistortionShader(QString shader);
		void changeScreenShader(QString shader);
		void changeRiftConfig(rift_t config);

};

#endif // OVRIMAGEQUALITY_H
