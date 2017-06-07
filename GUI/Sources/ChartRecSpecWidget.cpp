#include "../Headers/ChartRecSpecWidget.hpp"

void Ui::ChartRecSpecWidget::start(AudioProcess* process){
	// Make connection between the thread and this class
	connect(process, SIGNAL(raiseError(QString)),
			this, SLOT(propagateError(QString)));
	connect(process, SIGNAL(notableSpectrum(QSharedPointer<QVector<double>>)),
			this, SLOT(addSeries(QSharedPointer<QVector<double>>)));
	QThreadPool::globalInstance()->start(process);
}

double Ui::ChartRecSpecWidget::indexConversion(int k) {
	return bin2freq(double(getSampleRate()), double(Parameters::getParameter(Parameter::ParRecLength).toInt()), double(k));
}

void Ui::ChartRecSpecWidget::updateAxes(){
	// Compute maximum and minimum value accross every line
	qreal minX = __DBL_MAX__, maxX = __DBL_MIN__;
	qreal minY = __DBL_MAX__, maxY = __DBL_MIN__;
	QList<QAbstractSeries*> listOfSeries = chart()->series();
	for (QAbstractSeries* series: listOfSeries) {
		QLineSeries* line = dynamic_cast<QLineSeries*>(series);
		if (line == NULL) continue;
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
	chart()->axisX()->setTitleText("Frequency (Hz)");
	
	if (logscale){
		QLogValueAxis* logAxisY = new QLogValueAxis;
		logAxisY->setBase(10);
		chart()->removeAxis(chart()->axisY());
		chart()->addAxis(logAxisY, Qt::AlignLeft);
		for (QAbstractSeries* series: listOfSeries) {
			QLineSeries* line = dynamic_cast<QLineSeries*>(series);
			if (line == NULL) continue;
			line->attachAxis(logAxisY);
		}
		logAxisY->setRange(minY, maxY);
	} else {
		chart()->axisY()->setRange(minY, maxY);
	}
}
