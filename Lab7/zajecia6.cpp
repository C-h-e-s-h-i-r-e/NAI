#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <list>
#include <chrono>
#include <fstream>
#include <cstdio>

using namespace std;
random_device r;
default_random_engine e1(r());

time_t t = time(0);
struct tm * now = localtime( & t );

string nazwa;

double fRand(double fMin, double fMax)
    {
        double f = (double)rand() / RAND_MAX;
        return fMin + f * (fMax - fMin);
    };

// random hill climbing alg
auto hill_climbing_alg_random = [](auto goal, vector<pair<double,double>> ogr_kostk,int max_iterations,  auto callback){

    auto init = [&ogr_kostk]() {
        vector<double> x(ogr_kostk.size());
        for(int i = 0; i < ogr_kostk.size(); i++){
            uniform_real_distribution<double> uniform_dist(ogr_kostk[i].first, ogr_kostk[i].second);
            x[i] = uniform_dist(e1);
        }
        return x;
    };

    auto current_solution = init();

    for(int iter = 0; iter < max_iterations; iter ++){

        auto prev_solution = current_solution;

        auto next_solution = prev_solution;

        next_solution[0] += fRand(-0.1,0.1);
        next_solution[1] += fRand(-0.1,0.1);

        if(goal(next_solution) < goal(current_solution)) {

            current_solution = next_solution;
        }
        callback(iter, current_solution);
    }
    return current_solution;
};

// minimalizacja
auto hill_climbing_alg = [](auto get_random_sol, auto get_all_neighbours, auto goal, int max_iterations, auto callback) {
    auto current_solution = get_random_sol();
    for (int iteration = 0; iteration < max_iterations; iteration++)
    {
        auto next_solutions = get_all_neighbours(current_solution);
        next_solutions.push_back(current_solution);
        current_solution = *(max_element(next_solutions.begin(), next_solutions.end(), [&goal](auto a, auto b) {
            return goal(a) > goal(b);
        }));
        callback(iteration, current_solution);
    }
    return current_solution;
};

//minimalizacja
auto hill_climbing_r_alg = [](auto get_random_sol, auto get_all_neighbours, auto goal, int max_iterations, auto callback) {
    auto current_solution = get_random_sol();
    for (int iteration = 0; iteration < max_iterations; iteration++)
    {
        auto next_solutions = get_all_neighbours(current_solution);
        uniform_int_distribution<int> uniform_dist(0, next_solutions.size() - 1);
        auto next_sol = next_solutions.at(uniform_dist(e1));
        if (goal(current_solution) > goal(next_sol))
        {
            current_solution = next_sol;
        }
        callback(iteration, current_solution);
    }
    return current_solution;
};

//minimalizacja
auto simulated_annealing = [](auto get_random_sol, auto N, auto goal, auto T, int max_iterations, auto callback) {
    using namespace std;
    auto current_solution = get_random_sol();
    auto global_best = current_solution;
    uniform_real_distribution<double> uk(0.0, 1.0);
    for (int iteration = 0; iteration < max_iterations; iteration++)
    {
        auto next_sol = N(current_solution);
        if (goal(current_solution) > goal(next_sol))
        {
            current_solution = next_sol;
        }
        else
        {
            if (uk(e1) < exp(-abs(goal(next_sol) - goal(current_solution)) / T(iteration)))
            {
                current_solution = next_sol;
            }
        }
        if (goal(current_solution) < goal(global_best))
            global_best = current_solution;
        callback(iteration, current_solution);
    }
    return global_best;
};

// minimalizacja
auto tabu_alg = [](auto get_random_sol, auto get_all_neighbours, auto goal, int max_iterations, int tabu_size, auto callback) {
    auto current_solution = get_random_sol();
    auto global_best = current_solution; // musimy znalezc najlepsze w ogole
    using namespace std;
    list<decltype(current_solution)> tabu_list; // lista tabu - czyli punkty do ktorych juz nie wracamy
    tabu_list.push_back(current_solution);
    for (int iteration = 0; iteration < max_iterations; iteration++)
    {
        list<decltype(current_solution)> next_solutions;
        for (auto &n : get_all_neighbours(current_solution))
            if (std::find(tabu_list.begin(), tabu_list.end(),n)==tabu_list.end()) // czy punkt jest w tabu?
                next_solutions.push_back(n); // jesli nie, to dopisujemy punkt
        if (next_solutions.size() == 0) {cerr << "Tabu blocked" << endl; break;}
        current_solution = *(max_element(next_solutions.begin(), next_solutions.end(), [&goal](auto a, auto b) {
            return goal(a) > goal(b);
        })); // wybieramy najlepszy i dopisujemy do tabu
        tabu_list.push_back(current_solution);
        if (tabu_list.size() > tabu_size) // ograniczamy rozmiar tabu
            tabu_list.pop_front(); // czyli kasujemy najstarszy
        if (goal(current_solution) < goal(global_best)) // jesli znalezlismy cos lepszego niz do tej pory
            global_best = current_solution; // to aktualizujemy najlepszy globalnie znaleziony
        callback(iteration, current_solution);
    }
    return global_best;
};

int main(int argc, char **argv)
{
 auto start = chrono::high_resolution_clock::now();

std::string a1 = argv[1];
std::string a2 = argv[2];

auto min = -10;
auto max = 10;

    if(argc > 4){
        min = stoi(argv[3]); 
        max = stoi(argv[4]);
        cerr << "From : " << min << " To : " << max << endl;
    }

    uniform_real_distribution<double> uniform_dist(min, max);

    int max_iterations = (argc > 5) ? stoi(argv[5]) : 10000;

    auto sphere_f = [](vector<double> x) {
        double sum = 0;
        for (auto e : x)
        {
            sum += e * e;
        }
        return sum;
    };

    auto himmelblau = [](vector<double> d) {
        double x = d.at(0);
        double y = d.at(1);
        return -20 * exp(-0.2 * sqrt(0.5 * (x * x + y * y))) - exp(0.5 * cos(2 * M_PI * x) + cos(2 * M_PI * y)) + M_E + 20;
    };

        auto matyas = [](vector<double> d) {
        double x = d.at(0);
        double y = d.at(1);
        return 0.26*(x*x+y*y)-(0.48*x*y);
    };

    auto booth = [](vector<double> d) {
        double x = d.at(0);
        double y = d.at(1);
        return (x+2*y-7)*(x+2*y-7)+(2*x+y-5)*(2*x+y-5);
    };

        auto treehumpcamel = [](vector<double> d) {
        double x = d.at(0);
        double y = d.at(1);
        return 2*(x*x) - 1.05*(x*x*x*x) + (x*x*x*x*x*x)/6 + (x*y) + (y*y);
    };

    auto ackley = [](vector<double> d) {
        double x = d.at(0);
        double y = d.at(1);
        return pow(x * x + y - 11, 2.0) + pow(x + y * y - 7, 2);
    };

    auto init = [&uniform_dist]() {
        vector<double> x(2);
        x[0] = uniform_dist(e1);
        x[1] = uniform_dist(e1);
        return x;
    };

    /// f(x)    x należy do R^n
    auto neighbours = [&min,&max](vector<double> x, double dx = 0.01) {
        vector<vector<double>> ret;
        for (int i = 0; i < x.size(); i++)
        {
            auto nx = x;
            nx[i] += dx;
            if (nx[i] < max)
                ret.push_back(nx);
            nx[i] -= 2.0 * dx;
            if (nx[i] > min)
                ret.push_back(nx);
        }
        return ret;
    };

    auto goal = himmelblau;

    if ((argc > 2) && (string(argv[1]) == "hi")){
    auto goal = himmelblau;
    cerr << "Function : " << "hi" << endl;
    }

    if ((argc > 2) && (string(argv[1]) == "thc")){
    auto goal = treehumpcamel;
    cerr << "Function : " << "thc" << endl;
    }

    if ((argc > 2) && (string(argv[1]) == "bo")){
    auto goal = booth;
    cerr << "Function : " << "bo" << endl;
    }

    if ((argc > 2) && (string(argv[1]) == "ma")){
    auto goal = matyas;
    cerr << "Function : " << "ma" << endl;
    }

    if ((argc > 2) && (string(argv[1]) == "ac")){
    auto goal = ackley;
    cerr << "Function : " << "ac" << endl;
    }

    if ((argc > 2) && (string(argv[1]) == "sf")){
    auto goal = sphere_f;
    cerr << "Function : " << "sf" << endl;
    }

    auto debug_print = [=](int i, vector<double> v) {
        int stepprint = max_iterations / 100;
        //if ((i % stepprint) == 0)
            //            cout << i << " " << goal(v) << endl;
            cout << v.at(0) << " " << v.at(1) << " " << goal(v) << endl;
            nazwa = "Wyniki|"+ a1 +"|"+ a2 +"|"+"|"+"czyste";
            ofstream wyniki;
            wyniki.open(nazwa, ios_base::app);
            wyniki << goal(v)<< endl;
            wyniki.close();
    };
    vector<double> solution;

     if ((argc > 2) && (string(argv[2]) == "hcr"))
    {
        cerr << "Algorithm : " << "hcr" << endl;

        vector<pair<double,double>> d;
        d.push_back(pair<double,double>(min, max));
        d.push_back(pair<double,double>(min, max));

        solution = hill_climbing_alg_random(
            goal,
            d,
            max_iterations,
            debug_print);
    }

    if ((argc > 2) && (string(argv[2]) == "hc"))
    {
        cerr << "Algorithm : " << "hc" << endl;
        solution = hill_climbing_alg(
            init,
            neighbours,
            goal,
            max_iterations,
            debug_print);
    }
    if ((argc > 2) && (string(argv[2]) == "rhc"))
    {
        cerr << "Algorithm : " << "rhc" << endl;
        solution = hill_climbing_r_alg(
            init,
            neighbours,
            goal,
            max_iterations,
            debug_print);
    }
    if ((argc > 2) && (string(argv[2]) == "sa"))
    {
        cerr << "Algorithm : " << "sa" << endl;
        solution = simulated_annealing(
            init,
            [](vector<double> v) {
                std::normal_distribution<double> n;
                for (auto &e : v)
                    e = e + n(e1) * 0.002;
                return v;
            },
            goal,
            [](auto k) { return 200000.0 / k; },
            max_iterations,
            debug_print);
    }
    if ((argc > 2) && (string(argv[2]) == "tabu"))
    {
        cerr << "Algorithm : " << "tabu" << endl;
        solution = tabu_alg(
            init,
            neighbours,
            goal,
            max_iterations,
            1000, // tabu size
            debug_print);
    }

    auto finish = chrono::high_resolution_clock::now();

    chrono::duration<double> runtime = finish - start;

    cerr << "#result: [ ";
    for (auto e : solution)
        cerr << e << " ";
    cerr << "] -> " << goal(solution) << endl;

        auto wynik_programu = goal(solution);

        std::string w_p_s = std::to_string(wynik_programu);
        std::string r_t_s = std::to_string(runtime.count());

        double * wyniki_czyste = new double[max_iterations];

        ifstream input( nazwa );
        int iteracje = 0;
        string line; 

    while (getline(input, line))
    {
        wyniki_czyste[iteracje] = stod(line);
        iteracje++;
    }
    input.close();

    ofstream wyniki_obliczone;
     string nazwa2 = "Wyniki|"+ a1 +"|"+ a2 +"|"+w_p_s+"|"+r_t_s+"|"+"przeliczone";
    wyniki_obliczone.open(nazwa2, ios_base::app); 
    for (int i = 0; i<iteracje; i++) 
    {   

        double wynik_miara = (abs(wynik_programu - wyniki_czyste[i]) * runtime.count())*10000000;
        wyniki_obliczone << setprecision(10) << ' ' << wynik_miara << endl;  

    }
    wyniki_obliczone.close();
    remove((nazwa).c_str());
}
