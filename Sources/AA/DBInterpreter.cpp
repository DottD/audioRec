#include <AA/DBInterpreter.hpp>

Ui::DBInterpreter::DBInterpreter(QObject* parent):
QObject(parent){
	qRegisterMetaType<QVector<double>>();
}

QVector<double> Ui::DBInterpreter::getIntraCoeffs(){
	return this->intraCoeffs;
}

QVector<double> Ui::DBInterpreter::getExtraCoeffs(){
	return this->extraCoeffs;
}

void Ui::DBInterpreter::setIntraCoeffs(const QVector<double>& intraCoeffs){
	this->intraCoeffs = intraCoeffs;
}

void Ui::DBInterpreter::setExtraCoeffs(const QVector<double>& extraCoeffs){
	this->extraCoeffs = extraCoeffs;
}

QSharedPointer<QStringList> Ui::DBInterpreter::getIntraFeats(){
	return this->intraFeats;
}

QSharedPointer<QStringList> Ui::DBInterpreter::getExtraFeats(){
	return this->extraFeats;
}

void Ui::DBInterpreter::setIntraFeats(QSharedPointer<QStringList> intraFeats){
	this->intraFeats = intraFeats;
}

void Ui::DBInterpreter::setExtraFeats(QSharedPointer<QStringList> extraFeats){
	this->extraFeats = extraFeats;
}

bool Ui::DBInterpreter::readDBFromFile(QFile& file){
	// Try to open the given file
	if (!file.open(QIODevice::ReadOnly)) return false;
	// Associate the file with a binary data stream
	QDataStream stream(&file);
	// Read the features
	stream >> intraCoeffs;
	stream >> extraCoeffs;
	// Close the file
	file.close();
	// Everything is ok
	return true;
}

bool Ui::DBInterpreter::writeDBToFile(QFile& file){
	// Try to open the given file
	if (!file.open(QIODevice::WriteOnly)) return false;
	// Associate the file with a binary data stream;
	QDataStream ostream(&file);
	// Write data into stream
	ostream << intraCoeffs;
	ostream << extraCoeffs;
	// Close the file
	file.close();
	// Everything is ok
	return true;
}

void Ui::DBInterpreter::computeDB(){
	// Check list of file names
	if (intraFeats.isNull() or extraFeats.isNull()) return;
	if (intraFeats->isEmpty() or extraFeats->isEmpty()) return;
	// Pack the two lists of features file names, for better readability
	QVector<QSharedPointer<QStringList>> lists = {intraFeats, extraFeats};
	// For each list compute distances among the related features
	bool count = false;
	for(const QSharedPointer<QStringList>& fileNames: lists){
		// Load each file and compare with any other (apart from it)
		QFile out_file, inn_file;
		VoiceFeatures out_feat, inn_feat;
		QVector<double> distances;
		distances.reserve(fileNames->size() * (fileNames->size()-1));
		for (const QString& out_name: *fileNames){
			// Read current features vector from file
			out_file.setFileName(out_name);
			out_feat.readFromFile(out_file);
			// Loop over the same list of files, looking for string different from "name"
			for (const QString& inn_name: *fileNames){
				// Check if the inner name is the same of the outer
				if (out_name == inn_name) continue;
				// Load the inner feature vector from file
				inn_file.setFileName(inn_name);
				inn_feat.readFromFile(inn_file);
				// Compute the distance between the vectors
				distances << out_feat.distance(inn_feat);
			}
		}
		// Compute the histogram of the given data
		double barsStep = Parameters::getParameter(Parameter::ParBarsStep).toDouble();
		QVector<double> x, y;
		histogramCreation(distances, x, y, barsStep);
		Q_EMIT histogramReady(x, y);
		// Compute coefficients
		arma::vec coeff = gaussExpFit::fit(arma::vec(x.data(), x.size(), false, false),
										   arma::vec(y.data(), y.size(), false, false));
		QVector<double> coefficients;
		convert::arma2qvec(coeff, coefficients);
		Q_EMIT fittingReady(coefficients, x);
		// Set the proper coefficients
		if(count){
			// extraFeats computed
			setExtraCoeffs(coefficients);
		}else{
			// intraFeats computed
			setIntraCoeffs(coefficients);
		}
		// Update count
		count = not count;
	}
	Q_EMIT databaseCreated();
}

