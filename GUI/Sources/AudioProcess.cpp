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
	// Load parameters from Application class
	int recLength = Parameters::getParameter(Parameter::ParRecLength).toInt();
	int gaussFilterRad = Parameters::getParameter(Parameter::ParGaussFilterRad).toInt();
	int backEstMinFilterRad = Parameters::getParameter(Parameter::ParBackEstMinFilterRad).toInt();
	double backEstMaxPeakWidthAllowed = Parameters::getParameter(Parameter::ParBackEstMaxPeakWidthAllowed).toDouble();
	int backEstDerEstimationDiam = Parameters::getParameter(Parameter::ParBackEstDerEstimationDiam).toInt();
	int backEstMaxIterations = Parameters::getParameter(Parameter::ParBackEstMaxIterations).toInt();
	double backEstMaxAllowedInconsistency = Parameters::getParameter(Parameter::ParBackEstMaxAllowedInconsistency).toDouble();
	int backEstMaxDistNodes = Parameters::getParameter(Parameter::ParBackEstMaxDistNodes).toInt();
	double intervalStartFreq = Parameters::getParameter(Parameter::ParIntervalStartFreq).toDouble();
	int foreGaussFilterRad = Parameters::getParameter(Parameter::ParForeGaussFilterRad).toInt();
	double buttFiltTail = Parameters::getParameter(Parameter::ParTailSuppression).toDouble();
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
	// Get the number of records
	unsigned int numRecords = ceil(double(reader->getSampleCount()) / double(recLength));
	// Define a matrix to store the formants information
	arma::mat corrSet(numRecords, 3);
	// Cycle over the records
	if (computeOnlyIdx >= numRecords) {
		emit raiseError("Index of record to be computed out of bounds");
		emit processEnded(dir);
		return;
	}
	for (unsigned int recIdx = 0; recIdx < numRecords; recIdx++) {
		if (computeAll || recIdx == computeOnlyIdx) {
			arma::vec samples = allsamples.rows(recIdx*recLength, std::min<unsigned int>((recIdx+1)*recLength-1, allsamples.size()-1));
			emit notableSeries(armaToQ(samples));
			// The last record may be unreliable and we can exploit too few frequencies, so skip it
			if (samples.size() < recLength) {
				corrSet.shed_row(corrSet.n_rows-1);
				continue;
			}
			// Windowing (butterworth window)
			{
				double buttFiltOrder = 15.0;
				double cutoff = (1.0-2.0*buttFiltTail)/2.0 * double(samples.size());
				double center = double(samples.size()-1)/2.0;
				for (double k = 0.0; k < samples.size(); k += 1.0)
					samples[k] *= 1.0 / sqrt(1.0 + pow((k-center)/cutoff, 2.0*buttFiltOrder));
			}
			emit notableSeries(armaToQ(samples));
			// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
			arma::vec wholeSpectrum = arma::square(arma::abs(arma::fft(samples)))/double(samples.size());
			arma::vec spectrum = wholeSpectrum.subvec(0, wholeSpectrum.size()/2+1); // take relevant part
			// Apply a gaussian smoothing to the spectrum
			arma::vec averagedSpectrum = gaussianFilter(spectrum, gaussFilterRad, border_type::replicate);
			// Estimate the background and subtract it from the spectrum
			arma::vec estBackSpectrum = backgroundEstimation(averagedSpectrum,
															 backEstMinFilterRad, backEstMaxPeakWidthAllowed,
															 backEstDerEstimationDiam, backEstMaxIterations,
															 backEstMaxAllowedInconsistency, backEstMaxDistNodes);
			// Final gaussian smoothing
			arma::vec foreSpectrum = averagedSpectrum-estBackSpectrum;
			arma::vec smoothForeground = gaussianFilter(foreSpectrum, foreGaussFilterRad, border_type::replicate);
			// Split the spectrum in three intervals and get the formants
			double intervalStartBin = round(freq2bin(reader->getSampleRate(), recLength, intervalStartFreq));
			//			unsigned int intervalWidthBin = round(freq2bin(reader->getSampleRate(), recLength, intervalWidthFreq));
			double intervalWidthBin = floor((smoothForeground.size()-intervalStartBin)/3);
			arma::vec4 edges = intervalStartBin + arma::regspace(0, 3) * intervalWidthBin;
			arma::vec2 F1, F2, F3;
			// Find the max position in each subarray
			F1(0) = smoothForeground.subvec(edges(0), edges(1)-1).index_max();
			F2(0) = smoothForeground.subvec(edges(1), edges(2)-1).index_max();
			F3(0) = smoothForeground.subvec(edges(2), edges(3)-1).index_max();
			// Find the max variation in each subarray (height of each peak)
			F1(1) = smoothForeground(F1(0)) - smoothForeground.subvec(edges(0), edges(1)-1).min();
			F2(1) = smoothForeground(F2(0)) - smoothForeground.subvec(edges(1), edges(2)-1).min();
			F3(1) = smoothForeground(F3(0)) - smoothForeground.subvec(edges(2), edges(3)-1).min();
			// Express position from the beginning of the array
			F1(0) += edges(0);
			F2(0) += edges(1);
			F3(0) += edges(2);
			// Compute the features
			corrSet(recIdx, 0) = (F2(0)-F1(0))/double(2*intervalWidthBin);
			corrSet(recIdx, 1) = (F3(0)-F2(0))/double(2*intervalWidthBin);
			//			corrSet(recIdx, 0) = F1(0)/F2(0);
			//			corrSet(recIdx, 1) = F2(0)/F3(0);
			arma::vec F = { F1(1), F2(1), F3(1) };
			corrSet(recIdx, 2) = 1 - exp( - pow( arma::prod(F), 1/double(F.size()) ));
			
			emit notableSpectrum(armaToQ(arma::abs(smoothForeground.subvec(intervalStartBin, intervalStartBin+3*intervalWidthBin-1))));
		}
	}
	
	// Do the binning matrix computation only if every record has been computed
	if (computeAll) {
		// Generate the binning matrix
		arma::vec binSpec = arma::linspace(0, 1, 100);
		arma::umat matCorr = binCounts(corrSet.cols(0, 1), {binSpec, binSpec}); /* trovare massimo valore e creare i bin in base a quello (più veloce) */
		arma::umat::elem_type maxVal = matCorr.max();
		arma::mat matCorrd = arma::conv_to<arma::mat>::from(matCorr) / arma::mat::elem_type(maxVal);
		arma::uchar_mat matCorrb = arma::conv_to<arma::uchar_mat>::from(matCorrd * 255);
		corr = QSharedPointer<QImage>(new QImage(matCorrb.memptr(), matCorrb.n_cols, matCorrb.n_rows, QImage::Format_Grayscale8));
		emit imageGenerated(dir, corr);
		
		// Signals the features array
		QSharedPointer<QVector<QVector<double>>> features(new QVector<QVector<double>>(corrSet.n_cols));
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
