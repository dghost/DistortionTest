#ifndef OVRSTATUSBAR_H
#define OVRSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>

class StatusWidget : public QStatusBar
{
	Q_OBJECT

public:
	StatusWidget(QWidget *parent);
	~StatusWidget();

public slots:
	void setScreenSize(QSize temp);
	void setDistortionSize(QSize temp);
	void setSourceSize(QSize temp);

private:
	QSize statusSize;
	QLabel screenResolution;
	QLabel distortionResolution;
	QLabel sourceResolution;
};

#endif // OVRSTATUSBAR_H
