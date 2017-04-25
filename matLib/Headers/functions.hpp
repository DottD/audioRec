#ifndef functions_h
#define functions_h

#include <exception>
#include <armadillo>

#define SQ2(x) ((x)*(x))

enum class border_type {
	constant,		// iiiiii|abcdefgh|iiiiiii with some specified i
	replicate,		// aaaaaa|abcdefgh|hhhhhhh
	reflect,			// fedcba|abcdefgh|hgfedcb
	wrap,			// cdefgh|abcdefgh|abcdefg
	reflect_101		// gfedcb|abcdefgh|gfedcba
};

int borderInterpolate(const int& pos,
					  const int& len,
					  const border_type& bd);

arma::vec arrayPad(const arma::vec& array,
				   const unsigned int& m,
				   const border_type& bd = border_type::constant);

arma::vec movingAverage(const arma::vec& array,
						const unsigned int& m,
						const border_type& bd = border_type::reflect);

arma::vec bandPassFilter(const arma::vec& array,
						 const double& freq0,
						 const double& freq1,
						 const double& freq2,
				   const double& sampleRate);
double freq2bin(const double& sampleRate,
				const double& totalSamples,
				const double& frequency);
double bin2freq(const double& sampleRate,
				const double& totalSamples,
				const double& bin);


arma::umat binCounts(const arma::mat& array,
					 const std::vector<arma::vec>& bins);

arma::vec minFilter(const arma::vec& array,
					const unsigned int& m);

#endif /* functions_h */
