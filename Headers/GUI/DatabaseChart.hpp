#ifndef DatabaseChart_hpp
#define DatabaseChart_hpp

#include <QVector>
#include <QtCharts>
#include <armadillo>
#include <UMF/functions.hpp>
#include <AA/Application.hpp>

namespace Ui{
	class DatabaseChart;
}

class Ui::DatabaseChart : public QtCharts::QChartView {
	Q_OBJECT
	
	Q_PROPERTY(bool fitOverlay READ getFitOverlay WRITE setFitOverlay);
	
	/** Whether the fitting will be plot or not.
	 After an histogram is added to the chart, the chart can overlay the fitting curve
	 related to the histogram. The default value is false.
	 @sa getFitOverlay, setFitOverlay
	 */
	bool fitOverlay = true;
	
	/** Generates a random new color, suitable for chart drawings. */
	QColor genNewColor();
	
	/** Plot the histogram in the chart.
	 @param[in] x Bars positions.
	 @param[in] y Bars height.
	 @param[in] color The color used for drawing.*/
	void plotHistogram(const arma::vec& x,
					   const arma::vec& y,
					   const QColor& color);
	
	/** Plot a gaussian curve, given its coefficients. The gaussian function is
	 of the form
	 \f[
		f(x) = c_1 e^{\frac{(x-c_1)^2}{2c_3^2}}
	 \f]
	 where \f$ c_1 \f$, \f$ c_2 \f$ and \f$ c_3 \f$ are the parameters.
	 @param[in] coefficients Coefficients of the gaussian function.
	 @param[in] domain The set of points which the function must be evalutated on.
	 @param[in] color Color to use when drawing.
	 */
	void plotGaussianCurve(const arma::vec& coefficients,
						   const arma::vec& domain,
						   const QColor& color);
	
public:
	/** Constructor with initial settings. */
	DatabaseChart(QWidget* parent = Q_NULLPTR);
	
	/** Get the current fitOverlay value.
	 @sa fitOverlay, setFitOverlay
	 */
	bool getFitOverlay();
	
	/** Set the fitOverlay value.
	 @sa fitOverlay, getFitOverlay
	 */
	void setFitOverlay(bool val = true);
	
	public Q_SLOTS:
	/** Compute and plot the histogram in the chart.
	 This function is an overload.
	 @param[in] x Bars positions.
	 @param[in] y Bars height.*/
	Q_SLOT void plotHistogram(QVector<double> x,
							  QVector<double> y);
	
	/** Plot a gaussian curve, given its coefficients. 
	 This function is an overload. The gaussian function is
	 of the form
	 \f[
		f(x) = c_1 e^{\frac{(x-c_1)^2}{2c_3^2}}
	 \f]
	 where \f$ c_1 \f$, \f$ c_2 \f$ and \f$ c_3 \f$ are the parameters.
	 @param[in] coefficients Coefficients of the gaussian function.
	 @param[in] domain The set of points which the function must be evalutated on.
	 */
	Q_SLOT void plotGaussianCurve(QVector<double> coefficients,
								  QVector<double> domain);
	
	Q_SLOT void addSpot(double);
};

#endif /* DatabaseChart_hpp */
