#ifndef ChiSquareTest_hpp
#define ChiSquareTest_hpp

#include <QAlgorithm.hpp>
#include <armadillo>
#include <alglib/statistics.h>

namespace UMF {
	class ChiSquareTest;
}

class UMF::ChiSquareTest : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT_LIST(QVector<double>, Distributions)
	QA_OUTPUT(double, Similarity)
	QA_OUTPUT(bool, DistrAreEqual)
	QA_PARAMETER(double, Confidence, double())
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(ChiSquareTest)
	
public:
	void run();
};

#endif /* ChiSquareTest_hpp */
