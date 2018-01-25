#include <QApplication>
#include <GUI/Window.hpp>

int main(int argc, char* argv[]){
	QApplication::setApplicationName("CAVA");
	QApplication::setOrganizationName("DMind");
	QApplication app(argc, argv);
	GUI::Window* win = new GUI::Window;
	win->show();
//	QThreadPool::globalInstance()->setMaxThreadCount(1);
	return app.exec();
}
