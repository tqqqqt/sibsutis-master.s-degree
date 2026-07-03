#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <climits>
using namespace std;

//#define DEBUG

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

bool PairsCheck(int a1, int b1, int a2, int b2){
	int l1=min(a1,b1), l2=min(a2,b2);
	int r1=max(a1,b1), r2=max(a2,b2);

	return !(r1<l2 || r2<l1);
}

void WriteLsb(vector<int>& lsb_save, vector<vector<unsigned char>>& mas, int& pp, int val, v3_header v3header)
{
	for(int c=0;pp<v3header.width && c<8;++pp){
		lsb_save.push_back(mas[0][pp]&1);
		mas[0][pp]=((mas[0][pp]>>1)<<1)|(val&1);
		val=val>>1;
		c+=1;
	}
}

void WriteInfo(int a, int b, int& p, vector<vector<unsigned char>>& mas, 
	vector<int>& histog, const v3_header& v3header, 
	vector<int>& lsb_save, vector<unsigned char> text)
{
	if(a==-1) return;

#if defined(DEBUG)
	cout<<"{start p="<<p<<"}";
#endif

	int step=1;
	vector<int> cord_map;
	if(a>b) step=-1;
	if((int)histog[b]!=0){
		for(int i=1;i<v3header.height;++i){
			for(int j=0;j<v3header.width;++j){
				if((int)mas[i][j]!=(int)b) continue;
				cord_map.push_back(i);
				cord_map.push_back(j);
			}
		}
	}
	histog[b]=0;

	int count_lsb_save=lsb_save.size(), count_cord=cord_map.size()/2, c=0, ttt=0, tt=count_cord*2-1;
	unsigned char znak='`';

#if defined(DEBUG)
	cout<<"|"<<a<<','<<b<<"{{"<<count_lsb_save<<"=="<<count_cord<<"}}\n";
	cout<<"lsb_save: ";
	for(int i=0;i<count_lsb_save;++i) cout<<lsb_save[i]<<' ';
	cout<<"\ncord_map: ";
	for(int i=0;i<count_cord*2;++i) cout<<cord_map[i]<<' ';
	cout<<'\n';
#endif

	if(count_lsb_save==0) count_lsb_save=-1;
	if(count_cord==0) count_cord=-1;
	for(int i=1;i<v3header.height;++i){
		for(int j=0;j<v3header.width;++j){
			if((a<b && (mas[i][j]>a && mas[i][j]<b)) || (a>b && (mas[i][j]<a && mas[i][j]>b))){
				mas[i][j]=mas[i][j]+step;
				continue;
			}
			if(mas[i][j]!=a) continue;
			if(count_lsb_save!=-1){
				if((count_lsb_save&1)==1) mas[i][j]=mas[i][j]+step;
				count_lsb_save=count_lsb_save>>1;
				c+=1;
				if(c==8){
					c=0;
					count_lsb_save=-1;
				}
			}
			else if(!lsb_save.empty()){
				if((lsb_save[0]&1)==1) mas[i][j]=mas[i][j]+step;
				lsb_save.erase(lsb_save.begin());
			}
			else if(count_cord!=-1){
				if((count_cord&1)==1) mas[i][j]=mas[i][j]+step;
				count_cord=count_cord>>1;
				c+=1;
				if(c==8){
					c=0;
					count_cord=-1;
				}
			}
			else if(!cord_map.empty()){
				if((cord_map[0]&1)==1) mas[i][j]=mas[i][j]+step;
#if defined(DEBUG)
				if(cord_map.size()==tt) cout<<'*'<<cord_map[0]<<'-'<<(int)mas[i][j]<<"->";
#endif
				cord_map[0]=cord_map[0]>>1;
				c+=1;
#if defined(DEBUG)
				if(cord_map.size()==tt) cout<<cord_map[0]<<','<<c<<'\n';
#endif
				if(c==9){
					c=0;
					cord_map.erase(cord_map.begin());
				}
			}
			else if(ttt==0){
				if((znak&1)==1) mas[i][j]=mas[i][j]+step;
				znak=znak>>1;
				c+=1;
				if(c==8){
					c=0;
					ttt=1;
				}
			}
			else{
				if((text[p]&1)==1) mas[i][j]=mas[i][j]+step;
				text[p]=text[p]>>1;
				c+=1;
				if(c==8){
					c=0;
					p+=1;
				}
			}
		}
	}

#if defined(DEBUG)
	cout<<"{end p="<<p<<"}";
#endif
}

void ReadInfo(int a, int b, int& lsb_flag, int& map_flag, vector<vector<unsigned char>>& mas,
	const v3_header& v3header, vector<unsigned char>& text, vector<int>& lsb_save)
{
	int step=1;
	if(a>b) step=-1;
	vector<int> cord_map;
	int ttemp=0;

	int c=0, count_lsb=0, count_cord=0;
	for(int i=1;i<v3header.height;++i){
		for(int j=0;j<v3header.width;++j){
			if(mas[i][j]==(a+step)){
#if defined(DEBUG)
				if(cord_map.size()==1){
					cout<<"Y";
					cout<<(int)ttemp<<' '<<(int)(ttemp|(1<<c))<<' '<<(int)(ttemp*2);
				}
#endif
				ttemp=(ttemp|(1<<c));
			}
			else if(mas[i][j]!=a) continue;
			c+=1;
			mas[i][j]=a;

			if(lsb_flag==0){
				if(c==8){
					lsb_flag=1;
					count_lsb=(int)ttemp;
#if defined (DEBUG)
					cout<<"{"<<count_lsb<<'\n';
#endif
					ttemp=0;
					c=0;
				}
			}
			else if(lsb_flag==1 && count_lsb>0){
				lsb_save.push_back((int)ttemp);
				ttemp=0;
				count_lsb-=1;
				c=0;
				if(count_lsb==0) lsb_flag=2;
			}
			else if(lsb_flag==2 && map_flag==0){
				if(c==8){
					map_flag=1;
					if((char)ttemp=='`'){
						ttemp=0;
						map_flag=2;
						c=0;
						continue;
					}
					count_cord=((int)ttemp)*2;
#if defined(DEBUG)
					cout<<"{{"<<count_cord/2<<'\n';
#endif		
					ttemp=0;
					c=0;
				}
			}
			else if(lsb_flag==2 && map_flag==1 && count_cord>0){
#if defined(DEBUG)
				if(cord_map.size()==1) cout<<'*'<<(int)ttemp<<'-'<<(int)mas[i][j]<<'\n';
#endif
				if(c==9){
					cord_map.push_back((int)ttemp);
					ttemp=0;
					count_cord-=1;
					c=0;
				}
				if(count_cord==0) map_flag=2;
			}
			else if(lsb_flag==2 && map_flag==2){
				if(c==8){
					text.push_back((unsigned char)ttemp);
					ttemp=0;
					c=0;
				}
			}
		}
	}

#if defined(DEBUG)
	cout<<"}"<<a<<'-'<<b<<"{\n";
	cout<<"lsb_save: ";
	for(int i=0;i<lsb_save.size();++i) cout<<lsb_save[i]<<',';
	cout<<"\ncord_map: ";
	for(int i=0;i<cord_map.size();++i) cout<<cord_map[i]<<',';
	cout<<'\n';
#endif

	step*=-1;
	for(int i=1;i<v3header.height;++i){
		for(int j=0;j<v3header.width;++j){
			if((a<b && (mas[i][j]>a && mas[i][j]<=b)) || (a>b && (mas[i][j]<a && mas[i][j]>=b))){
				mas[i][j]=mas[i][j]+step;
				continue;
			}
		}
	}

	int sz=cord_map.size();
	for(int i=0;i<sz;i+=2) mas[cord_map[i]][cord_map[i+1]]=b;
}

int main()
{
	string p_orig="";
	cout<<"File -> "; cin>>p_orig;

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
	file_in.seekg(header.bm_offset,std::ios::beg);

	vector<vector<unsigned char>> mas;
	cout<<"Read file...";
	int rs=0, bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
	do{
		mas.push_back(vector<unsigned char>(bpr,0));
		file_in.read(reinterpret_cast<char*>(mas.back().data()),bpr);
		rs=file_in.gcount();			
	} while(rs>0);
	file_in.close();
	cout<<"complete\n";
	mas.pop_back();

	std::ifstream textfile("alice.txt",std::ios::binary);
	std::vector<unsigned char> text, buffer(1024);
	cout<<"Read text...";
	while(1){
		textfile.read(reinterpret_cast<char*>(buffer.data()),1024);
		rs=textfile.gcount();
		if(rs<=0) break;
		text.insert(text.end(),buffer.begin(),buffer.begin()+rs);
	}
	textfile.close();
	cout<<"complete\n";

	// 1
	vector<int> histog(256,0);
	for(int i=0;i<v3header.height;++i){
		for(int j=0;j<v3header.width;++j) histog[mas[i][j]]+=1;
	}

#if defined(DEBUG)
	cout<<'\n';
	for(int i=0;i<256;++i) cout<<i<<'='<<histog[i]<<'\n';
#endif

	// 2
	int v1=INT_MAX, v2=INT_MAX, v3=INT_MAX;
	int b1=0, b2=0, b3=0;
	for(int i=0;i<256;++i){
		if(histog[i]<v1){
			v1=histog[i];
			b1=i;
		}
	}
	for(int i=0;i<256;++i){
		if(histog[i]>v1 && histog[i]<v2 && abs(i-b1)>3){
			v2=histog[i];
			b2=i;
		}
	}
	for(int i=0;i<256;++i){
		if(histog[i]>v2 && histog[i]<v3 && abs(i-b1)>3 && abs(i-b2)>3){
			v3=histog[i];
			b3=i;
		}
	}
#if defined(DEBUG)
	cout<<"b1 p="<<b1<<'-'<<histog[b1]<<'\n';
	cout<<"b2 p="<<b2<<'-'<<histog[b2]<<'\n';
	cout<<"b3 p="<<b3<<'-'<<histog[b3]<<'\n';
#endif

	// 3
	v1=-1; v2=-1;
	int a1=-1, a3=-1;
	for(int i=0;i<=b1;++i){
		if(histog[i]>v1){
			v1=histog[i];
			a1=i;
		}
	}
	for(int i=b3;i<256;++i){
		if(histog[i]>v2){
			v2=histog[i];
			a3=i;
		}
	}
#if defined(DEBUG)
	cout<<'\n'<<"a1 p="<<a1<<'-'<<histog[a1]<<'\n';
	cout<<"a3 p="<<a3<<'-'<<histog[a3]<<'\n';
#endif

	// 4
	v1=-1; v2=-1; v3=-1;
	int v4=-1;
	int a12=-1, a21=-1, a23=-1, a32=-1;
	for(int i=b1+1;i<b2;++i){
		if(histog[i]>v1){
			v1=histog[i];
			a12=i;
		}
	}
	for(int i=b1+1;i<b2;++i){
		if(histog[i]>v2 && i!=a12){
			v2=histog[i];
			a21=i;
		}
	}
	if(a12>a21) swap(a12,a21);
	for(int i=b2+1;i<b3;++i){
		if(histog[i]>v3){
			v3=histog[i];
			a23=i;
		}
	}
	for(int i=b2+1;i<b3;++i){
		if(histog[i]>v4 && i!=a23){
			v4=histog[i];
			a32=i;
		}
	}
	if(a23>a32) swap(a23,a32);

#if defined(DEBUG)
	cout<<'\n'<<"a12 p="<<a12<<'-'<<histog[a12]<<'\n';
	cout<<"a21 p="<<a21<<'-'<<histog[a21]<<'\n';
	cout<<"a23 p="<<a23<<'-'<<histog[a23]<<'\n';
	cout<<"a32 p="<<a32<<'-'<<histog[a32]<<'\n';
#endif

	// 5
	int res1=-1, res2=-1, res3=-1;
	if(a12==-1 && a1!=-1) res1=a1;
	else res1=(histog[a1]>histog[a12])?a1:a12;

	if(a21!=-1 && a23!=-1) res2=(histog[a21]>histog[a23])?a21:a23;
	else if(a21==-1 && a23!=-1) res2=a23;
	else if(a21!=-1 && a23==-1) res2=a21;

	if(a32==-1 && a3!=-1) res3=a3;
	else res3=(histog[a32]>histog[a3])?a32:a3;

#if defined(DEBUG)
	cout<<'\n'<<"res1 p="<<res1<<'-';
	if(res1!=-1) cout<<histog[res1]<<'\n';
	else cout<<'\n';
	cout<<"res2 p="<<res2<<'-';
	if(res2!=-1) cout<<histog[res2]<<'\n';
	else cout<<'\n';
	cout<<"res3 p="<<res3<<'-';
	if(res3!=-1) cout<<histog[res3]<<'\n';
	else cout<<'\n';
#endif


	vector<int> lsb_save;
	int pp=0, flag=0, sum=0;
	if(histog[res1]>histog[b1] && histog[res1]>(histog[b1]*2*9) && histog[b1]<256) flag=flag|1;
	if(histog[res2]>histog[b2] && histog[res2]>(histog[b2]*2*9) && histog[b2]<256 
		&& b1!=b2 && res1!=res2 && !PairsCheck(res1,b1,res2,b2)) flag=flag|2;
	if(histog[res3]>histog[b3] && histog[res3]>(histog[b3]*2*9) && histog[b3]<256 
		&& b1!=b3 && b2!=b3 && res1!=res3 && res2!=res3
		&& !PairsCheck(res1,b1,res3,b3) && !PairsCheck(res2,b2,res3,b3)) flag=flag|4;

	sum=((flag&1)+((flag>>1)&1)+((flag>>2)&1));

#if defined(DEBUG)
	cout<<"flag="<<flag<<" - "<<sum<<'\n';
#endif

	if(sum==0){
		cout<<"no pairs";
		return 4;
	}
	cout<<"Write head info...";
	WriteLsb(lsb_save,mas,pp,sum,v3header);
	if((flag&1)==1){
		WriteLsb(lsb_save,mas,pp,res1,v3header);
		WriteLsb(lsb_save,mas,pp,b1,v3header);
	}
	if(((flag>>1)&1)==1){
		WriteLsb(lsb_save,mas,pp,res2,v3header);
		WriteLsb(lsb_save,mas,pp,b2,v3header);
	}
	if(((flag>>2)&1)==1){
		WriteLsb(lsb_save,mas,pp,res3,v3header);
		WriteLsb(lsb_save,mas,pp,b3,v3header);
	}
	cout<<"complete\n";

	cout<<"Write info...";
	int p=0;
	if((flag&1)==1) WriteInfo(res1,b1,p,mas,histog,v3header,lsb_save,text);
	if(((flag>>1)&1)==1) WriteInfo(res2,b2,p,mas,histog,v3header,lsb_save,text);
	if(((flag>>2)&1)==1) WriteInfo(res3,b3,p,mas,histog,v3header,lsb_save,text);
	cout<<"complete\n";

#if defined(DEBUG)
	cout<<"text p="<<p<<'\n';
#endif

	cout<<"Write new file...";
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
	cout<<"complete\n";




// -------------0-0-0-0-0--0-0-0-0-0-00-0--0-0-0-0-0-0
	cout<<"\n \nStart decode\n";
	ifstream file_inn("new_file.bmp",std::ios::binary);
	file_inn.read(reinterpret_cast<char*>(&header),sizeof(file_header));
	file_inn.read(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
	file_inn.read(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
	file_inn.seekg(header.bm_offset,std::ios::beg);

	mas.clear();
	cout<<"Read file...";
	rs=0; bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
	do{
		mas.push_back(vector<unsigned char>(bpr,0));
		file_inn.read(reinterpret_cast<char*>(mas.back().data()),bpr);
		rs=file_inn.gcount();			
	} while(rs>0);
	file_inn.close();
	cout<<"complete\n";
	mas.pop_back();

	cout<<"try find count of pairs...";
	sum=0;
	for(int i=0, temp=0;i<8;++i){
		temp=(mas[0][i]&1);
		if(temp==0) continue;
		sum=sum|(temp<<i);
	}
	if(sum==0){
		cout<<"no pairs\n";
		return 4;
	}
	cout<<"complete\n";

#if defined(DEBUG)
	cout<<"find="<<sum<<'\n';
#endif

	cout<<"try find (P,Z)...";
	vector<int> pairs;
	for(int i=0, temp=0, pp=8;i<sum*2;++i){
		pairs.push_back(0);
		temp=0;
		for(int j=pp;j<pp+8;++j){
			temp=(mas[0][j]&1);
			if(temp==0) continue;
			pairs.back()=pairs.back()|(temp<<(j%8));
		}
		pp+=8;
	}
	if(pairs.size()==0){
		cout<<"no pairs\n";
		return 4;
	}
	cout<<"complete\n";

#if defined(DEBUG)
	for(int i=0;i<pairs.size();i+=2) cout<<pairs[i]<<','<<pairs[i+1]<<' ';
	cout<<'\n';
#endif

	cout<<"Start read info...";
	int lsb_flag=0, map_flag=0, sz=pairs.size();
	text.clear();
	lsb_save.clear();
	for(int i=0;i<sz;i+=2){
		ReadInfo(pairs[i],pairs[i+1],lsb_flag,map_flag,mas,v3header,text,lsb_save);
#if defined(DEBUG)
		cout<<"||lsbflag="<<lsb_flag<<", map_flag="<<map_flag<<'\n';
		cout<<"||textsize="<<text.size()<<", lsbsize="<<lsb_save.size()<<'\n';
#endif
		map_flag=0;
	} 
	for(int i=0;i<lsb_save.size();++i) mas[0][i]=((mas[0][i]>>1)<<1)|(lsb_save[i]&1);
	cout<<"complete\n";

#if defined(DEBUG)
	cout<<"lsbflag="<<lsb_flag<<", map_flag="<<map_flag<<'\n';
	cout<<"text size="<<text.size()<<'\n';
#endif

	cout<<"Write decode info...";
	ofstream tx("decode.txt",std::ios::binary);
	if(!tx){
		cout<<"error create txt\n";
		return 4;
	}
	sz=text.size();
	for(int i=0;i<sz;++i) tx.put((char)text[i]);
	tx.close();
	cout<<"complete\n";

	cout<<"Create original image...";
	std::ofstream file_outt("orig_file.bmp",std::ios::binary);
	if(!file_outt){
		cout<<"error out file";
		return 4;
	}
	file_outt.write(reinterpret_cast<char*>(&header),sizeof(file_header));
	file_outt.write(reinterpret_cast<char*>(&v3header),sizeof(v3_header));
	file_outt.write(reinterpret_cast<char*>(color_table),256*sizeof(color_info));
	bpr=floor((v3header.bit_per_pixel*v3header.width+31)/32)*4;
	for(int i=0;i<mas.size();i++){
		file_outt.write(reinterpret_cast<char*>(mas[i].data()),bpr);
	}
	file_outt.close();
	cout<<"complete\n";

	return 0;
}