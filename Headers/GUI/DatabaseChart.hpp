#ifndef DatabaseChart_hpp
#define DatabaseChart_hpp

#include <QVector>
#include <QtCharts>
#include <armadillo>
#include <UMF/functions.hpp>
#include <UMF/ComputeHistogram.hpp>
#include <UMF/Fitting1D.hpp>
#include <UMF/Evaluate1D.hpp>

namespace GUI{
	class DatabaseChart;
}

class GUI::DatabaseChart : public QtCharts::QChartView {
	
	Q_OBJECT
	
private:
	QColor genNewColor();
	
public:
	/** Constructor with initial settings. */
	DatabaseChart(QWidget* parent = Q_NULLPTR);
	
	public Q_SLOTS:
	/** Compute and plot the histogram in the chart.
	 This function is an overload.
	 @param[in] x Bars positions.
	 @param[in] y Bars height.*/
	Q_SLOT void plotHistogram();
	
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
	Q_SLOT void plotGaussianCurve(QVector<double> X = {},
								  QVector<double> C = {});
	
	Q_SLOT void addSpot(double);
	
Q_SIGNALS:
	Q_SIGNAL void raiseError(QString);
};

#endif /* DatabaseChart_hpp */
