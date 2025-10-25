#include <iostream>
#include <vector>
#include <cmath>

#include "matrix.h"
#include "transform.h"

int main(){
	Matrix temp_matrix(2,2), mul_matrix(8), nul_matrix(2), res(1);

	// point 1.1 bool function paralel
	boolFunc(temp_matrix,{{0,0,0},{0,1,1},{1,0,1},{1,1,1}});
	mul_matrix.coef=0.5;
	mul_matrix.data={{1},{0},{1},{0},{1},{0},{1},{0}};
	res=Matrix::mull(temp_matrix,mul_matrix);
	res.print();
	res.printBracket(true);
	std::cout<<'\n';

	// point 1.2 mod function paralel
	mul_matrix=Matrix(4);
	mul_matrix.coef=0.5;
	mul_matrix.data={{1},{1},{1},{1}};
	int y_count=selectSize(M_VALUE);
	nul_matrix.data={{1},{0}};
	temp_matrix=Matrix(2);
	temp_matrix.data={{1},{0}};
	for(int i=0;i<y_count-1;++i){
		nul_matrix=Matrix::tensorMull(nul_matrix,temp_matrix);
	}
	mul_matrix=Matrix::tensorMull(mul_matrix,nul_matrix);
	modFunc(temp_matrix);
	res=Matrix::mull(temp_matrix,mul_matrix);
	res.print();
	res.printBracket(true);
	std::cout<<'\n';


	// point 2.1
	Matrix one_matrix(2), h(2,2), temp_save(2);
	h.coef=(double)(1/(double)std::sqrt(2));
	h.data={{1,1},{1,-1}};
	nul_matrix=Matrix(2);
	one_matrix.data={{0},{1}};
	nul_matrix.data={{1},{0}};
	// create |0*|0*|1
	nul_matrix=Matrix::tensorMull(nul_matrix,nul_matrix);
	nul_matrix=Matrix::tensorMull(nul_matrix,one_matrix);
	// create h*h*h
	h=Matrix::tensorMull(h,Matrix::tensorMull(h,h));
	// create h*|001
	nul_matrix=Matrix::mull(h,nul_matrix);
	temp_save=nul_matrix;
	// create after_h|001*U_f
	boolFunc(temp_matrix,{{0,0,0},{0,1,0},{1,0,1},{1,1,1}});
	temp_matrix=Matrix::mull(temp_matrix,nul_matrix);
	// create h*|00
	h=Matrix(2,2);
	h.coef=(double)(1/(double)std::sqrt(2));
	h.data={{1,1},{1,-1}};
	one_matrix=Matrix(2,2);
	one_matrix.data={{1,0},{0,1}};
	h=Matrix::tensorMull(Matrix::tensorMull(h,h),one_matrix);
	// create hh|00*after_U_f|001
	temp_matrix=Matrix::mull(h,temp_matrix);
	temp_matrix.mullCoefData();
	temp_matrix.print();
	// very funny thing
	double v_1=std::pow(temp_matrix.data[0][0],2), v_2=std::pow(temp_matrix.data[1][0],2);
	std::cout<<"Bool func: OR\nResult: |00>="<<(double)(v_1+v_2)<<"\n \n";

	// point 2.2
	boolFunc(temp_matrix,{{0,0,1},{0,1,1},{1,0,1},{1,1,1}});
	temp_matrix=Matrix::mull(temp_matrix,temp_save);
	// create h*|00
	h=Matrix(2,2);
	h.coef=(double)(1/(double)std::sqrt(2));
	h.data={{1,1},{1,-1}};
	one_matrix=Matrix(2,2);
	one_matrix.data={{1,0},{0,1}};
	h=Matrix::tensorMull(Matrix::tensorMull(h,h),one_matrix);
	// create hh|00*after_U_f|001
	temp_matrix=Matrix::mull(h,temp_matrix);
	temp_matrix.mullCoefData();
	temp_matrix.print();
	// fix error with nums
	v_1=std::pow(temp_matrix.data[0][0],2);
	v_2=std::pow(temp_matrix.data[1][0],2);
	std::cout<<"Bool func: all 1\nResult: |00>="<<(double)(v_1+v_2);

	return 0;
}