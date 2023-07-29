#include <bits/stdc++.h>
using namespace std;

long double miss_num, hit_num = 0;
string C_address,C_Tag, C_data;
int C_Index, C_wayNum, C_offset;

string convert(string inst, int mem_size, int block_size){
    if(inst=="*end:"){
        string temp;
        for(int i=0;i<(1+log2(mem_size/block_size)+log2(block_size)+32);i++)temp=temp+'1';
        return temp;
    }
    int blocks_in_mem_bits = log2(mem_size/block_size);
    string temp,b_str,c_str;
    if(inst[0]=='r'){
        temp=temp+'0';
        int ind=-1;
        for(int i=3;i<inst.size();i++){
            if(inst[i]==' '){
                ind=i; break;
            }
        }
        bitset<64> b(stoi(inst.substr(3,ind-3)));
        b_str = b.to_string();
        b_str = b_str.substr(64-blocks_in_mem_bits);
        temp=temp+b_str;

        bitset<64> c(stoi(inst.substr(ind+1)));
        c_str = c.to_string();
        c_str = c_str.substr(64-log2(block_size));
        temp=temp+c_str;
        for(int i=0;i<32;i++)temp=temp+'0';
    }else{
        temp=temp+'1';
        int ind=-1;
        for(int i=3;i<inst.size();i++){
            if(inst[i]==' '){
                ind=i; break;
            }
        }
        bitset<64> b(stoi(inst.substr(3,ind-3)));
        b_str = b.to_string();
        b_str = b_str.substr(64-blocks_in_mem_bits);
        temp=temp+b_str;

        int ind2;
        for(int i=ind+1;i<inst.size();i++){
            if(inst[i]==' '){
                ind2=i; break;
            }
        }
        bitset<64> c(stoi(inst.substr(ind+1,ind2-ind-1)));
        c_str = c.to_string();
        c_str = c_str.substr(64-log2(block_size));
        temp=temp+c_str;
        
        bitset<32> e(stoi(inst.substr(ind2+1)));
        temp=temp+e.to_string();
    }
    return temp;
}

vector<string> getInstructions(int mem_size, int block_size){
    string end="*end:";
    cin.ignore();
    vector<string> inst;
    while(1){
        string s;
        getline(cin,s);
        inst.push_back(s);
        if(s==end)break;
    }
    for(int i=0;i<inst.size();i++){
        inst[i]=convert(inst[i],mem_size,block_size); 
    }
    return inst;
}

string hextoint(string x){
    int w = stoi(x,0,16);
    return to_string(w);
}

void updateCounters(int index, int way, int mem_size, int block_size, int cache_size, int ways, vector<vector<int> > &counter){ 
    int temp=counter[index][way];
    for(int i=0;i<ways;i++){
        if(counter[index][i]>temp)counter[index][i]--;
    }
    counter[index][way] = ways-1;
}

int whichToReplace(int index,vector<vector<int> > &counter,int ways){
    for(int i=0;i<ways;i++){
        if(counter[index][i]==0){
            return i;
        }
    }
    return 0;
}

void evict(int index, int way,vector<vector<bool> > &modified, int sets, vector<vector<string> > &tag, vector<string> &Mem,vector<vector<string> > &data_arr){
    if(modified[index][way]){
        modified[index][way]=0;
        bitset<64> b_(index);
        string b_str = b_.to_string();
        string b = b_str.substr(64-log2(sets));
        int address=stoi(tag[index][way]+b,0,2);
        Mem[address]=data_arr[index][way];
    }
}

string getTag(string address, int mem_size, int sets, int block_size){
    return address.substr(0,(log2(mem_size/(sets*block_size))));
}

int getIndex(string address, int mem_size, int sets, int block_size){
    if(sets==1){
        return 0;
    }
    return stoi(address.substr(log2(mem_size/(sets*block_size)),log2(sets)), 0, 2);
}

string getOffset(string address, int mem_size, int sets, int block_size){
    return address.substr(log2(mem_size/(block_size*sets))+log2(sets), log2(block_size));
}

string readMem(string address, int mem_size, int sets, int block_size, vector<string> &Mem){
    return Mem[stoi(address.substr(0, (log2(mem_size/(block_size*sets))+log2(sets))), 0, 2)];
}

void replace(string Memdata, int way, int index, string Tag, vector<vector<string> > &data_arr, vector<vector<string> > &tag, vector<vector<bool> > &empty_bit){
    data_arr[index][way]=Memdata;
    tag[index][way]=Tag;
    empty_bit[index][way]=0;
}


int hitWay(int mem_size, int ways, int sets, int block_size,vector<vector<string> > &tag, vector<vector<bool> > &empty_bit){
    int indexing = getIndex(C_address,mem_size,sets,block_size);
    C_Index = indexing;
    string tagtomatch = getTag(C_address,mem_size,sets,block_size);
    for (int i = 0; i < ways; i++){
        if (tag[indexing][i] == tagtomatch && !empty_bit[indexing][i]){
            C_wayNum = i;
            return i;
        }
    }
    C_wayNum = -1;
    return -1;
}

int readCache(int offset,vector<vector<string> > &data_arr){
    string dataBlock = data_arr[C_Index][C_wayNum];
    string dataOffset=dataBlock.substr(8*offset, 32);
    return stoi(dataOffset, 0, 2);
}

void writeCache(string writeData, int offset, int block_size, string Tag,vector<vector<string> > &data_arr,vector<vector<bool> > &modified, vector<vector<bool> > &empty_bit, vector<vector<string> > &tag){
    string dataCur=data_arr[C_Index][C_wayNum];
    string data2;
    if(offset+4!=block_size){
        data2=dataCur.substr(0, 8*offset)+writeData+dataCur.substr(8*offset+32, block_size*8-8*offset-32);
    }
    else{
        data2=dataCur.substr(0, 8*offset)+writeData;
    }
    data_arr[C_Index][C_wayNum] = data2;
    modified[C_Index][C_wayNum] = 1;
    empty_bit[C_Index][C_wayNum] = 0;
    tag[C_Index][C_wayNum] = Tag;
}

void miss(vector<vector<bool> > &modified, int mem_size, int sets, int cache_size, int block_size, int ways, vector<vector<string> > &tag, vector<string> &Mem, vector<vector<string> > &data_arr, vector<vector<bool> > &empty_bit,  vector<vector<int> > &counter){
    evict(C_Index, C_wayNum, modified, sets, tag, Mem, data_arr);
    replace(readMem(C_address, mem_size, sets, block_size, Mem), C_wayNum, C_Index, getTag(C_address, mem_size, sets, block_size), data_arr, tag, empty_bit);
    updateCounters(C_Index, C_wayNum, mem_size, block_size, cache_size, ways, counter);
}

void initialise(int block_size, int mem_size, vector<string> &Mem, vector<vector<bool> > &empty_bit, vector<vector<string> > &data_arr){
	string temp1="";
    for(int j=0;j<(block_size*8);j++) temp1=temp1+'0';
    for(int i=0;i<(mem_size/block_size);i++){
        Mem[i]=temp1;
    }
    for(int i=0;i<empty_bit.size();i++){
        for(int j=0;j<empty_bit[i].size();j++){
            empty_bit[i][j]=1;
            data_arr[i][j] = temp1;
        }
    }
    cout<<"Initialisation Commplete!\n";
}

void parse_instruction(string s, int mem_size, int cache_size, int block_size){
	C_address=s.substr(1, (log2(mem_size/block_size)+log2(block_size)));
    C_data = s.substr((1+log2(mem_size/block_size)+log2(block_size)), 32);
    C_offset=stoi(s.substr(1+log2(mem_size/block_size), log2(block_size)), 0, 2);
}


void read_miss(int ways, int mem_size, int sets, int cache_size, int block_size, vector<vector<int> > &counter, vector<vector<bool> > &modified, vector<vector<string> > &tag, vector<string> &Mem, vector<vector<string> > &data_arr, vector<vector<bool> > &empty_bit){
	C_wayNum=whichToReplace(C_Index, counter, ways);
    miss(modified, mem_size, sets, cache_size, block_size, ways, tag, Mem, data_arr, empty_bit, counter);
    miss_num++;
}

void read_hit(int mem_size, int block_size, int cache_size, int ways, vector<vector<int> > &counter){
	updateCounters(C_Index, C_wayNum, mem_size, block_size, cache_size, ways, counter);
    hit_num++;
}


void write_miss(int ways, int mem_size, int sets, int cache_size, int block_size, vector<vector<int> > &counter, vector<vector<bool> > &modified, vector<vector<string> > &tag, vector<string> &Mem, vector<vector<string> > &data_arr, vector<vector<bool> > &empty_bit){
	C_wayNum=whichToReplace(C_Index, counter, ways);
    miss(modified, mem_size, sets, cache_size, block_size, ways, tag, Mem, data_arr, empty_bit, counter);
    writeCache(C_data, C_offset,block_size,getTag(C_address, mem_size, sets, block_size), data_arr, modified, empty_bit, tag);
    miss_num++;
}

void write_hit(int ways, int mem_size, int sets, int cache_size, int block_size, vector<vector<int> > &counter, vector<vector<bool> > &modified, vector<vector<string> > &tag, vector<string> &Mem, vector<vector<string> > &data_arr, vector<vector<bool> > &empty_bit){
    writeCache(C_data, C_offset,block_size,getTag(C_address, mem_size, sets, block_size), data_arr, modified, empty_bit, tag);
    updateCounters(C_Index, C_wayNum, mem_size, block_size, cache_size, ways, counter);
    hit_num++;
}

int main(){
    long long cache_size,block_size,ways,mem_size,sets,instruction_length,tag_length,index_length,offset_length;
    cout<<"WELCOME TO OUR CACHE\n";
    cout<<"Enter the cache size in bytes: ";
    cin>>cache_size;
    cout<<"Enter the block size in bytes: ";
    cin>>block_size;
    cout<<"Enter the number of ways in the cache: ";
    cin>>ways;
    cout<<"Enter the memory size in bytes: ";
    cin>>mem_size;
    sets = cache_size/(block_size*ways);
    vector<string> Mem(mem_size/block_size);
    vector<vector<string> > tag(sets, vector<string>(ways));
    vector<vector<bool> > empty_bit(sets, vector<bool>(ways));
    vector<vector<bool> > modified(sets, vector<bool>(ways));
    vector<vector<string> > data_arr(sets, vector<string>(ways));
    vector<vector<int> > counter(sets, vector<int>(ways));
    index_length = log2(sets);
    tag_length = log2(mem_size/block_size) - index_length;
    long long read_hit_counter=0, write_hit_counter=0;
    cout<<"INDEX: "<<index_length<<" bits\n";
    cout<<"TAG: "<<tag_length<<" bits\n";
    cout<<"SETS: "<<sets<<"\n";
    int num,spatial_l,num_instruc_shuffle;
    vector<string> vec;
    cout<<"The instruction set:\n\nExample: rd 24 4\nHere, rd -> read operation, 24 -> memory location,  4 -> block offset\n\n";
    cout<<"Example: wr 67 8 100\nHere, wr -> write operation,  67 -> memory location,  8 -> block offset,  100 -> data to be written\n\nInstructions:\n";
    vec=getInstructions(mem_size,block_size);
    initialise(block_size, mem_size, Mem, empty_bit, data_arr);
    for(int i=0;i<vec.size()-1;i++){
        parse_instruction(vec[i], mem_size, cache_size, block_size);
        int hitStatus=hitWay(mem_size, ways, sets, block_size, tag, empty_bit);
        if(vec[i][0]=='0'){
            if(hitStatus==-1){
                read_miss(ways, mem_size, sets, cache_size, block_size, counter, modified, tag, Mem, data_arr, empty_bit);
                cout<<"Miss! ";
                cout<<readCache(C_offset,data_arr)<<"\n";
            }
            else{
                read_hit_counter++;
                read_hit(mem_size, block_size, cache_size, ways, counter);
                cout<<"Hit! ";
                cout<<readCache(C_offset,data_arr)<<"\n";
            }
        }
        else{
            if(hitStatus==-1){
                write_miss(ways, mem_size, sets, cache_size, block_size, counter, modified, tag, Mem, data_arr, empty_bit);
                cout<<"Miss!\n";
            }
            else{
                write_hit_counter++;
                write_hit(ways, mem_size, sets, cache_size, block_size, counter, modified, tag, Mem, data_arr, empty_bit);
                cout<<"Hit! ";
                cout<<readCache(C_offset,data_arr)<<"\n";
            }
        }
    }
    cout<<"Write hits: "<<write_hit_counter<<"\n";
    cout<<"Read hits: "<<read_hit_counter<<"\n";
    cout<<"Hit rate: ";
    cout<<hit_num/(hit_num+miss_num)<<setprecision(12)<<"\n";
}
