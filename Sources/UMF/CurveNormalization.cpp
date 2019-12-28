#include <UMF/CurveNormalization.hpp>

void UMF::CurveNormalization::run(){
	// Create an evaluator instance
	auto evaluator = UMF::EvaluateGaussExp::create();
	evaluator->setCoefficients(getInCoefficients());
	// Compute the function integral
	auto func = [](double x, double xminusa, double bminusx, double &y, void *ptr){
		auto evaluator = reinterpret_cast<UMF::EvaluateGaussExp*>(ptr);
		evaluator->setInX({x});
		evaluator->run();
		y = evaluator->getOutMoveY().takeFirst();
	};
	auto cast_func = static_cast<void (*)(double, double, double, double&, void*)>(func);
	alglib::autogkstate state;
	double value;
	alglib::autogkreport report;
	alglib::autogksmooth(getLeftExtremum(), getRightExtremum(), state);
	alglib::autogkintegrate(state, cast_func, evaluator.data());
	alglib::autogkresults(state, value, report);
	// Normalization
	auto coeff = getInMoveCoefficients();
	if(value != 0.0) coeff[0] /= value;
	setOutCoefficients(std::move(coeff));
}
