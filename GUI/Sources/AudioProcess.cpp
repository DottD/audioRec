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
	int recLength = Application::getParameter(ParRecLength).toInt();
	int movAvgRadius = Application::getParameter(ParMovAvgRadius).toInt();
	double lowpassFreq = Application::getParameter(ParLowpassFreq).toDouble();
	double minFilterFreq = Application::getParameter(ParMinFilterFreq).toDouble();
	double maxFilterFreq = Application::getParameter(ParMaxFilterFreq).toDouble();
	int movAvgSpecRad = Application::getParameter(ParMovAvgSpecRad).toInt();
	int estBackAveRadius = Application::getParameter(ParEstBackAveRadius).toInt();
	int estBackMinRadius = Application::getParameter(ParEstBackMinRadius).toInt();
	int intervalStartFreq = Application::getParameter(ParIntervalStartFreq).toDouble();
	int intervalWidthFreq = Application::getParameter(ParIntervalWidthFreq).toDouble();
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
			if (samples.size() < recLength) continue;
			// Cut unreliable frequencies in the whole audio file
			arma::vec filteredSamples = bandPassFilter(samples, lowpassFreq, minFilterFreq, maxFilterFreq, reader->getSampleRate()); // -> wiener filter
			emit notableSeries(armaToQ(filteredSamples));
			// Apply the moving average
			arma::vec averagedSamples = movingAverage(filteredSamples, movAvgRadius, border_type::replicate);
			emit notableSeries(armaToQ(averagedSamples));
			// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
			arma::vec spectrum = arma::square(arma::abs(arma::fft(averagedSamples)))/double(averagedSamples.size());
			spectrum = spectrum.subvec(0, spectrum.size()/2+1);
			// Compute the moving average on the spectrum
			arma::vec averagedSpectrum = movingAverage(spectrum, movAvgSpecRad, border_type::replicate);
			emit notableSpectrum(armaToQ(averagedSpectrum));
			// Estimate the background by moving average and subtract it from the spectrum
			arma::vec estBackSpectrum = movingAverage(minFilter(averagedSpectrum, estBackMinRadius), estBackAveRadius, border_type::reflect);
			emit notableSpectrum(armaToQ(estBackSpectrum));
			arma::vec foreSpectrum = averagedSpectrum-estBackSpectrum;
			// Split the spectrum in three intervals and get the formants
			unsigned int intervalStartBin = round(freq2bin(reader->getSampleRate(), samples.size(), intervalStartFreq));
			unsigned int intervalWidthBin = round(freq2bin(reader->getSampleRate(), samples.size(), intervalWidthFreq));
			unsigned int F1x = intervalStartBin + foreSpectrum.subvec(intervalStartBin, intervalStartBin+intervalWidthBin-1).index_max();
			unsigned int F2x = intervalStartBin+intervalWidthBin + foreSpectrum.subvec(intervalStartBin+intervalWidthBin, intervalStartBin+2*intervalWidthBin-1).index_max();
			unsigned int F3x = intervalStartBin+2*intervalWidthBin + foreSpectrum.subvec(intervalStartBin+2*intervalWidthBin, intervalStartBin+3*intervalWidthBin-1).index_max();
			corrSet(recIdx, 0) = F2x - F1x;
			corrSet(recIdx, 1) = F3x - F2x;
			corrSet(recIdx, 2) = 1.0/sqrt(SQ2(foreSpectrum(F1x)) + 2*SQ2(foreSpectrum(F2x)) + SQ2(foreSpectrum(F3x)));
		}
	}
	
	// Do the binning matrix computation only if every record has been computed
	if (computeAll) {
		// Generate the binning matrix
		arma::vec binSpec = arma::linspace(0, 1000, 100);
		arma::umat matCorr = binCounts(corrSet.cols(0, 1), {binSpec, binSpec}); /* trovare massimo valore e creare i bin in base a quello (più veloce) */
		arma::umat::elem_type maxVal = matCorr.max();
		arma::mat matCorrd = arma::conv_to<arma::mat>::from(matCorr) / arma::mat::elem_type(maxVal);
		arma::uchar_mat matCorrb = arma::conv_to<arma::uchar_mat>::from(matCorrd * 255);
		
		// Save the binning matrix to image file
		QScopedPointer<QDir> outputDir(new QDir(*dir));
		QScopedPointer<QString> baseName(new QString(outputDir->dirName().split(".").takeFirst().append(".png")));
		if (!outputDir->cdUp()) throw std::runtime_error("Cannot change directory upwards");
		if (!outputDir->exists("Results") && !outputDir->mkdir("Results")) throw std::runtime_error("Cannot mkdir here");
		if (!outputDir->cd("Results")) throw std::runtime_error("Cannot cd into subfolder Results");
		corr = QSharedPointer<QImage>(new QImage(matCorrb.memptr(), matCorrb.n_cols, matCorrb.n_rows, QImage::Format_Grayscale8));
		if (!corr->save(outputDir->absoluteFilePath(*baseName)))
			throw std::runtime_error((QString("Unable to save QImage as ")+outputDir->absoluteFilePath(*baseName)).toStdString());
	}
	
	// Signals ended process
	emit processEnded(dir);
}
