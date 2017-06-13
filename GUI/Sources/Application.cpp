#include "../Headers/Application.hpp"

QSharedPointer<QSet<Ui::Parameter>> Ui::Parameters::parameters = QSharedPointer<QSet<Ui::Parameter>>(new QSet<Ui::Parameter>);

QDebug Ui::operator<<(QDebug debug, const Ui::Parameter& c) {
	QDebugStateSaver saver(debug);
	
	if (c.get().type() == QVariant::Type::Int)
		debug.nospace() << "(" << c.getKey() << ", " << c.get().toInt() << ")";
	else if (c.get().type() == QVariant::Type::Double)
		debug.nospace() << "(" << c.getKey() << ", " << c.get().toDouble() << ")";
	else
		debug.nospace() << "(" << c.getKey() << ", incorrect-type)";
	
	return debug;
}

uint Ui::qHash(Ui::Parameter key){
	return key.getKey();
}
uint Ui::qHash(Ui::Parameter key, uint seed){
	return key.getKey();
}

Ui::Parameter::Parameter(){
	setKey(ParName::ParNull);
	set(0);
}

Ui::Parameter::Parameter(ParName key){
	// Set the given key
	setKey(key);
	// Set the default value for that key
	switch (key) {
		case ParNull:
			set(0);
			break;
		case ParRecLength:
			set(int(18432));
			break;
		case ParGaussFilterRad:
			set(int(16-1));
			break;
		case ParBackEstMinFilterRad:
			set(int(32-1));
			break;
		case ParBackEstMaxPeakWidthAllowed:
			set(double(0.1));
			break;
		case ParBackEstDerEstimationDiam:
			set(int(4));
			break;
		case ParBackEstMaxIterations:
			set(int(10));
			break;
		case ParBackEstMaxAllowedInconsistency:
			set(double(2));
			break;
		case ParBackEstMaxDistNodes:
			set(int(3));
			break;
		case ParIntervalStartFreq:
			set(double(200));
			break;
		case ParIntervalWidthFreq:
			set(double(1024));
			break;
		case ParForeGaussFilterRad: // not used
			set(int(32-1));
			break;
		case ParBinWidth:
			set(int(8));
			break;
		case ParTailSuppression:
			set(double(0.1));
			break;
		case ParPeaksRelevance:
			set(double(0.3));
			break;
		case ParPeakMinVariationInfluence:
			set(double(0.01));
			break;
		case ParPeakHeightThreshold:
			set(double(0.03));
			break;
	}
}

Ui::Parameter::Parameter(ParName key, QVariant value){
	// Set the given key and value
	setKey(key);
	set(value);
}

bool Ui::Parameter::isIntParameter(ParName key) {
	switch (key) {
		case ParNull:
			return true;
			break;
		case ParRecLength:
			return true;
			break;
		case ParGaussFilterRad:
			return true;
			break;
		case ParBackEstMinFilterRad:
			return true;
			break;
		case ParBackEstMaxPeakWidthAllowed:
			return false;
			break;
		case ParBackEstDerEstimationDiam:
			return true;
			break;
		case ParBackEstMaxIterations:
			return true;
			break;
		case ParBackEstMaxAllowedInconsistency:
			return false;
			break;
		case ParBackEstMaxDistNodes:
			return true;
			break;
		case ParIntervalStartFreq:
			return false;
			break;
		case ParIntervalWidthFreq:
			return false;
			break;
		case ParForeGaussFilterRad:
			return true;
			break;
		case ParBinWidth:
			return true;
			break;
		case ParTailSuppression:
			return false;
			break;
		case ParPeaksRelevance:
			return false;
			break;
		case ParPeakMinVariationInfluence:
			return false;
			break;
		case ParPeakHeightThreshold:
			return false;
			break;
	}
}
