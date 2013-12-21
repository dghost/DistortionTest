#include "DistortionTest.h"
#include "RenderWidget.h"

#include <QMessageBox>
#include <QDir>
#include <QDateTime>

DistortionTest::DistortionTest(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	glwidget = new RenderWidget(this);
	glwidget->setFixedSize(1280,800);
	QObject::connect(glwidget,SIGNAL(sizeScreenChanged(QSize)),ui.statusBar,SLOT(setScreenSize(QSize)));
	QObject::connect(glwidget,SIGNAL(sizeSourceChanged(QSize)),ui.statusBar,SLOT(setSourceSize(QSize)));
	QObject::connect(glwidget,SIGNAL(sizeDistortionChanged(QSize)),ui.statusBar,SLOT(setDistortionSize(QSize)));

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
	patternGroup->addAction(ui.actionSolid);
	patternGroup->addAction(ui.actionCheckered);
	patternGroup->addAction(ui.actionLines);
	patternGroup->addAction(ui.actionGradient);
	connect(patternGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredPattern(QAction*)));

	ui.actionSolid->setCheckable(true);
	ui.actionCheckered->setCheckable(true);
	ui.actionLines->setCheckable(true);
	ui.actionGradient->setCheckable(true);
	ui.actionSolid->setChecked(true);
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
		glwidget->setDistortionShader(QString(""));
	} else if (action == ui.actionStandard)
	{
		QString shader;
		QFile file("shaders/barrel.frag");
		if(!file.open(QIODevice::ReadOnly)) {
			QMessageBox::information(0, "error", file.errorString());
		}

		QTextStream in(&file);

		while(!in.atEnd()) {
			shader.append( in.readLine());
			shader.append("\n");
		}
//		QMessageBox::information(0, "info", shader);
		file.close();
		glwidget->setDistortionShader(shader);
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
	QString sourceFile;
	if (action == ui.actionCheckered)
	{
		sourceFile = "shaders/checker.frag";
	} else if (action == ui.actionLines)
	{
		sourceFile = "shaders/lines.frag";
	} else if (action == ui.actionGradient)
	{
		sourceFile = "shaders/gradient.frag";
	}

	QString shader;

	if (!sourceFile.isEmpty())
	{
		QFile file(sourceFile);
		if(!file.open(QIODevice::ReadOnly)) {
			QMessageBox::information(0, "error", file.errorString());
		}

		QTextStream in(&file);

		while(!in.atEnd()) {
			shader.append( in.readLine());
			shader.append("\n");
		}
		//		QMessageBox::information(0, "info", shader);
		file.close();
	}
	glwidget->setSourceShader(shader);


}

void DistortionTest::saveScreenShot(void)
{
	QImage temp = glwidget->saveScreenShot();
	QDateTime tempTime = QDateTime::currentDateTime();
	QString time = tempTime.toString("yyyyMMdd-hh-mm-ss-zzz");
	QString filename = QString("screenshots/%1-%2x%3").arg(time).arg(temp.width()).arg(temp.height()).append(".png");
	QDir dir("screenshots");
	if(!dir.exists())
	{
		dir = QDir::current();
		dir.mkdir("screenshots");
	}
	if (!temp.save(filename,"PNG"))
		qDebug() << "Error saving screenshot '" << filename <<"'\n";

}