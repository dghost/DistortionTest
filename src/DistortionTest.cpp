#include "DistortionTest.h"
#include "RenderWidget.h"

#include <QMessageBox>

DistortionTest::DistortionTest(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	glwidget = new RenderWidget(this);
	glwidget->setFixedSize(1280,800);
	QObject:connect(glwidget,SIGNAL(sizeChanged(QSize)),ui.statusBar,SLOT(setSize(QSize)));
	ui.mainLayout->addWidget(glwidget);
	ui.mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	this->setFixedSize(glwidget->width(),glwidget->height() + ui.statusBar->height());
	
	/* set up shader selection menu */
	distortionGroup = new QActionGroup(this);
	distortionGroup->addAction(ui.actionNone);
	distortionGroup->addAction(ui.actionStandard);

	connect(distortionGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredDistortion(QAction*)));
	
	ui.actionNone->setCheckable(true);
	ui.actionStandard->setCheckable(true);
	ui.actionNone->setChecked(true);

	/* set up filter selection menu */

	filteringGroup = new QActionGroup(this);
	filteringGroup->addAction(ui.actionGL_NEAREST);
	filteringGroup->addAction(ui.actionGL_LINEAR);
	connect(filteringGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredFiltering(QAction*)));
	
	ui.actionGL_NEAREST->setCheckable(true);
	ui.actionGL_LINEAR->setCheckable(true);
	ui.actionGL_NEAREST->setChecked(true);

	/* set up pattern selection menu */

	patternGroup = new QActionGroup(this);
	patternGroup->addAction(ui.actionCheckered);
	patternGroup->addAction(ui.actionLines);
	patternGroup->addAction(ui.actionGradient);
	connect(patternGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredPattern(QAction*)));
	
	ui.actionCheckered->setCheckable(true);
	ui.actionLines->setCheckable(true);
	ui.actionGradient->setCheckable(true);
	ui.actionCheckered->setChecked(true);
}

DistortionTest::~DistortionTest()
{
	delete glwidget;
	delete distortionGroup;
	delete filteringGroup;
	delete patternGroup;
}

void DistortionTest::about()
{
	QMessageBox msgBox;
	msgBox.setWindowTitle("About");
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setText("Oculus Rift Distortion Test<br>Version 0.2<br><br>Luke Groeninger<br><a href='http://dghost.net'>http://dghost.net</a>");
	msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
}

void DistortionTest::triggeredDistortion(QAction *action)
{
	if (action == ui.actionNone)
	{
		glwidget->setShader(DISTORTION_NONE);
	} else if (action == ui.actionStandard)
	{
		glwidget->setShader(DISTORTION_BARREL);
	}
}

void DistortionTest::triggeredFiltering(QAction *action)
{
	if (action == ui.actionGL_NEAREST)
	{
		glwidget->setTextureFilter(FILTER_NEAREST);
	} else if (action == ui.actionGL_LINEAR)
	{
		glwidget->setTextureFilter(FILTER_BILINEAR);
	}
}

void DistortionTest::triggeredPattern(QAction *action)
{
	if (action == ui.actionCheckered)
	{
		glwidget->setPattern(PATTERN_GRID);
	} else if (action == ui.actionLines)
	{
		glwidget->setPattern(PATTERN_LINES);
	} else if (action == ui.actionGradient)
	{
		glwidget->setPattern(PATTERN_GRADIENT);
	}
}

void DistortionTest::reloadShaders(void)
{
	glwidget->reloadShaders();
}