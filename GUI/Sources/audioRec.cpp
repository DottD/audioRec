#include <QApplication>
#include <Headers/audioRecMainWin.hpp>

using namespace std;

int main(int argc, char* argv[]){
	/* Inizializzo e lancio l'interfaccia dei parametri in Qt */
	QApplication Application(argc, argv);
	Ui::AudioRecMainWin MainWin;
	MainWin.show();
	
	return Application.exec();
}
