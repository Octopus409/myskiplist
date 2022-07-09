#include <iostream>
#include "skiplist.h"

using namespace std;

void test01(){
    Skiplist<int,int> sk(5);
    
    for(int i=0; i<1000000; ++i){
        sk.insert(i,i);
    }


    sk.find(555);
    sk.find(9999);
    sk.find(100000);
    sk.find(182394);
    sk.find(182394);
    sk.find(232323);

    //cout << sk.size() << endl;

    //sk.display();

    //sk.dump_file();

    //sk.load_file();

    //sk.rebuild(10);
    //sk.rebuild(16);
    //sk.rebuild(7);

    //sk.display();
}

void test02(){

    string a;
    Skiplist<int,double> sk(10);
    
    for(int i=0; i<10000; ++i){
        sk.insert(i,1.0+i);
    }

    sk.erase(5000,6000);

    cout << sk.size() << endl;
    sk.find(5555);

    sk.dump_file();

    sk.dump_file_binary("source/table2.txt");

    sk.load_file("source/table2.txt");

    sk.rebuild(7);

}

class A{
public:
    A():_val(0){}
    A(int a):_val(a){
    }
    A(int a,const char* st):_val(a){
        strcpy(str,st);
    }
    int _val = 3;
    char str[20];
    bool operator<(const A& a)const{
        return this->_val < a._val;
    }

    bool operator>(const A& a)const{
        return this->_val > a._val;
    }
};


void test03(){
    

    Skiplist<A,string> sk(3);
    sk.insert(A(3,"2"),"是真滴皮");
    sk.insert(A(9,"alsdkfj"),"是真滴捞");
    sk.insert(A(1),"是真滴好用");
    sk.insert(A(3),"是真滴好用");
    sk.insert(A(4),"是真滴好用");
    //sk.display();
    
    //Node<A,string>* cur = sk._header->forward[0];
    //cout << cur->get_key()._val << endl;
    //cur = cur->forward[0];
    //cout << cur->get_key()._val << endl;
    //cur = cur->forward[0];
    //cout << cur->get_key()._val << endl;
    //cur = cur->forward[0];
    //cout << cur->get_key()._val << endl;

}

void test04(){

    // 测试一下二进制写和读
    ofstream ofs;

    ofs.open("a.txt", ios::out | ios::binary);

    A a(2,"sss"),b(3,"666"),c(4,"iii");
    ofs.write((const char*)&a,sizeof(A));
    ofs.write((const char*)&b,sizeof(A));
    ofs.write((const char*)&c,sizeof(A));


    ofs.close();

    ifstream ifs;

    A d,e,f;

    ifs.open("a.txt", ios::in | ios::binary);

    ifs.read((char*)&d,sizeof(A));
    ifs.read((char*)&e,sizeof(A));
    ifs.read((char*)&f,sizeof(A));

    ifs.close();

    cout << d._val << d.str <<  " " << e._val << e.str << " " << f._val << f.str << endl;


}

int main(){

    test02();
    

    return 0;
}
