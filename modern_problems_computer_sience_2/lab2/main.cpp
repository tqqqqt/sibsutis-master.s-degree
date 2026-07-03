#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <vector>
using namespace std;

#define TIMEC 10

struct shareddata{
	int lock, end;
};

const char* file_name="data.txt";
const int num_read=4;

void writer(shareddata* data){
	cout<<"Start writer\n";
	data->end=0;
	for(int i=0;i<TIMEC;++i){
		data->lock=1;
		cout<<"start write... ";
		ofstream ofs(file_name);
		if(ofs.is_open()){
			char val='A'+i;
			ofs<<val<<flush;
			ofs.close();
		}
		sleep(1);
		cout<<"end write\n";
		data->lock=0;
		usleep(100000);
	}
	data->end=1;
	cout<<"End writer\n";
}

void reader(int id, shareddata* data, int& count){
	int priority=(id)*5;
	setpriority(PRIO_PROCESS, 0, priority);
	cout<<"Reader id="<<id<<" prior="<<priority<<" start work\n";
	for(int i=0;i<TIMEC && data->end==0;++i){
		while(data->lock==1) usleep(1000);
		ifstream ifs(file_name);
		std::string content;
		while(data->lock==0 && data->end==0){
		//if(ifs.is_open()){
			ifs>>content;
			cout<<"reader id="<<id<<" read="<<content<<'\n';
			count+=1;
			ifs.seekg(0,std::ios::beg);
			//ifs>>content;
			//cout<<"reader id="<<id<<" sec read="<<content<<'\n';
			//ifs.close();
		//}
		}
		ifs.close();
		sleep(1);
	}
	cout<<"reader id="<<id<<" end work\n";
}

int main(){
	ofstream ofs(file_name,std::ios::trunc);
	ofs.close();

	int shm_id=shmget(IPC_PRIVATE,sizeof(shareddata),IPC_CREAT | 0666);
	if(shm_id<0){
		cout<<"error create memory\n";
		return -4;
	}
	shareddata* shared=(shareddata*)shmat(shm_id,NULL,0);
	shared->lock=0;

	pid_t pid=fork();
	if(pid==0){
		writer(shared);
		shmdt(shared);
		return 0;
	}

	for(int i=0;i<num_read;++i){
		pid_t r_pid=fork();
		int count=0;
		if(r_pid==0){
			reader(i,shared,count);
			cout<<i<<"id pr="<<(i)*5<<" read="<<count<<'\n';
			shmdt(shared);
			return 0;
		}
	}

	for(int i=0;i<num_read;++i) wait(NULL);

	shmdt(shared);
	shmctl(shm_id,IPC_RMID,NULL);
	cout<<"End work";
	return 0;
}
