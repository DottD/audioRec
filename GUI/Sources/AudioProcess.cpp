#include "../Headers/AudioProcess.hpp"
#include <QDebug>
#include <iostream>

void Ui::AudioProcess::setFileName(QSharedPointer<QDir> dir) {
	this->dir = dir;
}

QSharedPointer<QVector<double>> Ui::AudioProcess::armaToQ(const arma::vec& vec) const{
	QSharedPointer<QVector<double>> qvec(new QVector<double>);
	qvec->resize(vec.size());
	memcpy(qvec->data(), vec.memptr(), vec.size()*sizeof(double));
	return qvec;
}

void Ui::AudioProcess::run() {
	// Load parameters from Parameters class
	double maxFreq = Parameters::getParameter(Parameter::ParMaxFreq).toDouble();
	double minFreq = Parameters::getParameter(Parameter::ParMinFreq).toDouble();
	int oversampling = Parameters::getParameter(Parameter::ParOversampling).toInt();
	int backEstMinFilterRad = Parameters::getParameter(Parameter::ParBackEstMinFilterRad).toInt();
	double backEstMaxPeakWidthAllowed = Parameters::getParameter(Parameter::ParBackEstMaxPeakWidthAllowed).toDouble();
	int backEstDerEstimationDiam = Parameters::getParameter(Parameter::ParBackEstDerEstimationDiam).toInt();
	int backEstMaxIterations = Parameters::getParameter(Parameter::ParBackEstMaxIterations).toInt();
	double backEstMaxAllowedInconsistency = Parameters::getParameter(Parameter::ParBackEstMaxAllowedInconsistency).toDouble();
	int backEstMaxDistNodes = Parameters::getParameter(Parameter::ParBackEstMaxDistNodes).toInt();
	double buttFiltTail = Parameters::getParameter(Parameter::ParTailSuppression).toDouble();
	double peakRelevance = Parameters::getParameter(Parameter::ParPeaksRelevance).toDouble();
	double peakMinVarInfluence = Parameters::getParameter(Parameter::ParPeakMinVariationInfluence).toDouble();
	int binWidth = Parameters::getParameter(Parameter::ParBinWidth).toInt();
	double peakHeightThreshold = Parameters::getParameter(Parameter::ParPeakHeightThreshold).toDouble();
	
	// If samples is already allocated use it, otherwise load from file
	if (reader.isNull() || reader->getSamplesPtr()->isEmpty()) {
		// Read the audio file associated with this class instance
		reader.reset(new AudioReader);
		if (!reader->loadFile(dir)) {
			emit raiseError("Unable to read file");
			emit processEnded(dir);
			return;
		}
		if (Caller != NULL) {
			ChartRecWidget* crw = dynamic_cast<ChartRecWidget*>(Caller);
			if (crw != NULL) crw->setReader(reader);
		}
	}
	// Get a pointer to samples data
	QSharedPointer<QVector<double>> samples = reader->getSamplesPtr();
	// Get the vector of samples
	arma::vec allsamples(samples->toStdVector()); // is the data copied?
	// Compute the record length (the first multiple of 1024 greater than the needed value)
	// Divide by oversampling: during the processing the signal is oversampled
	int recLength = ceil(0.5 * (1.0 + sqrt(1.0 + 4.0 * reader->getSampleRate() * maxFreq)) * 2.0 / 1024.0) * 1024 / oversampling;
	// Get the number of records
	unsigned int numRecords = ceil(double(reader->getSampleCount()) / double(recLength));
	// Define a matrix to store the formants information
	std::vector<std::tuple<double, double, double>> corrSetVec;
	// Cycle over the records
	if (computeOnlyIdx >= numRecords) {
		emit raiseError("Index of record to be computed out of bounds");
		emit processEnded(dir);
		return;
	}
	for (unsigned int recIdx = 0; recIdx < numRecords; recIdx++) {
		if (computeAll || recIdx == computeOnlyIdx) {
			arma::vec samples = allsamples.rows(recIdx*recLength, std::min<unsigned int>((recIdx+1)*recLength-1, allsamples.size()-1));
			// The last record may be unreliable and we can exploit too few frequencies, so skip it
			if (samples.size() < recLength) continue;
			// Take into account the oversampling parameter
			samples = oversample(samples, oversampling);
			// Windowing (butterworth window)
			{
				double buttFiltOrder = 5.0;
				double cutoff = (1.0-2.0*buttFiltTail)/2.0 * double(samples.size());
				double center = double(samples.size()-1)/2.0;
				for (double k = 0.0; k < samples.size(); k += 1.0)
					samples[k] *= 1.0 / sqrt(1.0 + pow((k-center)/cutoff, 2.0*buttFiltOrder));
			}
			// Binning
			arma::vec binnedSamples(samples.size()/binWidth);
			for(unsigned int k = 0; k < binnedSamples.size(); k++){
				// Store the mean in each bin
				binnedSamples[k] = arma::sum(samples.subvec(k*binWidth, (k+1)*binWidth-1)) / double(binWidth);
			}
			emit notableSeries(armaToQ(binnedSamples));
			// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
			arma::vec wholeSpectrum = arma::square(arma::abs(arma::fft(binnedSamples)))/double(binnedSamples.size());
			arma::vec spectrum = wholeSpectrum.subvec(0, wholeSpectrum.size()/2+1); // take relevant part
			// Estimate the background and subtract it from the spectrum
			arma::vec estBackSpectrum = backgroundEstimation(spectrum,
															 backEstMinFilterRad, backEstMaxPeakWidthAllowed,
															 backEstDerEstimationDiam, backEstMaxIterations,
															 backEstMaxAllowedInconsistency, backEstMaxDistNodes);
			// Final gaussian smoothing
			arma::vec foreSpectrum = spectrum-estBackSpectrum;
			// Split the spectrum in three intervals and get the formants
			double intervalStartBin = round(freq2bin(reader->getSampleRate(), recLength, minFreq) / double(binWidth));
			double intervalWidthBin = floor((foreSpectrum.size()-intervalStartBin)/3);
			arma::vec4 edges = intervalStartBin + arma::regspace(0, 3) * (intervalWidthBin-1);
			foreSpectrum = foreSpectrum.subvec(edges(0), edges(3));
			emit notableSpectrum(armaToQ(arma::abs(foreSpectrum)));
			// Peaks detection
			std::vector<uint64_t> maxPeaks, minPeaks;
			std::vector<double> maxPeaksProm, minPeaksProm;
			peakDetect(foreSpectrum, maxPeaks, maxPeaksProm, minPeaks, minPeaksProm, peakRelevance, peakMinVarInfluence, true);
			// Compute the maximum value of the peaks
			double maxPeakValue = 0.0;
			for (uint64_t& idx: maxPeaks) maxPeakValue = std::max(maxPeakValue, foreSpectrum[idx]);
			// Remove peaks with smaller values
			std::vector<uint64_t> acceptedPeaks;
			acceptedPeaks.reserve(maxPeaks.size());
			for(uint64_t& idx: maxPeaks){
				if (foreSpectrum[idx] > maxPeakValue * peakHeightThreshold) acceptedPeaks.push_back(idx);
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
			corrSetVec.emplace_back(V1, V2, V3);
			// Peaks visualization
			{
				arma::vec selpeaks(foreSpectrum.size());
				selpeaks.zeros();
				double val = (foreSpectrum.max()-foreSpectrum.min())/2;
				for(uint64_t& k: selectedPeaks) selpeaks[k] = val;
				emit notableSpectrum(armaToQ(selpeaks));
			}
		}
	}
	
	// Do the binning matrix computation only if every record has been computed
	if (computeAll and not corrSetVec.empty()) {
		// Generate the binning matrix
		arma::mat corrSet(corrSetVec.size(), 3);
		for(uint64_t k = 0; k < corrSetVec.size(); k++) {
			const std::tuple<double, double, double>& t = corrSetVec[k];
			corrSet(k, 0) = std::get<0>(t);
			corrSet(k, 1) = std::get<1>(t);
			corrSet(k, 2) = std::get<2>(t);
		}
		arma::vec binSpec = arma::linspace(0, 1, 100);
		arma::umat matCorr = binCounts(corrSet.cols(0, 1), {binSpec, binSpec}); /* trovare massimo valore e creare i bin in base a quello (più veloce) */
		arma::umat::elem_type maxVal = matCorr.max();
		arma::mat matCorrd = arma::conv_to<arma::mat>::from(matCorr) / arma::mat::elem_type(maxVal);
		arma::uchar_mat matCorrb = arma::conv_to<arma::uchar_mat>::from(matCorrd * 255);
		corr = QSharedPointer<QImage>(new QImage(matCorrb.memptr(), matCorrb.n_cols, matCorrb.n_rows, QImage::Format_Grayscale8));
		emit imageGenerated(dir, corr);
		
		// Signals the features array
		features->resize(corrSet.n_cols);
		for (unsigned int j = 0; j < corrSet.n_cols; j++){
			QVector<double> vec(corrSet.n_rows);
			for (unsigned int i = 0; i < corrSet.n_rows; i++) vec[i] = corrSet(i, j);
			(*features)[j] = vec;
		}
		emit newFeatures(dir, features);
	}
	
	// Signals ended process
	emit processEnded(dir);
}
