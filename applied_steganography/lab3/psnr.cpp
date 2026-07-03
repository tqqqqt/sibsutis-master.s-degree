#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
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
	string f1, f2;
	cout<<"f1 = "; cin>>f1;
	cout<<"f2 = "; cin>>f2;

	std::ifstream file_1(f1,std::ios::binary);
	if(!file_1){
		cout<<"error file orig";
		return 4;
	}
	file_header f1header;
	v3_header f1v3header;
	color_info f1color_table[256];
	file_1.read(reinterpret_cast<char*>(&f1header),sizeof(file_header));
	file_1.read(reinterpret_cast<char*>(&f1v3header),sizeof(v3_header));
	file_1.read(reinterpret_cast<char*>(f1color_table),256*sizeof(color_info));
	file_1.seekg(f1header.bm_offset,std::ios::beg);

	int rs=0, bpr=floor((f1v3header.bit_per_pixel*f1v3header.width+31)/32)*4;
	vector<unsigned char> mas1, buf(bpr);
	do{
		file_1.read(reinterpret_cast<char*>(buf.data()),bpr);
		rs=file_1.gcount();
		if(rs==0) break;
		mas1.insert(mas1.end(),buf.begin(),buf.end());
	} while(rs>0);

	std::ifstream file_2(f2,std::ios::binary);
	if(!file_2){
		cout<<"error file orig";
		return 4;
	}
	file_header f2header;
	v3_header f2v3header;
	color_info f2color_table[256];
	file_2.read(reinterpret_cast<char*>(&f2header),sizeof(file_header));
	file_2.read(reinterpret_cast<char*>(&f2v3header),sizeof(v3_header));
	file_2.read(reinterpret_cast<char*>(f2color_table),256*sizeof(color_info));
	file_2.seekg(f2header.bm_offset,std::ios::beg);

	bpr=floor((f2v3header.bit_per_pixel*f2v3header.width+31)/32)*4;
	vector<unsigned char> mas2;
	buf.resize(bpr);
	do{
		file_2.read(reinterpret_cast<char*>(buf.data()),bpr);
		rs=file_2.gcount();
		if(rs==0) break;
		mas2.insert(mas2.end(),buf.begin(),buf.end());
	} while(rs>0);

	double mse=0, d=0;
	int errors=0;
	size_t s=mas1.size();
	for(size_t i=0;i<s;i++){
		if(mas1[i]!=mas2[i]) errors+=1;
		d=double(int(mas1[i])-int(mas2[i]));
		mse+=d*d;
	}
	mse/=double(s);
	if(mse==0) cout<<"PSNR = "<<INFINITY<<'\n';
	else cout<<"PSNR = "<<(10.0*log10((255.0*255.0)/mse))<<'\n';

	cout<<"Errors = "<<errors<<'\n';

	return 0;
}