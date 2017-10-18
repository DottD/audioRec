#ifndef Application_hpp
#define Application_hpp

#include <QSet>
#include <QSharedPointer>
#include <QVariant>
#include <QMetaType>
#include <QDebug>

namespace Ui {
	class Parameter;
	
	/** Computes a hash value for the given parameter. */
	uint qHash(Ui::Parameter key);
	
	/** Computes a hash value for the given parameter (overload). */
	uint qHash(Ui::Parameter key, uint seed);
	
	class Parameters;
	
	/** Print debug information about a parameter. */
	QDebug operator<<(QDebug debug, const Ui::Parameter& c);
}

/** Class that stores a parameter.
 This class provides methods to handle the key and the value of the parameter,
 along with its default value.*/
class Ui::Parameter {
public:
	enum ParName : uint {
		ParNull,
		ParRecLength,
		ParMaxFreq,
		ParMinFreq,
		ParOversampling,
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
		ParPeaksRelevance,
		ParPeakMinVariationInfluence,
		ParPeakHeightThreshold,
		ParBarsStep,
		ParRecWFactor
	};
	
private:
	ParName key; /**< Current parameter key. */
	
	QVariant value; /**< Current parameter value. */
	
public:
	/** Empty constructor. */
	Parameter();
	
	/** Creates a parameter with the given key and its default value. */
	Parameter(ParName key);
	
	/** Creates a parameter with given key and value. */
	Parameter(ParName key, QVariant value);
	
	/** Set the key of the parameter. */
	void setKey(ParName key) {this->key = key;}
	
	/** Set the value of the parameter. */
	void set(QVariant value) {this->value = value;}
	
	/** Returns the current value for the parameter. */
	QVariant get() const {return this->value;}
	
	/** Returns the key of the parameter. */
	uint getKey() const {return static_cast<uint>(this->key);}
	
	/** Compares two parameters, considering them equal if they share the same key. */
	bool operator==(const Parameter& a) const {
		return (a.getKey()==this->getKey());
	}
	
	/** Returns whether the parameter is an integer or a double. */
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
