#include "../Headers/functions.hpp"

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
		output.rows(0, m-1) = 0;
		output.rows(array.size()+m, output.size()-1) = 0;
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

arma::vec gaussianFilter(const arma::vec& array,
						 const unsigned int& m,
						 const border_type& bd){
	vec padded = arrayPad(array, m, bd);
	vec kernel = exp( - square(linspace(0, 2*m, 2*m+1) - m) / double(m*m) * 2.0 );
	kernel = arma::normalise(kernel, 1/*1-norm*/);
	vec filtered = conv(padded, kernel, "same");
	return filtered.rows(m, m+array.size()-1);
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
		std::list<std::tuple<uword, uword, double>> ranges;
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
			const std::tuple<uword, uword, double>& range = ranges.front();
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
//				std::cout << "range=(" << std::get<0>(range) << ", " << std::get<1>(range) << ", " << std::get<2>(range) << ") ";
//				std::cout << "c=" << c << " s=" << s << " w=" << weight << " ";
//				std::cout << "node=(" << nodesX[k] << ", " << nodesY[k] << ") ";
				nodesY[k] *= (1 - std::get<2>(range) * weight);
//				std::cout << "--> node=(" << nodesX[k] << ", " << nodesY[k] << ") " << std::endl;
			}
		}
		// Compute inconsistency
		inconsistency = double(pos.size()) / double(n_elem) * 100;
	}
	
	// Return the background
	return background;
}
