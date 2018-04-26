#ifndef FeaturesToCorrMat_hpp
#define FeaturesToCorrMat_hpp

#include <QImage>
#include <QAlgorithm.hpp>
#include <armadillo>
#include <UMF/functions.hpp>

namespace AA {
	class FeaturesToCorrMat;
}

class AA::FeaturesToCorrMat : public QAlgorithm {
	
	Q_OBJECT
	
	QA_INPUT(QVector<double>, Features)
	QA_OUTPUT(QImage, CorrMat)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(FeaturesToCorrMat)
	
public:
	void run();
};

#endif /* FeaturesToCorrMat_hpp */
