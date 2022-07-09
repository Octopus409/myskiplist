#include <string>
#include <memory.h>
#include <string.h>
#include <iomanip>
#include <fstream>
#include <vector>

template<typename T>
class myless{
public:
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

template<typename K,typename V>
class Node{
public:
    Node(K key,V value,int level);
    ~Node();
    
    inline void set_value(V value){
        this->_value = value;
    }

    inline const K& get_key() const {
        return _key;
    }

    inline const V& get_value() const{
        return _value;
    }

    int node_level;
    Node<K,V>** forward;

private:
    K _key;
    V _value;
};


template<typename K,typename V>
Node<K,V>::Node(K key,V value,int level):_key(key),_value(value),node_level(level){
    this->forward = new Node<K,V>*[level+1];
    // forward数组初始化为nullptr
    for(int i=0; i<=level; ++i){
        this->forward[i] = nullptr;
    }
}


// 析构，主要是释放forward
template<typename K,typename V>
Node<K,V>::~Node(){
    delete[] forward;
}


template<typename K,typename V, typename CMP=myless<K>>
class Skiplist{
public:


    Skiplist(int max_level = 5);
    ~Skiplist();
    inline long long unsigned size() const{
        return _ele_num;
    }

    // 插入元素
    bool insert(K key,V value);

    // 删除元素
    bool erase(K key);

    // 删除范围内的元素
    bool erase(K begin,K end);

    // 查询元素
    Node<K,V>* find(K key);

    //范围查询，找到第一个满足的元素，返回指针。否则返回null
    Node<K,V>* find(K begin, K end);

    // 展示元素
    void display();

    // 保存到磁盘
    void dump_file(std::string path="");

    // 保存到磁盘，二进制
    void dump_file_binary(std::string path="");

    // 从磁盘读取文件
    bool load_file(std::string path="");

    // 清空跳表，除了头
    void clear();

    // 重构跳表，改变max_level
    void rebuild(int newlevel);

private:

    // 读取文件的辅助函数
    void get_data_from_line(const std::string& line,std::string& key,std::string& value,std::string& node_level);

    // 创建节点
    Node<K,V>* create_node(K key,V value,int level);

    // 寻找函数的辅助函数
    Node<K,V>* _find(K key);

    // 扩容或者压缩头节点的forward数组
    void extend_header(int oldlevel, int newlevel);

    //获得一个[0,_max_level]的数
    int get_random_level();

    // 展示函数的辅助函数
    void display_the_header();

    //最大级
    int _max_level;

    // 当前级
    int _current_level;

    // 当前高度的理想最大容纳数量
    int _max_num;

    //元素数量
    int _ele_num;

    // 表的默认保存路径
    std::string default_path = "source/table.txt";

    //头节点
    Node<K,V>* _header;

    //比较的仿函数
    CMP cmp_func;
};

template<typename K,typename V,typename CMP>
Node<K,V>* Skiplist<K,V,CMP>::create_node(K key,V value,int level){
    Node<K,V>* ret = new Node<K,V>(key,value,level);
    return ret;
}

template<typename K,typename V,typename CMP>
bool Skiplist<K,V,CMP>::insert(K key,V value){   

    Node<K,V>* current = _header;
    //更新数组
    Node<K,V>* update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    for(int i=_current_level; i>=0; --i){

        while(current->forward[i]!=nullptr && (cmp_func(current->forward[i]->get_key(),key)) ){
            current = current->forward[i];
        }
        update[i] = current;

    }
    
    current = current->forward[0];

    // current非空，且current->get_key()!=key的情况下。
    if(current!=nullptr && (!cmp_func(current->get_key(),key) && !cmp_func(key,current->get_key()))){
        // std::cout <<  key << " already exist." <<  std::endl;
        return false;
    }

    // current空，或者满足cmp_func的情况下。    
    if(current==NULL || (cmp_func(key,current->get_key()))){
        // 尾插和正常插，放在一起

        int random_level = get_random_level();

        Node<K,V>* newnode = create_node(key,value,random_level);
        // 前面是从高度_current_level出发的，这里要补全，update矩阵
        if(random_level>_current_level){
            for(int i=random_level; i>_current_level; --i){
                update[i] = _header;
            }
            _current_level = random_level;
        }

        for(int i=0; i<=random_level; ++i){
            newnode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newnode;
        }
        // std::cout << "Succesfully insert key: " << key << " value: "<< value <<  std::endl;
        _ele_num++;

        // 判断是否需要对高度扩容
        
    }

    // 扩容
    
    if(_ele_num>_max_num){
        int newlevel = ((_max_level+3)>=28) ? 28 : _max_level+3;
        if(newlevel==_max_level) return true;

        //下面正式扩容
        _max_num = _max_num * (1<<(newlevel-_max_level));
        rebuild(newlevel);
            
    }
    

    return true;

}


// 删除元素 log(n)
template<typename K,typename V,typename CMP>
bool Skiplist<K,V,CMP>::erase(K key){

    Node<K,V>* update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));
    Node<K,V>* current = this->_header;


    // 找到第一个大于等于key的ele
    for(int i=_current_level; i>=0; --i){
        while(current->forward[i]!=nullptr && (cmp_func(current->forward[i]->get_key(),key)) ){
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if(current==nullptr ||  (cmp_func(current->get_key(),key) || cmp_func(key,current->get_key()))){
        // 不存在该元素，current!=key
        std::cout << "Ele Key=" << key << "doesn't exist" << std::endl;
        return false;
    }

    if(!cmp_func(current->get_key(),key) && !cmp_func(key,current->get_key())){
        // current == key
        for(int i=current->node_level; i>=0; --i){
            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        _ele_num--;
        std::cout << "Deleted ele Key=" << key << std::endl;
        
    }

    return true;

}

// 范围删除 log(n) + n
template<typename K,typename V,typename CMP>
bool Skiplist<K,V,CMP>::erase(K begin, K end){

    Node<K,V>* update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));
    
    // 找到第一个大于等于key的ele
    Node<K,V>* current = this->_header;
    for(int i=_current_level; i>=0; --i){
        while(current->forward[i]!=nullptr && (cmp_func(current->forward[i]->get_key(),begin)) ){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    int sum = 0; // 统计删除了几个

    // 由于不能从后往前遍历，只能一个一个删除
    if(current==nullptr ||  cmp_func(end,current->get_key())){
        // 范围内没有元素
        std::cout << "Element with Key between " << begin << " and " << end << " doesn't exist" << std::endl;
        return false;
    }

    // 小于或者和end相等
    Node<K,V>* nexttmp = nullptr;
    while(cmp_func(current->get_key(),end) || (!cmp_func(current->get_key(),end) && !cmp_func(end,current->get_key()))){
        // current == key
        for(int i=current->node_level; i>=0; --i){
            update[i]->forward[i] = current->forward[i];
        }
        nexttmp = current->forward[0];
        delete current;
        current = nexttmp;
        sum++;
        _ele_num--;
        if(!current) break;
    }

    std::cout << "Delete " << sum << " element totally between "<< begin << " and " << end << std::endl;
    // 拼接update和last后面
    

}

//根据新的最大高度重构跳表
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::rebuild(int level){

    int old_max_level = _max_level;
    int new_max_level = level;
    _max_level = new_max_level;  // 这里更新了最大高度
    _current_level = _max_level;   // 更新整个跳表的当前最大高度。偷懒行为，这里为了不破坏原来代码。

    // 先更新头节点，注意newlevel小于oldlevel的情况
    extend_header(old_max_level, new_max_level);

    std::vector<Node<K,V>*> update(new_max_level+1,_header);

    Node<K,V>* current = this->_header->forward[0];
    

    for(int i=0; i<_ele_num; ++i){

        int newlevel = get_random_level();
        Node<K,V>** newforward = new Node<K,V>*[newlevel+1];
        memset(newforward, 0, sizeof(Node<K, V>*)*(newlevel + 1));
		// 初始化newforward数组，这里很关键，先把后面接起来。防止错误后，数据丢失。
		// 确实关键，这个bug找了三四个小时。 不能直接接在一起！！！！！！！！！
		// 直接接，可能后一个节点的高度不够高，然后insert的时候，会导致forward数组越界访问。
		newforward[0] = current->forward[0];

        delete[] current->forward;
        current->forward = newforward;
        current->node_level = newlevel;   // 更新节点的高度

        for(int j = 0; j<=newlevel; ++j){
            // 连接，更新
            update[j]->forward[j] = current;
            update[j] = current;
        }

        current = current->forward[0];
    }

    std::cout << "Rebuild successfully. Now the max level is " << _max_level << std::endl;
    return;

}


// 寻找函数的辅助函数
template<typename K,typename V,typename CMP>
Node<K,V>* Skiplist<K,V,CMP>::_find(K key){

    Node<K,V>* current = this->_header;
    // 找到第一个大于等于key的ele
    for(int i=_current_level; i>=0; --i){
        while(current->forward[i]!=nullptr && (cmp_func(current->forward[i]->get_key(),key)) ){
            current = current->forward[i];
        }
    }

    current = current->forward[0];
    return current;

}

// 寻找元素，找到就返回指针，否则返回nullptr
template<typename K,typename V,typename CMP>
Node<K,V>* Skiplist<K,V,CMP>::find(K key){

    Node<K,V>* ret = _find(key);
    if(ret==nullptr || cmp_func(ret->get_key(),key) || cmp_func(key,ret->get_key())) 
    std::cout << "Couldn't found the element Key=" << key << std::endl;
    else 
    std::cout << "Found the element Key=" << ret->get_key() << " ,value = "<< ret->get_value() << std::endl;

}


// 范围查找，找到第一个符合的
template<typename K,typename V,typename CMP>
Node<K,V>* Skiplist<K,V,CMP>::find(K begin, K end){

    if(cmp_func(end,begin)){
        return nullptr;
    }
    Node<K,V>* ret = _find(begin);

    if(ret==nullptr || cmp_func(end,ret->get_key())) return nullptr;
    
    return ret;
}


// 展示函数，要突出链表的结构。
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::display(){
    Node<K,V>* current = _header->forward[0];
    this->display_the_header();

    for(int i=0; i<_ele_num; ++i){
        K key = current->get_key();
        for(int j = 0; j<=(current->node_level); ++j){
            std::cout << std::left;
            std::cout << std::setfill(' ');
            std::cout << std::setw(4) << key << "    ";
        }
        std::cout << std::endl;

        current = current->forward[0];
    }

    this->display_the_header();
}


// 展示函数的辅助函数， 主要画出头和尾的边界。
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::display_the_header(){

    // 一个key占四个位，两个key之间相隔四个位
    for(int i=0; i<_current_level; ++i){
        std::cout << "--------";
    }
    std::cout << "----" << std::endl;

}


// 保存到磁盘 格式: key : value : node_level ，中间用冒号隔开
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::dump_file(std::string path){

    // path是空的话，使用默认路径
    if(path.empty()){
        path = this->default_path;
    }

    std::ofstream ofs;
    ofs.open(path,std::ios::out);

    if(!ofs.is_open()){
        // 打开文件失败
        std::cout << "The file " << path << " doesn't exist" << std::endl;
        return;
    } 

    Node<K,V>* current = _header->forward[0];

    for(int i=0; i<_ele_num; ++i){
        ofs << current->get_key() << ":" << current->get_value() << ":" << current->node_level << std::endl;
        current = current->forward[0];
    }

    ofs.close();

}


// 二进制保存到磁盘
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::dump_file_binary(std::string path){
    
    if(path.empty()){
        path = this->default_path;
    }

    std::ofstream ofs;
    ofs.open(path,std::ios::out | std::ios::binary);

    if(!ofs.is_open()){
        std::cout << "The file " << path << " doesn't exist" << std::endl;
        return;
    }

    Node<K,V>* current = _header->forward[0];

    // 我们不能直接把节点Node写进磁盘，Node的forward数组记录的是指针，这种信息经过读取后会失效。
    // 所以我们把K，V，node_level 写进磁盘。

    for(int i=0; i<_ele_num; ++i){
        
        ofs.write((char *)&(current->get_key()),sizeof(K));
        ofs.write((char *)&(current->get_value()),sizeof(V));
        ofs.write((char *)&(current->node_level),sizeof(int));
        current = current->forward[0];

    }

    ofs.close();

}

// 对header节点的forward数组进行修剪的函数
template<typename K, typename V, typename CMP>
void Skiplist<K,V,CMP>::extend_header(int oldlevel, int newlevel){
    // 这个函数不确保安全，错误使用会导致高级的索引丢失。仅是为了缩减代码量。
    Node<K,V>** tmp_forward = new Node<K,V>*[newlevel+1];
    memset(tmp_forward, 0, sizeof(Node<K, V>*)*(newlevel + 1));
	tmp_forward[0] = _header->forward[0];
    delete[] _header->forward;
    _header->forward = tmp_forward;
}


// 普通版本的load_file，是二进制的读
template<typename K, typename V, typename CMP>
bool Skiplist<K,V,CMP>::load_file(std::string path){

    using std::string;

    if(path.empty()){
        // 文件为空
        path = this->default_path;
    }

    std::ifstream ifs;

    ifs.open(path,std::ios::in | std::ios::binary);
    if(!ifs.is_open()){
        // 打开文件失败
        std::cout << "The file " << path << " doesn't exist" << std::endl;
        return false;
    }

    // 读取文件前，先清空内存中的链表
    this->clear();

    std::cout << "Load file " << path  << "....."<< std::endl;

    K* curKey = new K;
    V* curValue = new V;
    int cur_level;

    // 将_header->forward扩展成32层的，然后再修剪。_now_max是记录节点的动态记录节点的当前最大高度。
    extend_header(0,28);
    int _now_max = 0;

    std::vector<Node<K,V>*> update(32+1,_header);  // 由于不知道最大level，默认是32先。后面再修改。

    // sum统计有多个元素被成功读入
    while(true){
        if(!ifs.read((char*)curKey,sizeof(K))) break;
        if(!ifs.read((char*)curValue,sizeof(V))) break;
        if(!ifs.read((char*)&cur_level,sizeof(int))) break;
        _now_max = (_now_max > cur_level) ? _now_max : cur_level;

        Node<K,V>* newnode = new Node<K,V>(*curKey,*curValue,cur_level);
        for(int i=0; i<=cur_level; ++i){
            update[i]->forward[i] = newnode;
            update[i] = newnode;
        }

        ++this->_ele_num;
    }

    extend_header(28,_now_max);  // 修剪到最大高度
    _max_level = _now_max;  // 更新最大高度

    std::cout << "Loading successfully. Totally "<< this->_ele_num << " element, and max level is " << _max_level << std::endl;
    ifs.close();

    // 这里出现过一个错误，尝试读写string，这是不行的，因为string的真正数据开辟在堆区。强行二进制读写，记录的只会是迭代器。
    delete curKey;
    delete curValue;

    return true;

}


// 从磁盘读取文件，string的偏特化版本
template<>
bool Skiplist<std::string,std::string,myless<std::string>>::load_file(std::string path){

    using std::string;

    if(path.empty()){
        // 文件为空
        path = this->default_path;
    }

    std::ifstream ifs;

    ifs.open(path,std::ios::in);
    if(!ifs.is_open()){
        // 打开文件失败
        std::cout << "The file " << path << " doesn't exist" << std::endl;
        return false;
    }

    // 读取文件前，先清空内存中的链表
    this->clear();

    extend_header(0,28);
    int _now_max = 0;

    std::cout << "Load file " << path  << "....."<< std::endl;
    string line="";
    string key="";
    string value="";
    string node_level = "";
    std::vector<Node<string,string>*> update(32+1,_header);
    while(getline(ifs,line)){
        get_data_from_line(line,key,value,node_level);
        if(key.empty() || value.empty() || node_level.empty()){
            // 这一行解析错误
            continue;
        }
        // 默认K,V类型有string参数类型的构造函数，否则报错
        int level = stoi(node_level);
        _now_max = (_now_max > level) ? _now_max : level;
        Node<string,string>* newnode = create_node(key,value,level);
        
        // 连接newnode 并更新update数组
        for(int i=0; i<=level; ++i){
            update[i]->forward[i] = newnode;
            update[i] = newnode;
        }

        this->_ele_num++;

    }

    extend_header(28,_now_max);
    _max_level = _now_max;

    std::cout << "Loading successfully." << std::endl;
    ifs.close();

    return true;
    
}



// 从磁盘读取文件，int的偏特化版本
template<>
bool Skiplist<int,int,myless<int>>::load_file(std::string path){

    using std::string;

    if(path.empty()){
        // 文件为空
        path = this->default_path;
    }

    std::ifstream ifs;

    ifs.open(path,std::ios::in);
    if(!ifs.is_open()){
        // 打开文件失败
        std::cout << "The file " << path << " doesn't exist" << std::endl;
        return false;
    }

    // 读取文件前，先清空内存中的链表
    this->clear();

    extend_header(0,28);
    int _now_max = 0;

    std::cout << "Load file " << path  << "....."<< std::endl;
    string line="";
    string key="";
    string value="";
    string node_level = "";
    std::vector<Node<int,int>*> update(32+1,_header);
    while(getline(ifs,line)){
        get_data_from_line(line,key,value,node_level);
        if(key.empty() || value.empty() || node_level.empty()){
            // 这一行解析错误
            continue;
        }
        // 默认K,V类型有string参数类型的构造函数，否则报错
        int level = stoi(node_level);
        int _key = stoi(key);
        int _value = stoi(value);
        Node<int,int>* newnode = create_node(_key,_value,level);
        
        _now_max = (_now_max > level) ? _now_max : level;

        // 连接newnode 并更新update数组
        for(int i=0; i<=level; ++i){
            update[i]->forward[i] = newnode;
            update[i] = newnode;
        }

        this->_ele_num++;

    }

    extend_header(28,_now_max);
    _max_level = _now_max;

    std::cout << "Loading successfully." << std::endl;
    return true;

    ifs.close();
}

template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::get_data_from_line(const std::string& line,std::string& key,std::string& value,std::string& node_level){

    if(line.empty()){
        key.clear();
        value.clear();
        node_level.clear();
        return;
    }

    int first_pos = 0, second_pos = 0;
    first_pos = line.find(':');
    if(first_pos==std::string::npos || first_pos==line.size()-1){
        // 找不到第一个:，或者已经是末尾
        key.clear();
        value.clear();
        node_level.clear();
        return;
    }
    second_pos = line.find(':',first_pos+1);
    if(second_pos==std::string::npos || second_pos==first_pos+1 || second_pos==line.size()-1){
        //找不到第二个，或者两者之间没有数据，再或者second_pos已经是末尾
        key.clear();
        value.clear();
        node_level.clear();
        return;
    }

    key = line.substr(0,first_pos);
    value = line.substr(first_pos+1,second_pos-first_pos-1);
    node_level = line.substr(second_pos+1,line.size()-1-second_pos);

    return;

}



template<typename K,typename V,typename CMP>
int Skiplist<K,V,CMP>::get_random_level(){
    int ret = 0;

    // 根据时间给一个随机种子。
    // 这里巧妙地用while，改变了不同level的概率。0级是100%，1级是1/2,2级是1/4，以此类推
    while(rand()%2){
        ret++;
    }
    return ret >= _max_level ? _max_level : ret;
}


// 构造函数
template<typename K,typename V,typename CMP>
Skiplist<K,V,CMP>::Skiplist(int max_level):_ele_num(0),_current_level(0),cmp_func(CMP()){
    if(max_level<0 || max_level>28){
        std::cout << "Max_level cannot be minus or greater than 28" << std::endl;
        exit(0);
    }
    _max_level = max_level;

    //处理理想容纳数量
    if(_max_level<=5) _max_num = 100;
    else{
        _max_num = (1<<(_max_level-5))*100;
    }

    _header = new Node<K,V>(K(),V(),_max_level);
    // 设置随机数种子
    srand((unsigned int)time(NULL));

}


// 析构函数
template<typename K,typename V,typename CMP>
Skiplist<K,V,CMP>::~Skiplist(){

    // 析构整条链表
    this->clear();
    delete _header;

}

//析构函数的辅助函数，load_file要先释放内存中的数据，再导入。
template<typename K,typename V,typename CMP>
void Skiplist<K,V,CMP>::clear(){

    // 析构整条链表

    Node<K,V>* cur = _header->forward[0];
    Node<K,V>* next;
    while(cur){
        next = cur->forward[0];
        delete cur;
        cur = next;
    }

    this->_ele_num = 0;

}
