#include "../Headers/AudioProcess.hpp"

Ui::AudioProcess::AudioProcess() {}

Ui::AudioProcess::AudioProcess(const QString& fileName) {
	this->setFileName(fileName);
}

QString Ui::AudioProcess::getFileName() const {
	return this->fileName;
}

void Ui::AudioProcess::setFileName(const QString &fileName) {
	this->fileName = fileName;
}

void Ui::AudioProcess::run() {
	std::cout << "Hi, from the " << QThread::currentThreadId() << "-th thread!!" << std::endl;
	
	emit processEnded(fileName);
}
