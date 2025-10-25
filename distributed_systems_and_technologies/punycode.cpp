#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>


int base=36, t_min=1, t_max=26, skew=38, damp=700, bias=72, n=0x80;
int count_symbols=0, turn=0, first_flg=0;
std::vector<char> alp={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
						't','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9'};


std::vector<int> calculateT(){
	std::vector<int> res;
	int count_eq=0, j=0, temp=0, siz=0;
	while(1){
		temp=(base*(j+1))-bias;
		if(temp<t_min) temp=t_min;
		else if(temp>t_max) temp=t_max;
		res.push_back(temp);
		++siz;
		++j;
		if(siz>1 && res[siz-1]==res[siz-2]) count_eq+=1;
		else count_eq=0;
		if(count_eq==6) break;
	}
	return res;
}

std::vector<int> calculateW(std::vector<int> t){
	std::vector<int> res{1};
	int t_size=t.size();
	for(int i=1;i<t_size;++i){
		res.push_back(res[i-1]*(base-t[i-1]));
	}
	return res;
}

int calculateBias(int _delta){
	if(first_flg==0) _delta=std::trunc(_delta/(float)damp);
	else _delta=std::trunc(_delta/(float)2);
	_delta=_delta+std::trunc(_delta/(float)count_symbols);
	int k=0;
	while(_delta>std::trunc(((base-t_min)*t_max)/(float)2)){
		++k;
		_delta=_delta/std::trunc(base-t_min);
	}
	bias=(base*k)+std::trunc(((base-t_min+1)*_delta)/(float)(_delta+skew));
	return bias;
}


int main(){
	// --------------------------------------------------------
	char32_t *input_string=U"ууpааcннeгг";
	int input_string_size=11;
	// --------------------------------------------------------

	std::cout<<"Input str: ";
	for(int i=0;i<input_string_size;i++) std::cout<<(char)input_string[i];
	std::cout<<'\n';

	std::string encode="xn--";
	std::vector<int> symbols;
	std::vector<char32_t> curent_str;

	// collect encode symbols
	for(int i=0;i<input_string_size;++i){
		if((input_string[i]>='a' && input_string[i]<='z') || input_string[i]=='-'){
			encode+=input_string[i];
			curent_str.push_back(input_string[i]);
			count_symbols+=1;
			continue;
		}
		symbols.push_back(i);
	}
	encode+='-';

	// sort on unicode values
	std::sort(symbols.begin(),symbols.end(),[&input_string](const int& x, const int& y){ return input_string[x]<input_string[y]; });

	// encode
	int temp_size=symbols.size(), temp_save_value=0;
	std::vector<int> t, w;
	int dist_symbol=0, dist_end=0, count_pass=0, code_dist=0, t_size=0, sizz=curent_str.size();
	turn=sizz+1;
	for(int i=0;i<temp_size;++i, ++turn){
		// symbol in 10 system
		std::cout<<"\nSymb="<<input_string[symbols[i]]<<'\n';
		std::cout<<"Curent str: ";
		for(int i=0;i<sizz;++i) std::cout<<(char)curent_str[i];
		std::cout<<'\n';

		dist_symbol=(int)input_string[symbols[i]]-(int)n;
		if(n==0x80) dist_end=0;
		else{
			// try find distance to end
			int pos=0;
			for(int j=sizz-1;j>=0;--j){
				if(curent_str[j]!=n) continue;
				pos=j;
				break;
			}
			dist_end=sizz-pos;
			dist_symbol-=1;
		}
		count_pass=0;
		// try find position where need set symbol
		for(int j=0;j<sizz;++j){
			int before=0;
			// check what first our encode symbol or symbol in curent encode str
			for(int k=0;k<input_string_size;++k){
				if(input_string[k]==curent_str[j]){
					before=1;
					break;
				}
				if(input_string[k]==input_string[symbols[i]]) break;
			}
			if(before==1) count_pass+=1;
			else break;
		}
		// calculate code value
		code_dist=turn*dist_symbol+dist_end+count_pass;
		// save to calculate bias
		temp_save_value=code_dist;
		std::cout<<"dist_symbol="<<dist_symbol<<", dist_end="<<dist_end<<", count_pas="<<count_pass<<", turn="<<turn<<'\n';
		std::cout<<"Code dist="<<code_dist<<'\n';

		t=calculateT();
		t_size=t.size();
		// encode our symbol
		for(int j=0;j<t_size;){
			std::cout<<code_dist;

			if(code_dist<t[j]){
				std::cout<<'<'<<t[j]<<'=';
				encode+=alp[code_dist];
				std::cout<<"\'"<<encode.back()<<"\' ";
				break;
			}

			std::cout<<'>'<<t[j]<<'=';
			std::cout<<(t[j]+((code_dist-t[j])%(base-t[j])));
			encode+=alp[(t[j]+((code_dist-t[j])%(base-t[j])))];
			std::cout<<"=\'"<<encode.back()<<"\' \n";
			code_dist=std::trunc((code_dist-t[j])/(float)(base-t[j]));

			if(j>0 && j==(t_size-1) && t[j]==t[j-1]) continue;
			j+=1;
		}
		std::cout<<'\n';

		++sizz;
		++count_symbols;
		bias=calculateBias(temp_save_value);
		std::cout<<"new bias="<<bias<<'\n';

		n=(int)input_string[symbols[i]];
		curent_str.insert(curent_str.begin()+count_pass,input_string[symbols[i]]);
		first_flg+=1;
	}

	std::cout<<"\n---------Result encode----------\n";
	std::cout<<encode<<'\n';
	std::cout<<"--------------------------------\n";

	// clear states to decode
	curent_str.clear();
	bias=72;
	n=0x80;

	int pos=0;
	temp_size=encode.length();
	// find last -
	for(int i=temp_size-1;i>3;--i){
		if(encode[i]!='-') continue;
		pos=i;
		break;
	}
	// collect all before last -
	// 4 becouse xn--
	for(int i=4;i<pos;++i) curent_str.push_back(encode[i]);
	count_symbols=curent_str.size();
	
	std::cout<<"Curent str: ";
	for(const char32_t& x:curent_str) std::cout<<(char)x;
	std::cout<<'\n';
	
	int steps=0, insert_pos=0, jj=0, i_t=0, i_w=0, symb_val=0;
	first_flg=0;
	for(int i=pos+1;i<temp_size;){
		i_t=0;
		i_w=0;
		code_dist=0;

		t=calculateT();
		std::cout<<"\nt: ";
		for(const int& x:t) std::cout<<x<<' ';
		std::cout<<'\n';
		t_size=t.size();
		w=calculateW(t);
		std::cout<<"w: ";
		for(const int& x:w) std::cout<<x<<' ';
		std::cout<<'\n';

		// collect symbols while value symbol < t
		for(jj=i;jj<temp_size;){
			if(encode[jj]>='a' && encode[jj]<='z') symb_val=(encode[jj]-'a');
			else symb_val=(26+(encode[jj]-'0'));
			code_dist+=symb_val*w[i_w];
			std::cout<<"\'"<<encode[jj]<<"\'="<<symb_val<<", ";

			jj+=1;
			if(symb_val<t[i_t]) break;

			i_w+=1;
			if(i_t>0 && i_t==(t_size-1) && t[i_t]==t[i_t-1]) continue;
			i_t+=1;
		}

		// calculate decode symbol and insert position
		steps=code_dist;
		if(first_flg!=0) steps+=1;
		n+=std::floor((insert_pos+steps)/(float)(curent_str.size()+1));
		insert_pos=std::floor((insert_pos+steps)%(curent_str.size()+1));
		std::cout<<"Value="<<steps<<", n="<<n<<", pos="<<insert_pos<<'\n';

		curent_str.insert(curent_str.begin()+insert_pos,n);
		std::cout<<"Curent str: ";
		for(const char32_t& x:curent_str) std::cout<<(char)x;
		std::cout<<'\n';

		i=jj;
		count_symbols=curent_str.size();
		bias=calculateBias(code_dist);
		std::cout<<"new bias="<<bias<<'\n';
		first_flg+=1;
	}

	std::cout<<"\n---------Result decode----------\n";
	for(const char32_t& x:curent_str) std::cout<<(char)x;
	std::cout<<"\n--------------------------------\n \n";

	std::cout<<"Start str: ";
	for(int i=0;i<input_string_size;i++) std::cout<<(char)input_string[i];
	std::cout<<'\n';
	std::cout<<"Encode str: "<<encode<<'\n';
	std::cout<<"Decode str: ";
	for(const char32_t& x:curent_str) std::cout<<(char)x;
	std::cout<<'\n';

	return 0;
}