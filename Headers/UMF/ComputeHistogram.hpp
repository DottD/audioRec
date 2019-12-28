#ifndef ComputeHistogram_hpp
#define ComputeHistogram_hpp

#include <QAlgorithm.hpp>
#include <armadillo>

namespace UMF {
	class ComputeHistogram;
}

class UMF::ComputeHistogram : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT_VEC(double, Values)
	QA_OUTPUT(QVector<double>, HistX)
	QA_OUTPUT(QVector<double>, HistY)
	QA_PARAMETER(double, BarStep, 0.02)
	QA_PARAMETER(double, MinimumValue, 0.0)
	QA_PARAMETER(double, MaximumValue, 2.0)
	QA_PARAMETER(bool, SuppressZeroCount, true)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(ComputeHistogram)
	
public:
	void run();
	
Q_SIGNALS:
	Q_SIGNAL void histogramReady(QVector<double> Bin, QVector<double> Count);
};

#endif /* ComputeHistogram_hpp */
