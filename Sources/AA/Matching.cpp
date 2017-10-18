#include <AA/Matching.hpp>

Ui::Matching::Matching(QObject* parent) :
QObject(parent){
	qRegisterMetaType<QList<QVariant>>();
}

QVector<double> Ui::Matching::getIntraCoeffs(){
	return this->intraCoeffs;
}

QVector<double> Ui::Matching::getExtraCoeffs(){
	return this->extraCoeffs;
}

QStringList Ui::Matching::getCulpritFiles(){
	return this->culpritFiles;
}

QStringList Ui::Matching::getSuspectsFiles(){
	return this->suspectsFiles;
}

void Ui::Matching::setIntraCoeffs(const QVector<double>& intraCoeffs){
	this->intraCoeffs = intraCoeffs;
}

void Ui::Matching::setExtraCoeffs(const QVector<double>& extraCoeffs){
	this->extraCoeffs = extraCoeffs;
}

void Ui::Matching::setCulpritFiles(const QStringList& culpritFiles){
	this->culpritFiles = culpritFiles;
}

void Ui::Matching::setSuspectsFiles(const QStringList& suspectsFiles){
	this->suspectsFiles = suspectsFiles;
}

void Ui::Matching::setTestIntra(bool testIntra){
	this->testIntra = testIntra;
}

void Ui::Matching::eval(double x, double xminusa, double xminusb, double& y, void* ptr){
	alglib::real_1d_array *c = (alglib::real_1d_array*)ptr;
	alglib::real_1d_array xx;
	xx.setcontent(1, &x);
	gaussExpFit::funcVal(*c, xx, y, nullptr);
}

void Ui::Matching::match(){
	// Normalize both the curves through setting A = 1
	qDebug() << "intraCoeffs" << intraCoeffs;
	qDebug() << "extraCoeffs" << extraCoeffs;
	alglib::real_1d_array intraC, extraC;
	intraC.setcontent(intraCoeffs.size(), intraCoeffs.data());
	extraC.setcontent(extraCoeffs.size(), extraCoeffs.data());
	intraC[1] = 1;
	extraC[1] = 1;
	// Prepare to compute the distances weighted average
	QVector<double> distances;
	distances.reserve(culpritFiles.size()*suspectsFiles.size());
	for(const QString& culpritFile: culpritFiles){
		// Process culprit file
		QSharedPointer<QDir> culpritDir(new QDir(culpritFile));
		AudioProcess process;
		process.setFileName(culpritDir);
		process.setCaller(this);
		process.run();
		if (process.getFeatures()->isEmpty()) continue;
		VoiceFeatures culpritFeat(process.getFeatures());
		// Compare with every suspect
		for(const QString& suspectFile: suspectsFiles){
			// Process suspect file
			QSharedPointer<QDir> suspectDir(new QDir(suspectFile));
			AudioProcess suspectProcess;
			suspectProcess.setFileName(suspectDir);
			suspectProcess.setCaller(this);
			suspectProcess.run();
			if (suspectProcess.getFeatures()->isEmpty()) continue;
			VoiceFeatures suspectFeat(suspectProcess.getFeatures());
			// Compute the distance
			distances << culpritFeat.distance(suspectFeat);
		}
	}
	qDebug() << "distances" << distances;
	if (distances.isEmpty()) {
		QList<QVariant> score;
		score << "Test Failed (no features)" << 0;
		Q_EMIT newScore(score);
		return;
	}
	// Compute the histogram of the given data
	double barsStep = Parameters::getParameter(Parameter::ParBarsStep).toDouble();
	QVector<double> x, y;
	histogramCreation(distances, x, y, barsStep);
	qDebug() << "x" << x;
	qDebug() << "y" << y;
	// Compute coefficients
	arma::vec coeff = gaussExpFit::fit(arma::vec(x.data(), x.size(), false, false),
									   arma::vec(y.data(), y.size(), false, false));
	QVector<double> coefficients;
	convert::arma2qvec(coeff, coefficients);
	qDebug() << "coefficients" << coefficients;
	Q_EMIT fittingReady(coefficients, x);
	coefficients[1] = 1; // normalization
	// Compute chi square
	QVector<double> chi2list;
	chi2list.reserve(distances.length());
	for(uint k = 0; k < distances.length(); k++){
		double stored, tested;
		// Get value from distribution stored in the database
		alglib::real_1d_array xx;
		xx.setcontent(1, distances.data()+k);
		if(testIntra)
			gaussExpFit::funcVal(intraC, xx, stored, nullptr);
		else
			gaussExpFit::funcVal(extraC, xx, stored, nullptr);
		// Get value from distribution to be tested
		alglib::real_1d_array testedCC;
		testedCC.setcontent(coefficients.length(), coefficients.data());
		gaussExpFit::funcVal(testedCC, xx, tested, nullptr);
		// Compute local chi2
		chi2list << std::pow(std::abs(tested-stored)/stored, 2);
		qDebug() << "tested" << tested << "stored" << stored << "chi2" << chi2list.last();
	}
	qDebug() << "chi2list" << chi2list;
	double chi2 = arma::mean(arma::vec(chi2list.data(), chi2list.size(), false, false));
	qDebug() << "chi2" << chi2;
	double similarity = 1-std::erf(chi2);
	qDebug() << "similarity" << similarity;
	//		// Compute true acceptance
	//		double TA;
	//		alglib::autogkstate tastate;
	//		alglib::autogkreport report;
	//		alglib::autogksmooth(distance, maxDistance, tastate);
	//		alglib::autogkintegrate(tastate, eval, &intraC);
	//		alglib::autogkresults(tastate, TA, report);
	//		qDebug() << "True Positive" << TA;
	//		// Compute false refusal
	//		double FR;
	//		alglib::autogkstate frstate;
	//		alglib::autogksmooth(minDistance, distance, frstate);
	//		alglib::autogkintegrate(frstate, eval, &extraC);
	//		alglib::autogkresults(frstate, FR, report);
	//		qDebug() << "False Negative" << FR;
	// Generate score
	QList<QVariant> score;
	QString name;
	if (testIntra) name = "Test Intra - ";
	else name = "Test Extra - ";
	qDebug() << name;
	score.append(QVariant(name));
	score.append(QVariant(similarity));
	Q_EMIT newScore(score);
}
