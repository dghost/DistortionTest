#include "DistortionTest.h"
#include <QtWidgets/QApplication>
#include "GL/glew.h"

int main(int argc, char *argv[])
{
	glewInit();
	QApplication a(argc, argv);
	DistortionTest w;
	w.show();
	return a.exec();
}
