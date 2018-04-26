#include <UMF/ComputeHistogram.hpp>

void UMF::ComputeHistogram::run(){
	const QVector<double> values = getInValues();
	if (values.isEmpty()){
		qWarning("%s", "Cannot compute histogram: values empty!");
	}
	// Create an arma header over the input vector
	const arma::vec data(values.data(), values.size());
	// Preallocate the right memory in two QVectors, then create an arma header over them.
	// Then converse would cause copy
	double min = data.min();
	double max = data.max();
	int n_elems = (max-min)/getBarStep()+1;
	arma::vec x( n_elems );
	arma::vec y( n_elems );
	// Compute the histogram
	x = arma::regspace(data.min(), getBarStep(), data.max());
	y = arma::normalise(arma::conv_to<arma::vec>::from(arma::hist(data, x)), 1);
	// Set the output
	QVector<double> X, Y;
	X.reserve(x.size());
	Y.reserve(y.size());
	for(double& elx: x) X << elx;
	for(double& ely: y) Y << ely;
	setOutHistX(X);
	setOutHistY(Y);
}
