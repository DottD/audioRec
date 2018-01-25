#ifndef AudioReadExtract_hpp
#define AudioReadExtract_hpp

#include <QAlgorithm.hpp>
#include <AA/AudioReader.hpp>
#include <AA/FeaturesExtractor.hpp>
#include <QVector>

namespace GUI {
	class AudioReadExtract;
}

class GUI::AudioReadExtract : public QAlgorithm {
	
	Q_OBJECT
	
	/** The features computed during the process. */
	QA_OUTPUT(QVector<double>, Features)
	
	/** Directory pointing to the file to be read. */
	QA_PARAMETER(QString, File, QString())
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
	QA_IMPL_CREATE(AudioReadExtract)
	
public:
	bool perform();
	
	private Q_SLOTS:
	Q_SLOT void on_arrayToEmit(QVector<double>);
	
Q_SIGNALS:
	void emitArray(QVector<double>);
};

#endif /* AudioReadExtract_hpp */
