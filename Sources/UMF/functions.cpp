#include <UMF/functions.hpp>

using namespace arma;

/* Gives an array with m 0s of padding on every side.
 
 If border is given (not constant), uses the specified padding scheme. */
vec arrayPad(const vec& array,
			 const unsigned int& m,
			 const border_type& bd){
	// Construct an array with 2*m more element than the input one
	vec output(array.size()+2*m);
	// Fill the central part with the input array
	output.rows(m, array.size()-1+m) = array;
	// Correctly fill the output array
	if (bd == border_type::constant){
		// Pad with m zeros on both sides
		output.rows(0, m-1) = zeros<vec>(m);
		output.rows(array.size()+m, output.size()-1) = zeros<vec>(m);
	} else {
		for(unsigned int k = 0; k < m; k++){
			output(k) = array(borderInterpolate(k-m, array.size(), bd));
			output(array.size()+m+k) = array(borderInterpolate(array.size()+k, array.size(), bd));
		}
	}
	return output;
}

/* Computes the moving average of the input array.
 The peculiarity of this function is that it doesn't reduce
 the array length, because of the initial array padding.
 
 The padding method can be chosen through the parameter bd. */
vec movingAverage(const vec& array,
						const unsigned int& m,
						const border_type& bd){
	vec padded = arrayPad(array, m, bd);
	vec kernel(2*m+1);
	kernel.fill(1.0/kernel.size());
	vec filtered = conv(padded, kernel, "same");
	return filtered.rows(m, m+array.size()-1);
}

vec gaussianFilter(const vec& array,
						 const unsigned int& m,
						 const border_type& bd){
	vec padded = arrayPad(array, m, bd);
	vec kernel = exp( - square(linspace(0, 2*m, 2*m+1) - m) / double(m*m) * 2.0 );
	kernel = arma::normalise(kernel, 1/*1-norm*/);
	vec filtered = conv(padded, kernel, "same");
	return filtered.rows(m, m+array.size()-1);
}

vec movingStdDev(const vec& signal,
				 const unsigned int& m,
				 const border_type& bd){
	// Compute the moving average
	vec movAvgSignal = movingAverage(signal, m, bd);
	// Compute the squared deviation
	movAvgSignal = square(movAvgSignal-signal);
	// Compute the squared deviation moving average (moving variance)
	movAvgSignal = movingAverage(movAvgSignal, m, bd);
	// Compute the moving standard deviation
	return sqrt(movAvgSignal);
}


/* Applies a gaussian band pass filter to the array. The filter is designed
 to cut the frequencies below freq1 and above freq2.
 
 If freq2 is too high, a highpass filter will be used.
 
 It preserves the low frequencies up to freq0, if freq0 != 0. */
vec bandPassFilter(const vec& array,
						 const double& freq0,
						 const double& freq1,
						 const double& freq2,
				   const double& sampleRate){
	// Transform freq information to bin information
	double binfreq0 = freq2bin(sampleRate, array.size(), freq0);
	double binfreq1 = freq2bin(sampleRate, array.size(), freq1);
	double binfreq2 = freq2bin(sampleRate, array.size(), freq2);
	// Check parameters
	if (binfreq1 >= binfreq2) throw std::runtime_error("Cutoff frenquencies must be freq1 < freq2");
	if (binfreq0 > array.size()/2) throw std::runtime_error("Cutoff frequency freq0 cannot exceed half of the array size");
	// Compute the Fast Fourier Transform
	cx_vec FFT = fft(array);
	// Define the filter (centered in the middle)
	const double D0_2 = pow((binfreq1+binfreq2)/2.0, 2.0);
	const double freq0_2 = pow(binfreq0, 2.0);
	const double W = binfreq2-binfreq1;
	const double C = ceil(double(array.size())/2.0);
	vec d = abs(linspace(0, array.size()-1, array.size())-C);
	vec d_2 = square(d);
	vec filter;
	if (binfreq2 > array.size()/2) filter = exp(-d_2 * 0.5/(binfreq1*binfreq1)); // lowpass
	else filter = exp(-square( (d_2-D0_2)/(d*W) )); // bandpass
	if (binfreq0 != 0) filter = (1.0-filter) % (1.0 - exp(-d_2 * 0.5/freq0_2)); // custom-reject
	filter = 1.0 - filter; // custom-pass
	// Circular shift
	filter = shift(filter, 1-C);
	// Apply the filter
	FFT = FFT % filter; // element-wise multiplication
	// Go back to the spatial domain
	return real(ifft(FFT));
}
double freq2bin(const double& sampleRate,
				const double& totalSamples,
				const double& frequency){
	return frequency*(sampleRate/totalSamples);
}
double bin2freq(const double& sampleRate,
				const double& totalSamples,
				const double& bin){
	return bin*(totalSamples/sampleRate);
}

/* The function computes and returns the coordinate of a donor pixel corresponding
 to the specified extrapolated pixel when using the specified extrapolation border mode.
 
 If the border type is constant, then -1 is returned.
 
 There is no check whether the final result is out of bounds: it may happen
 that a pos too much out of bounds leads to negative returning value. */
int borderInterpolate(const int& pos,
					  const int& len,
					  const border_type& bd){
	switch (bd) {
		case border_type::constant:
			if (pos < 0 || pos > len-1) return -1; else return pos;
			break;
		case border_type::replicate:
			if (pos < 0) return 0; else if (pos > len-1) return len-1; else return pos;
			break;
		case border_type::reflect:
			if (pos < 0) return -pos-1; else if (pos > len-1) return 2*len-pos-1; else return pos;
			break;
		case border_type::reflect_101:
			if (pos < 0) return -pos; else if (pos > len-1) return 2*len-pos-2; else return pos;
			break;
		case border_type::wrap:
			if (pos < 0) return len+pos; else if (pos > len-1) return pos-len; else return pos;
			break;
		default:
			throw std::runtime_error("Border type not handled");
	}
}

umat binCounts(const mat& array,
					 const std::vector<vec>& bins){
	// By now binCounts is available only for two-column arrays
	if (array.n_cols != 2) throw std::runtime_error("By now binCounts is available only for two-column arrays");
	// Check if the number of columns in array equals the number of bin specifications
	if (array.n_cols != bins.size()) throw std::runtime_error("There must be a bin spec for each column of the input array");
	// Check if each bin specification is monotonically increasing
	bool eachSorted = true;
	for (const vec& v: bins) eachSorted &= v.is_sorted("ascend");
	if (!eachSorted) throw std::runtime_error("Each bin spec must be monotonically increasing");
	// Compute the bin counts
	vec::const_row_iterator lowerBound;
	umat output(bins[0].size(), bins[1].size());
	output.zeros();
	std::vector<unsigned int> indices(array.n_cols/*2*/);
	for (unsigned int i = 0; i < array.n_rows; i++){
		for (unsigned int j = 0; j < array.n_cols/*2*/; j++){
			if ((lowerBound = std::lower_bound(bins[j].begin(), bins[j].end(), array(i, j))) == bins[j].end()) continue;
			indices[j] = std::distance(bins[j].begin(), lowerBound);
		}
		output(indices[0], indices[1]) += 1;
	}
	
	return output;
}

/* Compute the minimum filter of a 1D array.
 The window is reduced near array edges. */
arma::vec minFilter(const arma::vec& array,
					const unsigned int& m){
	arma::vec out(array.size());
	// Compute the border mins
	for(unsigned int k = 0; k <= m; k++) out(k) = array.subvec(0, m+k).min();
	for(unsigned int k = 0; k <= m; k++) out(out.size()-1-k) = array.subvec(array.size()-1-m-k, array.size()-1).min();
	// Compute the other mins faster
	double nval, oval;
	for(unsigned int k = m+1; k <= array.size()-m-1; k++){
		nval = array(k+m);
		oval = array(k-m-1);
		// If new and old values are equal, keep the previous min as min
		if (nval == oval) out(k) = out(k-1);
		else {// nval != oval
			// If the new point is less than or equal to the previous min => use the new value as min
			if (nval <= out(k-1)) out(k) = nval;
			else { // nval > prev.min.
				// If the new point is smaller than the old one, the min is preserved
				if (nval < oval) out(k) = out(k-1);
				else { // nval > oval
					// If the old value is not the min, the min is preserved
					if (oval > out(k-1)) out(k) = out(k-1);
					else { // oval == prev.min.
						// Recompute the minimum
						out(k) = array.subvec(k-m, k+m).min();
					}
				}
			}
		}
	}
	
	return out;
}

arma::vec backgroundEstimation(const arma::vec& array,
							   const unsigned int& minFilterRad,
							   const double& maxPeakWidthAllowed,
							   const unsigned int& derEstimationDiam,
							   const unsigned int& maxIterations,
							   const double& maxAllowedInconsistency,
							   const unsigned int& maxDistNodes){
	arma::vec arrayMin, indices, nodesX, nodesY, background, relDiff;
	alglib::real_1d_array x, y;
	alglib::spline1dinterpolant interp;
	unsigned int n_elem = array.size();
	unsigned int n_nodes = round(maxPeakWidthAllowed * n_elem);
	double adjacentNodesDist;
	double inconsistency = __DBL_MAX__;
	// Compute the minimum filter of the input array
	arrayMin = minFilter(array, minFilterRad);
	
	// Get the array of nodes (subsampling arrayMin at equally spaced points)
	indices = arma::linspace(0, n_elem-1, n_elem); // abscissas are elements positions
	x.setcontent(n_elem, indices.memptr());
	y.setcontent(n_elem, arrayMin.memptr()); // ordinates are the arrayMin values
	alglib::spline1dbuildlinear(x, y, interp); // create a 1d spline interpolant
	nodesX = arma::linspace(0, n_elem-1, n_nodes); // abscissas are eq.spaced nodes
	adjacentNodesDist = nodesX[1]-nodesX[0];
	nodesY.zeros(n_nodes); // nodesVal contains interpolated arrayMin values over indices
	for (unsigned int i = 0; i < n_nodes; i++)
		nodesY[i] = alglib::spline1dcalc(interp, nodesX[i]);
	
	// Estimate the first derivative of the array near the endpoints
	int boundType = 0; // this is the default value (used when derEstimationRad < 2)
	double leftDer = 0, rightDer = 0; //
	if (derEstimationDiam > 1) {
		boundType = 1;
		leftDer = arma::mean(arma::diff(arrayMin.subvec(0,derEstimationDiam-1)));
		rightDer = arma::mean(arma::diff(arrayMin.subvec(arrayMin.size()-derEstimationDiam,arrayMin.size()-1)));
	}
	
	// Iterate a given number of times...
	unsigned int iter = 1;
	while(1){
		// Set x and y content to the data in nodesX and nodesY
		x.setcontent(n_nodes, nodesX.memptr());
		y.setcontent(n_nodes, nodesY.memptr());
		// Create the cubic spline interpolant of the nodes
		alglib::spline1dbuildcubic(x, y,
								   x.length(),
								   boundType, leftDer,
								   boundType, rightDer,
								   interp);
		// Get the background from this interpolant
		background.zeros(n_elem);
		for (unsigned int k = 0; k < n_elem; k++)
			background[k] = alglib::spline1dcalc(interp, indices[k]);
		// Check inconsistency or iteration number
		if (inconsistency < maxAllowedInconsistency || iter++ > maxIterations) break;
		// Evaluate the difference between foreground and background, relative to the background
		relDiff = arma::abs(1 - array/background);
		// Get the positions where background is over foreground
		arma::uvec pos = arma::find(background > array);
		if (pos.empty()) break;
		// Cluster those positions by relative distance less than maxDistNodes
		arma::uvec posdiff = arma::diff(pos); // compute the differences between adjacent positions
		posdiff.insert_rows(0, 1);  // prepend a zero to the difference array to keep the same dimension
		double maxDist = maxDistNodes * adjacentNodesDist;
		uword m = pos[0], M = pos[0], n = 1;
		double v = relDiff[0];
		std::list<std::tuple<double, double, double>> ranges;
		for (unsigned int k = 0; k < pos.size(); k++){ // cycle over pos array
			if (posdiff[k] < 2*maxDist && k < pos.size()-1) { // position is part of the previous cluster (update range)
				M = pos[k];
				n++;
				v += relDiff[M];
			} else { // position is in a new cluster (finalize range and reset)
				// To ensure that the range covers at least one node, we add maxDist to both ends
				ranges.emplace_back(m-maxDist, M+maxDist, v/n);
				m = pos[k];
				M = pos[k];
				n = 1;
				v = relDiff[M];
			}
		}
		// Update each node in one of the ranges
		for(unsigned int k = 0; k < n_nodes; k++){
			if (ranges.empty()) break;
			const std::tuple<double, double, double>& range = ranges.front();
			if (nodesX[k] < std::get<0>(range)) continue; // node before first range - do nothing
			else if (nodesX[k] > std::get<1>(range)) { // node after first range
				ranges.pop_front(); // remove the unuseful range (nodes are ordered)
				k--; // take again this node with next range
				continue;
			} else { // node is in range
				double c = (std::get<0>(range) + std::get<1>(range)) / 2.0; // range center
				double s = c-std::get<0>(range); // range radius
				// Compute a gaussian weight according to the node position in the range
				double weight = exp( - pow(nodesX[k]-c, 2) / (2.0 * pow(s, 2) / 3.0) );
				nodesY[k] *= (1 - std::get<2>(range) * weight);
			}
		}
		// Compute inconsistency
		inconsistency = double(pos.size()) / double(n_elem) * 100;
	}
	
	// Return the background
	return background;
}

void peakDetect(const arma::vec& signal,
				std::vector<uint64_t>& emissionPeaks,
				std::vector<double>& emissionPeaksProminence,
				std::vector<uint64_t>& absorptionPeaks,
				std::vector<double>& absorptionPeaksProminence,
				const double& minRelVariation,
				const double& minRelVarInfluence,
				const bool& emissionFirst){
	// Compute moving average standard deviation
	vec movStdDev = movingStdDev(signal, (unsigned int)(round(minRelVarInfluence*signal.size())));
	// Compute minimum variation (3*stdDev contains is almost the total variation)
	vec minVariation = (minRelVariation * 3) * movStdDev;
	// Set the initial conditions up
	uint64_t relMinIdx = 0;
	uint64_t relMaxIdx = 0;
	double relMin = signal[0];
	double relMax = signal[0];
	bool emission = emissionFirst;
	// Scan the signal
	for(uint64_t k = 0; k < signal.n_elem; ++k){
		if (signal[k] > relMax){
			// Case 1: increasing over local max, update max
			relMax = signal[k];
			relMaxIdx = k;
		} else if (signal[k] < relMin){
			// Case 2: decreasing under local min, update min
			relMin = signal[k];
			relMinIdx = k;
		} else if (emission and signal[k] < relMax-minVariation[k]){
			// Case 3: signal moves away from max, store emission peak and search for absorption peaks
			emissionPeaks.push_back(relMaxIdx);
			emission = not emission;
			// Update min
			relMin = signal[k];
			relMinIdx = k;
		} else if (not emission and signal[k] > relMin+minVariation[k]){
			// Case 4: signal moves away from min, store absorption peak and search for emission peaks
			absorptionPeaks.push_back(relMinIdx);
			emission = not emission;
			// Update max
			relMax = signal[k];
			relMaxIdx = k;
		}
	}
	// Compute prominence as the geometric mean of difference between neighbors
	if (emissionPeaks.size() < 2 || absorptionPeaks.size() < 2 || std::abs(double(emissionPeaks.size())-double(absorptionPeaks.size())) > 1) return;
	uint64_t maxIdx = std::min(emissionPeaks.size(), absorptionPeaks.size());
	if (emissionFirst) {
		emissionPeaksProminence.push_back(signal[emissionPeaks.front()] - signal[absorptionPeaks.front()]);
		for(uint64_t k = 1; k < maxIdx; k++){
			double emiLeftDiff = signal[emissionPeaks[k]] - signal[absorptionPeaks[k-1]];
			double emiRightDiff = signal[emissionPeaks[k]] - signal[absorptionPeaks[k]];
			emissionPeaksProminence.push_back( sqrt(emiLeftDiff * emiRightDiff) );
			double absLeftDiff = signal[emissionPeaks[k-1]] - signal[absorptionPeaks[k-1]];
			double absRightDiff = signal[emissionPeaks[k]] - signal[absorptionPeaks[k-1]];
			absorptionPeaksProminence.push_back( sqrt(absLeftDiff * absRightDiff) );
		}
		if (emissionPeaks.size() == absorptionPeaks.size())
			absorptionPeaksProminence.push_back(signal[emissionPeaks.back()] - signal[absorptionPeaks.back()]);
	} else {
		absorptionPeaksProminence.push_back(signal[emissionPeaks.front()] - signal[absorptionPeaks.front()]);
		for(uint64_t k = 1; k < maxIdx; k++){
			double absLeftDiff = signal[emissionPeaks[k-1]] - signal[absorptionPeaks[k]];
			double absRightDiff = signal[emissionPeaks[k]] - signal[absorptionPeaks[k]];
			absorptionPeaksProminence.push_back( sqrt(absLeftDiff * absRightDiff) );
			double emiLeftDiff = signal[emissionPeaks[k-1]] - signal[absorptionPeaks[k-1]];
			double emiRightDiff = signal[emissionPeaks[k-1]] - signal[absorptionPeaks[k]];
			emissionPeaksProminence.push_back( sqrt(emiLeftDiff * emiRightDiff) );
		}
		if (emissionPeaks.size() == absorptionPeaks.size())
			emissionPeaksProminence.push_back(signal[emissionPeaks.back()] - signal[absorptionPeaks.back()]);
	}
}

void histogramCreation(QVector<double>& data,
					   QVector<double>& x,
					   QVector<double>& y,
					   const double& barsStep){
	arma::vec xx, yy, adata;
	// Convert input to an arma vector
	convert::qvec2arma(data, adata);
	// Compute histogram
	histogramCreation(adata, xx, yy, barsStep);
	// Convert output to qt vector
	convert::arma2qvec(xx, x);
	convert::arma2qvec(yy, y);
}

void histogramCreation(arma::vec& data,
					   arma::vec& x,
					   arma::vec& y,
					   const double& barsStep){
	// Compute the histogram
	x = arma::regspace(data.min(), barsStep, data.max());
	y = arma::normalise(arma::conv_to<arma::vec>::from(arma::hist(data, x)), 1);
}

void convert::arma2qvec(const arma::vec& avec,
						QVector<double>& qvec){
	qvec.reserve(avec.size());
	for(const double& d: avec) qvec << d;
}

void convert::qvec2arma(QVector<double> &qvec,
						arma::vec &avec){
	avec = arma::vec(qvec.data(), qvec.size(), false, true);
}

arma::vec gaussFit::fit(const arma::vec& x,
						const arma::vec& y){
	// Assign values to alglib-type array
	alglib::real_2d_array xx;
	alglib::real_1d_array yy;
	xx.setcontent(x.size(), 1, x.memptr());
	yy.setcontent(y.size(), y.memptr());
	alglib::real_1d_array c = "[1.0, 0.0, 1.0]";
	// Initialize other variables
	alglib::lsfitstate state;
	alglib::lsfitreport report;
	alglib::ae_int_t maxIters = 40;
	double epsF = 1e-6 * (x.max()-x.min());
	double epsX = 1e-6;
	// Initialize the algorithm
	alglib::lsfitcreatefgh(xx, yy, c, state);
	alglib::lsfitsetcond(state, epsF, epsX, maxIters);
	alglib::ae_int_t info;
	// Perform fitting
	alglib::lsfitfit(state, gaussFit::funcVal, gaussFit::funcGrad, gaussFit::funcHess);
	alglib::lsfitresults(state, info, c, report);
	return arma::vec(c.getcontent(), c.length());
}

void gaussFit::coreFuncVal(const alglib::real_1d_array& c,
						   const alglib::real_1d_array& x,
						   double& func,
						   void* ptr){
	// Compute the function core value
	func = std::exp( - std::pow(x[0]-c[1], 2.0) / (2.0*c[2]*c[2]) );
}

void gaussFit::funcVal(const alglib::real_1d_array& c,
					   const alglib::real_1d_array& x,
					   double& func,
					   void* ptr){
	// Compute the function value
	coreFuncVal(c, x, func, ptr);
	func *= c[0];
}

arma::vec gaussFit::eval(const arma::vec& c,
						 const arma::vec& x){
	double func;
	alglib::real_1d_array cvec, xvec;
	cvec.setcontent(c.size(), c.memptr());
	arma::vec y(x.size());
	for(uint64_t k = 0; k < x.size(); k++){
		xvec.setcontent(1, x.memptr()+k);
		funcVal(cvec, xvec, func, NULL);
		y[k] = func;
	}
	return y;
}

void gaussFit::funcGrad(const alglib::real_1d_array& c,
						const alglib::real_1d_array& x,
						double& func,
						alglib::real_1d_array& grad,
						void* ptr){
	// Compute the function value
	funcVal(c, x, func, ptr);
	// Compute the function gradient value
	coreFuncVal(c, x, grad[0], ptr);
	grad[1] = func * (x[0]-c[1]) / (c[2]*c[2]);
	grad[2] = grad[1] * (x[0]-c[1]) / c[2];
}

void gaussFit::funcHess(const alglib::real_1d_array& c,
						const alglib::real_1d_array& x,
						double& func,
						alglib::real_1d_array& grad,
						alglib::real_2d_array& hess,
						void* ptr){
	// Compute the function value and the function gradient value
	funcGrad(c, x, func, grad, ptr);
	// Compute the function hessian value
	double core2 = grad[0] / (c[2]*c[2]);
	hess[0][0] = 0;
	hess[0][1] = core2 * (x[0]-c[1]);
	hess[1][0] = hess[0][1];
	hess[0][2] = hess[0][1] * (x[0]-c[1]) / c[2];
	hess[2][0] = hess[0][2];
	core2 *= c[0] / (c[2]*c[2]);
	hess[1][1] = - core2 * (-c[2]*c[2]+std::pow(c[1]-x[0], 2.0));
	double core3 = core2 * (c[1]-x[0]);
	hess[1][2] = hess[1][1] * (c[1]-x[0]) / c[2] + core3 * c[2];
	hess[2][1] = hess[1][2];
	hess[2][2] = hess[1][2] * (c[1]-x[0]) / c[2] + core3 * (c[1]-x[0]) / c[2];
}

void gaussFit::print(const alglib::lsfitreport& report){
	std::cout << "average error = " << report.avgerror << std::endl;
	std::cout << "average relative error = " << report.avgrelerror << std::endl;
	std::cout << "maximum error = " << report.maxerror << std::endl;
	std::cout << "iterations count = " << report.iterationscount << std::endl;
	std::cout << "non-adjusted coefficient of determination = " << report.r2 << std::endl;
	std::cout << "rms error on the (X,Y) = " << report.rmserror << std::endl;
}

arma::vec oversample(const arma::vec& samples,
					 const double& factor){
	// Convert to alglib vectors
	alglib::real_1d_array x, y;
	arma::vec armax = arma::regspace(0, double(samples.size())-1);
	x.setcontent(samples.size(), armax.memptr());
	y.setcontent(samples.size(), samples.memptr());
	// Build interpolant
	alglib::spline1dinterpolant interp;
	alglib::spline1dbuildcatmullrom(x, y, interp);
	// Compute the output values
	double step = 1.0/factor, xx = 0.0;
	arma::vec out(floor(samples.size()*factor));
	for(uint64_t k = 0; k < out.size(); k++){
		out[k] = alglib::spline1dcalc(interp, xx);
		xx += step;
	}
	return out;
}

arma::mat weightMean(const arma::mat& X,
					 const arma::mat& W){
	if (not W.is_vec())
		throw std::runtime_error("W must be a vector");
	else {
		if (W.size() > 1 and ((W.is_colvec() and X.n_rows != W.n_rows) or (W.is_rowvec() and X.n_cols != W.n_cols)))
			throw std::runtime_error("Wrong dimensions");
		else if (W.size() == 1 and X.n_rows != 1 and X.n_cols != 1)
			throw std::runtime_error("Wrong dimensions");
		else {
			if (W.size() == 1 and X.n_rows == 1)
				return arma::sum(X.each_col() % arma::normalise(W, 1), 0);
			else if (W.size() == 1 and X.n_cols == 1)
				return arma::sum(X.each_row() % arma::normalise(W, 1), 1);
			else if(W.is_colvec())
				return arma::sum(X.each_col() % arma::normalise(W, 1), 0);
			else
				return arma::sum(X.each_row() % arma::normalise(W, 1), 1);
		}
	}
}

arma::mat weightCov(const arma::mat& X,
					const arma::vec& W){
	// row - observations, col - variables
	// Check dimensions
	if (X.n_rows != W.n_rows) throw std::runtime_error("X and W must have the same number of columns");
	// Compute the weighted mean along the observations
	arma::rowvec m = weightMean(X, W);
	// Compute the covariance matrix
	arma::vec NW = arma::normalise(W, 1);
	arma::mat C(X.n_cols, X.n_cols);
	C.zeros();
	for(arma::uword i = 0; i < X.n_rows; i++){
		C += (X.row(i)-m).t() * (X.row(i)-m) * NW(i);
	}
	return C;
}

arma::vec gaussExpFit::fit(const arma::vec& x,
						   const arma::vec& y){
	// Assign values to alglib-type array
	alglib::real_2d_array xx;
	alglib::real_1d_array yy;
	xx.setcontent(x.size(), 1, x.memptr());
	yy.setcontent(y.size(), y.memptr());
	alglib::real_1d_array c = "[0.2, 0.036, 0.1, 0.2]";
	// Initialize other variables
	alglib::lsfitstate state;
	alglib::lsfitreport report;
	alglib::ae_int_t maxIters = 0;
	double epsF = 1e-10 * (x.max()-x.min());
	double epsX = 1e-10;
	// Initialize the algorithm
//	alglib::lsfitcreatefgh(xx, yy, c, state);
	alglib::lsfitcreatef(xx, yy, c, 0.001, state);
	alglib::lsfitsetcond(state, epsF, epsX, maxIters);
	alglib::ae_int_t info;
	// Perform fitting
	alglib::lsfitfit(state, gaussExpFit::funcVal);
//	alglib::lsfitfit(state, gaussExpFit::funcVal, gaussExpFit::funcGrad, gaussExpFit::funcHess);
	alglib::lsfitresults(state, info, c, report);
	return arma::vec(c.getcontent(), c.length());
}

arma::vec gaussExpFit::eval(const arma::vec& c,
							const arma::vec& x){
	double func;
	alglib::real_1d_array cvec, xvec;
	cvec.setcontent(c.size(), c.memptr());
	arma::vec y(x.size());
	for(uint64_t k = 0; k < x.size(); k++){
		xvec.setcontent(1, x.memptr()+k);
		funcVal(cvec, xvec, func, NULL);
		y[k] = func;
	}
	return y;
}

void gaussExpFit::funcVal(const alglib::real_1d_array& cc,
					   const alglib::real_1d_array& xx,
					   double& func,
					   void* ptr){
	// Take useful references
	const double& c = cc[0], a = cc[1], s = cc[2], t = cc[3], x = xx[0];
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
}

void gaussExpFit::funcGrad(const alglib::real_1d_array& cc,
						   const alglib::real_1d_array& xx,
						   double& func,
						   alglib::real_1d_array& grad,
						   void* ptr){
	// Take useful references
	const double& c = cc[0], a = cc[1], s = cc[2], t = cc[3], x = xx[0];
	// Useful quantities
	double c_x = c-x;
	double c_xt = c_x * t;
	double s2 = s * s;
	double A = s2 + c_xt;
	double F = s2 - c_xt;
	double B = A/(M_SQRT2 * s * t);
	double B2 = B*B;
	double _2t = 2 * t;
	double C = (A + c_xt)/(_2t * t);
	double a_2t = a / _2t;
	double erfcB = std::erfc(B);
	double expC = std::exp(C);
	double E = std::exp(- c_x * c_x / (2 * s2) );
	double aE = a * E;
	double expB2 = std::exp(B2);
	double SQRTPI = 2.0/M_2_SQRTPI;
	double _2SQRTPI = 2.0 * SQRTPI;
	double SQRT2PI = M_SQRT2 * SQRTPI;
	double t2 = t * t;
	double t3 = t2 * t;
	double t4 = t3 * t;
	
	// Compute the function value
	func = a_2t * expC * erfcB;
	// Compute the function gradient value
	grad[0] = aE * ( - M_SQRT2 * t + SQRTPI * s * expB2 * erfcB ) / (_2SQRTPI * s * t2);
	grad[1] = expC * erfcB / _2t;
	grad[2] = - F * aE / (SQRT2PI * s2 * t2) + expB2 * aE * s * erfcB / (2 * t3);
	grad[3] = - aE * ( - M_SQRT2 * s * t + expB2 * SQRTPI * (s2 + t*(c_x+t)) * erfcB ) / (_2SQRTPI * t4);
}

void gaussExpFit::funcHess(const alglib::real_1d_array& cc,
						   const alglib::real_1d_array& xx,
						   double& func,
						   alglib::real_1d_array& grad,
						   alglib::real_2d_array& hess,
						   void* ptr){
	// Take useful references
	const double& c = cc[0], a = cc[1], s = cc[2], t = cc[3], x = xx[0];
	// Useful quantities
	double c_x = c-x;
	double c_xt = c_x * t;
	double s2 = s * s;
	double A = s2 + c_xt;
	double F = s2 - c_xt;
	double B = A / (M_SQRT2 * s * t);
	double B2 = B * B;
	double _2t = 2 * t;
	double C = (A + c_xt) / (_2t * t);
	double a_2t = a / _2t;
	double erfcB = std::erfc(B);
	double expC = std::exp(C);
	double E = std::exp(- c_x * c_x / (2 * s2) );
	double aE = a * E;
	double expB2 = std::exp(B2);
	double SQRT2_PI = M_2_SQRTPI*M_SQRT1_2;
	double SQRTPI = 2.0/M_2_SQRTPI;
	double _2SQRTPI = 2.0 * SQRTPI;
	double SQRT2PI = M_SQRT2 * SQRTPI;
	double t2 = t * t;
	double t3 = t2 * t;
	double s3 = s2 * s;
	double s4 = s3 * s;
	double t4 = t3 * t;
	double s5 = s3 * s2;
	double t5 = t4 * t;
	double exp_B2 = 1.0 / expB2;
	double s6 = s5 * s;
	double t6 = t5 * t;
	double c_xt2 = c_xt * c_xt;
	double c_xt3 = c_xt2 * c_xt;
	
	// Compute the function value
	func = a_2t * expC * erfcB;
	// Compute the function gradient value
	grad[0] = aE * ( - M_SQRT2 * t + SQRTPI * s * expB2 * erfcB ) / (_2SQRTPI * s * t2);
	grad[1] = expC * erfcB / _2t;
	grad[2] = - F * aE / (SQRT2PI * s2 * t2) + expB2 * aE * s * erfcB / (2 * t3);
	grad[3] = - aE * ( - M_SQRT2 * s * t + expB2 * SQRTPI * (s2 + t*(c_x+t)) * erfcB ) / (_2SQRTPI * t4);
	// Compute the function hessian matrix
	hess[0][0] = aE * (-F * SQRT2_PI * t / s3 + expB2 * erfcB) / (2 * t3);
	hess[1][0] = expC * (- exp_B2 * SQRT2_PI * t / s + erfcB) / (2 * t2);
	hess[0][1] = hess[1][0];
	hess[1][1] = 0.0;
	hess[2][0] = aE * (- M_SQRT2 * t * (s4 + c_xt2 - s2 * t * (c_x + t)) + expB2 * SQRTPI * s5 * erfcB) / (_2SQRTPI * s4 * t4);
	hess[0][2] = hess[2][0];
	hess[2][1] = -F * E / (SQRT2PI * s2 * t2) + (expB2 * E * s * erfcB) / (2 * t3);
	hess[1][2] = hess[2][1];
	hess[2][2] = aE * (-M_SQRT2 * t * (s6 - c_xt * s4 - c_xt3 + c_x * s2 * t2 * (c_x + 2*t)) + expB2 * SQRTPI * s5 * (s2 + t2) * erfcB) / (_2SQRTPI * s5 * t5);
	hess[3][0] = aE * (SQRT2_PI * t * (s2 + t2) - expB2 * s * (s2 + t * (c_x + 2*t)) * erfcB) / (2 * s * t5);
	hess[0][3] = hess[3][0];
	hess[3][1] = E * s / (SQRT2PI * t3) - expB2 * E * (s2 + t * (c_x + t)) * erfcB / (2 * t4);
	hess[1][3] = hess[3][1];
	hess[3][2] = aE * (s4 + 2 * s2 * t2 - c_x * t3) / (SQRT2PI * s2 * t5) - expB2 * aE * s * (s2 + t * (c_x + 3 * t)) * erfcB / (2 * t6);
	hess[2][3] = hess[3][2];
	hess[3][3] = aE * (-M_SQRT2 * s * t * (s2 + t * (c_x + 4 * t)) + expB2 * SQRTPI * erfcB * (s4 + s2 * t * (2 * c_x + 5 * t) + t2 * (c_x*c_x + 4 * c_xt + 2 * t2))) / (_2SQRTPI * t6 * t);
	
//	std::cout << "Hessian function" << std::endl;
//	std::cout << "coeff: "; for(int k = 0; k < cc.length(); k++) std::cout << cc[k] << " "; std::cout << std::endl;
//	std::cout << "func: " << func << std::endl;
//	std::cout << "grad: "; for(int k = 0; k < grad.length(); k++) std::cout << grad[k] << " "; std::cout << std::endl;
//	std::cout << "hess: ";
//	for(int i = 0; i < hess.rows(); i++) {
//		for(int j = 0; j < hess.cols(); j++)
//			std::cout << hess[i][j] << " ";
//		std::cout << std::endl;
//	}
//	std::cout << std::endl;
}

void gaussExpFit::print(const alglib::lsfitreport& report){
	std::cout << "average error = " << report.avgerror << std::endl;
	std::cout << "average relative error = " << report.avgrelerror << std::endl;
	std::cout << "maximum error = " << report.maxerror << std::endl;
	std::cout << "iterations count = " << report.iterationscount << std::endl;
	std::cout << "non-adjusted coefficient of determination = " << report.r2 << std::endl;
	std::cout << "rms error on the (X,Y) = " << report.rmserror << std::endl;
}
