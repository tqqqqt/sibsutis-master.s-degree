#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <ctime>


#define TYPE_FLO
// #define TYPE_INT


#if defined(TYPE_FLO)
	#define TYPE float
#else
	#define TYPE int
#endif

struct Matrix{
	int rows, columns;
	TYPE coef;
	std::vector<std::vector<TYPE>> data;

	Matrix(int _r, int _c){
		if(_r<=0 || _r!=_c) throw "Incorect matrix in constructor";
		if((_r&(_r-1))!=0) throw "Incorect matrix in constructor";

		rows=_r;
		columns=_c;
		coef=1;
		data=std::vector<std::vector<TYPE>>(_r,std::vector<TYPE>(_c,0));
	}

	Matrix(int _r){
		if(_r<=0) throw "Incorect vector in constructor";
		if((_r&(_r-1))!=0) throw "Incorect vector in constructor";

		rows=_r;
		columns=1;
		coef=1;
		data=std::vector<std::vector<TYPE>>(_r,std::vector<TYPE>(1,0));
	}

	void print() const{
		std::cout<<"Coef="<<coef<<'\n';
		for(int i=0;i<rows;++i){
			std::cout<<'|';
			for(int j=0;j<columns;++j) std::cout<<(double)data[i][j]<<' ';
			std::cout<<"|\n";
		}
	}

	void mullCoefData(){
		for(int i=0;i<rows;++i){
			for(int j=0;j<columns;++j) data[i][j]*=coef;
		}
		coef=1;
	}

	void printBracket(bool full_flg) const{
		std::string state="";
		int temp=0, count=0;
		if(full_flg==true) std::cout<<"|f>=";
		for(int i=0;i<rows;++i){
			if(full_flg==false && data[i][0]==0) continue;

			state="";
			temp=i;
			while(temp>0 || state.length()<(rows/2)){
				state=(char)('0'+(temp&1))+state;
				temp=temp>>1;
			}
			if(count!=0) std::cout<<'+';
			std::cout<<data[i][0]<<'|'<<state<<'>';
			count+=1;
		}
		std::cout<<'\n';
	}

	static Matrix inputBracketValues(){
		Matrix res(1,1);
		int temp=0;
		std::cout<<"\nHow many nums (only 2^x nums) = ";
		std::cin>>temp;
		if(temp==1 || (temp&(temp-1))!=0) throw "Incorect num in bracket";
		res=Matrix(temp);
		std::string state="";
		for(int i=0;i<res.rows;++i){
			state="";
			temp=i;
			while(temp>0 || state.length()<(res.rows/2)){
				state=(char)('0'+(temp&1))+state;
				temp=temp>>1;
			}
			std::cout<<"Value of |"<<state<<"> state = ";
			std::cin>>res.data[i][0];
		}
		return res;
	}

	void inputValue(){
		std::cout<<"\nInput coef=";
		std::cin>>coef;
		for(int i=0;i<rows;++i){
			std::cout<<"Input "<<i<<"row = ";
			for(int j=0;j<columns;++j) std::cin>>data[i][j];
			std::cout<<'\n';
		}
	}

	static Matrix mullNum(Matrix a, int num){
		Matrix res(a.rows,a.columns);
		for(int i=0;i<a.rows;++i){
			for(int j=0;j<a.columns;++j) res.data[i][j]=a.data[i][j]*num;
		}
		return res;
	}

	static Matrix mull(Matrix a, Matrix b){
		if(a.columns!=b.rows) throw "Incorect matrix in mul";

		Matrix res(a.columns,b.rows);
		if(a.columns==1 || b.columns==1) res=Matrix(b.rows);
		res.coef=a.coef*b.coef;
		if(res.coef==0) res.coef=1;
		TYPE temp=0;
		for(int i=0;i<res.rows;++i){
			for(int j=0;j<res.columns;++j){
				temp=0;
				for(int k=0;k<a.columns;++k) temp+=a.data[i][k]*b.data[k][j];
				res.data[i][j]=temp;
			}
		}
		return res;
	}

	static Matrix tensorMull(Matrix a, Matrix b){
		if(a.columns!=b.columns || a.rows!=b.rows) throw "Incorect matrix in tensor";

		Matrix res(1,1);
		if(a.columns==1 && b.columns==1) res=Matrix(a.rows*b.rows);
		else res=Matrix(a.rows*b.rows,a.columns*b.columns);
		res.coef=a.coef*b.coef;
		if(res.coef==0) res.coef=1;
		for(int i=0;i<res.rows;++i){
			for(int j=0;j<res.columns;++j){
				res.data[i][j]=b.data[i%b.rows][j%b.columns]*a.data[i/a.rows][j/a.columns];
			}
		}
		return res;
	}
};

std::vector<std::vector<TYPE>> generQubit(int size, TYPE sum){
#if defined(TYPE_INT)
	std::vector<std::vector<TYPE>> mas(size*2,std::vector<TYPE>(size,0));
	for(int i=0;i<(size*2);i+=2){
		mas[i][i/2]=-1;
		mas[i+1][i/2]=1;
	}

	int index=std::rand()%(size*2);
	std::vector<std::vector<TYPE>> res(size,std::vector<TYPE>(1,0));
	for(int i=0;i<size;++i) res[i][0]=mas[index][i];
	return res;
#else
	if(size==1) return {{(float)std::sqrt(sum)}};

	int new_size=size/2;
	double first_sum=(double)(rand()/(double)RAND_MAX*sum), second_sum=sum-first_sum;
	std::vector<std::vector<TYPE>> tt=generQubit(new_size,first_sum), ttt=generQubit(new_size,second_sum);
	tt.insert(tt.end(),ttt.begin(),ttt.end());
	return tt;
#endif
}

void randomQubit(std::vector<std::vector<TYPE>> ver){
	std::vector<TYPE> prob;
	for(int i=0;i<ver.size();++i) prob.push_back(std::pow(ver[i][0],2));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::discrete_distribution<> distribution(prob.begin(),prob.end());

	std::string state="";
	int res=distribution(gen), siz=prob.size()/2;
	while(res!=0 || state.length()<siz){
		state=(char)('0'+(res&1))+state;
		res=res>>1;
	}
	std::cout<<"\nQubit rand = "<<state<<'\n';
}

int main(){
	srand(time(NULL));

	try{
		std::cout<<"Matrix * Matrix:\n";
		Matrix m1(2,2), m2(2,2);
		m1.data={{1,2},{3,4}};
		m1.coef=2;
		m2.data={{5,6},{7,8}};
		m2.coef=3;
		m1.print();
		m2.print();
		std::cout<<"Result:\n";
		m1=Matrix::mull(m1,m2);
		m1.print();

		std::cout<<"\nMatrix * Vector:\n";
		m2=Matrix(2);
		m2.data={{1},{2}};
		m1.print();
		m2.print();
		std::cout<<"Result:\n";
		m1=Matrix::mull(m1,m2);
		m1.print();

		std::cout<<"\nTensor Matrix * Matrix:\n";
		m1=Matrix(2,2);
		m2=Matrix(2,2);
		m1.data={{1,2},{3,4}};
		m1.coef=2;
		m2.data={{5,6},{7,8}};
		m2.coef=3;
		m1.print();
		m2.print();
		std::cout<<"Result:\n";
		m1=Matrix::tensorMull(m1,m2);
		m1.print();

		std::cout<<"\nTensor Vector * Vector:\n";
		m1=Matrix(2);
		m2=Matrix(2);
		m1.data={{1},{2}};
		m1.coef=2;
		m2.data={{3},{4}};
		m2.coef=3;
		m1.print();
		m2.print();
		std::cout<<"Result:\n";
		m1=Matrix::tensorMull(m1,m2);
		m1.print();

		std::cout<<"\nVector in bracket notation:\n";
		m2.print();
		m2.printBracket(true);

		std::cout<<"\nBracket in vector:\n";
		m1=Matrix::inputBracketValues();
		m1.print();

		std::cout<<"\nQubit check:\n";
		m1=Matrix(2);
		m1.data=generQubit(2,1.0);
		m1.print();
		randomQubit(m1.data);
		randomQubit(m1.data);
		randomQubit(m1.data);

		std::cout<<"\nQubit obrat check:\n";
		Matrix h(2,2);
		h.coef=(double)(1/(double)sqrt(2));
		h.data={{1,1},{1,-1}};
		h.print();
		m1=Matrix::mull(h,m1);
		m1.print();
		std::cout<<"\nWithout coef:\n";
		randomQubit(m1.data);
		randomQubit(m1.data);
		randomQubit(m1.data);
		m1.mullCoefData();
		std::cout<<"\nWith coef:\n";
		m1.print();
		randomQubit(m1.data);
		randomQubit(m1.data);
		randomQubit(m1.data);

		std::cout<<"\nRegist check:";
		m1=Matrix(4);
		m1.data=generQubit(4,1.0);
		m1.print();
		randomQubit(m1.data);
		randomQubit(m1.data);
		randomQubit(m1.data);

		m1=Matrix(8);
		m1.data=generQubit(8,1.0);
		m1.print();
		randomQubit(m1.data);
		randomQubit(m1.data);
		randomQubit(m1.data);
	}
	catch(const char* exp){
		std::cout<<exp<<'\n';
	}

	return 0;
}