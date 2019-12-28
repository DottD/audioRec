#include <AA/FeaturesDistance.hpp>

void AA::FeaturesDistance::run(){
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
		arma::vec WA = A.row(2).t();
		arma::mat CA = arma::zeros<arma::mat>(A.n_rows-1, A.n_rows-1);
		if(A.n_cols == 1){
			CA.eye();
		}else{
			CA = weightCov(A.rows(0, 1).t(), WA);
		}
		arma::vec WB = B.row(2).t();
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
		raise(exc.what());
	}
}

arma::mat AA::FeaturesDistance::weightMean(const arma::mat& X,
					 const arma::mat& W){
	if (!W.is_vec())
		throw std::runtime_error("W must be a vector");
	else {
		if (W.size() > 1 && ((W.is_colvec() && X.n_rows != W.n_rows) || (W.is_rowvec() && X.n_cols != W.n_cols)))
			throw std::runtime_error("Wrong dimensions");
		else if (W.size() == 1 && X.n_rows != 1 && X.n_cols != 1)
			throw std::runtime_error("Wrong dimensions");
		else {
			if (W.size() == 1 and X.n_rows == 1)
				return arma::sum(X.each_col() % arma::normalise(W, 1), 0);
			else if (W.size() == 1 and X.n_cols == 1)
				return arma::sum(X.each_row() % arma::normalise(W, 1), 1);
			else if(W.is_colvec())
				return arma::sum(X.each_col() % arma::normalise(W, 1), 0);
			else
				return arma::sum(X.each_row() % arma::normalise(W, 1), 1);
		}
	}
}

arma::mat AA::FeaturesDistance::weightCov(const arma::mat& X,
					const arma::vec& W){
	// row - observations, col - variables
	// Check dimensions
	if (X.n_rows != W.n_rows) throw std::runtime_error("X and W must have the same number of columns");
	// Compute the weighted mean along the observations
	arma::rowvec m = weightMean(X, W);
	// Compute the covariance matrix
	arma::vec NW = arma::normalise(W, 1);
	arma::mat C(X.n_cols, X.n_cols);
	C.zeros();
	for(arma::uword i = 0; i < X.n_rows; i++){
		C += (X.row(i)-m).t() * (X.row(i)-m) * NW(i);
	}
	return C;
}
