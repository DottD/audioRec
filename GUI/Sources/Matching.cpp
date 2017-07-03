#include "Headers/Matching.hpp"

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

void Ui::Matching::eval(double x, double xminusa, double xminusb, double& y, void* ptr){
	alglib::real_1d_array *c = (alglib::real_1d_array*)ptr;
	alglib::real_1d_array xx;
	xx.setcontent(1, &x);
	gaussFit::funcVal(*c, xx, y, nullptr);
}

void Ui::Matching::match(){
	// Compute intra integral normalization factor
	double intraNormFactor, extraNormFactor;
	alglib::real_1d_array intraC, extraC;
	intraC.setcontent(intraCoeffs.size(), intraCoeffs.data());
	extraC.setcontent(extraCoeffs.size(), extraCoeffs.data());
	alglib::autogkstate intraState, extraState;
	alglib::autogksmooth(minDistance, maxDistance, intraState);
	alglib::autogksmooth(minDistance, maxDistance, extraState);
	alglib::autogkreport report;
	alglib::autogkintegrate(intraState, eval, &intraC);
	alglib::autogkresults(intraState, intraNormFactor, report);
	alglib::autogkintegrate(extraState, eval, &extraC);
	alglib::autogkresults(extraState, extraNormFactor, report);
	
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
			double distance = culpritFeat.distance(suspectFeat);
			Q_EMIT newDistance(distance);
			// Compute true acceptance
			double TA;
			alglib::autogkstate tastate;
			alglib::autogksmooth(distance, maxDistance, tastate);
			alglib::autogkintegrate(tastate, eval, &intraC);
			alglib::autogkresults(tastate, TA, report);
			TA /= intraNormFactor;
			// Compute false refusal
			double FR;
			alglib::autogkstate frstate;
			alglib::autogksmooth(minDistance, distance, frstate);
			alglib::autogkintegrate(frstate, eval, &extraC);
			alglib::autogkresults(frstate, FR, report);
			FR /= extraNormFactor;
			// Generate score
			QList<QVariant> score;
			QString name = culpritDir->dirName().split(".").first();
			qDebug() << name;
			name += "-" + suspectDir->dirName().split(".").first();
			qDebug() << name;
			score.append(QVariant(name));
			score.append(QVariant(TA));
			score.append(QVariant(FR));
			Q_EMIT newScore(score);
		}
	}
}
