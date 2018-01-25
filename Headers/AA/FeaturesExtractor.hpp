#ifndef FeaturesExtractor_hpp
#define FeaturesExtractor_hpp

#include <QScopedPointer>
#include <QAlgorithm.hpp>
#include <UMF/functions.hpp>
#include <AA/AudioReader.hpp>

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
	
	/** Samples read from audio file. */
	QA_INPUT(QVector<double>, Samples)
	QA_INPUT(quint32, SampleRate)
	/** The features computed during the process. */
	QA_OUTPUT(QVector<double>, Features)
	
	QA_PARAMETER(int, SelectRecord, int())
	/** Minimum frequency allowed */
	QA_PARAMETER(double, MinFreq, double())
	/** Maximum frequency allowed */
	QA_PARAMETER(double, MaxFreq, double())
	/** Oversampling */
	QA_PARAMETER(int, Oversampling, int())
	/** Radius of the minimum filter used in the background estimation stage */
	QA_PARAMETER(int, BEMinFilterRad, int())
	/** Maximum peak width allowed during background estimation */
	QA_PARAMETER(double, BEMaxPeakWidth, double())
	/** Derivative estimation diameter used during the background estimation */
	QA_PARAMETER(int, BEDerivDiam, int())
	/** Maximum number of iterations performed in the background estimation */
	QA_PARAMETER(int, BEMaxIterations, int())
	/** Maximum inconsistency allowed, used as halting condition in b.e. */
	QA_PARAMETER(double, BEMaxInconsistency, double())
	/** Maximum distance between the nodes (b.e.) */
	QA_PARAMETER(int, BEMaxDistNodes, int())
	/** Size of the Butterworth filter tail */
	QA_PARAMETER(double, ButtFilterTail, double())
	QA_PARAMETER(double, PeakRelevance, double())
	QA_PARAMETER(double, PeakMinVarInfluence, double())
	QA_PARAMETER(int, BinWidth, int())
	QA_PARAMETER(double, PeakHeightThreshold, double())
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(FeaturesExtractor)
	
public:
	bool perform();
	
Q_SIGNALS:
	void emitArray(QVector<double>);
};

#endif /* FeaturesExtractor_hpp */
