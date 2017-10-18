#ifndef AudioReader_hpp
#define AudioReader_hpp

#include <QSharedPointer>
#include <QVector>
#include <QDir>
#include <SFML/Audio/SoundBuffer.hpp>

namespace Ui {
	class AudioReader;
}

/** Class to help reader audio files.
 This class is a wrapper for SFML methods to read audio files. The only method
 to call to load a file is loadFile, after the class has been initialized.
 To retrieve information and data from this class, you can use the getters
 methods provided.
 */
class Ui::AudioReader {
private:
	static QMutex mutex;
	
	QSharedPointer<QVector<double>> samples; /**< Array with the samples data. */
	quint32 channelCount; /**< Total number of channels in the audio file. */
	quint32 sampleRate; /**< Sample rate read from the audio file. */
	
	public slots:
	/** Loads the audio file with given file name.
	 This function fill the samples attribute with the content of the audio file with
	 the specified name. This is the slot/function that should be called after initialization.
	 */
	bool loadFile(QSharedPointer<QDir> dir);
	
public:
	AudioReader();
	
	/** Returns a shared pointer to the samples array.
	 This function returns a QSharedPointer to the samples array, and no data is copied in the process.
	 @return QSharedPointer to samples data.
	 @sa getSamples(), getSamplesPtr() const
	 */
	QSharedPointer<QVector<double>> getSamplesPtr();
	
	/** Returns a const shared pointer to the samples array.
	 This function returns a const QSharedPointer to the samples array, and no data is copied in the process.
	 @return const QSharedPointer to samples data.
	 @sa getSamples(), getSamplesPtr()
	 */
	QSharedPointer<QVector<double>> getSamplesPtr() const;
	
	/** Returns the samples array.
	 This function copies the samples data into a newly allocated QVector<double>.
	 @return Samples data as QVector<double>.
	 @sa getSamplesPtr(), getSamplesPtr() const
	 */
	QVector<double> getSamples();
	
	/** Returns the total duration of the sound in seconds.
	 @return Duration of the sound in seconds.
	 @sa getSampleCount(), getSampleRate(), getChannelCount()
	 */
	double getDuration();
	
	/** Returns the total number of samples.
	 This function returns the total number of samples, and it is equivalent to call samples->size().
	 @return Total number of samples.
	 @sa getDuration(), getSampleRate(), getChannelCount()
	 */
	quint64 getSampleCount();
	
	/** Returns the sampling rate of the sound.
	 @return Sampling rate of the sound.
	 @sa getDuration(), getSampleCount(), getChannelCount()
	 */
	quint32 getSampleRate();
	
	/** Returns the total number of channels.
	 @return Total number of channels.
	 @sa getDuration(), getSampleCount(), getSampleRate()
	 */
	quint32 getChannelCount();
};

#endif /* AudioReader_hpp */
