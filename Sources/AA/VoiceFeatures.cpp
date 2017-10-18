#include <AA/VoiceFeatures.hpp>

bool Ui::VoiceFeatures::readFromFile(QFile& file){
	// Try to open the given file
	if (!file.open(QIODevice::ReadOnly)) return false;
	// Associate the file with a binary data stream
	QDataStream stream(&file);
	// Check if features is allocated
	if (features.isNull()) features = QSharedPointer<QVector<QVector<double>>>(new QVector<QVector<double>>);
	// Read the features
	stream >> *features;
	// Close the file
	file.close();
	// Everything is ok
	return true;
}

bool Ui::VoiceFeatures::writeToFile(QFile& file){
	// Try to open the given file
	if (!file.open(QIODevice::WriteOnly)) return false;
	// Associate the file with a binary data stream;
	QDataStream ostream(&file);
	// Check if features is allocated
	if (features.isNull()) features = QSharedPointer<QVector<QVector<double>>>(new QVector<QVector<double>>);
	// Write data into stream
	ostream << *features;
	// Close the file
	file.close();
	// Everything is ok
	return true;
}

arma::mat Ui::VoiceFeatures::toArmaMat() const{
	if (features.isNull() || features->isEmpty()) return arma::mat();
	// Create the matrix (suppose that each inner vector has the same size)
	arma::mat mat(features->first().size(), features->size());
	// Fill each column of the matrix with the inner vectors
	for (unsigned int k = 0; k < features->size(); k++)
		mat.col(k) = arma::vec((*features)[k].data(), features->at(k).size(), false, false);
	return mat;
}

double Ui::VoiceFeatures::distance(const VoiceFeatures& features){
	const double maxCond = 0.1;
	if (this->features.isNull() || features.features.isNull()) return -1.0;
	// Get the matrices
	arma::mat A = this->toArmaMat();
	arma::mat B = features.toArmaMat();
	A.col(2).ones(); // comment these lines to get weighted covariance matrices
	B.col(2).ones();
	// Compute the weighted cross-covariance matrices
	arma::mat CA = arma::zeros<arma::mat>(A.n_cols-1, A.n_cols-1);
	if(A.n_rows == 1) CA.eye();
	else CA = weightCov(A.cols(0, 1), A.col(2));
	arma::mat CB = arma::zeros<arma::mat>(B.n_cols-1, B.n_cols-1);
	if(B.n_rows == 1) CB.eye();
	else CB = weightCov(B.cols(0, 1), B.col(2));
	// Average the two cross-covariance matrices
	arma::mat C = (double(A.n_rows) * CA + double(B.n_rows) * CB) / double(A.n_rows+B.n_rows);
	if(arma::rcond(C) < maxCond) C.eye();
	// Compute the weighted mean value of the columns in the second matrix
	arma::rowvec m = weightMean(A.cols(0, 1), A.col(2)) - weightMean(B.cols(0, 1), B.col(2));
	// Compute the Mahalanobis distance
	return sqrt( arma::as_scalar(m * arma::inv(C) * m.t()) );
}

void Ui::VoiceFeatures::setFeatures(QSharedPointer<QVector<QVector<double>>> features){
	this->features = features;
}

void Ui::VoiceFeatures::setFeatures(const QVector<QVector<double>>& features){
	*(this->features) = features;
}
