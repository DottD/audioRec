#ifndef AudioProcess_hpp
#define AudioProcess_hpp

#include <QApplication>
#include <QDir>
#include <QSharedPointer>
#include <QRunnable>
#include <QThread>
#include <QString>
#include <QVector>
#include <QImage>
#include <QSize>
#include <SFML/Audio/SoundBuffer.hpp>
#include <armadillo>
#include <Headers/functions.hpp>
#include <QDebug>

Q_DECLARE_METATYPE(QSharedPointer<QDir>)

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
	QSharedPointer<QDir> dir;
	QSharedPointer<QImage> corr;
	// Define some constant value
	const int recLength = 16384;
	const unsigned int movAvgRadius = 15;
	const double lowpassFreq = 0;
	const double minFilterFreq = 400;// prev.200
	const double maxFilterFreq = 3500; // prev.2600
	const unsigned int movAvgSpecRad = 7;
	const unsigned int estBackAveRadius = 15;
	const unsigned int estBackMinRadius = 7;
	const unsigned int intervalStartFreq = 500;
	const unsigned int intervalWidthFreq = 1000;
	
public:
	AudioProcess(QObject* parent = Q_NULLPTR) : QObject(parent) {
		qRegisterMetaType<QSharedPointer<QDir>>();
	}; /**< Constructor with parent argument (same as QObject one) */
	
	QDir getFileName() const; /**< Getter for the fileName property */
	void setFileName(QSharedPointer<QDir>); /**< Setter for the fileName property */
	
	/** Reimplementation of the run method.
	 This is the main function of the class: it performs all required operations on the file
	 associated with the class instance.
	 */
	void run();
	
signals:
	void processEnded(QSharedPointer<QDir>); /**< Signal emitted at the end of the run method */
};

#endif /* AudioProcess_hpp */
