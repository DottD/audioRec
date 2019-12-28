#ifndef FeaturesExtractor_hpp
#define FeaturesExtractor_hpp

#include <QScopedPointer>
#include <QAlgorithm.hpp>
#include <UMF/SignalProcessing.hpp>
#include <SFML/Audio/InputSoundFile.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <TMath.h>
#include <ROOT/TSeq.hxx>
#include <TSpectrum.h>
#include <TVector.h>
#include <TH1D.h>
#include <TF1.h>
#include <alglib/fasttransforms.h>

namespace AA {
	class FeaturesExtractor;
}

/** Class to process an audio file supporting multithreading.
 This class is both a QObject and a QRunnable, so that it can be executed in QThreadPool,
 but it is also able to signal that the process has ended, and that some notable series
 has been computed, allowing them to be exploited in other threads.
 */
class AA::FeaturesExtractor : public QAlgorithm {
	
	Q_OBJECT
	
	/** Sample rate of the audio input. */
	QA_OUTPUT(double, SampleRate)
	/** Total number of records in the file. */
	QA_OUTPUT(int, TotalRecords)
	/** Record length in samples */
	QA_OUTPUT(int, RecordLength)
	/** The features computed during the process. */
	QA_OUTPUT(QVector<double>, Features)
	
	/** Directory pointing to the file to be read. */
	QA_PARAMETER(QString, File, QString())
	/** Record index to be evaluated. */
	QA_PARAMETER(int, SelectRecord, -1)
	/** Minimum frequency allowed */
	QA_PARAMETER(double, MinimumFrequency, 200.0)
	/** Maximum frequency allowed */
	QA_PARAMETER(double, MaximumFrequency, 4000.0)
	/** Maximum allowed spectrum leakage.
	 This parameter will affect the record length. Namely,
	 a larger value will reduce the record length, thus speeding up the algorithm
	 and, on the other hand, will increase the spectrum leakage, as described
	 in <a href="http://www.jot.fm/issues/issue_2009_11/column2/">Douglas A. Lyon: “The Discrete Fourier Transform, Part 4: Spectral Leakage”</a>.
	 */
	QA_PARAMETER(double, MaximumSpectrumLeakage, 10.0)
	/** Operation performed to reduce channles.
	 @sa UMF::ReduceChannels, UMF::ReduceChannels::operation
	 */
	QA_PARAMETER(int, ChannelsOperation, UMF::ReduceChannels::average)
	/** How channels are arranged in the input file.
	 @sa UMF::ReduceChannels, UMF::ReduceChannels::channels
	 */
	QA_PARAMETER(int, ChannelsArrangement, UMF::ReduceChannels::interleaved)
	/** Function used for windowing.
	 @sa UMF::Windowing, UMF::Windowing::function
	 */
	QA_PARAMETER(int, WindowingFunction, UMF::Windowing::hann)
	/** Border extrapolation method.
	 @sa UMF::ArrayPad, UMF::ArrayPad::border_type
	 */
	QA_PARAMETER(int, ExtrapolationMethod, UMF::ArrayPad::constant)
	/** Width of the Gaussian filter applied to the input signal.
	 @sa UMF::GaussianFilter
	 */
	QA_PARAMETER(int, GaussianFilterWidth, 5)
	/** Number of iterations for background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(int, BackIterations, 6)
	/** Direction used for background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(int, BackDirection, TSpectrum::kBackIncreasingWindow)
	/** Filter order used for background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(int, BackFilterOrder, TSpectrum::kBackOrder2)
	/** Whether applying a smoothing in background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(bool, BackSmoothing, false)
	/** Smoothing window size used for background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(int, BackSmoothWindow, TSpectrum::kBackSmoothing3)
	/** Whether computing Compton edges in background suppression.
	 @sa UMF::SpectrumRemoveBackground
	 */
	QA_PARAMETER(bool, BackCompton, false)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(FeaturesExtractor)
	
public:
	void run();
	
Q_SIGNALS:
	Q_SIGNAL void timeSeries(QVector<double>);
	Q_SIGNAL void frequencySeries(QVector<double>);
	Q_SIGNAL void pointSeries(QVector<int>);
	
private:
	static sf::Mutex mutex;
};

#endif /* FeaturesExtractor_hpp */
