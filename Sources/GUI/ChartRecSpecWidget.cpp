#include <GUI/ChartRecSpecWidget.hpp>

double GUI::ChartRecSpecWidget::indexConversion(int k) {
	double SampleRate = reader->getOutSampleRate();
	double MaxFreq = QSettings().value("FE/MaxFreq").toDouble();
	double Oversampling = QSettings().value("FE/Oversampling").toDouble();
	double RecLength = ceil(0.5 * (1.0 + sqrt(1.0 + 4.0 * SampleRate * MaxFreq)) * 2.0 / 1024.0) * 1024 / Oversampling;
	return bin2freq(SampleRate, RecLength, double(k));
}

void GUI::ChartRecSpecWidget::updateAxes(){
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

GUI::ChartRecSpecWidget::ChartRecSpecWidget(QWidget* parent) : ChartRecWidget(parent) {};

void GUI::ChartRecSpecWidget::setLogScale(){
	this->logscale = true;
}

void GUI::ChartRecSpecWidget::setNaturalScale(){
	this->logscale = false;
}

