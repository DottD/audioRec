#ifndef Evaluate1D_hpp
#define Evaluate1D_hpp

#include <QVector>
#include <alglib/ap.h>
#include <alglib/interpolation.h>
#include <algorithm>
#include <cmath>
#include <QAlgorithm.hpp>
#include <UMF/CurveNormalization.hpp>

namespace UMF {
	
	class Evaluate1D : public QAlgorithm {
		
		Q_OBJECT
		
		QA_INPUT(QVector<double>, X)
		QA_OUTPUT(QVector<double>, Y)
		QA_PARAMETER(QVector<double>, Coefficients, QVector<double>())
		
		QA_CTOR_INHERIT
		
	public:
		typedef void(*EvalFuncType)(const alglib::real_1d_array&, const alglib::real_1d_array&, double&, void*);
		
	protected:
		EvalFuncType eval = Q_NULLPTR;
		
	public:
		void run() final;
		
		EvalFuncType getInternalFunc() const {return eval;};
		
		QVector<double> evaluate(QVector<double> C,
								 QVector<double> X);
	};
	
	class EvaluateGaussExp : public Evaluate1D {
		
		Q_OBJECT
		
		QA_IMPL_CREATE(EvaluateGaussExp)
		
	public:
		using Evaluate1D::Evaluate1D;
		
		void init();
	};
	
	class EvaluateGauss : public Evaluate1D {
		
		Q_OBJECT
		
		QA_IMPL_CREATE(EvaluateGauss)
		
	public:
		using Evaluate1D::Evaluate1D;
		
		void init();
	};
	
	class Fitting1D : public QAlgorithm {
		
		Q_OBJECT
		
		QA_INPUT(QVector<double>, X)
		QA_INPUT(QVector<double>, Y)
		QA_OUTPUT(QVector<double>, Coefficients)
		QA_OUTPUT(double, AvgError)
		QA_OUTPUT(double, AvgRelError)
		
		QA_CTOR_INHERIT
		
	protected:
		QSharedPointer<Evaluate1D> evaluator;
		
	public:
		void run();
		
	Q_SIGNALS:
		Q_SIGNAL void fittingReady(QVector<double> Parameters,
								   double min, double max,
								   double avgerr, double avgrelerr);
	};
	
	class FittingGauss: public Fitting1D {
		
		Q_OBJECT
		
		QA_IMPL_CREATE(FittingGauss)
		
	public:
		using Fitting1D::Fitting1D;
		
		void init();
	};
	
	class FittingGaussExp: public Fitting1D {
		
		Q_OBJECT
		
		QA_IMPL_CREATE(FittingGaussExp)
		
	public:
		using Fitting1D::Fitting1D;
		
		void init();
	};
}

#endif /* Evaluate1D_hpp */
