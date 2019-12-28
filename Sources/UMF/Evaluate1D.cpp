#include <UMF/Evaluate1D.hpp>

void UMF::Evaluate1D::run(){
	Q_ASSERT(!getInX().empty());
	// Perform fitting
	setOutY( evaluate(getCoefficients(), getInX()) );
}

QVector<double> UMF::Evaluate1D::evaluate(QVector<double> C,
										 QVector<double> X) {
	alglib::real_1d_array cvec, xvec;
	cvec.setcontent(C.size(), C.data());
	QVector<double> Y;
	Y.reserve(X.size());
	double func;
	for(const double& x : X){
		xvec.setcontent(1, &x);
		eval(cvec, xvec, func, NULL);
		Y << func;
	}
	return Y;
}

void UMF::EvaluateGaussExp::init(){
	eval = [](const alglib::real_1d_array& cc,
			  const alglib::real_1d_array& xx,
			  double& func,
			  void* ptr) {
		const auto& x = xx[0];
		const auto& nu = cc[0];
		const auto& mu = cc[1];
		const auto& sigma = cc[2];
		const auto& tau = cc[3];
		func = nu * alglib::normaldistribution(( x - mu ) / sigma) * std::exp(- x / tau);
	};
}

void UMF::EvaluateGauss::init(){
	eval = [](const alglib::real_1d_array& c,
			  const alglib::real_1d_array& x,
			  double& func,
			  void* ptr) {
		func = std::exp( - std::pow(x[0]-c[1], 2.0) / (2.0*c[2]*c[2]) );
		func *= c[0];
	};
}

void UMF::Fitting1D::run(){
	Q_ASSERT(getInX().size() == getInY().size());
	const QVector<double>& X = getInX();
	auto Y = getInMoveY();
	// Normalize the vector Y with respect to the sum
	// (useful for fitting, starting with the same parameters)
	auto scaleFactor = 1.0 / std::reduce(Y.begin(), Y.end());
	std::for_each(Y.begin(), Y.end(), [scaleFactor](auto& x){x *= scaleFactor;});
	// Port input to AlgLib compliant array
	alglib::real_2d_array xx;
	alglib::real_1d_array yy;
	xx.setcontent(X.size(), 1, X.data());
	yy.setcontent(Y.size(), Y.data());
	// Initialize other variables
	alglib::real_1d_array c("[5, 0.3, 0.1, 0.1]");
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
	alglib::lsfitsetbc(state, "[0.0, 0.0, 0.0, 0.0]", "[+INF, +INF, +INF, +INF]");
	alglib::lsfitsetcond(state, epsF, epsX, maxIters);
	alglib::ae_int_t info;
	// Perform fitting
	alglib::lsfitfit(state, evaluator->getInternalFunc());
	alglib::lsfitresults(state, info, c, report);
	// Export results
	QVector<double> C({c[0], c[1], c[2], c[3]});
	setOutCoefficients(C);
	setOutAvgError(report.avgerror);
	setOutAvgRelError(report.avgrelerror);
	auto [Left, Right] = std::minmax_element(X.begin(), X.end(), std::less<>{});
	// Normalize the curve to unit integral (this should not be here)
	auto normalizer = CurveNormalization::create();
	normalizer->setLeftExtremum(*Left);
	normalizer->setRightExtremum(*Right);
	normalizer->setInCoefficients(C);
	normalizer->run();
	C = normalizer->getOutMoveCoefficients();
	Q_EMIT fittingReady(C, *Left, *Right, getOutAvgError(), getOutAvgRelError());
}

void UMF::FittingGauss::init(){
	evaluator = EvaluateGauss::create();
}

void UMF::FittingGaussExp::init(){
	evaluator = EvaluateGaussExp::create();
}
