#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <fstream>

using namespace std;

void list_dir(const char *path) {
    double tab[200] = {};
    int liczba = 0;
   struct dirent *entry;
   DIR *dir = opendir(path);
   
   if (dir == NULL) {
      return;
   }
   while ((entry = readdir(dir)) != NULL) {
string nazwa = entry->d_name;
       if(nazwa.size() > 5){
           liczba ++;
       ifstream input( nazwa );

        int iteracje = 0;
        string line; 

    while (getline(input, line))
    {
        double liczba2 = tab[iteracje];
        double liczba3 = stoi(line);

        double wynik = liczba2 + liczba3;

        tab[iteracje] = wynik;

        iteracje++;
    }
    input.close();

       
       }

   cout << entry->d_name << endl;
   }
   closedir(dir);


for(int i =0;i<200;i++){
            ofstream wyniki;
            wyniki.open("/home/asus/Dokumenty/NAI/Lab6/result", ios_base::app);
            wyniki << i << " " << tab[i] << endl;
            wyniki.close();
}

}
int main() {
   list_dir("/home/asus/Dokumenty/NAI/Lab6/results");
}