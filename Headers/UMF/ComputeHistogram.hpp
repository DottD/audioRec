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
	QA_PARAMETER(double, BarStep, double())
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(ComputeHistogram)
	
public:
	bool perform();
};

#endif /* ComputeHistogram_hpp */
