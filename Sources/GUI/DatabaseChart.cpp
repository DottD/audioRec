#include <GUI/DatabaseChart.hpp>

Ui::DatabaseChart::DatabaseChart(QWidget* parent) :
QtCharts::QChartView(parent){
	// Set chart appearance
	chart()->setTitle("Distribution");
	chart()->setTheme(QChart::ChartThemeBlueCerulean);
	chart()->setAnimationOptions(QChart::AnimationOption::AllAnimations);
	this->setRubberBand(QChartView::RubberBand::RectangleRubberBand);
	chart()->legend()->setVisible(false);
}

QColor Ui::DatabaseChart::genNewColor(){
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.4, 0.9);
	return QColor::fromRgbF(dis(gen), dis(gen), dis(gen), 0.7);
}

void Ui::DatabaseChart::plotHistogram(const arma::vec& x,
									  const arma::vec& y,
									  const QColor& color){
	const double barsStep = Parameters::getParameter(Parameter::ParBarsStep).toDouble();
	const double recWFactor = Parameters::getParameter(Parameter::ParRecWFactor).toDouble();
	// Draw the rectangles composing the histogram
	for (unsigned int k = 0; k < y.size(); k++){
		QLineSeries* topLine = new QLineSeries;
		topLine->append(x(k)-barsStep/recWFactor, y(k));
		topLine->append(x(k)+barsStep/recWFactor, y(k));
		QAreaSeries* rectangle = new QAreaSeries(topLine);
		rectangle->setColor(color);
		rectangle->setBorderColor(QColor(0, 0, 0, 0));
		chart()->addSeries(rectangle);
	}
	chart()->createDefaultAxes();
}

void Ui::DatabaseChart::plotGaussianCurve(const arma::vec& c,
										  const arma::vec& x,
										  const QColor& color){
	// Evaluate the gaussian function over the given domain
	arma::vec y = gaussExpFit::eval(c, x);
	// Plot the line
	QLineSeries* fittingCurve = new QLineSeries;
	fittingCurve->setColor(color);
	QPen linePen = fittingCurve->pen();
	linePen.setWidth(3);
	fittingCurve->setPen(linePen);
	for(uint64_t k = 0; k < y.size(); k++) fittingCurve->append(x[k], y[k]);
	chart()->addSeries(fittingCurve);
	chart()->createDefaultAxes();
}

bool Ui::DatabaseChart::getFitOverlay(){
	return this->fitOverlay;
}

void Ui::DatabaseChart::setFitOverlay(bool val){
	this->fitOverlay = val;
}

void Ui::DatabaseChart::plotHistogram(QVector<double> x,
									  QVector<double> y){
	arma::vec xx, yy;
	convert::qvec2arma(x, xx);
	convert::qvec2arma(y, yy);
	// Plot the histogram of the given data
	const QColor color = genNewColor();
	plotHistogram(xx, yy, color);
	// Plot the fitting curve if requested
	if(getFitOverlay()){
		const arma::vec coeff = gaussExpFit::fit(xx, yy);
		plotGaussianCurve(coeff, xx, color.lighter());
	}
}

void Ui::DatabaseChart::plotGaussianCurve(QVector<double> c,
										  QVector<double> x){
	plotGaussianCurve(arma::vec(c.data(), c.size(), false, false),
					  arma::vec(x.data(), x.size(), false, false),
					  genNewColor());
}

void Ui::DatabaseChart::addSpot(double x){
	QScatterSeries* point = new QScatterSeries;
	point->append(x, 0);
	chart()->addSeries(point);
	chart()->createDefaultAxes();
}
