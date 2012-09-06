#ifndef MATRIXHELPER_HPP
#define MATRIXHELPER_HPP

#include <Eigen/Dense>

/**
 * @brief Test, if a matrix is diagonal, or not.
 * 
 * This function inspects the non-diagonal elements of a matrix.
 * If they all are smaller in magnitude than epsilon,
 * the matrix will be treated as diagonal.
 * 
 * @param matrix The matrix that will be examined.
 * @param epsilon The epsilon that nondiagonal elements may differ from zero
 *      to still be taken as zero.
 * @tparam ScalarType The type of the scalars of the matrix.
 * 
 * @ingroup tools
 * @return if the matrix is diagonal (up to an epsilon), or not.
 */
template <typename ScalarType> bool isDiagonal(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix, double epsilon=10e-14)
{
    assert (matrix.cols() == matrix.rows());    //supposed to be run on square matricies.
    
    bool retVal=true;
    for (int i=0; i<matrix.rows(); i++)
    {
        //don't look at elements above and on the diagonal
        for (int j=0; j<matrix.cols(); j++)
        {
            if (i==j)
                break;
            retVal &= fabs(matrix(i, j)) < epsilon;
        }
    }
    return retVal;
}

#endif  //MATRIXHELPER_HPP
