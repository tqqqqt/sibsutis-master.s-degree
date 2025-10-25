#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "matrix.h"

#define A_VALUE 3
#define M_VALUE 15

// find count for f columns
int selectSize(int m){
	int res=1, temp=2;
	while(temp<m){
		res+=1;
		temp*=2;
	}
	return res;
}

int fx(int a, int x, int m){
	return ((int)(std::pow(a,x))%m);
}


void boolFunc(Matrix& _res, std::vector<std::vector<int>> bool_func){
	// point 1.
	// create main table with transformation
	std::vector<int> y_value{0,1,0,1,0,1,0,1}, f_value;
	for(int i=0;i<4;++i){
		// first row where 0 at end
		f_value.push_back(y_value[i*2]^bool_func[i][2]);

		// second row where 1 at end
		f_value.push_back(y_value[(i*2)+1]^bool_func[i][2]);
	}

	// create matrix
	Matrix f(8,8);
	for(int i=0;i<8;){
		if(y_value[i]==f_value[i]){
			f.data[i][i]=1;
			i+=1;
		}
		// dont think becouse only 2 variants 
		else{
			f.data[i][i+1]=1;
			f.data[i+1][i]=1;
			i+=2;
		}
	}
	_res=f;
}


void modFunc(Matrix& _res){
	// point 2. (a^x)mod(m)
	int vec_size=selectSize(M_VALUE), vec_row=std::pow(2,vec_size);
	int temp=0;
	// yy for all variants, yy_short for only function result, ff for transformation
	std::vector<std::vector<int>> yy, yy_short, ff;
	// create all yy rows with nums [0..2^vec_size] in binary
	int ttemp_pos=vec_size-1;
	for(int i=0;i<vec_row;++i){
		yy.push_back(std::vector<int>(vec_size,0));
		ttemp_pos=vec_size-1;
		temp=i;
		while(temp>0 || ttemp_pos>=0){
			yy[i][ttemp_pos]=(temp&1);
			temp=temp>>1;
			ttemp_pos-=1;
		}
		// if error in rows size
		if(temp>0){
			std::cout<<"Error: Incorect calculate vec size.\n";
			return;
		}
	}

	// collect function result
	for(int i=0;i<4;++i){
		temp=fx(A_VALUE,i,M_VALUE);
		yy_short.push_back(yy[temp]);
	}
	
	// calculating transformation
	for(int i=0;i<4;++i){
		for(int j=0;j<vec_row;++j){
			ff.push_back(std::vector<int>(vec_size,0));
			for(int k=0;k<vec_size;++k){
				ff.back()[k]=yy_short[i][k]^yy[j][k];
			}
		}
	}

	// create matrix for output
	temp=ff.size();
	Matrix res=Matrix(temp,temp);
	int err=0, part=0;
	// index start with 1 becouse need calculate part for every ff combinations
	for(int i=1;i<=temp;++i){
		// check in part
		for(int j=(part*vec_row);j<((part*vec_row)+vec_row) && j<temp;++j){
			err=0;
			// check every symbol
			for(int k=0;k<vec_size;++k){
				if(yy[(i-1)%vec_row][k]!=ff[j][k]){
					err=1;
					break;
				}
			}
			if(err==0){
				ttemp_pos=j;
				break;
			}
		}
		// set move
		res.data[i-1][ttemp_pos]=1;
		//std::cout<<i-1<<"->"<<ttemp_pos<<'\n';
		if((i%(vec_row))==0) part+=1;
	}
	_res=res;
}