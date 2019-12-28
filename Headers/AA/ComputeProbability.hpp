#ifndef ComputeProbability_hpp
#define ComputeProbability_hpp

#include <QAlgorithm.hpp>
#include <UMF/Evaluate1D.hpp>
#include <alglib/integration.h>

namespace AA {
	class ComputeProbability : public QAlgorithm {
		
		Q_OBJECT
		
		QA_INPUT(QVector<double>, IntraX)
		QA_INPUT(QVector<double>, IntraY)
		QA_INPUT(QVector<double>, IntraCoefficients)
		
		QA_INPUT(QVector<double>, ExtraX)
		QA_INPUT(QVector<double>, ExtraY)
		QA_INPUT(QVector<double>, ExtraCoefficients)
		
		QA_PARAMETER(double, LeftExtremum, 0.0)
		QA_PARAMETER(double, RightExtremum, 1.0)
		
		QA_OUTPUT(double, MatchingScore)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(ComputeProbability)
		
	public:
		void run();
	};
}

#endif /* ComputeProbability_hpp */
