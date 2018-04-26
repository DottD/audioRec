#include <UMF/ChiSquareTest.hpp>

void UMF::ChiSquareTest::run(){
	// Check if input is two distributions of equal size
	Q_ASSERT(getInDistributions().size() == 2);
	Q_ASSERT(getInDistributions().at(0).size() == getInDistributions().at(1).size());
	// Compute the chi square test statistic
	double chisquare = 0.0;
	double diff = 0.0;
	for(QVector<double>::const_iterator a = getInDistributions().at(0).constBegin(),
		b = getInDistributions().at(1).constBegin();
		a != getInDistributions().at(0).constEnd() && b != getInDistributions().at(1).constEnd();
		++a, ++b){
		diff = *a - *b;
		// Assuming the second distribution is the theoretical one
		chisquare += diff*diff / *b;
	}
	// Given the p-value (Confidence) and the degrees of freedom (samples-fitting parameters),
	// compute the chi square distribution critical value
	double critVal = alglib::invchisquaredistribution(getInDistributions().at(0).size()-4, 1-getConfidence());
	// Compare the chi square test statistic with the critical value
	setOutDistrAreEqual(chisquare < critVal); // H0 hypothesis is accepted when the statistic is less than the critical value
	setOutSimilarity( alglib::errorfunctionc(chisquare / double(getInDistributions().at(0).size())) );
}
