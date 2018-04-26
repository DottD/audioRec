#ifndef Evaluate1D_hpp
#define Evaluate1D_hpp

#include <QVector>
#include <QAlgorithm.hpp>
#include <alglib/interpolation.h>
#include <algorithm>
#include <cmath>

namespace UMF {
	class Evaluate1D;
	class EvaluateGaussExp;
	class EvaluateGauss;
}

class UMF::Evaluate1D : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT(QVector<double>, X)
	QA_OUTPUT(QVector<double>, Y)
	QA_PARAMETER(QVector<double>, Coefficients, QVector<double>())
	
	QA_CTOR_INHERIT
	
public:
	typedef void(*EvalFuncType)(const alglib::real_1d_array&, const alglib::real_1d_array&, double&, void*);
	
protected:
	EvalFuncType eval = Q_NULLPTR;
	
	/** Define the behaviour of eval.
	 Each subclass must reimplement this function; it is called before using
	 the function pointer eval, that evaluates the fitting using the coefficients.
	 */
	virtual void defineEvalFunc() = 0;
	
public:
	void run() final;
	
	QVector<double> evaluate(QVector<double> C,
							 QVector<double> X);
};

class UMF::EvaluateGaussExp : public UMF::Evaluate1D {
	
	Q_OBJECT
	
	QA_IMPL_CREATE(EvaluateGaussExp)
	
protected:
	void defineEvalFunc();
public:
	using Evaluate1D::Evaluate1D;
};

class UMF::EvaluateGauss : public UMF::Evaluate1D {
	
	Q_OBJECT
	
	QA_IMPL_CREATE(EvaluateGauss)
	
protected:
	void defineEvalFunc();
public:
	using Evaluate1D::Evaluate1D;
};

#endif /* Evaluate1D_hpp */
