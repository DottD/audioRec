#include <AA/FeaturesToCorrMat.hpp>

void AA::FeaturesToCorrMat::run(){
	Q_ASSERT(!getInFeatures().empty());
	// Process each features set independently
	const QVector<double>& features = getInFeatures();
	// Generate the binning matrix
	const arma::mat corrSet(features.data(), 3, features.size()/3);
	arma::vec binSpec = arma::linspace(0, 1, 100);
	arma::umat matCorr = binCounts(corrSet.rows(0, 1).t(), {binSpec, binSpec}); /* trovare massimo valore e creare i bin in base a quello (più veloce) */
	arma::umat::elem_type maxVal = matCorr.max();
	arma::mat matCorrd = arma::conv_to<arma::mat>::from(matCorr) / arma::mat::elem_type(maxVal);
	arma::uchar_mat matCorrb = arma::conv_to<arma::uchar_mat>::from(matCorrd * 255);
	setOutCorrMat( QImage(matCorrb.memptr(), matCorrb.n_cols, matCorrb.n_rows, QImage::Format_Grayscale8) );
}
