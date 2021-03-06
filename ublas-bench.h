#include "taco/tensor.h"

using namespace taco;
using namespace std;

#ifdef UBLAS
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/matrix.hpp>
typedef boost::numeric::ublas::compressed_matrix<double,boost::numeric::ublas::column_major> UBlasCSC;
typedef boost::numeric::ublas::compressed_matrix<double,boost::numeric::ublas::row_major> UBlasCSR;
typedef boost::numeric::ublas::matrix<double,boost::numeric::ublas::column_major> UBlasColMajor;
typedef boost::numeric::ublas::matrix<double,boost::numeric::ublas::row_major> UBlasRowMajor;
typedef boost::numeric::ublas::vector<double> UBlasDenseVector;

  void UBLASTotaco(const UBlasCSC& src, Tensor<double>& dst){
    for (auto it1 = src.begin2(); it1 != src.end2(); it1++ )
      for (auto it2 = it1.begin(); it2 != it1.end(); ++it2 )
        dst.insert({(int)(it2.index1()),(int)(it2.index2())},*it2);
    dst.pack();
  }

  void tacoToUBLAS(const Tensor<double>& src, UBlasCSC& dst) {
    for (auto& value : iterate<double>(src))
      dst(value.first.at(0),value.first.at(1)) = value.second;
  }

  void tacoToUBLAS(const Tensor<double>& src, UBlasCSR& dst) {
    for (auto& value : iterate<double>(src))
      dst(value.first.at(0),value.first.at(1)) = value.second;
  }

  void tacoToUBLAS(const Tensor<double>& src, UBlasColMajor& dst) {
    dst.resize(src.getDimension(0), src.getDimension(1), false);
    for (auto& value : iterate<double>(src))
      dst(value.first.at(0),value.first.at(1)) = value.second;
  }

  void tacoToUBLAS(const Tensor<double>& src, UBlasRowMajor& dst) {
    dst.resize(src.getDimension(0), src.getDimension(1), false);
    for (auto& value : iterate<double>(src))
      dst(value.first.at(0),value.first.at(1)) = value.second;
  }

  void UBLASTotaco(const UBlasDenseVector& src, Tensor<double>& dst){
    for (int i=0; i<dst.getDimension(0); ++i)
      dst.insert({i}, src[i]);
    dst.pack();
  }

  void tacoToUBLAS(const Tensor<double>& src, UBlasDenseVector& dst)  {
    for (auto& value : iterate<double>(src))
      dst(value.first[0]) = value.second;
  }

  void exprToUBLAS(BenchExpr Expr, map<string,Tensor<double>> exprOperands,int repeat, taco::util::TimeResults timevalue) {
    switch(Expr) {
      case SpMV: {
        int rows=exprOperands.at("A").getDimension(0);
        int cols=exprOperands.at("A").getDimension(1);
        UBlasCSR Aublas(rows,cols);
        tacoToUBLAS(exprOperands.at("A"),Aublas);

        UBlasDenseVector xublas(cols), yublas(rows);
        tacoToUBLAS(exprOperands.at("x"),xublas);

        TACO_BENCH(boost::numeric::ublas::axpy_prod(Aublas, xublas, yublas, true);,"\nUBLAS",repeat,timevalue,true);

        Tensor<double> y_ublas({rows}, Dense);
        UBLASTotaco(yublas,y_ublas);

        validate("UBLAS", y_ublas, exprOperands.at("yRef"));
        break;
      }
      case PLUS3: {
        int rows=exprOperands.at("ARef").getDimension(0);
        int cols=exprOperands.at("ARef").getDimension(1);
        UBlasCSC Aublas(rows,cols);
        UBlasCSC Bublas(rows,cols);
        UBlasCSC Cublas(rows,cols);
        UBlasCSC Dublas(rows,cols);

        tacoToUBLAS(exprOperands.at("B"),Bublas);
        tacoToUBLAS(exprOperands.at("C"),Cublas);
        tacoToUBLAS(exprOperands.at("D"),Dublas);

        TACO_BENCH(noalias(Aublas) = Bublas + Cublas + Dublas;,"\nUBLAS",repeat,timevalue,true);

        Tensor<double> A_ublas({rows,cols}, CSC);
        UBLASTotaco(Aublas,A_ublas);

        validate("UBLAS", A_ublas, exprOperands.at("ARef"));
        break;
      }
      case MATTRANSMUL: {
        int rows=exprOperands.at("A").getDimension(0);
        int cols=exprOperands.at("A").getDimension(1);
        UBlasCSC Aublas(rows,cols);
        tacoToUBLAS(exprOperands.at("A"),Aublas);

        UBlasDenseVector xublas(cols), zublas(rows), yublas(rows), tmpublas(rows);
        tacoToUBLAS(exprOperands.at("x"),xublas);
        tacoToUBLAS(exprOperands.at("z"),zublas);
        double alpha = ((double*)(exprOperands.at("alpha").getStorage().getValues().getData()))[0];
        double beta = ((double*)(exprOperands.at("beta").getStorage().getValues().getData()))[0];

        TACO_BENCH(boost::numeric::ublas::axpy_prod(xublas, Aublas, tmpublas, true); yublas = alpha * tmpublas + beta * zublas;,"\nUBLAS",repeat,timevalue,true);

        Tensor<double> y_ublas({rows}, Dense);
        UBLASTotaco(yublas,y_ublas);

        validate("UBLAS", y_ublas, exprOperands.at("yRef"));
        break;
      }
      case RESIDUAL: {
        int rows=exprOperands.at("A").getDimension(0);
        int cols=exprOperands.at("A").getDimension(1);
        UBlasCSR Aublas(rows,cols);
        tacoToUBLAS(exprOperands.at("A"),Aublas);

        UBlasDenseVector xublas(cols), zublas(rows), yublas(rows), tmpublas(rows);
        tacoToUBLAS(exprOperands.at("x"),xublas);
        tacoToUBLAS(exprOperands.at("z"),zublas);
        double alpha = ((double*)(exprOperands.at("alpha").getStorage().getValues().getData()))[0];
        double beta = ((double*)(exprOperands.at("beta").getStorage().getValues().getData()))[0];

        TACO_BENCH(boost::numeric::ublas::axpy_prod(Aublas, xublas, tmpublas, true); yublas = zublas - tmpublas ;,"\nUBLAS",repeat,timevalue,true);

        Tensor<double> y_ublas({rows}, Dense);
        UBLASTotaco(yublas,y_ublas);

        validate("UBLAS", y_ublas, exprOperands.at("yRef"));
        break;
      }
      case SDDMM: {
        int rows=exprOperands.at("ARef").getDimension(0);
        int cols=exprOperands.at("ARef").getDimension(1);
        UBlasCSC Aublas(rows,cols);
        UBlasCSC Bublas(rows,cols);
        UBlasRowMajor Cublas;
        UBlasColMajor Dublas;

        tacoToUBLAS(exprOperands.at("B"),Bublas);
        tacoToUBLAS(exprOperands.at("C"),Cublas);
        tacoToUBLAS(exprOperands.at("D"),Dublas);

        TACO_BENCH(noalias(Aublas) = element_prod(Bublas, prod(Cublas, Dublas)) ;,"\nUBLAS",repeat,timevalue,true);

        Tensor<double> A_ublas({rows,cols}, CSC);
        UBLASTotaco(Aublas,A_ublas);

        validate("UBLAS", A_ublas, exprOperands.at("ARef"));
        break;
      }
      default:
        cout << " !! Expression not implemented for UBLAS" << endl;
        break;
    }
}
#endif

