#include <QApplication>
#include <Headers/Application.hpp>
#include <Headers/audioRecMainWin.hpp>

int main(int argc, char* argv[]){
	/* Inizializzo e lancio l'interfaccia dei parametri in Qt */
	Ui::Application app(argc, argv);
	Ui::AudioRecMainWin MainWin;
	MainWin.show();
	
	return app.exec();
}
