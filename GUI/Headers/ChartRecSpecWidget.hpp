#ifndef ChartRecSpecWidget_hpp
#define ChartRecSpecWidget_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QVector>
#include <Headers/ChartRecWidget.hpp>

namespace Ui {
	class ChartRecSpecWidget;
}

/** Class to show audio signals' spectrums in frequency domain. */
class Ui::ChartRecSpecWidget : public Ui::ChartRecWidget {
	
	Q_OBJECT
	
protected:
	
	/** Converts an array index to the real frequency.
	 Overridden method from Ui::ChartRecWidget::indexConversion.*/
	double indexConversion(int k);
	
	/** Make connection and start the given AudioProcess.
	 Overridden method from Ui::ChartRecWidget::start.*/
	void start(AudioProcess* process);
	
	/** Update the axes to show the whole scene.
	 A logarithmic scale is applied to the y-axis.
	 Overridden method from Ui::ChartRecWidget::updateAxes.*/
	void updateAxes();
	
public:
	ChartRecSpecWidget(QWidget* parent = Q_NULLPTR) : ChartRecWidget(parent) {};
};

#endif /* ChartRecSpecWidget_hpp */
