#ifndef Application_hpp
#define Application_hpp

#include <QApplication>
#include <QHash>
#include <QSharedPointer>
#include <QVariant>

namespace Ui {
	const quint8 ParNull = 0;
	const quint8 ParRecLength = 1;
	const quint8 ParMovAvgRadius = 2;
	const quint8 ParLowpassFreq = 3;
	const quint8 ParMinFilterFreq = 4;
	const quint8 ParMaxFilterFreq = 5;
	const quint8 ParMovAvgSpecRad = 6;
	const quint8 ParEstBackAveRadius = 7;
	const quint8 ParEstBackMinRadius = 8;
	const quint8 ParIntervalStartFreq = 9;
	const quint8 ParIntervalWidthFreq = 10;
	
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
					return QVariant(int(16384));
					break;
				case ParMovAvgRadius:
					return QVariant(int(15));
					break;
				case ParLowpassFreq:
					return QVariant(double(0));
					break;
				case ParMinFilterFreq:
					return QVariant(double(400));
					break;
				case ParMaxFilterFreq:
					return QVariant(double(3500));
					break;
				case ParMovAvgSpecRad:
					return QVariant(int(140));
					break;
				case ParIntervalStartFreq:
					return QVariant(double(500));
					break;
				case ParIntervalWidthFreq:
					return QVariant(double(1000));
					break;
				case ParEstBackAveRadius:
					return QVariant(int(50));
					break;
				case ParEstBackMinRadius:
					return QVariant(int(100));
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
