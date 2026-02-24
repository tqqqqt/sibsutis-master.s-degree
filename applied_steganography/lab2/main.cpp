#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
using namespace std;

#pragma pack(push,1)
struct file_header{
	unsigned short id;
	unsigned int f_size;
	unsigned short rez_1, rez_2;
	unsigned int bm_offset;
};

struct v3_header{
	unsigned int h_size;
	unsigned int width;
	unsigned int height;
	unsigned short planes, bit_per_pixel;
	unsigned int compression;
	unsigned int size_image;
	unsigned int h_res;
	unsigned int v_res;
	unsigned int clr_used;
	unsigned int clr_imp;
};

struct color_info{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char temp;
};
#pragma pack(pop)

int main(){
	int mode=0;
	cout<<"Mode -> "; cin>>mode;
	string p_orig, p_logo;
	cout<<"File -> "; cin>>p_orig;
	cout<<"Logo -> "; cin>>p_logo;

	std::ifstream file_in(p_orig,std::ios::binary);
	if(!file_in){
		cout<<"error file orig";
		return 4;
	}
	file_header header;
	v3_header v3header;
	color_info color_table[256];
	file_in.read(reinterpret_cast<char*>(&header),sizeof(file_header));
	file_in.read(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
	file_in.read(reinterpret_cast<char*>(color_table),256*sizeof(color_info));

	std::ifstream file_logo(p_logo,std::ios::binary);
	if(!file_logo){
		cout<<"error file logo";
		return 4;
	}
	file_header logo_header;
	v3_header logo_v3header;
	color_info logo_color_table[256];
	file_logo.read(reinterpret_cast<char*>(&logo_header),sizeof(file_header));
	file_logo.read(reinterpret_cast<char*>(&logo_v3header),sizeof(v3_header));
	file_logo.read(reinterpret_cast<char*>(logo_color_table),256*sizeof(color_info));
    
    file_logo.seekg(logo_header.bm_offset,std::ios::beg);
	file_in.seekg(header.bm_offset,std::ios::beg);

	if(mode==1){
		std::ofstream file_out("new_file.bmp",std::ios::binary);
		if(!file_out){
			cout<<"error out file";
			return 4;
		}
		file_out.write(reinterpret_cast<char*>(&header),sizeof(file_header));
	    file_out.write(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
		file_out.write(reinterpret_cast<char*>(color_table),256*sizeof(color_info));

		vector<int> indx(v3header.width);
		iota(indx.begin(),indx.end(),0);
		size_t seed=hash<string>{}("KEY");
		mt19937 rng((uint32_t)seed);
		shuffle(indx.begin(),indx.end(),rng);

		int bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4, logo_bpr=floor((logo_v3header.bit_per_pixel*logo_v3header.width+31)/32)*4;
		int rs=0, rs2=0, point=0, count=0, ccount=0;
		unsigned char buf[bpr], logo[bpr];
		file_logo.read(reinterpret_cast<char*>(logo),logo_bpr);
		rs2=file_logo.gcount();
		do{
			file_in.read(reinterpret_cast<char*>(buf),bpr);
			for(int i=0;i<v3header.width;i++){
				if(rs2==0) break;
				buf[indx[i]]=(buf[indx[i]]&252)|(logo[point]&3);
				logo[point]=logo[point]>>2;
				count+=2;
				ccount+=2;
				if(count==8){
					point++;
					count=0;
				}
				if(point==logo_v3header.width){
					file_logo.read(reinterpret_cast<char*>(logo),logo_bpr);
					rs2=file_logo.gcount();
					if(rs2==0) break;
					point=0;
				}
			}
			rs=file_in.gcount();
			file_out.write(reinterpret_cast<char*>(buf),bpr);
		} while(rs>0);
		file_out.close();
		cout<<"Coded = "<<ccount<<'\n';
	}
	else if(mode==2){
		vector<vector<unsigned char>> mas;
		int rs=0, bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
		do{
			mas.push_back(vector<unsigned char>(bpr,0));
			file_in.read(reinterpret_cast<char*>(mas.back().data()),bpr);
			rs=file_in.gcount();			
		} while(rs>0);
		mas.pop_back();
		
		vector<int> grad(v3header.width*v3header.height,0);
		for(int r=0;r<v3header.height;++r){
			for(int c=0;c<v3header.width;++c){
				int indx=r*v3header.width+c, val=mas[r][c]&252, g=0;
				if((c+1)<v3header.width) g+=abs((int(mas[r][c+1])&252)-val);
				if((r+1)<v3header.height) g+=abs((int(mas[r+1][c])&252)-val);
				grad[indx]=g;
			}
		}

		vector<int> indx(v3header.width*v3header.height);
		iota(indx.begin(),indx.end(),0);
		sort(indx.begin(),indx.end(),[&](const int& a, const int& b){
			if(grad[a]!=grad[b]) return grad[a]>grad[b];
			return a<b;
		});

		rs=0;
		bpr=floor((logo_v3header.bit_per_pixel*logo_v3header.width+31)/32)*4;
		int point=0, indx_size=indx.size(), x=0, y=0, count=0, ccount=0;
		unsigned char buf[bpr];
		do{
			file_logo.read(reinterpret_cast<char*>(buf),bpr);
			rs=file_logo.gcount();
			if(rs==0) break;
			for(int i=0;i<logo_v3header.width;){
				x=indx[point]/v3header.width;
				y=indx[point]%v3header.height;
				mas[y][x]=(mas[y][x]&252)|(buf[i]&3);
				buf[i]=buf[i]>>2;
				count+=2;
				ccount+=2;
				point+=1;
				if(count==8){
					count=0;
					i++;
				}
				if(point>=indx_size) break;
			}
			if(point>=indx_size) break;
		} while(rs>0);
		cout<<"Coded = "<<ccount<<'\n';


		std::ofstream file_out("new_file.bmp",std::ios::binary);
		if(!file_out){
			cout<<"error out file";
			return 4;
		}
		file_out.write(reinterpret_cast<char*>(&header),sizeof(file_header));
	    file_out.write(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
		file_out.write(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
		bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
		for(int i=0;i<mas.size();i++){
			file_out.write(reinterpret_cast<char*>(mas[i].data()),bpr);
		}
		file_out.close();
	}
	else if(mode==3){
		vector<int> indx(v3header.width);
		iota(indx.begin(),indx.end(),0);
		size_t seed=hash<string>{}("KEY");
		mt19937 rng((uint32_t)seed);
		shuffle(indx.begin(),indx.end(),rng);

		int bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4, logo_bpr=floor((logo_v3header.bit_per_pixel*logo_v3header.width+31)/32)*4;
		int rs=0, rs2=0, point=0, errors=0, count=0, ccount=0;
		unsigned char buf[bpr], logo[bpr], temp=0;
		file_logo.read(reinterpret_cast<char*>(logo),logo_bpr);
		rs2=file_logo.gcount();
		while(1){
			file_in.read(reinterpret_cast<char*>(buf),bpr);
			rs=file_in.gcount();
			if(rs==0) break;
			for(int i=0;i<v3header.width;i++){
				if(rs2==0) break;
				temp=temp|((buf[indx[i]]&3)<<count);
				count+=2;
				ccount+=2;
				if(count==8){
					if((int)temp!=(int)logo[point]) errors+=1;
					count=0;
					point++;
					temp=0;
				}
				if(point==logo_v3header.width){
					file_logo.read(reinterpret_cast<char*>(logo),logo_bpr);
					rs2=file_logo.gcount();
					if(rs2==0) break;
					point=0;
				}
			}
		}
		cout<<"Check logo result = "<<errors<<" errors\n";
		cout<<"Decoded = "<<ccount<<'\n';
	}
	else{
		vector<vector<unsigned char>> mas;
		int rs=0, bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
		do{
			mas.push_back(vector<unsigned char>(bpr,0));
			file_in.read(reinterpret_cast<char*>(mas.back().data()),bpr);
			rs=file_in.gcount();			
		} while(rs>0);
		mas.pop_back();
		
		vector<int> grad(v3header.width*v3header.height,0);
		for(int r=0;r<v3header.height;++r){
			for(int c=0;c<v3header.width;++c){
				int indx=r*v3header.width+c, val=mas[r][c]&252, g=0;
				if((c+1)<v3header.width) g+=abs((int(mas[r][c+1])&252)-val);
				if((r+1)<v3header.height) g+=abs((int(mas[r+1][c])&252)-val);
				grad[indx]=g;
			}
		}

		vector<int> indx(v3header.width*v3header.height);
		iota(indx.begin(),indx.end(),0);
		sort(indx.begin(),indx.end(),[&](const int& a, const int& b){
			if(grad[a]!=grad[b]) return grad[a]>grad[b];
			return a<b;
		});

		rs=0;
		bpr=floor((logo_v3header.bit_per_pixel*logo_v3header.width+31)/32)*4;
		int indx_size=indx.size(), point=0, x=0, y=0, count=0, ccount=0, errors=0;
		unsigned char buf[bpr], temp=0;
		do{
			file_logo.read(reinterpret_cast<char*>(buf),bpr);
			rs=file_logo.gcount();
			if(rs==0) break;
			for(int i=0;i<logo_v3header.width;){
				x=indx[point]/v3header.width;
				y=indx[point]%v3header.height;
				temp=temp|((mas[y][x]&3)<<count);
				count+=2;
				ccount+=2;
				point+=1;
				if(count==8){
					if((int)temp!=(int)buf[i]) errors+=1;
					count=0;
					temp=0;
					i++;
				}
				if(point>=indx_size) break;
			}
			if(point>=indx_size) break;
		} while(rs>0);
		cout<<"Check logo result = "<<errors<<" errors\n";
		cout<<"Decoded = "<<ccount<<'\n';
	}

	file_in.close();
	file_logo.close();
	return 0;
}