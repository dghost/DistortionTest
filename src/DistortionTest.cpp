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
	connect(this,SIGNAL(changeTexture(QImage)), glwidget,SLOT(setSourceTexture(QImage)));

	ui.mainLayout->addWidget(glwidget);
	ui.mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	this->setFixedSize(glwidget->width(),glwidget->height() + ui.statusBar->height());

	/* set up shader selection menu */
	distortionGroup = new QActionGroup(this);
	distortionGroup->addAction(ui.actionNone);

	connect(distortionGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredDistortion(QAction*)));

	ui.actionNone->setCheckable(true);
	ui.actionNone->trigger();

	/* set up filter selection menu */

	filteringGroup = new QActionGroup(this);
	filteringGroup->addAction(ui.actionGL_NEAREST);
	filteringGroup->addAction(ui.actionGL_LINEAR);
	connect(filteringGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredFiltering(QAction*)));

	ui.actionGL_NEAREST->setCheckable(true);
	ui.actionGL_LINEAR->setCheckable(true);
	ui.actionGL_LINEAR->trigger();

	/* set up pattern selection menu */

	patternGroup = new QActionGroup(this);
	patternGroup->addAction(ui.actionSourceNone);
	connect(patternGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredPattern(QAction*)));
	ui.actionSourceNone->setCheckable(true);
	ui.actionSourceNone->trigger();

	textureGroup = new QActionGroup(this);
	textureGroup->addAction(ui.actionTextureNone);
	connect(textureGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredTexture(QAction*)));
	ui.actionTextureNone->setCheckable(true);
	ui.actionTextureNone->trigger();

	hmdGroup = new QActionGroup(this);
	hmdGroup->addAction(ui.actionDK1);
	hmdGroup->addAction(ui.actionHD_DK);

	connect(hmdGroup,SIGNAL(triggered(QAction*)),this,SLOT(triggeredHMD(QAction*)));

	ui.actionDK1->setCheckable(true);
	ui.actionHD_DK->setCheckable(true);
	ui.actionDK1->trigger();

	//	QImage temp("textures/quake2-10.png");
	//	glwidget->setSourceTexture(temp);

	enumerateDistortionMenu("shaders/distortion");
	connect(&distortionDirectory,SIGNAL(directoryChanged(QString)),this,SLOT(enumerateDistortionMenu(QString)));
	distortionDirectory.addPath("shaders/distortion");

	enumerateSourceMenu("shaders/pattern");
	connect(&sourceDirectory,SIGNAL(directoryChanged(QString)),this,SLOT(enumerateSourceMenu(QString)));
	sourceDirectory.addPath("shaders/pattern");


	enumerateTextureMenu("textures/");
	connect(&textureDirectory,SIGNAL(directoryChanged(QString)),this,SLOT(enumerateTextureMenu(QString)));
	textureDirectory.addPath("textures/");

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
		emit changeDistortionShader(QString(""));
	} else 
	{
		QString shader;
		QString filePath("shaders/distortion/");
		filePath.append(action->text());
		filePath.append(".frag");
		QFile file(filePath);
		if(!file.open(QIODevice::ReadOnly)) {
			//QMessageBox::information(0, "error", file.errorString());
			qDebug() << file.errorString();	
		} else {

			QTextStream in(&file);

			while(!in.atEnd()) {
				shader.append( in.readLine());
				shader.append("\n");
			}
			file.close();
		}
		//		QMessageBox::information(0, "info", shader);
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
	if (action == ui.actionSourceNone)
	{
		emit changeSourceShader(QString(""));
	} else 
	{
		QString shader;
		QString filePath("shaders/pattern/");
		filePath.append(action->text());
		filePath.append(".frag");
		QFile file(filePath);
		if(!file.open(QIODevice::ReadOnly)) {
			//QMessageBox::information(0, "error", file.errorString());
			qDebug() << file.errorString();	
		} else {

			QTextStream in(&file);

			while(!in.atEnd()) {
				shader.append( in.readLine());
				shader.append("\n");
			}
			file.close();
		}
		//		QMessageBox::information(0, "info", shader);
		emit changeSourceShader(shader);
	}
}

void DistortionTest::triggeredTexture(QAction *action)
{
	if (action == ui.actionTextureNone)
	{
		emit changeTexture(QImage());
	} else 
	{
		QString shader;
		QString filePath("textures/");
		filePath.append(action->text());
		QImage file(filePath);
		emit changeTexture(file);
	}
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

void DistortionTest::enumerateDistortionMenu(QString path)
{
	ui.actionNone->trigger();

	foreach(QAction *action,distortionMenuActions)
	{
		distortionMenuActions.removeOne(action);
		ui.menuDistortionShader->removeAction(action);
		distortionGroup->removeAction(action);
		delete action;
	}


	QDir directory(path);
	if (!directory.exists())
	{
		qDebug() << "No such path";
		return;
	}
	QStringList filter;
	filter.append("*.frag");
	QFileInfoList files = directory.entryInfoList(filter,QDir::Files);
	QStringList names;
	foreach(QFileInfo file,files)
	{
		names.append(file.baseName());
	}
	names.sort();
	foreach (QString name, names)
	{
		qDebug() << name;
		QAction *temp = new QAction(name,ui.menuDistortionShader);
		temp->setCheckable(true);
		ui.menuDistortionShader->addAction(temp);
		distortionGroup->addAction(temp);
		distortionMenuActions.append(temp);
	}
}


void DistortionTest::enumerateSourceMenu(QString path)
{
	ui.actionSourceNone->trigger();
	foreach(QAction *action,sourceMenuActions)
	{
		sourceMenuActions.removeOne(action);
		ui.menuSourceShader->removeAction(action);
		patternGroup->removeAction(action);
		delete action;
	}


	QDir directory(path);
	if (!directory.exists())
	{
		qDebug() << "No such path";
		return;
	}
	QStringList filter;
	filter.append("*.frag");
	QFileInfoList files = directory.entryInfoList(filter,QDir::Files);
	QStringList names;
	foreach(QFileInfo file,files)
	{
		names.append(file.baseName());
	}
	names.sort();
	foreach (QString name, names)
	{
		qDebug() << name;
		QAction *temp = new QAction(name,ui.menuSourceShader);
		temp->setCheckable(true);
		ui.menuSourceShader->addAction(temp);
		patternGroup->addAction(temp);
		sourceMenuActions.append(temp);
	}
}

void DistortionTest::enumerateTextureMenu(QString path)
{
	ui.actionTextureNone->trigger();

	foreach(QAction *action,textureMenuActions)
	{
		textureMenuActions.removeOne(action);
		ui.menuTexture->removeAction(action);
		textureGroup->removeAction(action);
		delete action;
	}


	QDir directory(path);
	if (!directory.exists())
	{
		qDebug() << "No such path";
		return;
	}
	QStringList filter;
	filter.append("*.jpg");
	filter.append("*.jpeg");
	filter.append("*.png");
	filter.append("*.bmp");
	filter.append("*.gif");
	QFileInfoList files = directory.entryInfoList(filter,QDir::Files);
	QStringList names;
	foreach(QFileInfo file,files)
	{
		names.append(file.fileName());
	}
	names.sort();
	foreach (QString name, names)
	{
		qDebug() << name;
		QAction *temp = new QAction(name,ui.menuTexture);
		temp->setCheckable(true);
		ui.menuTexture->addAction(temp);
		textureGroup->addAction(temp);
		textureMenuActions.append(temp);
	}
}