#include <GUI/AudioReadExtract.hpp>

bool GUI::AudioReadExtract::perform(){
	QAlgorithm::PropertyMap FEPar = {
		{"KeepInput", getKeepInput()},
		{"SelectRecord", getSelectRecord()},
		{"MinFreq", getMinFreq()},
		{"MaxFreq", getMaxFreq()},
		{"Oversampling", getOversampling()},
		{"BEMinFilterRad", getBEMinFilterRad()},
		{"BEMaxPeakWidth", getBEMaxPeakWidth()},
		{"BEDerivDiam", getBEDerivDiam()},
		{"BEMaxIterations", getBEMaxIterations()},
		{"BEMaxInconsistency", getBEMaxInconsistency()},
		{"BEMaxDistNodes", getBEMaxDistNodes()},
		{"ButtFilterTail", getButtFilterTail()},
		{"PeakRelevance", getPeakRelevance()},
		{"PeakMinVarInfluence", getPeakMinVarInfluence()},
		{"BinWidth", getBinWidth()},
		{"PeakHeightThreshold", getPeakHeightThreshold()}
	};
	auto reader = AA::AudioReader::create({{"File", getFile()}});
	reader->run();
	auto extractor = AA::FeaturesExtractor::create(FEPar);
	connect(extractor.data(), SIGNAL(emitArray(QVector<double>)), this, SLOT(on_arrayToEmit(QVector<double>)));
	extractor->setInSamples(reader->getOutSamples());
	extractor->setInSampleRate(reader->getOutSampleRate());
	extractor->run();
	this->setOutFeatures(extractor->getOutFeatures());
	return true;
}

void GUI::AudioReadExtract::on_arrayToEmit(QVector<double> array){
	Q_EMIT emitArray(array);
}
