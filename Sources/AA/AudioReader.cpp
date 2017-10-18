#include <AA/AudioReader.hpp>

QMutex Ui::AudioReader::mutex;

Ui::AudioReader::AudioReader(){
	samples = QSharedPointer<QVector<double>>(new QVector<double>());
}

bool Ui::AudioReader::loadFile(QSharedPointer<QDir> dir){
	if (dir.isNull()) return false;
	// Read the audio file associated with this class instance
	sf::SoundBuffer buffer;
	AudioReader::mutex.lock();
	if (!buffer.loadFromFile(dir->absolutePath().toStdString())) {AudioReader::mutex.unlock();return false;}
	AudioReader::mutex.unlock();
	// Read samples and store in the corresponding QVector
	samples->reserve(buffer.getSampleCount());
	const qint16* bufferPtr = buffer.getSamples();
	for (quint64 k = 0; k < buffer.getSampleCount(); k++) samples->append(double(*(bufferPtr++)) / double(0x7fff));
	// Set sound attributes
	channelCount = buffer.getChannelCount();
	sampleRate = buffer.getSampleRate();
	
	return true;
}

QSharedPointer<QVector<double>> Ui::AudioReader::getSamplesPtr(){
	return samples;
}

QSharedPointer<QVector<double>> Ui::AudioReader::getSamplesPtr() const{
	return samples;
}

QVector<double> Ui::AudioReader::getSamples(){
	return *samples;
}

double Ui::AudioReader::getDuration(){
	return double(samples->size()) / double(sampleRate);
}

quint64 Ui::AudioReader::getSampleCount(){
	return samples->size();
}

quint32 Ui::AudioReader::getSampleRate(){
	return sampleRate;
}

quint32 Ui::AudioReader::getChannelCount(){
	return channelCount;
}
