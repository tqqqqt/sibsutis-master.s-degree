#include <iostream>
#include <vector>
#include <random>
#include <ctime>


#define SIZE 8


enum QUBIT_STATE{ ZERO=1, ONE=3, D_ZERO=2, D_ONE=4 };
enum QUBIT_CHECKER{ PLUS=1, CREST=2 };


int randomSelect(std::vector<float> ver){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::discrete_distribution<> distribution(ver.begin(),ver.end());
	return distribution(gen);
}


int main(){
	srand(time(NULL));
	
	std::vector<QUBIT_STATE> qubit_state{ZERO, ONE, D_ZERO, D_ONE};
	std::vector<QUBIT_CHECKER> qubit_checker{PLUS, CREST};

	std::vector<float> state_ver{0.25, 0.25, 0.25, 0.25};
	std::vector<float> checker_ver{0.5, 0.5};

	std::vector<QUBIT_STATE> state;
	std::vector<QUBIT_CHECKER> checker;
	std::vector<bool> answer;

	std::cout<<"States:\n|0>=1, |0>'=2\n|1>=3, |1>'=4\n";
	std::cout<<"Checkers:\n+=1, x=2\n";

	// point 1. A select state
	std::cout<<"\nAlice select: ";
	for(int i=0;i<SIZE;++i){
		state.push_back(qubit_state[randomSelect(state_ver)]);
		std::cout<<state[i]<<' ';
	}
	std::cout<<"\n";

	// point 2. B select checker and check qubit
	std::cout<<"\nBob checker:  ";
	for(int i=0;i<SIZE;++i){
		checker.push_back(qubit_checker[randomSelect(checker_ver)]);
		std::cout<<checker[i]<<' ';
	}
	std::cout<<"\nBob result:   ";
	for(int i=0;i<SIZE;++i){
		if((int)(checker[i]%2)==(int)(state[i]%2)){
			answer.push_back(true);
			if(state[i]==ZERO || state[i]==D_ZERO) std::cout<<"0 ";
			else std::cout<<"1 ";
			// std::cout<<state[i]<<' ';
		}
		else{
			answer.push_back(false);
			std::cout<<(char)('0'+(randomSelect(checker_ver)))<<' ';
		}
	}
	std::cout<<'\n';

	// point 3. Bob send to Alice
	// point 4. Alice answer bob
	std::cout<<"\nAlice answer: ";
	for(int i=0;i<SIZE;++i){
		if(answer[i]==true) std::cout<<"OK ";
		else std::cout<<"REPEAT ";
	}

	return 0;
}