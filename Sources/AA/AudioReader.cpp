#include <AA/AudioReader.hpp>

QMutex AA::AudioReader::mutex;

bool AA::AudioReader::perform(){
	// Check if the file exists
	if(!QFile(getFile()).exists()) abort("File not found");
	// Read the audio file associated with this class instance
	sf::SoundBuffer buffer;
	AudioReader::mutex.lock();
	bool answer = buffer.loadFromFile(getFile().toStdString());
	AudioReader::mutex.unlock();
	if (answer){
		// Read samples and store in the corresponding QVector
		QVector<double> samples;
		samples.reserve(buffer.getSampleCount());
		const qint16* bufferPtr = buffer.getSamples();
		for (quint64 k = 0; k < buffer.getSampleCount(); k++) samples << double(*(bufferPtr++)) / double(0x7fff);
		// Set sound attributes
		setOutSamples(samples);
		setOutChannelCount(buffer.getChannelCount());
		setOutSampleRate(buffer.getSampleRate());
		setOutDuration(double(samples.size()) / double(buffer.getSampleRate()));
		return true;
	} else {
		return false;
	}
}
