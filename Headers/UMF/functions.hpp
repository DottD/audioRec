#ifndef functions_h
#define functions_h

#include <exception>
#include <armadillo>
#include <alglib/interpolation.h>
#include <QVector>

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

arma::vec gaussianFilter(const arma::vec& array,
						 const unsigned int& m,
						 const border_type& bd = border_type::reflect);

arma::vec movingStdDev(const arma::vec& signal,
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
					const unsigned int& rad);

/**
 
 @param[in] maxIterations Maximum number of iterations to be performed.
 @param[in] maxDistNodes Maximum allowed distance (expressed in number of nodes) between two consecutive nodes marked as to be updated.
 */
arma::vec backgroundEstimation(const arma::vec& array,
							   const unsigned int& minFilterRad,
							   const double& maxPeakWidthAllowed,
							   const unsigned int& derEstimationDiam,
							   const unsigned int& maxIterations,
							   const double& maxAllowedInconsistency,
							   const unsigned int& maxDistNodes);

void peakDetect(const arma::vec& signal,
				std::vector<uint64_t>& emissionPeaks,
				std::vector<double>& emissionPeaksProminence,
				std::vector<uint64_t>& absorptionPeaks,
				std::vector<double>& absorptionPeaksProminence,
				const double& minRelVariation,
				const double& minRelVarInfluence,
				const bool& emissionFirst = true);

void histogramCreation(QVector<double>& data,
					   QVector<double>& x,
					   QVector<double>& y,
					   const double& barsStep);

void histogramCreation(arma::vec& data,
					   arma::vec& x,
					   arma::vec& y,
					   const double& barsStep);

namespace convert {
	void arma2qvec(const arma::vec& avec,
				   QVector<double>& qvec);
	void qvec2arma(QVector<double>& qvec,
				   arma::vec& avec);
}

namespace gaussFit {
	/** Fit data with a gaussian.
	 
	 */
	arma::vec fit(const arma::vec& x,
				  const arma::vec& y);
	
	void coreFuncVal(const alglib::real_1d_array& c,
					 const alglib::real_1d_array& x,
					 double& func,
					 void* ptr);
	
	void funcVal(const alglib::real_1d_array& c,
				 const alglib::real_1d_array& x,
				 double& func,
				 void* ptr);
	
	arma::vec eval(const arma::vec& c,
				   const arma::vec& x);
	
	void funcGrad(const alglib::real_1d_array& c,
				  const alglib::real_1d_array& x,
				  double& func,
				  alglib::real_1d_array& grad,
				  void* ptr);
	
	void funcHess(const alglib::real_1d_array& c,
				  const alglib::real_1d_array& x,
				  double& func,
				  alglib::real_1d_array& grad,
				  alglib::real_2d_array& hess,
				  void* ptr);
	
	void print(const alglib::lsfitreport& report);
}

arma::vec oversample(const arma::vec& samples,
					 const double& factor);

arma::mat weightMean(const arma::mat& X,
					 const arma::mat& W);

arma::mat weightCov(const arma::mat& X,
					const arma::vec& W);

namespace gaussExpFit {
	/** Fit data with a gaussian.
	 
	 */
	arma::vec fit(const arma::vec& x,
				  const arma::vec& y);
	
	void coreFuncVal(const alglib::real_1d_array& c,
					 const alglib::real_1d_array& x,
					 double& func,
					 void* ptr);
	
	void funcVal(const alglib::real_1d_array& c,
				 const alglib::real_1d_array& x,
				 double& func,
				 void* ptr);
	
	arma::vec eval(const arma::vec& c,
				   const arma::vec& x);
	
	void funcGrad(const alglib::real_1d_array& c,
				  const alglib::real_1d_array& x,
				  double& func,
				  alglib::real_1d_array& grad,
				  void* ptr);
	
	void funcHess(const alglib::real_1d_array& c,
				  const alglib::real_1d_array& x,
				  double& func,
				  alglib::real_1d_array& grad,
				  alglib::real_2d_array& hess,
				  void* ptr);
	
	void print(const alglib::lsfitreport& report);
}

#endif /* functions_h */
