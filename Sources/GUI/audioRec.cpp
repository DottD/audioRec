#include <QApplication>
#include <GUI/audioRecMainWin.hpp>

int main(int argc, char* argv[]){
	/* Inizializzo e lancio l'interfaccia dei parametri in Qt */
	QApplication app(argc, argv);
	Ui::AudioRecMainWin MainWin;
	MainWin.show();
	
	return app.exec();
}
