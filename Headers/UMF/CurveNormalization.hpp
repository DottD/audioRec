#ifndef CurveNormalization_hpp
#define CurveNormalization_hpp

#include <QAlgorithm.hpp>
#include <UMF/Evaluate1D.hpp>
#include <armadillo>
#include <alglib/integration.h>

namespace UMF {
	class CurveNormalization;
}

class UMF::CurveNormalization : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT(QVector<double>, Coefficients)
	QA_PARAMETER(double, LeftExtremum, 0.0)
	QA_PARAMETER(double, RightExtremum, 1.0)
	QA_OUTPUT(QVector<double>, Coefficients)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(CurveNormalization)
	
public:
	void run();
};

#endif /* CurveNormalization_hpp */
