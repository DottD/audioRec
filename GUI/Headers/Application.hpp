#ifndef Application_hpp
#define Application_hpp

#include <QApplication>
#include <QHash>
#include <QSharedPointer>
#include <QVariant>

namespace Ui {
	const quint8 ParNull = 0;
	const quint8 ParRecLength = 1;
	const quint8 ParGaussFilterRad = 2;
	const quint8 ParBackEstMinFilterRad = 3;
	const quint8 ParBackEstMaxPeakWidthAllowed = 4;
	const quint8 ParBackEstDerEstimationDiam = 5;
	const quint8 ParBackEstMaxIterations = 6;
	const quint8 ParBackEstMaxAllowedInconsistency = 7;
	const quint8 ParBackEstMaxDistNodes = 8;
	const quint8 ParIntervalStartFreq = 9;
	const quint8 ParIntervalWidthFreq = 10;
	const quint8 ParForeGaussFilterRad = 11;
	const quint8 ParBinWidth = 12;
	
	class Application;
}

/** Custom QApplication that handles user defined parameters.
 */
class Ui::Application : public QApplication {
private:
	static QSharedPointer<QHash<quint8, QVariant>> parameters; /**< Map with not-default parameters.  */
	
public:
	Application(int &argc, char **argv) : QApplication(argc, argv) {}; /**< Call super constructor. */
	
	/** Get the requested parameter value.
	 @return Value of the requested parameter.
	 @sa Ui::Application::setParameter.
	 */
	static QVariant getParameter(quint8 par) {
		// Return the parameter if set in the application, otherwise return its default value
		if (Application::parameters->contains(par))
			return Application::parameters->value(par);
		else {
			switch (par) {
				case ParNull:
					return QVariant();
					break;
				case ParRecLength:
					return QVariant(int(18432));
					break;
				case ParGaussFilterRad:
					return QVariant(int(16-1));
					break;
				case ParBackEstMinFilterRad:
					return QVariant(int(32-1));
					break;
				case ParBackEstMaxPeakWidthAllowed:
					return QVariant(double(0.05));
					break;
				case ParBackEstDerEstimationDiam:
					return QVariant(int(4));
					break;
				case ParBackEstMaxIterations:
					return QVariant(int(10));
					break;
				case ParBackEstMaxAllowedInconsistency:
					return QVariant(double(2));
					break;
				case ParBackEstMaxDistNodes:
					return QVariant(int(3));
					break;
				case ParIntervalStartFreq:
					return QVariant(double(256));
					break;
				case ParIntervalWidthFreq:
					return QVariant(double(1024));
					break;
				case ParForeGaussFilterRad:
					return QVariant(int(32-1));
					break;
				case ParBinWidth:
					return QVariant(int(8));
					break;
				default:
					return QVariant();
					break;
			}
		}
	};
	
	/** Set the parameter to the given value.
	@sa Ui::Application::getParameter.
	 */
	static void setParameter(const quint8& par, const QVariant& value) {
		Application::parameters->insert(par, value);
	}
};

#endif /* Application_hpp */
