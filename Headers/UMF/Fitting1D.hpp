#ifndef Fitting1D_hpp
#define Fitting1D_hpp

#include <QVector>
#include <QAlgorithm.hpp>
#include <alglib/interpolation.h>
#include <algorithm>
#include <cmath>

namespace UMF {
	class Fitting1D;
	class FittingGaussExp;
	class FittingGauss;
}

class UMF::Fitting1D : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT(QVector<double>, X)
	QA_INPUT(QVector<double>, Y)
	QA_OUTPUT(QVector<double>, Coefficients)
	
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

class UMF::FittingGaussExp : public UMF::Fitting1D {
	
	Q_OBJECT
	
	QA_IMPL_CREATE(FittingGaussExp)
	
protected:
	void defineEvalFunc();
public:
	using Fitting1D::Fitting1D;
};

class UMF::FittingGauss : public UMF::Fitting1D {
	
	Q_OBJECT
	
	QA_IMPL_CREATE(FittingGauss)
	
protected:
	void defineEvalFunc();
public:
	using Fitting1D::Fitting1D;
};

#endif /* Fitting1D_hpp */
