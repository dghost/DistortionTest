#include "DistortionTest.h"
#include "RenderWidget.h"

#include <QMessageBox>
#include <QDir>
#include <QDateTime>

DistortionTest::DistortionTest(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	distortionPath = DIRECTORY_SHADER_DISTORTION;
	sourceShaderPath = DIRECTORY_SHADER_SOURCE;
	sourceTexturePath = DIRECTORY_TEXTURE_SOURCE;

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

	enumerateDistortionMenu();
	enumerateSourceMenu();
	enumerateTextureMenu();

	connect(&directoryWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(handleDirectoryChange(QString)));

	directoryWatcher.addPath(distortionPath);
	directoryWatcher.addPath(sourceShaderPath);
	directoryWatcher.addPath(sourceTexturePath);

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
		QString filePath(distortionPath);
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
		QString filePath(sourceShaderPath);
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

void DistortionTest::enumerateDistortionMenu()
{
	QDir directory(distortionPath);
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

	distortionGroup->blockSignals(true);
	
	foreach (QString name, names)
	{
		qDebug() << name;
		if (!distortionMenuActions.contains(name))
		{
			QAction *temp = new QAction(name,distortionGroup);
			temp->setCheckable(true);
			distortionMenuActions[name] = temp;
		}
	}

	QAction *currentAction = distortionGroup->checkedAction();
	foreach(QString key, distortionMenuActions.keys())
	{
		if (!names.contains(key))
		{
			QAction *temp = distortionMenuActions[key];
			if (distortionGroup->checkedAction() == temp)
				currentAction = ui.actionNone;
			distortionMenuActions.remove(key);
			distortionGroup->removeAction(temp);
			delete temp;
		}
	}
	ui.menuDistortionShader->clear();
	ui.menuDistortionShader->addAction(ui.actionNone);
	ui.menuDistortionShader->addActions(distortionMenuActions.values());
	distortionGroup->blockSignals(false);
	if (currentAction == ui.actionNone)
		currentAction->trigger();
}


void DistortionTest::enumerateSourceMenu()
{
	QDir directory(sourceShaderPath);
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

	patternGroup->blockSignals(true);
	
	foreach (QString name, names)
	{
		qDebug() << name;
		if (!sourceMenuActions.contains(name))
		{
			QAction *temp = new QAction(name,patternGroup);
			temp->setCheckable(true);
			sourceMenuActions[name] = temp;
		}
	}

	QAction *currentAction = patternGroup->checkedAction();
	foreach(QString key, sourceMenuActions.keys())
	{
		if (!names.contains(key))
		{
			QAction *temp = sourceMenuActions[key];
			if (patternGroup->checkedAction() == temp)
				currentAction = ui.actionSourceNone;
			sourceMenuActions.remove(key);
			patternGroup->removeAction(temp);
			delete temp;
		}
	}
	ui.menuSourceShader->clear();
	ui.menuSourceShader->addAction(ui.actionSourceNone);
	ui.menuSourceShader->addActions(sourceMenuActions.values());
	patternGroup->blockSignals(false);
	if (currentAction == ui.actionSourceNone)
		currentAction->trigger();
}

void DistortionTest::enumerateTextureMenu()
{
	QDir directory(sourceTexturePath);
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

	
	textureGroup->blockSignals(true);
	
	foreach (QString name, names)
	{
		qDebug() << name;
		if (!textureMenuActions.contains(name))
		{
			QAction *temp = new QAction(name,textureGroup);
			temp->setCheckable(true);
			textureMenuActions[name] = temp;
		}
	}

	QAction *currentAction = textureGroup->checkedAction();
	foreach(QString key, textureMenuActions.keys())
	{
		if (!names.contains(key))
		{
			QAction *temp = textureMenuActions[key];
			if (textureGroup->checkedAction() == temp)
				currentAction = ui.actionTextureNone;
			textureMenuActions.remove(key);
			textureGroup->removeAction(temp);
			delete temp;
		}
	}
	ui.menuTexture->clear();
	ui.menuTexture->addAction(ui.actionTextureNone);
	ui.menuTexture->addActions(textureMenuActions.values());
	textureGroup->blockSignals(false);
	if (currentAction == ui.actionTextureNone)
		currentAction->trigger();
}


void DistortionTest::handleDirectoryChange(QString path)
{
	if (path == distortionPath)
		enumerateDistortionMenu();
	else if (path == sourceShaderPath)
		enumerateSourceMenu();
	else if (path == sourceTexturePath)
		enumerateTextureMenu();
}