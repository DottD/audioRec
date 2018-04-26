#include <GUI/DatabaseChart.hpp>

GUI::DatabaseChart::DatabaseChart(QWidget* parent) :
QtCharts::QChartView(parent){
	// Set chart appearance
	chart()->setTitle("Distribution");
	chart()->setTheme(QChart::ChartThemeBlueCerulean);
	chart()->setAnimationOptions(QChart::AnimationOption::AllAnimations);
	this->setRubberBand(QChartView::RubberBand::RectangleRubberBand);
	chart()->legend()->setVisible(false);
}

QColor GUI::DatabaseChart::genNewColor(){
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.4, 0.9);
	return QColor::fromRgbF(dis(gen), dis(gen), dis(gen), 0.7);
}

void GUI::DatabaseChart::plotHistogram(){
	// Get the sender object
	UMF::ComputeHistogram* histogram = dynamic_cast<UMF::ComputeHistogram*>(QObject::sender());
	// Check if it is a finished ComputeHistogram algorithm
	if(histogram == Q_NULLPTR || !histogram->hasFinished()) return;
	// Get the output
	const double BarStep = histogram->getBarStep();
	const QVector<double>& X = histogram->getOutHistX();
	const QVector<double>& Y = histogram->getOutHistY();
	QColor color = genNewColor();
	for(int l = 0; l < X.size(); ++l){
		QLineSeries* topLine = new QLineSeries;
		topLine->append(X[l]-BarStep/2.0, Y[l]);
		topLine->append(X[l]+BarStep/2.0, Y[l]);
		QAreaSeries* rectangle = new QAreaSeries(topLine);
		rectangle->setColor( color );
		rectangle->setBorderColor(QColor(0, 0, 0, 0));
		chart()->addSeries(rectangle);
	}
	chart()->createDefaultAxes();
}

void GUI::DatabaseChart::plotGaussianCurve(){
	// Define a short form to plot a given curve
	auto plotCurve = [this](QVector<double> X, QVector<double> Y){
		QLineSeries* fittingCurve = new QLineSeries;
		QPen linePen = fittingCurve->pen();
		linePen.setWidth(3);
		fittingCurve->setPen(linePen);
		for(int l = 0; l < X.size(); l++) fittingCurve->append(X[l], Y[l]);
		this->chart()->addSeries(fittingCurve);
	};
	// Get the sender object
	UMF::Fitting1D* fitting = dynamic_cast<UMF::Fitting1D*>(QObject::sender());
	// Check if it is a finished Fitting1D algorithm
	if(fitting != Q_NULLPTR && fitting->hasFinished()){
		Q_ASSERT(!fitting->getInX().empty());
		Q_ASSERT(!fitting->getOutCoefficients().empty());
		// Get the output
		const QVector<double>& x = fitting->getInX();
		const QVector<double>& c = fitting->getOutCoefficients();
		QVector<double> y = fitting->evaluate(c, x);
		plotCurve(x, y);
	} else {
		// Try to convert to an Evaluate1D
		UMF::Evaluate1D* evaluate = dynamic_cast<UMF::Evaluate1D*>(QObject::sender());
		// Check if it is a finished Evaluate1D algorithm
		if(evaluate != Q_NULLPTR && evaluate->hasFinished()){
			Q_ASSERT(!evaluate->getInX().empty());
			Q_ASSERT(!evaluate->getOutY().empty());
			// Get the output
			const QVector<double>& x = evaluate->getInX();
			const QVector<double>& y = evaluate->getOutY();
			plotCurve(x, y);
		} else {
			Q_EMIT raiseError("Sender must be a Fitting1D or an Evaluate1D");
		}
	}
	// Update the view
	this->chart()->createDefaultAxes();
}

void GUI::DatabaseChart::addSpot(double x){
	QScatterSeries* point = new QScatterSeries;
	point->append(x, 0);
	chart()->addSeries(point);
	chart()->createDefaultAxes();
}
