#include <UMF/ComputeHistogram.hpp>

void UMF::ComputeHistogram::run(){
	// Declare output variables
	QVector<double> X, Y;
	// Take input
	auto values = getInMoveValues();
	if (values.isEmpty()){
		qWarning() << printName() << "Cannot compute histogram: values empty!";
	}
	// Create an arma header over the input vector
	arma::vec data(values.data(), values.size(), false, true);
	// Filter the elements out of range
	if(getMinimumValue() >= 0.0 && getMaximumValue() >= 0.0 && getMaximumValue() > getMinimumValue()){
		// left is the first >= min
		// if not found, that is left == data.end(), all values are less than the required minimum
		// hence data must be returned empty.
		auto left = std::lower_bound(data.begin(), data.end(), getMinimumValue());
		if(left != data.end()){
			// Truncate the histogram to the given maximum value
			auto max = std::min(data.max(), getMaximumValue());
			// right is the first > max
			// if not found, that is right == data.end(), all values are less than or equal to the maximum,
			// so we can use right anyway as a range upper bound without further considerations.
			auto right = std::upper_bound(data.begin(), data.end(), max);
			// Move the new range to the left, until the beginning of data array
			std::move(left, right, data.begin());
			// Resize the data array to the new size (crop the last unused part)
			data.resize(std::distance(left, right));
			// Create a set of equally spaced points (they are the bar centers)
			arma::vec x = arma::regspace(getMinimumValue(), getBarStep(), max)+getBarStep()/2.0;
			// Compute the histogram
			arma::vec y = arma::conv_to<arma::vec>::from(arma::hist(data, x));
			// Set the output
			X.reserve(x.size());
			Y.reserve(y.size());
			{
				auto xit = std::make_move_iterator(x.begin());
				auto yit = std::make_move_iterator(y.begin());
				auto xit_end = std::make_move_iterator(x.end());
				auto yit_end = std::make_move_iterator(y.end());
				for(; xit != xit_end && yit != yit_end; ++xit, ++yit){
					auto yval = *yit;
					if(!getSuppressZeroCount() || yval > 0.){
						X << *xit;
						Y << std::move(yval);
					}
				}
			}
			X.squeeze();
			Y.squeeze();
		}
		// if the condition is not met the output variables are left empty
		setOutHistX(X);
		setOutHistY(Y);
		Q_EMIT histogramReady(X, Y);
	}
}
