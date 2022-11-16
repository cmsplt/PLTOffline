#ifndef GUARD_KmeansAnalyzer_h
#define GUARD_KmeansAnalyzer_h

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>


class KmeansAnalyzerFeatureExtractor
{
    
    public:
        KmeansAnalyzerFeatureExtractor() {};
        ~KmeansAnalyzerFeatureExtractor() {};

        void                initKmeansAnalyzerFeatureExtractor(const std::map<std::pair<unsigned, unsigned>, std::vector<int> >&);
        std::vector<double> getFeatures(const std::vector<std::vector<int> >&, std::vector<int>);
        double              hitsOutsideOfActiveArea(const std::vector<std::vector<int> >&, std::vector<int>);

    private:

        std::vector<double> _fitPlane(const std::vector<std::vector<double> >&);
        std::vector<std::vector<double> > _subtractPlane(const std::vector<std::vector<double> >&, std::vector<double> );

        std::vector<std::vector<double> > _cutActiveArea(const std::vector<std::vector<int> >&, std::vector<int> );

        std::vector<std::vector<double> > _3by3OnesConv(const std::vector<std::vector<double> >&);
        std::vector<std::vector<double> > _subtractConv(const std::vector<std::vector<double> >&, const std::vector<std::vector<double> >&, std::string );

        std::vector<std::vector<double> > _multiplyStripes(const std::vector<std::vector<double> >&, int);
        double _maxBlockStdDev(const std::vector<std::vector<double> >&, int, int);

        double              _mean2Dto0D(const std::vector<std::vector<double> >&);
        std::vector<double> _mean2Dto1D(const std::vector<std::vector<double> >&, int);
        double              _mean1Dto0D(const std::vector<double>&);

        double              _std2Dto0D(const std::vector<std::vector<double> >&);
        std::vector<double> _std2Dto1D(const std::vector<std::vector<double> >&, int);
        double              _std1Dto0D(const std::vector<double>&);

        double              _min2Dto0D(const std::vector<std::vector<double> >&);
        double              _max2Dto0D(const std::vector<std::vector<double> >&);
        double              _diffMaxMin2Dto0D(const std::vector<std::vector<double> >&);

        double              _countAbsGreaterThanThr1D(const std::vector<double>&, double);
        double              _countAbsGreaterThanThr2D(const std::vector<std::vector<double> >&, double);

        std::map<std::pair<unsigned, unsigned>, boost::numeric::ublas::matrix<double> > _planefitPseudoInverse;
};



class KmeansClassifier
{
    public:
        KmeansClassifier() {};
        ~KmeansClassifier() {};

        void    initKmeansClassifier(std::string, std::string);
        int     classify( const std::vector<double>& );
        bool    isNormal( int );

    private:
        std::vector< std::vector< double > > _readCSVTable(std::string);

        std::vector< boost::numeric::ublas::vector<double> > _centroids;
        std::vector< double > _normalization_mu;
        std::vector< double > _normalization_sigma;

        std::vector< double > _normalClassIndices = {0};
};



class KmeansAnalyzer
{
    public:
        KmeansAnalyzer() {};
        ~KmeansAnalyzer() {};

        void    initKmeansAnalyzer(std::string, std::string, const std::map<std::pair<unsigned, unsigned>, std::vector<int> >&);
        int     getClassIndex(const std::vector<std::vector<int> >&, std::vector<int>);
        double  getFractionOfHitsOutsideOfActiveArea(const std::vector<std::vector<int> >&, std::vector<int>);
        bool    getIsNormal( int );

    private:

        KmeansAnalyzerFeatureExtractor _KmeansAnalyzerFeatureExtractor;
        KmeansClassifier _KmeansClassifier;        
};


#endif
