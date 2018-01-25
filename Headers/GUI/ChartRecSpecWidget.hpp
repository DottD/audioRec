#ifndef ChartRecSpecWidget_hpp
#define ChartRecSpecWidget_hpp

#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QVector>
#include <GUI/ChartRecWidget.hpp>

namespace GUI {
	class ChartRecSpecWidget;
}

/** Class to show audio signals' spectrums in frequency domain. */
class GUI::ChartRecSpecWidget : public GUI::ChartRecWidget {
	
	Q_OBJECT
	
private:
	bool logscale = true;
	
protected:
	/** Converts an array index to the real frequency.
	 Overridden method from GUI::ChartRecWidget::indexConversion.*/
	double indexConversion(int k);
	
	/** Update the axes to show the whole scene.
	 A logarithmic scale is applied to the y-axis.
	 Overridden method from GUI::ChartRecWidget::updateAxes.*/
	void updateAxes();
	
public:
	ChartRecSpecWidget(QWidget* parent = Q_NULLPTR);
	
	/** Switch to a logarithmic scale. */
	void setLogScale();
	
	/** Switch to a natural scale. */
	void setNaturalScale();
};

#endif /* ChartRecSpecWidget_hpp */
