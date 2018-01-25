#ifndef ChartRecWidget_hpp
#define ChartRecWidget_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QDir>
#include <QtCharts>
#include <QString>
#include <QThread>
#include <QAlgorithm.hpp>
#include <AA/AudioReader.hpp>
#include <AA/FeaturesExtractor.hpp>

namespace GUI {
	class ChartRecWidget;
}

/** Class to show audio signals in time domain. */
class GUI::ChartRecWidget : public QtCharts::QChartView {
	
	Q_OBJECT
	
	const double zoomFactor = 1.2; /**< The zoom factor; 1/zoomFactor is used to zoom out. */
	const double moveFactor = 0.3; /**< The scrolling speed. */
	quint64 recIdx = 0; /**< The index of the record to visualize. */
	
	/** Reimplementation of the QChartView::keyPressEvent to handle keyboard interaction (zoom and scroll). */
	void keyPressEvent(QKeyEvent *event);
	
protected:
	QSharedPointer<AA::AudioReader> reader = Q_NULLPTR;
	/** Convert an array index to the corresponding time.
	 Can be overridden by subclasses to manage a different interpretation of the 
	 array index as a real x-axis position.
	 @sa GUI::ChartRecSpectrumWidget::indexConversion, addSeries
	 */
	virtual double indexConversion(int k);
	
	/** Update the axes to show the whole content of the scene.
	 This function can be overridden by subclasses to change its behaviour.
	 @sa addSeries, GUI::ChartRecSpectrumWidget::updateAxes
	 */
	virtual void updateAxes();
	
public:
	/** Constructs an instance and customizes its appearance. */
	ChartRecWidget(QWidget* parent = Q_NULLPTR);
	
	public Q_SLOTS:
	
	/** Loads the given file and processes a record from it, then shows results.
	 This function removes every series already added to the chart, then initializes
	 an AudioProcess instance to process the record indexed by the recIdx property.
	 Finally calls the start method to begin the computation.
	 @sa start, addSeries
	 */
	Q_SLOT void display(QString fileName = "");
	
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
	
	/** Increase the current record index and display the result. 
	 @sa stepDown, display
	 */
	Q_SLOT quint64 stepUp();
	
	/** Decrease the current record index and display the result. 
	 @sa stepUp, display
	 */
	Q_SLOT quint64 stepDown();
	
	/** Get the index of the current record.
	 @sa stepUp, stepDown
	 */
	Q_SLOT quint64 getRecordIndex();
	
	/** Simple slot that emits the raiseError signal with the same message received. */
	void propagateError(QString errorMsg);
	
Q_SIGNALS:
	Q_SIGNAL void raiseError(QString errorMsg);
};

#endif /* ChartRecWidget_hpp */
