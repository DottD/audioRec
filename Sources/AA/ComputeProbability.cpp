#include <AA/ComputeProbability.hpp>

void AA::ComputeProbability::run(){
	// Initialization
	setOutMatchingScore(-1.0);
	// Check if input is two distributions of equal size
	if(getInIntraX().isEmpty() || getInIntraY().isEmpty() || getInIntraCoefficients().isEmpty() ||
	   getInExtraY().isEmpty() || getInExtraY().isEmpty() || getInExtraCoefficients().isEmpty() ||
	   getInIntraX().size() != getInIntraY().size() ||
	   getInExtraX().size() != getInExtraY().size()){
		qWarning() << printName() << "Check your input";
		return;
	}
	auto normalizer = UMF::CurveNormalization::create({
		{"LeftExtremum", getLeftExtremum()},
		{"RightExtremum", getRightExtremum()}
	});
	auto evaluator = UMF::EvaluateGaussExp::create();
	// Build a function that makes the test, given coefficients and histogram
	auto tester = [this, normalizer, evaluator](QVector<double> X, QVector<double> Y, QVector<double> C)
	{
		// We assume that the observed array is a histogram with integer values that sum up to the total number of events
		// Compute the histogram bar step from the input X vector
		double barStep;
		{
			QVector<double> diff; diff.reserve(X.size()-1);
			std::transform(X.begin()+1, X.end(), X.begin(), std::back_inserter(diff), std::minus<>{});
			barStep = std::reduce(diff.begin(), diff.end()) / diff.size();
		}
		// Compute the input histogram centroid
		double centroid = 0.0;
		{
			auto area = std::reduce(Y.begin(), Y.end()) * barStep;
			std::transform(Y.begin(), Y.end(), X.begin(), Y.begin(), std::multiplies<>{});
			centroid = std::reduce(Y.begin(), Y.end()) * barStep / area;
		}
		// Normalize coefficients
		{
			normalizer->setInCoefficients(std::move(C));
			normalizer->run();
			C = normalizer->getOutMoveCoefficients();
		}
		// Compute the probability
		double probability;
		{
			// Compute the function integral
			auto func = [](double x, double xminusa, double bminusx, double &y, void *ptr){
				auto evaluator = reinterpret_cast<UMF::EvaluateGaussExp*>(ptr);
				evaluator->setInX({x});
				evaluator->run();
				y = evaluator->getOutMoveY().takeFirst();
			};
			auto cast_func = static_cast<void (*)(double, double, double, double&, void*)>(func);
			alglib::autogkstate state;
			alglib::autogkreport report;
			alglib::autogksmooth(getLeftExtremum(), centroid, state);
			// Intra
			evaluator->setCoefficients(C);
			alglib::autogkintegrate(state, cast_func, evaluator.data());
			alglib::autogkresults(state, probability, report);
		}
		return probability;
	};
	// Test intra and extra
	auto intra = tester(getInMoveIntraX(), getInMoveIntraY(), getInMoveIntraCoefficients());
	auto extra = tester(getInMoveExtraX(), getInMoveExtraY(), getInMoveExtraCoefficients());
	setOutMatchingScore((intra+extra)/2.0);
}
