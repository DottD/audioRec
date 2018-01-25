#include <UMF/Fitting1D.hpp>

bool UMF::Fitting1D::perform(){
	Q_ASSERT(getInX().size() == getInY().size());
	const QVector<double>& X = getInX();
	const QVector<double>& Y = getInY();
	// Port input to AlgLib compliant array
	alglib::real_2d_array xx;
	alglib::real_1d_array yy;
	xx.setcontent(X.size(), 1, X.data());
	yy.setcontent(Y.size(), Y.data());
	// Initialize other variables
	QVector<double> C({0.2, 0.036, 0.1, 0.2});
	alglib::real_1d_array c;
	c.setcontent(C.size(), C.data());
	alglib::lsfitstate state;
	alglib::lsfitreport report;
	// Compute two halting parameters
	auto P = std::minmax_element(X.constBegin(), X.constEnd());
	double epsF = 1e-10 * (*(P.second)-*(P.first));
	double epsX = 1e-10;
	const double diffStep = 0.001;
	alglib::ae_int_t maxIters = 0;
	// Initialize the algorithm
	alglib::lsfitcreatef(xx, yy, c, diffStep, state);
	alglib::lsfitsetcond(state, epsF, epsX, maxIters);
	alglib::ae_int_t info;
	// Perform fitting
	defineEvalFunc();
	alglib::lsfitfit(state, eval);
	alglib::lsfitresults(state, info, c, report);
	// C should have been modified during processing, as it shares memory with c
	setOutCoefficients(C);
	return true;
}

QVector<double> UMF::Fitting1D::evaluate(QVector<double> C,
										 QVector<double> X) {
	alglib::real_1d_array cvec, xvec;
	cvec.setcontent(C.size(), C.data());
	QVector<double> Y;
	Y.reserve(X.size());
	double func;
	defineEvalFunc();
	for(const double& x : X){
		xvec.setcontent(1, &x);
		eval(cvec, xvec, func, NULL);
		Y << func;
	}
	return Y;
}

void UMF::FittingGaussExp::defineEvalFunc(){
	eval = [](const alglib::real_1d_array& cc,
			  const alglib::real_1d_array& xx,
			  double& func,
			  void* ptr) {
		// Take useful references
		const double& c = cc[0];
		const double& a = cc[1];
		const double& s = cc[2];
		const double& t = cc[3];
		const double& x = xx[0];
		// Useful quantities
		double c_x = c-x;
		double c_xt = c_x * t;
		double s2 = s * s;
		double A = s2 + c_xt;
		double B = A/(M_SQRT2 * s * t);
		double _2t = 2 * t;
		double C = (A + c_xt)/(_2t * t);
		double a_2t = a / _2t;
		double erfcB = std::erfc(B);
		double expC = std::exp(C);
		// Compute the function value
		func = a_2t * expC * erfcB;
		double t2 = t * t;
		func = a_2t * std::exp(s2/(2*t2) + c_x/t) * (1 + std::erf((-c_x/s - s/t)/M_SQRT2));
	};
}

void UMF::FittingGauss::defineEvalFunc(){
	eval = [](const alglib::real_1d_array& c,
			  const alglib::real_1d_array& x,
			  double& func,
			  void* ptr) {
		func = std::exp( - std::pow(x[0]-c[1], 2.0) / (2.0*c[2]*c[2]) );
		func *= c[0];
	};
}