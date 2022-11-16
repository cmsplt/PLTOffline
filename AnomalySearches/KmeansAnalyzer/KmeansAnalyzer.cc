#include "KmeansAnalyzer.h"
// #include "bril/pltslinkprocessor/KmeansAnalyzer.h"
using namespace std;

// TODO: remove all std::cout, as this can lead to a crash when the local disc is full. logs are transmitted fine however. 

void KmeansAnalyzer::initKmeansAnalyzer(    std::string centroidsFile, 
                                            std::string normalizationFile, 
                                            const std::map<std::pair<unsigned, unsigned>, std::vector<int> >& maskEdges)
{
    _KmeansAnalyzerFeatureExtractor.initKmeansAnalyzerFeatureExtractor( maskEdges );
    _KmeansClassifier.initKmeansClassifier( centroidsFile, normalizationFile );
}


int KmeansAnalyzer::getClassIndex(const std::vector<std::vector<int> >& mx, std::vector<int> mask)
{
    std::vector<double> features    = _KmeansAnalyzerFeatureExtractor.getFeatures( mx, mask );
    int                 classIndex  = _KmeansClassifier.classify( features );

    return classIndex;
}

double KmeansAnalyzer::getFractionOfHitsOutsideOfActiveArea(const std::vector<std::vector<int> >& mx, std::vector<int> mask)
{
    return _KmeansAnalyzerFeatureExtractor.hitsOutsideOfActiveArea( mx, mask );
}

bool KmeansAnalyzer::getIsNormal( int classIndex )
{
    return _KmeansClassifier.isNormal( classIndex );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void KmeansAnalyzerFeatureExtractor::initKmeansAnalyzerFeatureExtractor(const std::map<std::pair<unsigned, unsigned>, std::vector<int> >& maskEdges)
{
    namespace ublas = boost::numeric::ublas;

    // precompute pseudoinverse for plane fitting
    for (auto const& m : maskEdges){

        int lRow = (m.second[1]-m.second[0]+1);
        int lCol = (m.second[3]-m.second[2]+1);
        std::pair<int,int> activeSize = std::make_pair(lRow, lCol);

        if ( _planefitPseudoInverse.count(activeSize) > 0 ) continue;

        ublas::matrix<double> A(lRow*lCol, 3);

        for (int iRow=0; iRow<lRow; iRow++){
           for (int iCol=0; iCol<lCol; iCol++){

               int i = iCol + lCol*iRow;
               int r = iRow - (lRow-1)/2; // centred coordinate system
               int c = iCol - (lCol-1)/2; // centred coordinate system
               
               A(i, 0) = 1;
               A(i, 1) = r;
               A(i, 2) = c;
            } 
        }

        ublas::matrix<double> AstarALU = ublas::prod(ublas::trans(A), A);

        ublas::permutation_matrix<std::size_t> permutationMatrix(AstarALU.size1());
        int res = ublas::lu_factorize(AstarALU, permutationMatrix); // AstarALU is changed here, L and U are stored in the same matrix: AstarALU
        if( res != 0 ) {
            std::cout<< "KmeansAnalyzerFeatureExtractor::Init: trouble inverting matrix" << std::endl;
        }
        ublas::matrix<double> invAstarA(AstarALU.size1(), AstarALU.size2());
        invAstarA.assign(ublas::identity_matrix<double>(AstarALU.size1()));
        ublas::lu_substitute(AstarALU, permutationMatrix, invAstarA); 
        
        _planefitPseudoInverse[activeSize]  = ublas::prod(invAstarA, ublas::trans(A)) ;
    }
}

std::vector<double> KmeansAnalyzerFeatureExtractor::_fitPlane(const std::vector<std::vector<double> >& maskedMx)
{
    namespace ublas = boost::numeric::ublas;

    int lRow = maskedMx.size();
    int lCol = maskedMx[0].size();
    std::pair<int,int> activeSize = std::make_pair(lRow, lCol);

    ublas::vector<double> v(lRow*lCol);

    for (int iRow=0; iRow<lRow; iRow++){
        for (int iCol=0; iCol<lCol; iCol++){

            int i = iCol + lCol*iRow;
            v(i) = maskedMx[iRow][iCol];
        } 
    }

    ublas::vector<double> paramsBV = ublas::prod( _planefitPseudoInverse[activeSize], v );
    std::vector<double> params;
    for (unsigned i=0; i<paramsBV.size(); i++) params.push_back(paramsBV(i));

    std::cout<<"fit params: ";
    for (auto p :params) std::cout<<p<<" ";
    std::cout<<std::endl;

    return params;
}

std::vector<std::vector<double> > KmeansAnalyzerFeatureExtractor::_subtractPlane(const std::vector<std::vector<double> >& maskedMx, std::vector<double> params)
{

    int lRow = maskedMx.size();
    int lCol = maskedMx[0].size();

    std::vector<std::vector<double> > result(maskedMx.size(), std::vector<double>( maskedMx[0].size()));

    for (int iRow=0; iRow<lRow; iRow++){
        for (int iCol=0; iCol<lCol; iCol++){

            int r = iRow - (lRow-1)/2; // centred coordinate system
            int c = iCol - (lCol-1)/2;

            result[iRow][iCol] = maskedMx[iRow][iCol] - (params[0] + params[1] * r + params[2] * c );
        } 
    }

    return result;
}

double KmeansAnalyzerFeatureExtractor::_maxBlockStdDev(const std::vector<std::vector<double> >& mx, int wRow, int wCol)
{
    
    double maxStdVar = 0;

    int lRow  = mx.size();
    int lCol  = mx[0].size();

    double n  = wRow * wCol;
    double nr = n / (n - 1);

    for (int iRow=0; iRow < (lRow - wRow + 1); iRow++){
        for (int iCol=0; iCol < (lCol - wCol + 1); iCol++){

            double sum  = 0;
            double sum2 = 0;

            for (int i=0; i<wRow; i++){
                for (int j=0; j<wCol; j++){

                    int     i_   = iRow + i;
                    int     j_   = iCol + j;
                    double  v    = mx[i_][j_];
                    
                    sum     += v;
                    sum2    += v * v;

                }
            }

            double stdVar =  (sum2/n - (sum/n)*(sum/n)) * nr;

            if ( stdVar > maxStdVar) maxStdVar = stdVar;
        } 
    }

    return std::sqrt(maxStdVar);
}

double KmeansAnalyzerFeatureExtractor::hitsOutsideOfActiveArea(const std::vector<std::vector<int> >& mx, std::vector<int> mask)
{
    double sum_outside = 0;
    double sum_all = 0;

    int lRow = mx.size();
    int lCol = mx[0].size();

    for (int iRow=0; iRow<lRow; iRow++){
        for (int iCol=0; iCol<lCol; iCol++){
            sum_all+= mx[iRow][iCol];

            if (
                iRow < mask[0] ||
                iRow > mask[1] ||
                iCol < mask[2] ||
                iCol > mask[3] 
            ){
                sum_outside+= mx[iRow][iCol];
            }

        }
    }

    return sum_outside / sum_all;
}

std::vector<std::vector<double> > KmeansAnalyzerFeatureExtractor::_cutActiveArea(const std::vector<std::vector<int> >& mx, std::vector<int> mask)
{
    int lRow = (mask[1]-mask[0]+1);
    int lCol = (mask[3]-mask[2]+1);

    std::vector<std::vector<double> > result(lRow, std::vector<double>( lCol ));

    for (int iRow=0; iRow<lRow; iRow++){
        for (int iCol=0; iCol<lCol; iCol++){
            result[iRow][iCol] = mx[mask[0]+iRow][mask[2]+iCol];
        }
    }

    return result;
}

std::vector<std::vector<double> > KmeansAnalyzerFeatureExtractor::_3by3OnesConv(const std::vector<std::vector<double> >& maskedMx)
{
    std::vector<std::vector<double> > result(maskedMx.size(), std::vector<double>( maskedMx[0].size()));

    int lRow  = maskedMx.size();
    int lCol  = maskedMx[0].size();

    for (int iRow=0; iRow<lRow; iRow++){
        for (int iCol=0; iCol<lCol; iCol++){

            for (int i=-1; i<=1; i++){
                for (int j=-1; j<=1; j++){
                    // mx is mirrored at the edges
                    int i_ = iRow + i;
                    if (i_<0) i_=1;
                    if (i_>=lRow) i_=lRow-2;
                    int j_ = iCol + j;
                    if (j_<0) j_=1;
                    if (j_>=lCol) j_=lCol-2;

                    result[iRow][iCol] += maskedMx[i_][j_];
                }
            }
        } 
    }
    return result;
}

std::vector<std::vector<double> > KmeansAnalyzerFeatureExtractor::_subtractConv(const std::vector<std::vector<double> >& maskedMx, const std::vector<std::vector<double> >& convedMx, std::string mode)
{
    std::vector<std::vector<double> > result(maskedMx.size(), std::vector<double>( maskedMx[0].size()));

    int lRow  = maskedMx.size();
    int lCol  = maskedMx[0].size();

    if (mode == "3by3"){
        for (int iRow=0; iRow<lRow; iRow++){
            for (int iCol=0; iCol<lCol; iCol++){
                result[iRow][iCol] = maskedMx[iRow][iCol] - convedMx[iRow][iCol]/9;
            } 
        }
        return result;
    }
    if (mode == "3by3hole"){
        for (int iRow=0; iRow<lRow; iRow++){
            for (int iCol=0; iCol<lCol; iCol++){
                result[iRow][iCol] = maskedMx[iRow][iCol]*(1.0 + 1.0/8.0) - convedMx[iRow][iCol]/8;
            } 
        }
        return result;
    }

    return result;
}

std::vector<std::vector<double> > KmeansAnalyzerFeatureExtractor::_multiplyStripes(const std::vector<std::vector<double> >& mx, int axis){

    int lRow  = mx.size();
    int lCol  = mx[0].size();

    std::vector<std::vector<double> > result(lRow, std::vector<double>( lCol ));

    if (axis == 0){
        for (int iRow=0; iRow<lRow; iRow++){
            for (int iCol=0; iCol<lCol; iCol++){
                if (iRow % 2 == 0) result[iRow][iCol] = mx[iRow][iCol];
                else result[iRow][iCol] = - mx[iRow][iCol];
            } 
        }
    }
    else{
        for (int iRow=0; iRow<lRow; iRow++){
            for (int iCol=0; iCol<lCol; iCol++){
                if (iCol % 2 == 0) result[iRow][iCol] = mx[iRow][iCol];
                else result[iRow][iCol] = - mx[iRow][iCol];
            } 
        }
    }

    return result;
}



double KmeansAnalyzerFeatureExtractor::_mean2Dto0D(const std::vector<std::vector<double> >& mx){
    double  sum = 0;
    int     counter = 0;

    for (unsigned i=0; i<mx.size(); i++){
        for (unsigned j=0; j<mx[i].size(); j++){
            sum += mx[i][j];
            counter++;
        }
    }

    return sum / counter;
}

std::vector<double> KmeansAnalyzerFeatureExtractor::_mean2Dto1D(const std::vector<std::vector<double> >& mx, int axis){
    std::vector<double> result;

    if (axis==0){
        for (unsigned j=0; j<mx[0].size(); j++){
            double  sum = 0;
            for (unsigned i=0; i<mx.size(); i++){
                sum += mx[i][j];
            }
            result.push_back(sum / mx.size());
        }
    }
    else
    {
        for (unsigned i=0; i<mx.size(); i++){
            double  sum = 0;
            for (unsigned j=0; j<mx[i].size(); j++){
                sum += mx[i][j];
            }
            result.push_back(sum / mx[i].size());
        }
    }

    return result;
}

double KmeansAnalyzerFeatureExtractor::_mean1Dto0D(const std::vector<double>& v){
    double sum = 0;

    for (unsigned i=0; i<v.size(); i++){
        sum += v[i];
    }

    return sum / v.size();
}



double KmeansAnalyzerFeatureExtractor::_std2Dto0D(const std::vector<std::vector<double> >& mx){
    double  sum  = 0;
    double  sum2 = 0;

    int lRow  = mx.size();
    int lCol  = mx[0].size();

    double n  = lRow * lCol;

    for (int i=0; i<lRow; i++){
        for (int j=0; j<lCol; j++){
            sum  += mx[i][j];
            sum2 += mx[i][j] * mx[i][j];
        }
    }

    return std::sqrt( (sum2/n - (sum/n) * (sum/n)) * (n / (n - 1)) );
}

std::vector<double> KmeansAnalyzerFeatureExtractor::_std2Dto1D(const std::vector<std::vector<double> >& mx, int axis){
    std::vector<double> result;

    int lRow  = mx.size();
    int lCol  = mx[0].size();

    if (axis==0)
    {
        for (int j=0; j<lCol; j++){
            double  sum  = 0;
            double  sum2 = 0;
            for (int i=0; i<lRow; i++){
                sum  += mx[i][j];
                sum2 += mx[i][j] * mx[i][j];
            }
            double n = lRow;
            result.push_back(std::sqrt( (sum2/n - (sum/n) * (sum/n)) * (n / (n - 1)) ));
        }
    }
    else
    {
        for (int i=0; i<lRow; i++){
            double  sum  = 0;
            double  sum2 = 0;
            for (int j=0; j<lCol; j++){
                sum  += mx[i][j];
                sum2 += mx[i][j] * mx[i][j];
            }
            double n = lCol;
            result.push_back(std::sqrt( (sum2/n - (sum/n) * (sum/n)) * (n / (n - 1)) ));
        }
    }

    return result;
}

double KmeansAnalyzerFeatureExtractor::_std1Dto0D(const std::vector<double>& v){
    double  sum  = 0;
    double  sum2 = 0;
    double  n    = v.size();

    for (int i=0; i<n; i++){
        sum  += v[i];
        sum2 += v[i] * v[i];
    }

    return std::sqrt( (sum2/n - (sum/n) * (sum/n)) * (n / (n - 1)) );
}



double KmeansAnalyzerFeatureExtractor::_min2Dto0D(const std::vector<std::vector<double> >& mx)
{
    double min=mx[0][0];

    for (unsigned i=0; i<mx.size(); i++){
        for (unsigned j=0; j<mx[i].size(); j++){
            if (mx[i][j]<min) min = mx[i][j];
        }
    }

    return min;
}

double KmeansAnalyzerFeatureExtractor::_max2Dto0D(const std::vector<std::vector<double> >& mx)
{
    double max=mx[0][0];

    for (unsigned i=0; i<mx.size(); i++){
        for (unsigned j=0; j<mx[i].size(); j++){
            if (mx[i][j]>max) max = mx[i][j];
        }
    }

    return max;
}

double KmeansAnalyzerFeatureExtractor::_diffMaxMin2Dto0D(const std::vector<std::vector<double> >& mx)
{
    double min=mx[0][0];
    double max=mx[0][0];

    for (unsigned i=0; i<mx.size(); i++){
        for (unsigned j=0; j<mx[i].size(); j++){
            if (mx[i][j]<min) min = mx[i][j];
            if (mx[i][j]>max) max = mx[i][j];
        }
    }

    return max - min;
}



double KmeansAnalyzerFeatureExtractor::_countAbsGreaterThanThr1D(const std::vector<double>& v, double thr)
{
    double counter=0;

    for (unsigned i=0; i<v.size(); i++){
        if (std::abs(v[i])>thr) counter++;
    }

    return counter;
}

double KmeansAnalyzerFeatureExtractor::_countAbsGreaterThanThr2D(const std::vector<std::vector<double> >& mx, double thr)
{
    double counter=0;

    for (unsigned i=0; i<mx.size(); i++){
        for (unsigned j=0; j<mx[i].size(); j++){
            if (std::abs(mx[i][j])>thr) counter++;
        }
    }

    return counter;
}

std::vector<double> KmeansAnalyzerFeatureExtractor::getFeatures(const std::vector<std::vector<int> >& mx, std::vector<int> mask)
{
    // these features are listed in Table 10 of DN-19-028.

    double tempD, tempD1, tempD2;
    std::vector<double> tempV;
    std::vector<double> featureVector(31);

    std::vector<std::vector<double> > maskedMx  = _cutActiveArea(mx, mask);
    std::vector<double> fitParams               = _fitPlane(maskedMx);
    std::vector<std::vector<double> > mx_f      = _subtractPlane(maskedMx, fitParams);
    std::vector<std::vector<double> > mx_c      = _subtractConv(maskedMx, _3by3OnesConv(maskedMx), "3by3hole");

    double maskedMxAvg      = _mean2Dto0D(maskedMx);
    double maskedMxAvgSqrt  = std::sqrt(maskedMxAvg);
    double maskedMxStd      = _std2Dto0D(maskedMx);

    std::cout<<"maskedMxAvg: "<< maskedMxAvg <<std::endl;
    std::cout<<"maskedMxStd: "<< maskedMxStd <<std::endl;

    featureVector[0] = _std2Dto0D(mx_c) / maskedMxAvgSqrt;
    featureVector[1] = _std2Dto0D(mx_f) / maskedMxAvgSqrt;

    std::cout<<"std: "<<_std2Dto0D(mx_f)<<std::endl;
    std::cout<<"nstd: "<<  _std2Dto0D(mx_f) / maskedMxAvgSqrt <<std::endl;
    
    tempD = _diffMaxMin2Dto0D(mx_c);
    featureVector[2] = tempD / maskedMxStd;
    featureVector[3] = tempD / maskedMxAvgSqrt;
    tempD = _diffMaxMin2Dto0D(mx_f);
    featureVector[4] = tempD / maskedMxStd;
    featureVector[5] = tempD / maskedMxAvgSqrt;

    featureVector[6] = _std1Dto0D(_mean2Dto1D(mx_c, 1)) / maskedMxAvgSqrt;
    featureVector[7] = _std1Dto0D(_mean2Dto1D(mx_c, 0)) / maskedMxAvgSqrt;
    featureVector[8] = _std1Dto0D( _std2Dto1D(mx_c, 1)) / maskedMxAvg;
    featureVector[9] = _std1Dto0D( _std2Dto1D(mx_c, 0)) / maskedMxAvg;

    featureVector[10] = _std1Dto0D(_mean2Dto1D(mx_f, 1)) / maskedMxAvgSqrt;
    featureVector[11] = _std1Dto0D(_mean2Dto1D(mx_f, 0)) / maskedMxAvgSqrt;
    featureVector[12] = _std1Dto0D( _std2Dto1D(mx_f, 1)) / maskedMxAvg;
    featureVector[13] = _std1Dto0D( _std2Dto1D(mx_f, 0)) / maskedMxAvg;

    tempV = _std2Dto1D(mx_c, 0);
    tempD = _mean1Dto0D( tempV );
    featureVector[14] = _countAbsGreaterThanThr1D( tempV, 1.5 * tempD );
    featureVector[16] = _countAbsGreaterThanThr1D( tempV, 2.0 * tempD );
    tempD1            = _countAbsGreaterThanThr1D( tempV, 1.25 * tempD );
    tempV = _std2Dto1D(mx_c, 1);
    tempD = _mean1Dto0D( tempV );
    featureVector[15] = _countAbsGreaterThanThr1D( tempV, 1.5 * tempD );
    featureVector[17] = _countAbsGreaterThanThr1D( tempV, 2.0 * tempD );
    tempD2            = _countAbsGreaterThanThr1D( tempV, 1.25 * tempD );

    featureVector[18] = std::abs( featureVector[14] - featureVector[15] );
    featureVector[19] = std::abs( featureVector[16] - featureVector[17] );
    featureVector[20] = std::abs( tempD1 - tempD2 );


    double mx_cStd = _std2Dto0D(mx_c);
    double mx_fStd = _std2Dto0D(mx_f);
    featureVector[21] = _countAbsGreaterThanThr2D( mx_c, 4 * mx_cStd );
    featureVector[22] = _countAbsGreaterThanThr2D( mx_f, 4 * mx_fStd );
    featureVector[23] = _countAbsGreaterThanThr2D( mx_c, 5 * mx_cStd );
    featureVector[24] = _countAbsGreaterThanThr2D( mx_f, 5 * mx_fStd );

    featureVector[25] = _countAbsGreaterThanThr2D( mx_c, 4 * maskedMxAvgSqrt );
    featureVector[26] = _countAbsGreaterThanThr2D( mx_f, 4 * maskedMxAvgSqrt );
    featureVector[27] = _countAbsGreaterThanThr2D( mx_c, 5 * maskedMxAvgSqrt );
    featureVector[28] = _countAbsGreaterThanThr2D( mx_f, 5 * maskedMxAvgSqrt );
    featureVector[29] = _countAbsGreaterThanThr2D( mx_c, 6 * maskedMxAvgSqrt );
    featureVector[30] = _countAbsGreaterThanThr2D( mx_f, 6 * maskedMxAvgSqrt );

    for (auto v : featureVector) std::cout<<v <<" ";
    std::cout<<std::endl;

    // xcheck with python implementation
    // std::cout<< "-----intermediate-----" << std::endl; 

    // std::cout<<"maskedMx min, max: "<<_min2Dto0D(maskedMx)<<" "<<_max2Dto0D(maskedMx)<<std::endl;
    // std::cout<<"maskedMx avg: "<<maskedMxAvg<<std::endl;

    // std::cout<<"fit parameters: "<<fitParams[0]<<" "<<fitParams[1]<<" "<<fitParams[2]<<std::endl;
    // std::cout<<"m_f min, max: "<<_min2Dto0D(mx_f)<<" "<<_max2Dto0D(mx_f)<<std::endl;
    // std::cout<<"m_f avg: "<<_mean2Dto0D(mx_f)<<std::endl;

    // std::cout<<"m_c min, max: "<<_min2Dto0D(mx_c)<<" "<<_max2Dto0D(mx_c)<<std::endl;
    // std::cout<<"m_c avg: "<<_mean2Dto0D(mx_c)<<std::endl;

    // std::cout<< "-----features-----" << std::endl;
    // for (int i=0; i<featureVector.size(); i++){
    //     std::cout<<i<<"\t"<< featureVector[i] << std::endl;
    // }
    // std::cout<< "------------------" << std::endl;

    return featureVector;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void KmeansClassifier::initKmeansClassifier(std::string centroidsFile, std::string normalizationFile)
{

    std::vector< std::vector< double > > centroidsData = _readCSVTable(centroidsFile);
    for ( std::vector< double > centroidV : centroidsData)
    {
        boost::numeric::ublas::vector<double> v(centroidV.size());
        for (unsigned i = 0; i < centroidV.size(); ++ i) v(i) = centroidV[i];
        _centroids.push_back(v);
    }

    std::vector< std::vector< double > > normalizationData = _readCSVTable(normalizationFile);
    _normalization_mu    = normalizationData[0];
    _normalization_sigma = normalizationData[1];
}

std::vector< std::vector< double > > KmeansClassifier::_readCSVTable(std::string filePath)
{

    std::vector< std::vector< double > > result;

    std::ifstream maskFile;
    maskFile.open(filePath);

    if(!maskFile.is_open()) {
        std::cout<< "KmeansClassifier::readCSVTable: Could not open file:"<< filePath << std::endl;
    }

    std::string line;
    while(getline(maskFile, line)) {

        if (line.size()<8) continue;
        if (line[0] == '#') continue;

        std::vector< double > parsed_line;
        std::stringstream ss(line);
        std::string buf;
        while (getline(ss, buf, ',')) parsed_line.push_back(std::stod(buf));

        result.push_back(parsed_line);
    }

    return result;
}

int KmeansClassifier::classify( const std::vector<double>& featureVector)
{
    namespace ublas = boost::numeric::ublas;

    ublas::vector<double> normalizedFeatureBVector(featureVector.size());
    for (unsigned i = 0; i < featureVector.size(); ++ i)
        normalizedFeatureBVector(i) = (featureVector[i] - _normalization_mu[i])/_normalization_sigma[i];

    int    best_centroid_index = -1;
    double best_centroid_distance = -1;
    for (unsigned index = 0; index < _centroids.size(); ++ index)
    {
        double centroid_distance = ublas::norm_2(normalizedFeatureBVector - _centroids[index]);
        if (best_centroid_distance==-1 || centroid_distance<best_centroid_distance){
            best_centroid_distance = centroid_distance;
            best_centroid_index = index;
        }
    }
    return best_centroid_index;
}

bool KmeansClassifier::isNormal( int classIndex )
{
    for (unsigned i=0; i<_normalClassIndices.size(); i++){
        if ( classIndex == _normalClassIndices[i] ) return true;
    }
    return false;
}