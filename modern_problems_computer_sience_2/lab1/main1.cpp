#include <iostream>
#include <vector>
using namespace std;

vector<int> mas;

void func(int curent, const int& max){
	//cout<<curent<<"->";
	mas[curent]=1;
	for(int i=0;i<max;++i){
		if(mas[i]==1) continue;
		func(i,max);
	}
	mas[curent]=0;
	// cout<<' ';
}

int main(int argc, char** argv){
	if(argc<2) return -4;
	mas.resize(stoi(argv[1]),0);
	func(0,stoi(argv[1]));
	return 0;
}