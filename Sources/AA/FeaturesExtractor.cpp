#include <AA/FeaturesExtractor.hpp>

bool AA::FeaturesExtractor::perform(){
	// Define a function to convert arma array to Qt vector
	auto armaToQ = [](const arma::vec& vec){
		QVector<double> qvec;
		qvec.resize(vec.size());
		memcpy(qvec.data(), vec.memptr(), vec.size()*sizeof(double));
		return qvec;
	};
	const QVector<double> Samples = getInSamples();
	const quint32& SampleRate = getInSampleRate();
	// Compute the record length (the first multiple of 1024 greater than the needed value)
	// Divide by oversampling: during the processing the signal is oversampled
	int recLength = ceil(0.5 * (1.0 + sqrt(1.0 + 4.0 * SampleRate * getMaxFreq())) * 2.0 / 1024.0) * 1024 / getOversampling();
	// Get the number of records
	int numRecords = ceil(double(Samples.size()) / double(recLength));
	// Scan each record
	QVector<double> features;
	for (unsigned int recIdx = 0; recIdx < numRecords; recIdx++) {
		if(getSelectRecord() > 0 && getSelectRecord() != recIdx) continue;
		int pos = recIdx * recLength;
		QVector<double> localSamples = Samples.mid(pos, recLength);
		arma::vec samples(localSamples.data(), localSamples.size(), false, false);
		// The last record may be unreliable and we can exploit too few frequencies, so skip it
		if (samples.size() < recLength) continue;
		// Take into account the oversampling parameter
		samples = oversample(samples, getOversampling());
		// Windowing (butterworth window)
		{
			double buttFiltOrder = 5.0;
			double cutoff = (1.0-2.0*getButtFilterTail())/2.0 * double(samples.size());
			double center = double(samples.size()-1)/2.0;
			for (double k = 0.0; k < samples.size(); k += 1.0)
				samples[k] *= 1.0 / sqrt(1.0 + pow((k-center)/cutoff, 2.0*buttFiltOrder));
		}
		// Binning
		arma::vec binnedSamples(samples.size()/getBinWidth());
		for(unsigned int k = 0; k < binnedSamples.size(); k++){
			// Store the mean in each bin
			binnedSamples[k] = arma::sum(samples.subvec(k*getBinWidth(), (k+1)*getBinWidth()-1)) / double(getBinWidth());
		}
		Q_EMIT emitArray((armaToQ(binnedSamples)));
		// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
		arma::vec wholeSpectrum = arma::square(arma::abs(arma::fft(binnedSamples)))/double(binnedSamples.size());
		arma::vec spectrum = wholeSpectrum.subvec(0, wholeSpectrum.size()/2+1); // take relevant part
		// Estimate the background and subtract it from the spectrum
		arma::vec estBackSpectrum = backgroundEstimation(spectrum,
														 getBEMinFilterRad(), getBEMaxPeakWidth(),
														 getBEDerivDiam(), getBEMaxIterations(),
														 getBEMaxInconsistency(), getBEMaxDistNodes());
		// Final gaussian smoothing
		arma::vec foreSpectrum = spectrum-estBackSpectrum;
		// Split the spectrum in three intervals and get the formants
		double intervalStartBin = round(freq2bin(SampleRate, recLength, getMinFreq()) / double(getBinWidth()));
		double intervalWidthBin = floor((foreSpectrum.size()-intervalStartBin)/3);
		arma::vec4 edges = intervalStartBin + arma::regspace(0, 3) * (intervalWidthBin-1);
		foreSpectrum = foreSpectrum.subvec(edges(0), edges(3));
		Q_EMIT emitArray((armaToQ(foreSpectrum)));
		// Peaks detection
		std::vector<uint64_t> maxPeaks, minPeaks;
		std::vector<double> maxPeaksProm, minPeaksProm;
		peakDetect(foreSpectrum, maxPeaks, maxPeaksProm, minPeaks, minPeaksProm, getPeakRelevance(), getPeakMinVarInfluence(), true);
		// Compute the maximum value of the peaks
		double maxPeakValue = 0.0;
		for (uint64_t& idx: maxPeaks) maxPeakValue = std::max(maxPeakValue, foreSpectrum[idx]);
		// Remove peaks with smaller values
		std::vector<uint64_t> acceptedPeaks;
		acceptedPeaks.reserve(maxPeaks.size());
		for(uint64_t& idx: maxPeaks){
			if (foreSpectrum[idx] > maxPeakValue * getPeakHeightThreshold()) acceptedPeaks.push_back(idx);
		}
		// For each interval sort peaks according to their prominence
		arma::vec maxProminancePerInterval = arma::zeros<arma::vec>(3);
		arma::uvec selectedPeaks = arma::zeros<arma::uvec>(3);
		for(int k = 0; k < 3; k++){
			// Find the peaks within the interval
			QVector<QPair<uint64_t,double>> intervalPeaks;
			intervalPeaks.reserve(acceptedPeaks.size());
			for(uint64_t n = 0; n < acceptedPeaks.size(); n++) {
				uint64_t idx = acceptedPeaks[n];
				if(idx >= edges[k] and idx <= edges[k+1]){
					intervalPeaks.push_back(qMakePair(idx, foreSpectrum[idx]));
				}
			}
			intervalPeaks.squeeze();
			// Check whether there is any peak in the interval
			if (intervalPeaks.empty()) break;
			// Sort peaks within the interval according to their value
			std::sort(intervalPeaks.begin(), intervalPeaks.end(), [](const auto& a, const auto& b){
				return a.second > b.second;
			});
			// Store the maximum peak and its prominence (i.e. value difference between max and the average of the others)
			selectedPeaks[k] = intervalPeaks.front().first;
			maxProminancePerInterval[k] = std::max(intervalPeaks.front().second, 0.0);
		}
		// Check if there is any zero among the prominances of the peaks
		if (arma::prod(maxProminancePerInterval) == 0) continue; // skip this record
		// Compute the features
		double V1 = double(selectedPeaks[1]-selectedPeaks[0])/double(2*intervalWidthBin);
		double V2 = double(selectedPeaks[2]-selectedPeaks[1])/double(2*intervalWidthBin);
		double V3 = 1.0 - exp( - pow(arma::prod(maxProminancePerInterval), 1.0/3.0));
		// Append to the features array
		features << V1 << V2 << V3;
	}
	setOutFeatures(features);
	return true;
}
