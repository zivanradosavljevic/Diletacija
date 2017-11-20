#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
using namespace std;

bool saturacija(vector<float> U);
vector<float> digFilt(vector<float> U);
vector<float> elmSpajk(vector<float> kanal);
float bazna(vector<float> U);
void nagib(vector<float> U, vector<float> t, float *a, float *b);

int main () {
      ifstream my ("podaci.dat",  ios::binary | ios::in);       //Otvaranje faja
  
  
    if (my.is_open())	cout << "Fajl je otvoren \n";           //Provera fajla
    else cout << "Otvaranje fajla nije uspelo";
    
    
   
    my.seekg (0, my.end);
    int siz =   my.tellg();
    my.seekg (0, my.beg);
    
    float f2= 0.0;          //__________promenljiva za citanje iz fajla
    char buffer[4];         //__________?
    char head[24];          //__________?
    //char * poz = buffer + 1031*4;
    vector<float> time;
    vector<float> chU1; 
    vector<float> chU2;
    //vector<float> dil;
    unsigned short nap;
    int n=0;
    siz/=(2056*4);
    
    ofstream dil("diletacija.csv");
    
    
    for(int i=0; i<siz; i++){     //glavna petlja
        
        
        my.seekg(i*2056*4+24);    //pomera na deo fajla koji se cita 
        
        chU1.clear();
        chU2.clear();
        
        for(int i=0; i<1024; i++){  //ucitavanje vremena 
            
            my.read((char *)&f2,sizeof(float));
            time.push_back(f2);
            //cout << time[i]<< "\t" <<  i+1   << endl ;
                      
        }
        
        my.read(buffer, 4);
        
        //cout << buffer << endl;
        
        
        for(int i=0; i<1024; i++){      //citanje napona sa kanala 2
            
            my.read((char *)&nap,sizeof(short));
            chU1.push_back((nap * 1.0 / 65535)-0.5);
            //cout << chU1[i] << "\t" <<  i+1   << endl ;
           //        poz += 4;
            
        }   
        //digFilt(chU1);
        my.read(buffer, 4);
        
        //cout << buffer << endl;
        
       //vector<float> chU1F = digFilt(chU1);
        
        for(int i=0; i<1024; i++){      //citanje napona sa kanala 3
            my.read((char *)&nap,sizeof(short));
            chU2.push_back((nap * 1.0 / 65535)-0.5);
            //cout << chU2[i]  << "\t" <<  i+1   << endl ;
           //        poz += 4;
        
        }  
        
       //vector<float> chU2F = digFilt(chU2);
        
     //   dil = pocetakPika(chU1, time);
        
        if (saturacija(chU1) || saturacija(chU2)) continue;     //provara da li u nekom od kanala ima saturacije


        vector<float> chU1F = digFilt(chU1);    //filtriranje podataka digitalnom LP metodom
        vector<float> chU2F = digFilt(chU2);

        vector<float> chU1Fe = elmSpajk(chU1F); //eliminacija spajkova iz kanala
        vector<float> chU2Fe = elmSpajk(chU2F); 
        
        float a = .0;
        float b = .0;
        
        nagib(chU1Fe, time, &a, &b);            //  fit pocetka pika
        float del =(bazna(chU1Fe)-a)/b;         //
                                                //
        nagib(chU2Fe, time, &a, &b);            //
        del = del - (bazna(chU2Fe)-a)/b;        //odredjivanje diletacije
        
        /*n++;
        std::stringstream ss;
        ss<<"Output"<< n <<".csv";
               
      
        ofstream dilet;                         //.csv fajl sa vremenskim diletacijama
        dilet.open (ss.str().c_str());
        dilet << "vreme, kanal2, kanal3, bazna, eliminisani spajkovi, nagib,presek   \n";
        for(int i=0; i < 1024; i++)    dilet << time[i] <<","<< chU1[i] << "," << chU2[i] << "," << bazna(chU1)<<","<< chU1Fe[i] << "," << b * time[i] + a <<","<< chU2Fe[i]     << ",\n";
            cout << a << "," << b << "," << "\n";
        dilet.close();*/
        dil<< del<< endl;       // upisivanje diletacije u fajl
       // chU1.clear();
        //chU2.clear();
        
        
        
//___ obrisi vektore        
        
        
        
        
        n++;
        if(n>siz/100) {
        cout<< i*100/siz<<"%"<< endl;
        n=0;
        }
         
        }
    
    my.close();       //zatvaranje fajlova
                      //
    dil.close();      //
    
    //cout<<siz/(2056*4)<<endl;
    
    
    return 0;
}


      //_______________________________funkcije 

bool saturacija(vector<float> U) { // ______ program za saturaciju 
    
    float max = 0.0;
    int s=0;
    for(int i=0; i<1024; i++)   if(U[i]<max) max = U[i];
    
    for(int i=0; i<1024; i++)   if(U[i]==max) s++;
    
    return s>2;
    
   }
   

vector<float> digFilt(vector<float> U){

    vector<float> filt;
    filt.push_back(U[0]);
    filt.push_back(U[1]);
    for(int i = 2; i < 1024; i++)  filt.push_back (0.20657*U[i] + 0.41314*U[i-1] + 0.20657*U[i-2] + 0.36953*U[i-1] - 0.19582*U[i-2]);
        
    return filt;
          
        
        
    }



vector<float> elmSpajk(vector<float> kanal){ //_____eliminacija spajkova
    
    vector<float> spajk;
    for(int i = 0; i < 30; i++)   spajk.push_back((kanal[51+i] + kanal[52+i] + kanal[53+i] + kanal[54+i] + kanal[55+i] + kanal[56+i] + kanal[57+i] + kanal[58+i]) / 8);
    
    for(int i = 30; i < 1000; i++)  spajk.push_back((kanal[i-6] + kanal[i-5] + kanal[i-4] + kanal[i-3] + kanal[i-2] + kanal[i-1] + kanal[i] + kanal[i+1] + kanal[i+2] +  kanal[i+3] + kanal[i+4] + kanal[i+5] + kanal[i+6]) / 13);
    
    for(int i = 1000; i < 1024; i++)    spajk.push_back((kanal[851] + kanal[852] + kanal[853] + kanal[854] + kanal[855] + kanal[856] + kanal[857] + kanal[858]) / 8);
    
    return spajk;
      
    }
    
    
float bazna(vector<float> U){
    
    float max=1.0;
    float min = -1.0;
    float s=0.0;
    float r=0.0;
    int br=0;
    //vector<unsigned int> Ub;
    for(int i=0; i<1024; i++)   if(U[i]<max) max = U[i];
    
    for(int i=0; i<1024; i++)   if(U[i]>min) min = U[i];
    s = max - (max - min) * 0.3;
    
    for(int i=0; i<1024; i++)   if(U[i] > s){
     r += U[i];
     br++;
        }
    return r/br;
    
    
    }    
   
void nagib(vector<float> U, vector<float> t, float *a, float *b) {// fitovanje linije za pocetak pika
     
    float max = .5;
    float min = -.5;
    int ind = 0;
    int br = 0;
        
    for(int i=0; i<1024; i++)   if(U[i]<max) {
    max = U[i];
    ind = i;

    }
    
    for(int i=0; i<1024; i++)   if(U[i]>min) min = U[i];  //nije potrebno ako imas baznu 
    
    
    max = max + (min - max)/5;
    min = min - (min - max)/4;
        
    float    Sx= .0;
    float   Sy= .0;
    float    Sxx = .0;
    float   Sxy = .0;
    float   del = .0;
     
    for(int i=ind; U[i]<min; i--){
        if(U[i]<max) continue;
        
        Sx += t[i];
        Sxx += t[i]*t[i];
        Sy += U[i];
        Sxy += U[i]*t[i];
        br++;
        }
        
    *a = ((Sxx * Sy) - (Sx * Sxy)) /( (br * Sxx) - (Sx * Sx));
    *b = (br * Sxy - Sx * Sy) /( (br * Sxx) - (Sx * Sx));
    
    
    }
    



