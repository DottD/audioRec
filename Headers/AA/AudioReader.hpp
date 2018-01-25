#ifndef AudioReader_hpp
#define AudioReader_hpp

#include <QObject>
#include <QVector>
#include <QFile>
#include <QAlgorithm.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace AA {
	class AudioReader;
}

/** Class to help reading audio files.
 This class is a QAlgorithm that wraps the SFML methods to read audio files.
 The only input property is the path to the file; then the user can access
 the output properties to get duration, length, channel count and the right
 samples array.
 */
class AA::AudioReader : public QAlgorithm {
	
	Q_OBJECT
	
	/** Directory pointing to the file to be read. */
	QA_PARAMETER(QString, File, QString())
	/** Array with the samples data. */
	QA_OUTPUT(QVector<double>, Samples)
	/** Total number of channels in the audio file. */
	QA_OUTPUT(quint32, ChannelCount)
	/** Sample rate read from the audio file. */
	QA_OUTPUT(quint32, SampleRate)
	/** Duration of the file in seconds. */
	QA_OUTPUT(double, Duration)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(AudioReader)
	
private:
	static QMutex mutex;
	
public:
	bool perform();
};

#endif /* AudioReader_hpp */
