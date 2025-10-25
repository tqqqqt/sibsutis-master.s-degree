#include <iostream>
#include <vector>
#include <cmath>

#include "matrix.h"

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


int main(){

	// point 1.
	std::vector<std::vector<int>> bool_func{{0,0,0},
											{0,1,1},
											{1,0,1},
											{1,1,1}};

	std::cout<<"Bool table:\nx1  x2  f\n";
	for(int i=0;i<4;++i){
		for(int j=0;j<3;++j) std::cout<<bool_func[i][j]<<"   ";
		std::cout<<'\n';
	}
	std::cout<<'\n';

	// create main table with transformation
	std::vector<int> y_value{0,1,0,1,0,1,0,1}, f_value;
	std::cout<<"x1  x2  y  ->  x1  x2  y*f\n";
	for(int i=0;i<4;++i){
		// first row where 0 at end
		std::cout<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   "<<y_value[i*2]<<"      "<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   ";
		f_value.push_back(y_value[i*2]^bool_func[i][2]);
		std::cout<<f_value.back()<<'\n';

		// second row where 1 at end
		std::cout<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   "<<y_value[(i*2)+1]<<"      "<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   ";
		f_value.push_back(y_value[(i*2)+1]^bool_func[i][2]);
		std::cout<<f_value.back()<<'\n';
	}
	std::cout<<'\n';

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
	f.print();

	// bracket output
	Matrix checker(8);
	checker.data={{0},{0},{0},{0},{0},{0},{0},{0}};
	Matrix res(2,2);
	int pos=0, temp=0;
	std::string state="";
	std::cout<<'\n';
	for(int i=0;i<8;++i){
		try{
			// create output state
			state="";
			temp=i;
			while(temp>0 || state.length()<3){
				state=(char)('0'+(temp&1))+state;
				temp=temp>>1;
			}
			std::cout<<"F|"<<state<<">=";

			// change checker
			checker.data[i][0]=1;
			res=Matrix::mull(f,checker);
			res.printBracket(false);
			std::cout<<'\n';

			// change ckecher back
			checker.data[i][0]=0;
		}
		catch(const char* exp){
			std::cout<<"Error: "<<exp<<'\n';
		}
	}


	// point 2. (a^x)mod(m)
	int a=2, m=15, vec_size=selectSize(m), vec_row=std::pow(2,vec_size);
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
			return 4;
		}
	}

	// output conditions and table for function
	std::cout<<"a^x mod m\na="<<a<<", m="<<m<<'\n';
	std::cout<<"x1  x2  ";
	for(int i=0;i<vec_size;++i) std::cout<<'f'<<i<<"  ";
	std::cout<<'\n';
	for(int i=0;i<4;++i){
		for(int j=0;j<2;++j) std::cout<<bool_func[i][j]<<"   ";
		temp=fx(a,i,m);
		for(int j=0;j<vec_size;++j) std::cout<<yy[temp][j]<<"   ";
		yy_short.push_back(yy[temp]);
		std::cout<<'\n';
	}

	// output header of table
	std::cout<<'\n';
	std::cout<<"x1  x2  ";
	for(int i=0;i<vec_size;++i) std::cout<<'y'<<i<<"  ";
	std::cout<<"->  x1  x2  ";
	for(int i=0;i<vec_size;++i) std::cout<<"y*f"<<i<<"  ";
	std::cout<<'\n';
	
	// output table with calculating transformation
	for(int i=0;i<4;++i){
		for(int j=0;j<vec_row;++j){
			// table part with x1 x2 and every yy rows combination
			std::cout<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   ";
			for(int k=0;k<vec_size;++k) std::cout<<yy[j][k]<<"   ";
			std::cout<<"    ";

			// table part with x1 x2 and transformation
			std::cout<<bool_func[i][0]<<"   "<<bool_func[i][1]<<"   ";
			ff.push_back(std::vector<int>(vec_size,0));
			// calculate transformation
			for(int k=0;k<vec_size;++k){
				ff.back()[k]=yy_short[i][k]^yy[j][k];
				std::cout<<ff.back()[k]<<"     ";
			}
			std::cout<<'\n';
		}
	}
	std::cout<<'\n';

	// create matrix for output
	temp=ff.size();
	res=Matrix(temp,temp);
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
	res.print();

	return 0;
}