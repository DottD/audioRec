#ifndef DatabaseChart_hpp
#define DatabaseChart_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QVector>
#include <QSettings>
#include <QtCharts>
#include <armadillo>
#include <functional>
#include <UMF/ComputeHistogram.hpp>
#include <UMF/CurveNormalization.hpp>
#include <UMF/Evaluate1D.hpp>
#include <AA/FeaturesExtractor.hpp>

namespace GUI{
	class DatabaseLine : public QLineSeries {
		
		Q_OBJECT
		
		Q_PROPERTY(double Minimum MEMBER m_Minimum READ getMinimum WRITE setMinimum NOTIFY parameterChanged)
		Q_PROPERTY(double Maximum MEMBER m_Maximum READ getMaximum WRITE setMaximum NOTIFY parameterChanged)
		Q_PROPERTY(int NumPoints MEMBER m_NumPoints READ getNumPoints WRITE setNumPoints NOTIFY parameterChanged)
		Q_PROPERTY(CurveType Type MEMBER m_Type READ getType WRITE setType NOTIFY parameterChanged)
		Q_PROPERTY(QVector<double> Coefficients MEMBER m_Coefficients READ getCoefficients WRITE setCoefficients NOTIFY parameterChanged)
		Q_PROPERTY(QList<QVector<double>> Features MEMBER m_Features READ getFeatures WRITE setFeatures NOTIFY featuresChanged)
		
	public:
		enum CurveType {
			Gaussian,
			GaussianExp
		};
		Q_ENUM(CurveType);
		
	private:
		double m_Minimum = 0.0;
		double m_Maximum = 1.0;
		int m_NumPoints = 100;
		CurveType m_Type = GaussianExp;
		QVector<double> m_Coefficients = {5.0, 0.3, 0.1, 0.1};
		QList<QVector<double>> m_Features;
		
	public:
		DatabaseLine(QObject* parente = Q_NULLPTR);
		
		double getMinimum() const {return m_Minimum;};
		double getMaximum() const {return m_Maximum;};
		int getNumPoints() const {return m_NumPoints;};
		CurveType getType() const {return m_Type;};
		QVector<double> getCoefficients() const {return m_Coefficients;};
		QList<QVector<double>> getFeatures() const {return m_Features;};
		
		void setMinimum(double min){m_Minimum=min; update(); Q_EMIT parameterChanged();};
		void setMaximum(double max){m_Maximum=max; update(); Q_EMIT parameterChanged();};
		void setNumPoints(int n_points){m_NumPoints=n_points; update(); Q_EMIT parameterChanged();};
		void setType(CurveType type){m_Type=type; update(); Q_EMIT parameterChanged();};
		void setCoefficients(QVector<double> coeff){m_Coefficients=coeff; update(); Q_EMIT parameterChanged();};
		void setFeatures(QList<QVector<double>> features){m_Features=features; Q_EMIT featuresChanged();};
		void addFeatures(QVector<double> features){m_Features << features; Q_EMIT featuresChanged();};
		
		static QVector<double> linspace(double min, double max, int points);
		static QVector<double> regspace(double min, double max, double step);
		
		static QColor genNewColor();
		
	Q_SIGNALS:
		Q_SIGNAL void parameterChanged();
		Q_SIGNAL void featuresChanged();
		
		public Q_SLOTS:
		Q_SLOT void update();
		
	};
	
	class QHistogramSeries : public QAreaSeries {
		
		Q_OBJECT
		
		Q_PROPERTY(QVector<double> X MEMBER m_X READ getX WRITE setX NOTIFY XChanged)
		Q_PROPERTY(QVector<double> Y MEMBER m_Y READ getY WRITE setY NOTIFY YChanged)
		
	private:
		QVector<double> m_X, m_Y;
		
		void tryBuildSeries();
		
	public:
		QHistogramSeries(QObject *parent = Q_NULLPTR):QAreaSeries(parent){};
		QHistogramSeries(QVector<double> X, QVector<double> Y);
		
		QVector<double> getX() const {return m_X;};
		QVector<double> getY() const {return m_Y;};
		
		void setX(QVector<double> X){m_X=X; tryBuildSeries(); Q_EMIT XChanged();};
		void setY(QVector<double> Y){m_Y=Y; tryBuildSeries(); Q_EMIT YChanged();};
		
	Q_SIGNALS:
		Q_SIGNAL void XChanged();
		Q_SIGNAL void YChanged();
	};
	
	class InteractiveChart : public QtCharts::QChartView {
		
		Q_OBJECT
		
		/** The zoom factor; 1/zoomFactor is used to zoom out. */
		const double zoomFactor = 1.2;
		
		/** The scrolling speed. */
		const double moveFactor = 0.3;
		
		/** Reimplementation of the QChartView::keyPressEvent to handle keyboard interaction (zoom and scroll). */
		void keyPressEvent(QKeyEvent *event);
		
	public:
		InteractiveChart(QWidget* parent = Q_NULLPTR);
	};
	
	class DatabaseChart : public InteractiveChart {
		
		Q_OBJECT
		
	public:
		/** Constructor with initial settings. */
		DatabaseChart(QWidget* parent = Q_NULLPTR);
		
		void updateViewWith(QAbstractSeries*);
		
		public Q_SLOTS:
		/** Compute and plot the histogram in the chart.
		 This function is an overload.
		 @param[in] x Bars positions.
		 @param[in] y Bars height.*/
		Q_SLOT void plotHistogram(QVector<double> X,
								  QVector<double> Y,
								  QString Name = "",
								  QColor Color = QColor::fromRgb(-1, -1, -1));
		
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
		Q_SLOT QLineSeries* plotCurve(QVector<double> X,
									  QVector<double> Y,
									  QString Name = "",
									  QColor Color = QColor::fromRgb(-1, -1, -1));
		
		Q_SLOT void addSpot(double);
		
		Q_SLOT void plotParametricCurve(QVector<double> Parameters,
										double min,
										double max,
										DatabaseLine::CurveType type,
										QString Name = "",
										QColor Color = QColor::fromRgb(-1, -1, -1));
		
	Q_SIGNALS:
		Q_SIGNAL void raise(QString);
	};
	
	/** Class to show audio signals in time domain. */
	class ChartRecWidget : public InteractiveChart {
		
		Q_OBJECT
		
	public:
		/** Constructs an instance and customizes its appearance. */
		ChartRecWidget(QWidget* parent = Q_NULLPTR);
		
		/** Convert a series index to the corresponding x value.
		 By default the function only casts the input value to double.
		 @sa addSeries, valueToYAxis
		 */
		std::function<double(int)> indexToXAxis = [](int i){return double(i);};
		
		/** Convert a series value to the corresponding y value.
		 By default the function only casts the input value to double.
		 @sa addSeries, indexToXAxis
		 */
		std::function<double(double)> valueToYAxis = [](double y){return y;};
		
		public Q_SLOTS:
		
		/** Add a series to the chart and adjust axes accordingly.
		 The data given as input contains only y-values, but not their corresponding
		 position on the x-axis; to insert a new series in the chart the position of a value
		 in the given array is interpreted as x, and the method indexConversion is used to rescale
		 the position. Subclasses can override indexConversion to convert an array index to
		 the real x position of the value.
		 To adjust the axes the function updateAxes is called, and it can be overridden by
		 subclasses to manage different behaviour.
		 @sa updateAxes, display, indexConversion
		 */
		Q_SLOT void addSeries(QVector<double> newData);
		
		Q_SLOT void addPoints(QVector<int> newData);
		
	Q_SIGNALS:
		Q_SIGNAL void raise(QString errorMsg);
	};
}

QDataStream& operator<<(QDataStream&, const GUI::DatabaseLine::CurveType&);
QDataStream& operator<<(QDataStream&, const GUI::DatabaseLine&);

QDataStream& operator>>(QDataStream&, GUI::DatabaseLine::CurveType&);
QDataStream& operator>>(QDataStream&, GUI::DatabaseLine&);

#endif /* DatabaseChart_hpp */
