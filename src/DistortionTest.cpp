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
	connect(this,SIGNAL(changeTextureFilter(unsigned int)), glwidget,SLOT(setTextureFilter(unsigned int)));
	connect(this,SIGNAL(changeSourceShader(QString)), glwidget,SLOT(setSourceShader(QString)));
	connect(this,SIGNAL(changeDistortionShader(QString)), glwidget,SLOT(setDistortionShader(QString)));
	connect(this,SIGNAL(changeScreenShader(QString)), glwidget,SLOT(setScreenShader(QString)));
	connect(this,SIGNAL(changeRiftConfig(rift_t)), glwidget,SLOT(setRiftConfig(rift_t)));
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
	ui.actionNone->trigger();

	/* set up filter selection menu */

	filteringGroup = new QActionGroup(this);
	filteringGroup->addAction(ui.actionGL_NEAREST);
	filteringGroup->addAction(ui.actionGL_LINEAR);
	connect(filteringGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredFiltering(QAction*)));

	ui.actionGL_NEAREST->setCheckable(true);
	ui.actionGL_LINEAR->setCheckable(true);
	ui.actionGL_NEAREST->trigger();

	/* set up pattern selection menu */

	patternGroup = new QActionGroup(this);
	patternGroup->addAction(ui.actionCheckered);
	patternGroup->addAction(ui.actionLines);
	patternGroup->addAction(ui.actionGradient);
	connect(patternGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredPattern(QAction*)));

	ui.actionCheckered->setCheckable(true);
	ui.actionLines->setCheckable(true);
	ui.actionGradient->setCheckable(true);
	ui.actionCheckered->trigger();

	hmdGroup = new QActionGroup(this);
	hmdGroup->addAction(ui.actionDK1);
	hmdGroup->addAction(ui.actionHD_DK);

	connect(hmdGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredHMD(QAction*)));

	ui.actionDK1->setCheckable(true);
	ui.actionHD_DK->setCheckable(true);
	ui.actionDK1->trigger();


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
	msgBox.setText("Oculus Rift Distortion Test<br>Version 0.3<br><br>Luke Groeninger<br><a href='http://dghost.net'>http://dghost.net</a>");
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
		emit changeDistortionShader(shader);
	}
}

void DistortionTest::triggeredFiltering(QAction *action)
{
	if (action == ui.actionGL_NEAREST)
	{
		emit changeTextureFilter(FILTER_NEAREST);
	} else if (action == ui.actionGL_LINEAR)
	{
		emit changeTextureFilter(FILTER_BILINEAR);
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

	emit changeSourceShader(shader);


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

void DistortionTest::triggeredHMD(QAction *action)
{
	if (action == ui.actionDK1)
	{
		rift_t temp = {1280, 800, 
			0.14976f, 0.0936f,
			0.064f,
			0.0635f,
			0.041f,
		{ 1.0f, 0.22f, 0.24f, 0.0f },
		{ 0.996f, -0.004f, 1.014f, 0.0f }};
		emit changeRiftConfig(temp);
	} else if (action == ui.actionHD_DK)
	{
		rift_t temp = {1920, 1080, 
			0.12096f, 0.06804f,
			0.064f,
			0.0635f,
			0.040f,
		{ 1.0f, 0.18f, 0.115f, 0.0f },
		{ 0.996f, -0.004f, 1.014f, 0.0f }};
		emit changeRiftConfig(temp);
	}
}