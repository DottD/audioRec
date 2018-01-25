#include <AA/FeaturesDistance.hpp>

bool AA::FeaturesDistance::perform(){
	// Check input
	Q_ASSERT(getInFeatures().size() == 2);
	const QVector<double>& Features1 = getInFeatures().at(0);
	const QVector<double>& Features2 = getInFeatures().at(1);
	// Create headers for the two input features
	const arma::mat A(Features1.data(), 3, Features1.size()/3);
	const arma::mat B(Features2.data(), 3, Features2.size()/3);
	// Maximum conditioning number, prevent errors with the matrix inverse
	const double maxCond = 0.1;
	try{
		// Compute the weighted cross-covariance matrices
		// (actually to compute a weighted statistic the weights must not be all ones)
		arma::vec WA = arma::ones<arma::vec>(A.n_cols);
		arma::mat CA = arma::zeros<arma::mat>(A.n_rows-1, A.n_rows-1);
		if(A.n_cols == 1){
			CA.eye();
		}else{
			CA = weightCov(A.rows(0, 1).t(), WA);
		}
		arma::vec WB = arma::ones<arma::vec>(B.n_cols);
		arma::mat CB = arma::zeros<arma::mat>(B.n_rows-1, B.n_rows-1);
		if(B.n_cols == 1){
			CB.eye();
		}else{
			CB = weightCov(B.rows(0, 1).t(), WB);
		}
		// Average the two cross-covariance matrices
		arma::mat C = (double(A.n_cols) * CA + double(B.n_cols) * CB) / double(A.n_cols+B.n_cols);
		if(arma::rcond(C) < maxCond){
			C.eye();
		}
		// Compute the weighted mean value of the columns in the second matrix
		arma::rowvec m = weightMean(A.rows(0, 1).t(), WA) - weightMean(B.rows(0, 1).t(), WB);
		// Compute the Mahalanobis distance
		setOutDistance( sqrt( arma::as_scalar(m * arma::inv(C) * m.t()) ) );
	}catch(std::exception exc){
		raiseError(exc.what());
		return false;
	}
	return true;
}
