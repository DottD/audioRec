#include "../Headers/ChartRecWidget.hpp"

void Ui::ChartRecWidget::display(QSharedPointer<QDir> dir){
	// Remove every series already displayed
	chart()->removeAllSeries();
	// Read audio file in a separate thread
	AudioProcess* process = new AudioProcess; // autodeleted by QThreadPool when finished
	// Check if the dir is empty (samples should be passed to child process) and if samples exist
	if (dir->path() == ".") {
		if (this->isEmpty()) {
			emit raiseError("Directory not set");
			return;
		} else process->setReader(reader);
	} else {
		process->setFileName(dir);
	}
	// Tell the child process who is calling it
	process->setCaller(this);
	// Tell the child process that must compute only the given record
	process->setComputeOnly(recIdx);
	// Make the right connection and start the child process (separate function to allow subclassing)
	start(process);
}

void Ui::ChartRecWidget::start(AudioProcess* process){
	// Make connection between the thread and this class
	connect(process, SIGNAL(raiseError(QString)),
			this, SLOT(propagateError(QString)));
	connect(process, SIGNAL(notableSeries(QSharedPointer<QVector<double>>)),
			this, SLOT(addSeries(QSharedPointer<QVector<double>>)));
	QThreadPool::globalInstance()->start(process);
}

double Ui::ChartRecWidget::indexConversion(int k) {
	return double(Application::getParameter(ParRecLength).toInt()*recIdx + k)/double(getSampleRate());
}

void Ui::ChartRecWidget::addSeries(QSharedPointer<QVector<double>> newData){
	// Reset chart to the initial zoom and scroll
	chart()->zoomReset();
	// Generate a line serie
	QLineSeries* line = new QLineSeries;
	// Use OpenGL acceleration
	line->setUseOpenGL();
	// Assign to the chart
	chart()->addSeries(line);
	// Fill the series with data, compute min and max
	for (int k = 0; k < newData->size(); k++)
		*line << QPointF(indexConversion(k), newData->at(k));
	// Update axes to visualize the full plot area
	updateAxes();
}

void Ui::ChartRecWidget::updateAxes(){
	// Compute maximum and minimum value accross every line
	qreal minX = __DBL_MAX__, maxX = __DBL_MIN__;
	qreal minY = __DBL_MAX__, maxY = __DBL_MIN__;
	QList<QAbstractSeries*> listOfSeries = chart()->series();
	for (QAbstractSeries* series: listOfSeries) {
		QLineSeries* line = dynamic_cast<QLineSeries*>(series);
		for (QPointF& point: line->points()) {
			minX = qMin(minX, point.x());
			maxX = qMax(maxX, point.x());
			minY = qMin(minY, point.y());
			maxY = qMax(maxY, point.y());
		}
	}
	
	// Update axes
	chart()->createDefaultAxes();
	
	chart()->axisX()->setRange(minX, maxX);
	chart()->axisX()->setTitleText("Time (s)");
	
	chart()->axisY()->setRange(minY, maxY);
}

bool Ui::ChartRecWidget::isEmpty(){
	return reader.isNull() || reader->getSamplesPtr()->isEmpty();
}

void Ui::ChartRecWidget::keyPressEvent(QKeyEvent *event){
	switch (event->key()) {
		case Qt::Key_Minus:
			chart()->zoom(1/zoomFactor);
			break;
		case Qt::Key_Plus:
			chart()->zoom(zoomFactor);
			break;
		case Qt::Key_Right:
			chart()->scroll(chart()->plotArea().width()*moveFactor, 0);
			break;
		case Qt::Key_Left:
			chart()->scroll(-chart()->plotArea().width()*moveFactor, 0);
			break;
		case Qt::Key_Up:
			chart()->scroll(0, chart()->plotArea().height()*moveFactor);
			break;
		case Qt::Key_Down:
			chart()->scroll(0, -chart()->plotArea().height()*moveFactor);
			break;
		case Qt::Key_R:
			chart()->zoomReset();
		default:
			break;
	}
}

quint64 Ui::ChartRecWidget::stepUp() {
	this->recIdx++;
	display();
	return this->recIdx;
}
quint64 Ui::ChartRecWidget::stepDown() {
	if (this->recIdx > 0) this->recIdx--;
	display();
	return this->recIdx;
}
