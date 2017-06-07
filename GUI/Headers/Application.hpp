#ifndef Application_hpp
#define Application_hpp

#include <QSet>
#include <QSharedPointer>
#include <QVariant>
#include <QMetaType>
#include <iostream>
#include <QDebug>

namespace Ui {
	class Parameter;
	
	uint qHash(Ui::Parameter key);
	uint qHash(Ui::Parameter key, uint seed);
	
	class Parameters;
	
	QDebug operator<<(QDebug debug, const Ui::Parameter& c);
}


class Ui::Parameter {
public:
	enum ParName : uint {
		ParNull,
		ParRecLength,
		ParGaussFilterRad,
		ParBackEstMinFilterRad,
		ParBackEstMaxPeakWidthAllowed,
		ParBackEstDerEstimationDiam,
		ParBackEstMaxIterations,
		ParBackEstMaxAllowedInconsistency,
		ParBackEstMaxDistNodes,
		ParIntervalStartFreq,
		ParIntervalWidthFreq,
		ParForeGaussFilterRad,
		ParBinWidth,
		ParTailSuppression,
		ParPeaksRelevance
	};
	
private:
	ParName key;
	
	QVariant value;
	
public:
	Parameter();
	Parameter(ParName key);
	Parameter(ParName key, QVariant value);
	
	void setKey(ParName key) {this->key = key;}
	void set(QVariant value) {this->value = value;}
	
	QVariant get() const {return this->value;}
	
	uint getKey() const {return static_cast<uint>(this->key);}
	
	bool operator==(const Parameter& a) const {
		return (a.getKey()==this->getKey());
	}
	
	static bool isIntParameter(ParName key);
};

/** Custom QApplication that handles user defined parameters.
 */
class Ui::Parameters : private QSet<Parameter> {
private:
	static QSharedPointer<QSet<Parameter>> parameters; /**< Map with not-default parameters.  */
	
public:
	Parameters() : QSet<Parameter>() {}; /**< Default constructor. */
	
	/** Get the requested parameter value.
	 @return Value of the requested parameter.
	 @sa Ui::Application::setParameter.
	 */
	static QVariant getParameter(Parameter::ParName key) {
		// Generate a Parameter with the given key
		Parameter par(key);
		// Return the parameter if set in the application, otherwise return its default value
		if (Parameters::parameters->contains(par))
			return Parameters::parameters->constFind(par)->get();
		else
			return par.get();
	};
	
	/** Set the parameter to the given value.
	 @sa Ui::Application::getParameter.
	 */
	static void setParameter(const Parameter::ParName& key, const QVariant& value) {
		// Instantiates a Parameter for searching in the container
		Parameter par(key);
		// Check value type consistency and eventually store it
		if (Parameter::isIntParameter(key) && value.type() == QVariant::Int) {
			// Check if the parameter is already contained in the set
			Parameters::parameters->remove(par);
			Parameters::parameters->insert(Parameter(key, QVariant(value.toInt())));
		}
		else if (!Parameter::isIntParameter(key) && value.type() == QVariant::Double) {
			// Check if the parameter is already contained in the set
			Parameters::parameters->remove(par);
			Parameters::parameters->insert(Parameter(key, QVariant(value.toDouble())));
		}
		else throw std::runtime_error("Value type not compatible");
	}
};

#endif /* Application_hpp */
