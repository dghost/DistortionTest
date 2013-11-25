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
	Ui::OVRImageQualityClass ui;
	QActionGroup *distortionGroup;
	QActionGroup *filteringGroup;
	QActionGroup *patternGroup;

	public slots:
		void about();
		void triggeredDistortion(QAction *action);
		void triggeredFiltering(QAction *action);
		void triggeredPattern(QAction *action);
};

#endif // OVRIMAGEQUALITY_H
