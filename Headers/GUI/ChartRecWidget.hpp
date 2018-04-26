#ifndef ChartRecWidget_hpp
#define ChartRecWidget_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QDir>
#include <QtCharts>
#include <QString>
#include <functional>
#include <QAlgorithm.hpp>
#include <AA/FeaturesExtractor.hpp>

namespace GUI {
	/** Class to show audio signals in time domain. */
	class ChartRecWidget : public QtCharts::QChartView {
		
		Q_OBJECT
		
		/** The zoom factor; 1/zoomFactor is used to zoom out. */
		const double zoomFactor = 1.2;
		
		/** The scrolling speed. */
		const double moveFactor = 0.3;
		
		/** Reimplementation of the QChartView::keyPressEvent to handle keyboard interaction (zoom and scroll). */
		void keyPressEvent(QKeyEvent *event);
		
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
		
	Q_SIGNALS:
		Q_SIGNAL void raiseError(QString errorMsg);
	};
}

#endif /* ChartRecWidget_hpp */
