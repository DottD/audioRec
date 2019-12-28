#ifndef FeaturesDistance_hpp
#define FeaturesDistance_hpp

#include <QVariantList>
#include <QAlgorithm.hpp>
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
	
private:
	arma::mat weightMean(const arma::mat& X,
						 const arma::mat& W);
	arma::mat weightCov(const arma::mat& X,
						const arma::vec& W);
	
public:
	void run();
};

#endif /* FeaturesDistance_hpp */
