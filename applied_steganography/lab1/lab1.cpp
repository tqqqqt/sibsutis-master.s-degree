#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
using namespace std;

int maskdel[8]={254, 253, 251, 247, 239, 223, 191, 127};

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
	std::cout<<"Mode -> ";
	std::cin>>mode;
	if(mode==1){
		int byte=0;
		std::string file_path;
		std::cout<<"Byte = ";
		std::cin>>byte;
		if(byte<0 or byte>7){
			cout<<"error byte";
			return 4;
		}
		std::cout<<"File = ";
		std::cin>>file_path;
		std::ifstream file_in(file_path,std::ios::binary);
		if(!file_in){
			cout<<"error file";
			return 4;
		}
		file_header header;
		v3_header v3header;
		file_in.read(reinterpret_cast<char*>(&header),sizeof(file_header));
		file_in.read(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
		color_info color_table[256];
		file_in.read(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
		std::ofstream file_out("new_file.BMP",std::ios::binary);
		if(!file_out){
			cout<<"error out file";
			return 4;
		}
		file_out.write(reinterpret_cast<char*>(&header),sizeof(file_header));
                file_out.write(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
		file_out.write(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
                file_in.seekg(header.bm_offset,std::ios::beg);
		int bpr=1024, rs=0;
		unsigned char buf[bpr];
		do{
			file_in.read(reinterpret_cast<char*>(buf),bpr);
			for(int i=0;i<v3header.width;i++){
				if(((buf[i]>>byte)&1)==1) buf[i]=255;
				else buf[i]=0;
			}
			rs=file_in.gcount();
			file_out.write(reinterpret_cast<char*>(buf),rs);
		} while(rs>0);
		file_in.close();
		file_out.close();
	}
	else if(mode==2){
		int byte=0, rd=0;
		cout<<"Byte = ";
		cin>>byte;
		if(byte<0 || byte>7){
			cout<<"error byte";
			return 4;
		}
		std::ifstream textfile("alice.txt",std::ios::binary);
		std::vector<unsigned char> text, buffer(1024);
		cout<<"Read text... ";
		while(1){
			textfile.read(reinterpret_cast<char*>(buffer.data()),1024);
			rd=textfile.gcount();
			if(rd<=0) break;
			text.insert(text.end(),buffer.begin(),buffer.begin()+rd);
		}
		textfile.close();
		cout<<"complete\n";
		std::string file_path;
		std::cout<<"File = ";
                std::cin>>file_path;
                std::ifstream file_in(file_path,std::ios::binary);
                if(!file_in){
                        cout<<"error file";
                        return 4;
                }
                file_header header;
                v3_header v3header;
                file_in.read(reinterpret_cast<char*>(&header),sizeof(file_header));
                file_in.read(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
                color_info color_table[256];
                file_in.read(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
                std::ofstream file_out("encode_file.BMP",std::ios::binary);
                if(!file_out){
                        cout<<"error out file";
                        return 4;
                }
                file_out.write(reinterpret_cast<char*>(&header),sizeof(file_header));
                file_out.write(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
                file_out.write(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
                file_in.seekg(header.bm_offset,std::ios::beg);
		int bpr=1024, rs=0, point=0, codelen=0, count=0, fulltextsize=text.size();
                unsigned char buf[bpr];
                do{
                        file_in.read(reinterpret_cast<char*>(buf),bpr);
                        for(int i=0;i<v3header.width;i++){
                                if(point==fulltextsize) break;
				if(codelen==8){
					point++;
					count++;
					codelen=0;
				}
				buf[i]=buf[i]&maskdel[byte];
				buf[i]=buf[i]|((text[point]&1)<<byte);
				text[point]=text[point]>>1;
				codelen++;
                        }
                        rs=file_in.gcount();
                        file_out.write(reinterpret_cast<char*>(buf),rs);
                } while(rs>0);
                file_in.close();
                file_out.close();
		cout<<"Encoded "<<count<<" symbols\n";
	}
	else{
		int byte=0, rd=0;
                cout<<"Byte = ";
                cin>>byte;
                if(byte<0 || byte>7){
                        cout<<"error byte";
                        return 4;
                }
                cout<<"complete\n";
		std:;string file_path;
                std::cout<<"File = ";
                std::cin>>file_path;
                std::ifstream file_in(file_path,std::ios::binary);
                if(!file_in){
                        cout<<"error file";
                        return 4;
                }
                file_header header;
                v3_header v3header;
		file_in.read(reinterpret_cast<char*>(&header),sizeof(file_header));
                file_in.read(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
                color_info color_table[256];
                file_in.read(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
                std::ofstream file_out("decode_file.txt",std::ios::binary);
                if(!file_out){
                        cout<<"error out file";
                        return 4;
                }
                file_in.seekg(header.bm_offset,std::ios::beg);
                int bpr=1024, rs=0, temp=0, codelen=0, count=0;
                unsigned char buf[bpr], decod;
                do{
                        file_in.read(reinterpret_cast<char*>(buf),bpr);
                        for(int i=0;i<v3header.width;i++){
                                if(codelen==8){
                                        file_out.put((char)decod);
                                        count++;
                                        codelen=0;
                                        decod=0;
                                }
				temp=(buf[i]>>byte)&1;
				decod=decod|(temp<<codelen);
				codelen++;
			}
                        rs=file_in.gcount();
                } while(rs>0);
                file_in.close();
                file_out.close();
		cout<<"Decoded "<<count<<" symbols\n";
	}
	return 0;
}
