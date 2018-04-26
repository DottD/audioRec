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
		*line << QPointF(indexToXAxis(k), valueToYAxis(newData.at(k)));
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
	chart()->axisX()->setTitleText(property("xAxisTitle").toString());
	
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
