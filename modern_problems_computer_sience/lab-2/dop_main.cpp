#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cmath>

#include "matrix.h"


int randomSelect(std::vector<float> ver){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::discrete_distribution<> distribution(ver.begin(),ver.end());
	return distribution(gen);
}


int main(){
	srand(time(NULL));

	std::vector<Matrix> matrix{Matrix(2), Matrix(2), Matrix(2), Matrix(2)};
	matrix[0].data={{1},{0}};
	matrix[1].data={{1},{1}};
	matrix[2].data={{0},{1}};
	matrix[3].data={{1},{-1}};
	Matrix h(2,2);
	h.data={{1,1},{1,-1}};
	h.coef=(double)(1/(double)sqrt(2));
	matrix[1]=Matrix::mull(h,matrix[1]);
	matrix[1].mullCoefData();
	matrix[3]=Matrix::mull(h,matrix[3]);
	matrix[3].mullCoefData();


// point 1. A select qubit
	int temp=randomSelect({0.25, 0.25, 0.25, 0.25});
	Matrix qubit=matrix[temp];
	int checker=(temp+1)%2;
	std::cout<<"Alice select:\n";
	qubit.print();
	std::cout<<"Alice checker: "<<checker<<'\n';


// point 2. B select checker
	int b_checker=randomSelect({0.5, 0.5});
	std::cout<<"\nBob checker: "<<b_checker<<'\n';
	// std::cout<<"Bob result: ";
	// if(b_checker==checker) randomQubit(qubit.data);
	// else std::cout<<(char)('0'+randomSelect({0.5, 0.5}))<<'\n';


// point 3. Bob send to Alice checker
// point 4. Alice answer Bob
	std::cout<<"\nAlice answer: ";
	if(checker==b_checker){
		std::cout<<"OK";
		std::cout<<"\n\nBob result: ";
		randomQubit(qubit.data);
	}
	else std::cout<<"REPEAT";

	return 0;
}