#include <AA/FeaturesExtractor.hpp>

sf::Mutex AA::FeaturesExtractor::mutex;

void AA::FeaturesExtractor::run(){
	// Open the audio file
	sf::InputSoundFile file; {
		sf::Lock lock(mutex);
		if (!file.openFromFile(getFile().toStdString())) abort("Unable to open "+getFile());
	}
	// Get audio info
	const auto sampleCount = file.getSampleCount();
	setOutSampleRate(file.getSampleRate());
	// Given the sampling frequency, we can check if maximum frequency required by the user
	// is compatible with the signal
	if (getMaximumFrequency() > getOutSampleRate()/2){
		qInfo() << "Maximum frequency required exceeds Nyquist frequency";
		setOutTotalRecords(0);
		setOutFeatures(QVector<double>());
		setOutRecordLength(0);
		return;
	}
	// Given the desired frequency precision (and the sampling frequency), we can compute
	// the optimal length a record should have. It will be the lowest power of 2 that is bigger
	// than the one that yields the desired frequency precision: hence the precision is only
	// used as a minimum.
	setOutRecordLength(pow(2, ceil(log2(getOutSampleRate()/getMaximumSpectrumLeakage()))));
	auto numSamplesPerRecord = getOutRecordLength()*file.getChannelCount();
	// Get the number of records
	setOutTotalRecords(ceil(double(sampleCount) / double(getOutRecordLength())));
//	qInfo() << "File" << QFileInfo(getFile()).baseName() << "has" << getOutTotalRecords() << "records with" << getOutRecordLength() << "for" << getOutSampleRate()/getOutRecordLength() << "Hz of spectral leakage";
 	// Initialization of variables and algorithms
	QVector<double> features;
	features.reserve(3 * getOutTotalRecords());
	QVector<sf::Int16> samplesData(numSamplesPerRecord);
	QVector<double> samples(numSamplesPerRecord);
	auto channelsReduce = UMF::ReduceChannels::create({
		{"NumberChannels", file.getChannelCount()},
		{"Operation", getChannelsOperation()},
		{"ChannelsArrangement", getChannelsArrangement()}
	});
	auto windowing = UMF::Windowing::create({
		{"Length", getOutRecordLength()},
		{"Type", getWindowingFunction()}
	});
	auto gaussianFilter = UMF::GaussianFilter::create({
		{"Radius", getGaussianFilterWidth()},
		{"BorderType", getExtrapolationMethod()}
	});
	auto spectrumMagnitude = UMF::SpectrumMagnitude::create();
	auto backgroundRemove = UMF::SpectrumRemoveBackground::create({
		{"NumberIterations", getBackIterations()},
		{"Direction", getBackDirection()},
		{"FilterOrder", getBackFilterOrder()},
		{"Smoothing", getBackSmoothing()},
		{"SmoothWindow", getBackSmoothWindow()},
		{"Compton", getBackCompton()}
	});
	// If a record is selected, change loop limits accordingly
	unsigned int recIdx = 0;
	unsigned int maxRecIdx = getOutTotalRecords();
	if (getSelectRecord() >= 0){
		recIdx = getSelectRecord();
		maxRecIdx = recIdx + 1;
		file.seek(recIdx * numSamplesPerRecord);
	}
	// Scan each record, or the selected one
	for (; recIdx < maxRecIdx; recIdx++) {
		// Read a record from file (the last, if incomplete, will be discarded)
		// "read"'s maxCount = maxSamplesPerChannel * numberOfChannels
		if (file.read(samplesData.data(), numSamplesPerRecord) < numSamplesPerRecord) break; // end of file reached
		// Convert to double
		std::transform(samplesData.constBegin(), samplesData.constEnd(), samples.begin(), [](const auto& x){return double(x/double(0x7FFF));});
		// Split the channels and compute their mean. Then apply a windowing function.
		channelsReduce->setInSignal(samples);
		channelsReduce->run();
		// Windowing
		windowing->getInput(channelsReduce);
		windowing->run();
		// Mean filter (former binning)
		gaussianFilter->getInput(windowing);
		gaussianFilter->run();
		Q_EMIT timeSeries(gaussianFilter->getOutSignal());
		// Compute the signal spectrum (the Fourier Mathematica command divide by sqrt(N))
		spectrumMagnitude->getInput(gaussianFilter);
		spectrumMagnitude->run();
		Q_EMIT frequencySeries(spectrumMagnitude->getOutSignal());
		// Estimate the background and subtract it from the spectrum
		backgroundRemove->getInput(spectrumMagnitude);
		backgroundRemove->run();
		Q_EMIT frequencySeries(backgroundRemove->getOutSignal());
		// Split the selected frequency range in three parts and compute the max in each
		// For each peak, compute the spectral concentration, that is the ratio between
		// the peak's energy and the total energy on the interval
		const auto& spectrum = backgroundRemove->getOutSignal();
		int bin_start = floor(getMinimumFrequency()/getOutSampleRate()*getOutRecordLength());
		int bin_end = ceil(getMaximumFrequency()/getOutSampleRate()*getOutRecordLength());
		int bin_step = ceil((bin_end-bin_start+1)/3.0); // both extrema included
		QVector<int> Formants;
		QVector<double> spectralConcentration;
		for(auto left = spectrum.constBegin() + bin_start, right = left + bin_step;
			left < spectrum.constBegin() + bin_end + 1; left += bin_step, right += bin_step){
			auto formant = std::distance(spectrum.constBegin(), std::max_element(left, right));
			Formants << formant;
			if(auto totalEnergy = std::reduce(left, right); totalEnergy > 0.0)
				spectralConcentration << sqrt(spectrum[formant] / totalEnergy);
			else
				spectralConcentration << 0.0;
		}
		auto meanSpectralConcentration = std::reduce(spectralConcentration.begin(), spectralConcentration.end());
		meanSpectralConcentration /= double(spectralConcentration.size());
		Q_EMIT pointSeries(Formants);
		// Append to the features array
		auto V1 = double(Formants[1]) / double(Formants[0]);
		auto V2 = double(Formants[2]) / double(Formants[0]);
		features << V1 << V2 << meanSpectralConcentration;
	}
	features.squeeze();
	// Normalize weights
	arma::mat F(features.data(), 3, features.size()/3, false, true);
	F.row(2) -= F.row(2).min();
	F.row(2) /= F.row(2).max();
	F.row(2) %= F.row(2);
	// Set output
	setOutFeatures(features);
}
