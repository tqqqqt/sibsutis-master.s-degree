#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
using namespace std;

int main(int argc, char** argv){
	if(argc<3){
		cout<<"error: no argums\n";
		return 4;
	}

	try{
		int temp=stoi(argv[1]);
		temp=stoi(argv[2]);
	}
	catch(exception &exp){
		cout<<"error: input values\n";
		return 4;
	}

	// if(stoi(argv[1])<stoi(argv[2])){
	// 	cout<<"error: val1 < val2\n";
	// 	return 4;
	// }

	int n_points=stoi(argv[1]), n_edges=stoi(argv[2]), n_elem=ceil(n_points/(double)n_edges);
	int cur_size_cur=n_points, cur_size_old=0;
	vector<int> points_cur(n_points), points_old, temp_points;
	vector<vector<int>> edges(n_edges,vector<int>()); 
	iota(points_cur.begin(),points_cur.end(),0);
	
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(0,n_points-1), dop_points(1,2);

	int select=0, old_elem=0;
	for(int i=0;i<n_edges;++i){
		for(int j=0;j<n_elem;++j){
			if(cur_size_cur==0) break;
			distrib=uniform_int_distribution<>(0,cur_size_cur-1);
			select=distrib(gen);
			
			temp_points.push_back(points_cur[select]);
			edges[i].push_back(points_cur[select]);
			points_cur.erase(points_cur.begin()+select);
			cur_size_old+=1;
			cur_size_cur-=1;
		}
		if(i!=0){
			old_elem=dop_points(gen);
			shuffle(points_old.begin(),points_old.end(),gen);
			for(int j=0;j<old_elem;++j) edges[i].push_back(points_old[j]);
		}
		for(int j=0;j<temp_points.size();){
			points_old.push_back(temp_points.back());
			temp_points.pop_back();
		}
	}

	for(int i=0;i<n_edges;++i){
		cout<<i<<" edge -> [ ";
		n_elem=edges[i].size();
		for(int j=0;j<n_elem;++j) cout<<edges[i][j]<<' ';
		cout<<"]\n";
	}

	return 0;
}