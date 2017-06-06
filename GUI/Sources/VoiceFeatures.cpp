#include "../Headers/VoiceFeatures.hpp"

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
	if (features.isNull()) return arma::mat();
	// Create the matrix (suppose that each inner vector has the same size)
	arma::mat mat(features->first().size(), features->size());
	// Fill each column of the matrix with the inner vectors
	for (unsigned int k = 0; k < features->size(); k++)
		mat.col(k) = arma::vec((*features)[k].data(), features->at(k).size(), false, false);
	return mat;
}

double Ui::VoiceFeatures::distance(const VoiceFeatures& features){
	if (this->features.isNull() || features.features.isNull()) return -1.0;
	// Define weighted mean functor
	auto weightedMean = [](arma::vec x, arma::vec w){ return arma::sum(x % w); };
	// Get the matrices
	arma::mat A = this->toArmaMat();
	arma::mat B = features.toArmaMat();
	// Separates the weights
	arma::vec wA = arma::normalise(A.col(2), 1);
	arma::vec wB = arma::normalise(B.col(2), 1);
	// Comptue the weighted mean of the columns of A and B
	arma::rowvec wmeanA, wmeanB;
	wmeanA << weightedMean(A.col(0), wA) << weightedMean(A.col(1), wA);
	wmeanB << weightedMean(B.col(0), wB) << weightedMean(B.col(1), wB);
	// Compute the weighted covariance matrix
	arma::mat22 CA = {
		weightedMean(arma::square(A.col(0)-wmeanA(0)), wA),
		weightedMean((A.col(0)-wmeanA(0))%(A.col(1)-wmeanA(1)), wA),
		weightedMean((A.col(0)-wmeanA(0))%(A.col(1)-wmeanA(1)), wA),
		weightedMean(arma::square(A.col(1)-wmeanA(1)), wA)
	};
	arma::mat22 CB = {
		weightedMean(arma::square(B.col(0)-wmeanB(0)), wB),
		weightedMean((B.col(0)-wmeanB(0))%(B.col(1)-wmeanB(1)), wB),
		weightedMean((B.col(0)-wmeanB(0))%(B.col(1)-wmeanB(1)), wB),
		weightedMean(arma::square(B.col(1)-wmeanB(1)), wB)
	};
	arma::mat22 C = (double(A.n_rows) * CA + double(B.n_rows) * CB) / double(A.n_rows+B.n_rows);
	// Compute the mean value of the columns in the second matrix
	arma::rowvec m = wmeanA - wmeanB;
	// Compute the Mahalanobis distance
	return sqrt( arma::as_scalar(m * arma::inv(C) * m.t()) );
}

void Ui::VoiceFeatures::setFeatures(QSharedPointer<QVector<QVector<double>>> features){
	this->features = features;
}

void Ui::VoiceFeatures::setFeatures(const QVector<QVector<double>>& features){
	*(this->features) = features;
}
