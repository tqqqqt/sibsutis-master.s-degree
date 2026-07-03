#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>
using namespace std;

#define ALL_OUTPUT

int main(int argc, char** argv){
	if(argc<2){
		cout<<"Error: no target\n";
		return 4;
	}
	try{
		int temp=stoi(argv[1]);
		if(temp<=0 || temp>4) throw exception();
	}
	catch(exception &exp){
		cout<<"Error: incorect target\n";
		return 4;
	}

	vector<int> inp(4,0);
	cout<<"Val 1 = "; cin>>inp[0];
	cout<<"Val 2 = "; cin>>inp[1];
	cout<<"Val 3 = "; cin>>inp[2];
	cout<<"Val 4 = "; cin>>inp[3];
	for(int i=0;i<4;++i){
		if(!(inp[i]>=0 && inp[i]<=19)){
			cout<<"Error: incorect value\n";
			return 4;
		}
		for(int j=0;j<4;++j){
			if(i==j) continue;
			if(inp[i]==inp[j]){
				cout<<"Error: same value\n";
				return 4;
			}
		}
	}
	
	vector<int> mas(20);
	iota(mas.begin(),mas.end(),0);

	random_device rd;
	mt19937 gen(rd());

	vector<unsigned int> trys(1,0);
	int count=0, max_val=0;
	while(1){
		count=0;
		trys.back()+=1;
		if(trys.back()==UINT_MAX) trys.push_back(0);
		shuffle(mas.begin(),mas.end(),gen);
#if defined(ALL_OUTPUT)
		cout<<"* "<<mas[0]<<' '<<mas[1]<<' '<<mas[2]<<' '<<mas[3]<<'\n';
#endif
		for(int i=0;i<4;++i){
			for(int j=0;j<4;++j){
				if(mas[i]!=inp[j]) continue;
				count+=1;
				break;
			}
		}

		if(count==0) continue;
		if(count!=stoi(argv[1])) continue;

		for(int i=0;i<trys.size();++i){
			if(trys[i]==UINT_MAX) max_val+=1;
		}
		cout<<"\n \nResult "<<count<<"/4\n";
		cout<<"Temps = ";
		if(max_val!=0) cout<<max_val<<'*'<<UINT_MAX;
		if(max_val!=0 && trys.back()!=UINT_MAX) cout<<'+';
		if(trys.back()!=UINT_MAX) cout<<trys.back();
		cout<<'\n';
		break;
	}

	return 0;
}