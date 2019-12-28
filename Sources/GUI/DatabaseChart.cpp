#include <GUI/DatabaseChart.hpp>

GUI::DatabaseLine::DatabaseLine(QObject* parent):
QLineSeries::QLineSeries(parent){
	qRegisterMetaType<CurveType>();
}

void GUI::DatabaseLine::update(){
	if(getMinimum() >= getMaximum() || getNumPoints() <= 0 || getCoefficients().isEmpty()) return;
	auto min = QSettings().value("Histogram/MinimumValue").toDouble();
	auto max = QSettings().value("Histogram/MaximumValue").toDouble();
	QVector<double> X = DatabaseLine::linspace(min, max, getNumPoints());
	QVector<double> Y;
	switch (getType()) {
		case Gaussian: {
			auto evaluation = UMF::EvaluateGauss::create({
				{"X", QVariant::fromValue(X)},
				QAlgorithm::makeOITable({{"Coefficients","Coefficients"}})
			});
			evaluation << UMF::CurveNormalization::create({
				{"LeftExtremum", min}, {"RightExtremum", max},
				{"Coefficients", QVariant::fromValue(getCoefficients())}
			});
			evaluation->serialExecution();
			Y = evaluation->getOutMoveY();
		} break;
		case GaussianExp: {
			auto evaluation = UMF::EvaluateGaussExp::create({
				{"X", QVariant::fromValue(X)},
				QAlgorithm::makeOITable({{"Coefficients","Coefficients"}})
			});
			evaluation << UMF::CurveNormalization::create({
				{"LeftExtremum", getMinimum()}, {"RightExtremum", getMaximum()},
				{"Coefficients", QVariant::fromValue(getCoefficients())}
			});
			evaluation->serialExecution();
			Y = evaluation->getOutMoveY();
		} break;
	}
	if(!Y.isEmpty()){
		if(!color().isValid()) setColor(DatabaseLine::genNewColor());
		clear();
		for(int k = 0; k < Y.size(); ++k){
			append(X[k], Y[k]);
		}
		QPen linePen = pen();
		linePen.setWidth(3);
		setPen(linePen);
	}
}

QVector<double> GUI::DatabaseLine::linspace(double min, double max, int N){
	QVector<double> X;
	auto step = (max-min)/(N-1);
	X.reserve(N);
	for(int n = 0; n < N; ++n) X << min + n * step;
	return X;
}

QVector<double> GUI::DatabaseLine::regspace(double min, double max, double step){
	QVector<double> X;
	int N = (max-min)/step;
	X.reserve(N);
	for(int n = 0; n < N; ++n) X << min + n * step;
	return X;
}

QColor GUI::DatabaseLine::genNewColor(){
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.5, 1.0);
	return QColor::fromRgbF(dis(gen), dis(gen), dis(gen), 0.7);
}

void GUI::QHistogramSeries::tryBuildSeries(){
	if(getX().isEmpty() || getY().isEmpty() || getX().size()!=getY().size()) return;
	const auto& X = getX();
	const auto& Y = getY();
	// Compute the barSteps as half the distance between consecutive Xs
	QVector<double> barSteps;
	barSteps.reserve(X.size()-1);
	std::transform(X.begin()+1, X.end(), X.begin(), std::back_inserter(barSteps), std::minus<>{});
	std::for_each(barSteps.begin(), barSteps.end(), [](auto& x){x/=2.0;});
	// Normalize the vector Y with respect to the sum (useful for comparing plots)
	// The vector still needs to be divided by twice the step
	auto scaleFactor = 1.0 / std::reduce(Y.begin(), Y.end());
	// Display the histogram from the input values
	setLowerSeries(new QLineSeries);
	setUpperSeries(new QLineSeries);
	for(int l = 0; l < getX().size(); ++l){
		double LW, RW;
		if(l==0){
			LW = barSteps.first();
			RW = LW;
		}else if(l==getX().size()-1){
			LW = barSteps.last();
			RW = LW;
		}else{
			LW = barSteps[l-1];
			RW = barSteps[l];
		}
		double value = Y[l]*scaleFactor/(RW+LW);
		double left = X[l]-LW;
		double right = X[l]+RW;
		upperSeries()->append(left, value);
		upperSeries()->append(right, value);
		lowerSeries()->append(left, 0.);
		lowerSeries()->append(right, 0.);
	}
}

GUI::QHistogramSeries::QHistogramSeries(QVector<double> X, QVector<double> Y){
	setX(X);
	setY(Y);
}

GUI::InteractiveChart::InteractiveChart(QWidget* parent) :
QtCharts::QChartView(parent){}

void GUI::InteractiveChart::keyPressEvent(QKeyEvent *event){
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

void GUI::DatabaseChart::updateViewWith(QAbstractSeries* series){
	chart()->addSeries(series);
	chart()->createDefaultAxes();
	if(auto aX = qobject_cast<QValueAxis*>(chart()->axisX()); aX){
		aX->setRange(QSettings().value("Histogram/MinimumValue").toDouble(),
					 QSettings().value("Histogram/MaximumValue").toDouble());
	}
}

GUI::DatabaseChart::DatabaseChart(QWidget* parent) :
InteractiveChart(parent){
	// Set chart appearance
	chart()->setTitle("Distribution");
	chart()->setTheme(QChart::ChartThemeBlueCerulean);
	chart()->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->setRubberBand(QChartView::RubberBand::RectangleRubberBand);
	chart()->legend()->setVisible(true);
	chart()->legend()->setAlignment(Qt::AlignmentFlag::AlignRight);
}

void GUI::DatabaseChart::plotHistogram(QVector<double> X,
									   QVector<double> Y,
									   QString Name,
									   QColor Color){
	// Choose color
	if(!Color.isValid()) Color = DatabaseLine::genNewColor();
	// Generate the histogram
	auto histogram = new QHistogramSeries(X, Y);
	histogram->setName(Name);
	histogram->setColor(Color);
	histogram->setBorderColor(QColor(0,0,0,0));
	chart()->legend()->setMarkerShape(QtCharts::QLegend::MarkerShapeCircle);
	// Create axis
	updateViewWith(histogram);
}

QLineSeries* GUI::DatabaseChart::plotCurve(QVector<double> X,
										   QVector<double> Y,
										   QString Name,
										   QColor Color){
	// Choose color
	if(!Color.isValid()) Color = DatabaseLine::genNewColor();
	// Define a short form to plot a given curve
	QLineSeries* line = new QLineSeries;
	QPen linePen = line->pen();
	linePen.setWidth(3);
	line->setPen(linePen);
	for(int l = 0; l < X.size(); l++) line->append(X[l], Y[l]);
	line->setColor(Color);
	line->setName(Name);
	updateViewWith(line);
	return line;
}

void GUI::DatabaseChart::addSpot(double x){
	// Also compute the mean value at the new points to draw
	double mean = 0.0;
	int count = 0;
	auto less_lower_x = [](const QPointF& a, const double& b){return a.x() < b;};
	auto less_upper_x = [](const double& a, const QPointF& b){return a < b.x();};
	for(auto series: chart()->series()){
		if (auto line = dynamic_cast<QLineSeries*>(series); line){
			auto points = line->pointsVector();
			if(points.size() >= 2){
				auto left = std::lower_bound(points.constBegin(), points.constEnd(), x, less_lower_x);
				auto right = std::upper_bound(points.constBegin(), points.constEnd(), x, less_upper_x);
				if(left != points.end() && right != points.end()){
					double innerSum = 0.0;
					for(; left < right; ++left) innerSum += left->y();
					mean += innerSum / std::distance(left, right);
					++count;
				}
			}
		}
	}
	if (count > 0) mean /= count;
	// Display point
	QScatterSeries* point = new QScatterSeries;
	point->append(x, mean);
	updateViewWith(point);
}

void GUI::DatabaseChart::plotParametricCurve(QVector<double> C,
											 double min,
											 double max,
											 DatabaseLine::CurveType type,
											 QString Name,
											 QColor Color){
	auto line = new DatabaseLine;
	line->setMinimum(min);
	line->setMaximum(max);
	line->setCoefficients(C);
	line->setType(type);
	line->setName(Name);
	line->setColor(Color);
	line->setNumPoints(QSettings().value("Plot/Points").toInt());
	updateViewWith(line);
}

GUI::ChartRecWidget::ChartRecWidget(QWidget* parent) :
InteractiveChart(parent){
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
	for (auto series: chart()->series()) {
		if(auto line = dynamic_cast<QXYSeries*>(series); line){
			for (QPointF& point: line->points()) {
				minX = qMin(minX, point.x());
				maxX = qMax(maxX, point.x());
				minY = qMin(minY, point.y());
				maxY = qMax(maxY, point.y());
			}
		}
	}
	
	// Update axes
	chart()->createDefaultAxes();
	
	chart()->axisX()->setRange(minX, maxX);
	chart()->axisX()->setTitleText(property("xAxisTitle").toString());
	
	chart()->axisY()->setRange(minY, maxY);
	chart()->axisY()->setTitleText(property("yAxisTitle").toString());
}

void GUI::ChartRecWidget::addPoints(QVector<int> newData){
	// Reset chart to the initial zoom and scroll
	chart()->zoomReset();
	// Generate a line serie
	QScatterSeries* points = new QScatterSeries;
	// Appearance
	points->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeCircle);
	points->setColor(QColor::fromRgb(255,0,0));
	points->setMarkerSize(5);
	// Use OpenGL acceleration
	points->setUseOpenGL();
	// Assign to the chart
	chart()->addSeries(points);
	
	// Compute the transformed new data X values
	QVector<double> newDataX;
	std::transform(newData.constBegin(), newData.constEnd(), std::back_inserter(newDataX), indexToXAxis);
	// Also compute the mean value at the new points to draw
	QVector<double> newDataY;
	for(auto k: newData){
		double mean = 0.0;
		int count = 0;
		for(auto series: chart()->series()){
			if (auto line = dynamic_cast<QLineSeries*>(series); line){
				const auto& point = line->at(k);
				if (newDataX.contains(point.x())){
					mean += point.y();
					++count;
				}
			}
		}
		if (count > 0) mean /= count;
		newDataY << std::move(mean);
	}
	// Add points to the chart
	for(int k = 0; k < newData.size(); ++k){
		*points << QPointF(newDataX[k], newDataY[k]);
	}
	
	// Compute maximum and minimum value accross every line
	qreal minX = __DBL_MAX__, maxX = __DBL_MIN__;
	qreal minY = __DBL_MAX__, maxY = __DBL_MIN__;
	for (auto series: chart()->series()) {
		if (auto line = dynamic_cast<QXYSeries*>(series); line){
			for (QPointF& point: line->points()) {
				minX = qMin(minX, point.x());
				maxX = qMax(maxX, point.x());
				minY = qMin(minY, point.y());
				maxY = qMax(maxY, point.y());
			}
		}
	}
	
	// Update axes
	chart()->createDefaultAxes();
	
	chart()->axisX()->setRange(minX, maxX);
	chart()->axisX()->setTitleText(property("xAxisTitle").toString());
	
	chart()->axisY()->setRange(minY, maxY);
	chart()->axisY()->setTitleText(property("yAxisTitle").toString());
}

QDataStream& operator<<(QDataStream& stream, const GUI::DatabaseLine::CurveType& type){
	quint8 value = static_cast<GUI::DatabaseLine::CurveType>(type);
	stream << value;
	return stream;
}

QDataStream& operator<<(QDataStream& stream, const GUI::DatabaseLine& line){
	stream << line.name() << line.getType() << line.color() << line.getMinimum()
	<< line.getMaximum() << line.getCoefficients() << line.getFeatures();
	return stream;
}

QDataStream& operator>>(QDataStream& stream, GUI::DatabaseLine::CurveType& type){
	quint8 value;
	stream >> value;
	type = static_cast<GUI::DatabaseLine::CurveType>(value);
	return stream;
}

QDataStream& operator>>(QDataStream& stream, GUI::DatabaseLine& line){
	auto name = line.name();
	auto minimum = line.getMinimum();
	auto maximum = line.getMaximum();
	auto type = line.getType();
	auto coefficients = line.getCoefficients();
	auto features = line.getFeatures();
	auto color = line.color();
	stream >> name >> type >> color >> minimum >> maximum >> coefficients >> features;
	line.setName(name);
	line.setType(type);
	line.setColor(color);
	line.setMinimum(minimum);
	line.setMaximum(maximum);
	line.setCoefficients(coefficients);
	line.setFeatures(features);
	line.update();
	return stream;
}
