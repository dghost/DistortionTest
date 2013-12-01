#include "StatusWidget.h"

StatusWidget::StatusWidget(QWidget *parent)
	: QStatusBar(parent)
{
	this->addPermanentWidget(&screenResolution);
	this->addPermanentWidget(&distortionResolution);	
	this->addPermanentWidget(&sourceResolution);
}

StatusWidget::~StatusWidget()
{

}

void StatusWidget::setScreenSize(QSize temp)
{
	statusSize = temp;
	QString text = QString("Screen: %1 x %2").arg(statusSize.width()).arg(statusSize.height());
	screenResolution.setText(text);
}

void StatusWidget::setSourceSize(QSize temp)
{
	statusSize = temp;
	QString text = QString("Source: %1 x %2").arg(statusSize.width()).arg(statusSize.height());
	sourceResolution.setText(text);
}

void StatusWidget::setDistortionSize(QSize temp)
{
	statusSize = temp;
	QString text = QString("Distortion: %1 x %2").arg(statusSize.width()).arg(statusSize.height());
	distortionResolution.setText(text);
}
