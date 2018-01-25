#include <GUI/ChartRecWidget.hpp>

GUI::ChartRecWidget::ChartRecWidget(QWidget* parent) :
QtCharts::QChartView(parent){
	// Enable to zoom in the area selected by mouse
	setRubberBand(QChartView::RubberBand::RectangleRubberBand);
	// Hide charts legend
	chart()->legend()->hide();
	// Enable every animation
	chart()->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	// Set theme
	chart()->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);
	// Set antialising
	setRenderHint(QPainter::RenderHint::HighQualityAntialiasing);
}

void GUI::ChartRecWidget::display(QString fileName){
	// Remove every series already displayed
	chart()->removeAllSeries();
	// Process file
	QAlgorithm::PropertyMap FEPars = {
		{"SelectRecord", recIdx},
		{"MinFreq", QSettings().value("FE/MinFreq")},
		{"MaxFreq", QSettings().value("FE/MaxFreq")},
		{"Oversampling", QSettings().value("FE/Oversampling")},
		{"BEMinFilterRad", QSettings().value("FE/BEMinFilterRad")},
		{"BEMaxPeakWidth", QSettings().value("FE/BEMaxPeakWidth")},
		{"BEDerivDiam", QSettings().value("FE/BEDerivDiam")},
		{"BEMaxIterations", QSettings().value("FE/BEMaxIterations")},
		{"BEMaxInconsistency", QSettings().value("FE/BEMaxInconsistency")},
		{"BEMaxDistNodes", QSettings().value("FE/BEMaxDistNodes")},
		{"ButtFilterTail", QSettings().value("FE/ButtFilterTail")},
		{"PeakRelevance", QSettings().value("FE/PeakRelevance")},
		{"PeakMinVarInfluence", QSettings().value("FE/PeakMinVarInfluence")},
		{"BinWidth", QSettings().value("FE/BinWidth")},
		{"PeakHeightThreshold", QSettings().value("FE/PeakHeightThreshold")}
	};
	reader = AA::AudioReader::create({{"File", fileName}});
	auto extractor = AA::FeaturesExtractor::create(FEPars);
	reader >> extractor;
	// Plot specified series and handle errors
	connect(extractor.data(), SIGNAL(raiseError(QString)), this, SLOT(propagateError(QString)));
	connect(extractor.data(), SIGNAL(emitArray(QVector<double>)), this, SLOT(addSeries(QVector<double>)));
	// Make the process start
	QThreadPool::globalInstance()->start(extractor.data());
}

double GUI::ChartRecWidget::indexConversion(int k) {
	double SampleRate = reader->getOutSampleRate();
	double MaxFreq = QSettings().value("FE/MaxFreq").toDouble();
	double Oversampling = QSettings().value("FE/Oversampling").toDouble();
	int RecLength = ceil(0.5 * (1.0 + sqrt(1.0 + 4.0 * SampleRate * MaxFreq)) * 2.0 / 1024.0) * 1024 / Oversampling;
	return double(RecLength * recIdx + k) / double(SampleRate);
}

void GUI::ChartRecWidget::addSeries(QVector<double> newData){
	// Reset chart to the initial zoom and scroll
	chart()->zoomReset();
	// Generate a line serie
	QLineSeries* line = new QLineSeries;
	// Use OpenGL acceleration
	line->setUseOpenGL();
	// Assign to the chart
	chart()->addSeries(line);
	// Fill the series with data, compute min and max
	for (int k = 0; k < newData.size(); k++)
		*line << QPointF(indexConversion(k), newData.at(k));
	// Update axes to visualize the full plot area
	updateAxes();
}

void GUI::ChartRecWidget::updateAxes(){
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

void GUI::ChartRecWidget::keyPressEvent(QKeyEvent *event){
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

quint64 GUI::ChartRecWidget::stepUp() {
	this->recIdx++;
	display();
	return this->recIdx;
}
quint64 GUI::ChartRecWidget::stepDown() {
	if (this->recIdx > 0) this->recIdx--;
	display();
	return this->recIdx;
}
quint64 GUI::ChartRecWidget::getRecordIndex(){
	return this->recIdx;
}

void GUI::ChartRecWidget::propagateError(QString errorMsg){
	emit raiseError(errorMsg);
}
