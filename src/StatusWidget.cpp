#include "StatusWidget.h"

StatusWidget::StatusWidget(QWidget *parent)
	: QStatusBar(parent)
{
	this->addPermanentWidget(&resolution);
}

StatusWidget::~StatusWidget()
{

}

void StatusWidget::setSize(QSize temp)
{
	statusSize = temp;
	QString text = QString("%1 x %2").arg(statusSize.width()).arg(statusSize.height());
	resolution.setText(text);
}
