#ifndef FeaturesDistance_hpp
#define FeaturesDistance_hpp

#include <QVariantList>
#include <QAlgorithm.hpp>
#include <UMF/functions.hpp>
#include <armadillo>

namespace AA {
	class FeaturesDistance;
}

class AA::FeaturesDistance : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT_LIST(QVector<double>, Features)
	QA_OUTPUT(double, Distance)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(FeaturesDistance)
	
public:
	bool perform();
};

#endif /* FeaturesDistance_hpp */
