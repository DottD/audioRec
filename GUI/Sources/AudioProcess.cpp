#include "../Headers/AudioProcess.hpp"

QDir Ui::AudioProcess::getFileName() const {
	return *dir;
}

void Ui::AudioProcess::setFileName(QSharedPointer<QDir> dir) {
	this->dir = dir;
}

void Ui::AudioProcess::run() {
	// Read the audio file associated with this class instance
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(dir->absolutePath().toStdString())) throw std::runtime_error("Unable to read the audio file");
	// Get the vector of samples
	arma::Col<int16_t> isamples(buffer.getSamples(),
								buffer.getSampleCount()/*memory is copied*/);
	arma::vec allsamples = arma::conv_to<arma::vec>::from(isamples)/double(INT16_MAX);
	isamples.clear();
	// Get the number of records
	unsigned int numRecords = ceil(double(buffer.getSampleCount()) / double(recLength));
	// Define a matrix to store the formants information
	arma::mat corrSet(numRecords, 3);
	// Cycle over the records
	for (unsigned int recIdx = 0; recIdx < numRecords; recIdx++) {
		arma::vec samples = allsamples.rows(recIdx*recLength, std::min<unsigned int>((recIdx+1)*recLength-1, allsamples.size()-1));
		// The last record may be unreliable and we can exploit too few frequencies, so skip it
		if (samples.size() < recLength) continue;
		// Cut unreliable frequencies in the whole audio file
		arma::vec filteredSamples = bandPassFilter(samples, lowpassFreq, minFilterFreq, maxFilterFreq, buffer.getSampleRate()); // -> wiener filter
		// Apply the moving average
		arma::vec averagedSamples = movingAverage(filteredSamples, movAvgRadius, border_type::replicate);
		// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
		arma::vec spectrum = arma::square(arma::abs(arma::fft(averagedSamples)))/double(averagedSamples.size());
		spectrum = spectrum.subvec(0, spectrum.size()/2);
		// Compute the moving average on the spectrum
		arma::vec averagedSpectrum = movingAverage(spectrum, movAvgSpecRad, border_type::replicate);
		// Estimate the background by moving average and subtract it from the spectrum
		arma::vec estBackSpectrum = movingAverage(minFilter(averagedSpectrum, estBackMinRadius), estBackAveRadius, border_type::reflect);
		arma::vec foreSpectrum = averagedSpectrum-estBackSpectrum;
		// Split the spectrum in three intervals and get the formants
		unsigned int intervalStartBin = round(freq2bin(buffer.getSampleRate(), samples.size(), intervalStartFreq));
		unsigned int intervalWidthBin = round(freq2bin(buffer.getSampleRate(), samples.size(), intervalWidthFreq));
		unsigned int F1x = intervalStartBin + foreSpectrum.subvec(intervalStartBin, intervalStartBin+intervalWidthBin-1).index_max();
		unsigned int F2x = intervalStartBin+intervalWidthBin + foreSpectrum.subvec(intervalStartBin+intervalWidthBin, intervalStartBin+2*intervalWidthBin-1).index_max();
		unsigned int F3x = intervalStartBin+2*intervalWidthBin + foreSpectrum.subvec(intervalStartBin+2*intervalWidthBin, intervalStartBin+3*intervalWidthBin-1).index_max();
		corrSet(recIdx, 0) = F2x - F1x;
		corrSet(recIdx, 1) = F3x - F2x;
		corrSet(recIdx, 2) = 1.0/sqrt(SQ2(foreSpectrum(F1x)) + 2*SQ2(foreSpectrum(F2x)) + SQ2(foreSpectrum(F3x)));
	}
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
	
	// Signals ended process
	emit processEnded(dir);
}
