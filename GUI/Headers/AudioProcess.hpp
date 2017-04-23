#ifndef AudioProcess_hpp
#define AudioProcess_hpp

#include <QRunnable>
#include <QThread>
#include <QString>
#include <iostream> // to be removed in the final version

namespace Ui {
	class AudioProcess;
}

/** Class to process an audio file supporting multithreading.
 This class is both a QObject and a QRunnable, so that it can be executed in QThreadPool,
 but it is also able to signal that the process has ended.
 */
class Ui::AudioProcess : public QObject, public QRunnable {
	Q_OBJECT
	
private:
	QString fileName;
	
public:
	/** Empty constructor.
	 This constructor simply do nothing; before running the process a fileName must be provided.
	 */
	AudioProcess();
	
	/** Constructor with given file name.
	 This constructor create an instance with the associated file name.
	 @param [in] fileName The path to the file to be processed.
	 */
	AudioProcess(const QString& fileName);
	
	QString getFileName() const; /**< Getter for the fileName property */
	void setFileName(const QString& fileName); /**< Setter for the fileName property */
	
	/** Reimplementation of the run method.
	 This is the main function of the class: it performs all required operations on the file
	 associated with the class instance.
	 */
	void run();
	
signals:
	void processEnded(QString); /**< Signal emitted at the end of the run method */
};

#endif /* AudioProcess_hpp */
