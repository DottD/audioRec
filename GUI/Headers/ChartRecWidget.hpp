#ifndef ChartRecWidget_hpp
#define ChartRecWidget_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QDir>
#include <QtCharts>
#include <QString>
#include <QThread>
#include <Headers/AudioReader.hpp>
#include <Headers/AudioProcess.hpp>
#include <Headers/Application.hpp>

#ifndef QSHARED_QDIR_META_DEF
#define QSHARED_QDIR_META_DEF
Q_DECLARE_METATYPE(QSharedPointer<QDir>)
#endif

#ifndef QSHARED_AUDIOREADER_META_DEF
#define QSHARED_AUDIOREADER_META_DEF
Q_DECLARE_METATYPE(QSharedPointer<Ui::AudioReader>)
#endif

namespace Ui {
	class ChartRecWidget;
}

/** Class to show audio signals in time domain. */
class Ui::ChartRecWidget : public QtCharts::QChartView {
	Q_OBJECT
	
	QSharedPointer<AudioReader> reader; /**< The AudioReader instance that contains the read file. */
	
	const double zoomFactor = 1.2; /**< The zoom factor; 1/zoomFactor is used to zoom out. */
	const double moveFactor = 0.3; /**< The scrolling speed. */
	quint64 recIdx = 0; /**< The index of the record to visualize. */
	
	/** Reimplementation of the QChartView::keyPressEvent to handle keyboard interaction (zoom and scroll). */
	void keyPressEvent(QKeyEvent *event);
	
protected:
	/** Convert an array index to the corresponding time.
	 Can be overridden by subclasses to manage a different interpretation of the 
	 array index as a real x-axis position.
	 @sa Ui::ChartRecSpectrumWidget::indexConversion, addSeries
	 */
	virtual double indexConversion(int k);
	
	/** Makes connections and then starts the given process.
	 This method can be overridden by subclasses to manage different behaviour.
	 @sa display, Ui::ChartRecSpectrumWidget::start
	 */
	virtual void start(AudioProcess* process);
	
	/** Update the axes to show the whole content of the scene.
	 This function can be overridden by subclasses to change its behaviour.
	 @sa addSeries, Ui::ChartRecSpectrumWidget::updateAxes
	 */
	virtual void updateAxes();
	
public:
	/** Constructs an instance and customizes its appearance. */
	ChartRecWidget(QWidget* parent = Q_NULLPTR) : QtCharts::QChartView(parent) {
		qRegisterMetaType<QSharedPointer<QDir>>();
		qRegisterMetaType<QSharedPointer<AudioReader>>();
		// Enable to zoom in the area selected by mouse
		this->setRubberBand(QChartView::RubberBand::RectangleRubberBand);
		// Hide charts legend
		chart()->legend()->hide();
		// Enable every animation
		chart()->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
		// Set theme
		chart()->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);
		// Set antialising
		this->setRenderHint(QPainter::RenderHint::HighQualityAntialiasing);
	};
	
	public slots:
	
	/** Loads the given file and processes a record from it, then shows results.
	 This function removes every series already added to the chart, then initializes
	 an AudioProcess instance to process the record indexed by the recIdx property.
	 Finally calls the start method to begin the computation.
	 @sa start, addSeries
	 */
	void display(QSharedPointer<QDir> dir = QSharedPointer<QDir>(new QDir));
	
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
	void addSeries(QSharedPointer<QVector<double>> newData);
	
	/** Increase the current record index and display the result. 
	 @sa stepDown, display
	 */
	void stepUp();
	
	/** Decrease the current record index and display the result. 
	 @sa stepUp, display
	 */
	void stepDown();
	
	/** Set the AudioReader property.
	 This function sets the AudioReader to a previously allocated instance.
	 @sa getReader
	 */
	void setReader(QSharedPointer<AudioReader> reader) {this->reader = reader;}
	
	/** Returned a QSharedPointer to the AudioReader of this instance.
	 No check is done on the AudioReader instance of this instance.
	 @sa isEmpty, getSampleCount, getSampleRate
	 */
	QSharedPointer<AudioReader> getReader() {return this->reader;}
	
	/** Return the total number of samples of the loaded audio file.
	 No check is done on the AudioReader instance of this instance.
	 @sa getSampleRate, getReader, isEmpty
	 */
	int getSampleCount() {return this->reader->getSampleCount();}
	
	/** Return the sample rate of the loaded audio file.
	 No check is done on the AudioReader instance of this instance.
	 @sa getSampleCount, getReader, isEmpty
	 */
	quint32 getSampleRate() {return this->reader->getSampleRate();}
	
	/** Gives true if the AudioReader has loaded the file. */
	bool isEmpty();
	
	/** Simple slot that emits the raiseError signal with the same message received. */
	void propagateError(QString errorMsg) {emit raiseError(errorMsg);};
	
signals:
	void raiseError(QString errorMsg);
};

#endif /* ChartRecWidget_hpp */
