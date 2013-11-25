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
	void setSize(QSize temp);


private:
	QSize statusSize;
	QLabel resolution;
};

#endif // OVRSTATUSBAR_H
