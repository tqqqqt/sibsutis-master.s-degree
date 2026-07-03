#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>
#include <functional>


class RandomWalk {
    int N;
    std::vector<std::vector<double>> mas;
    std::uniform_real_distribution<> coin{0.0, 1.0};
public:
    RandomWalk(int n) : N(n) {
        mas=std::vector<std::vector<double>>(N,std::vector<double>(N,0));
        for (int i=0;i<N;++i) {
            mas[i][(i-1+N)%N]=0.5;
            mas[i][(i+1)%N]=0.5;
        }
    }
    void PrintMas() const{
        for(int i=0;i<N;++i){
            for(int j=0;j<N;++j) std::cout<<std::fixed << std::setprecision(1)<<mas[i][j]<<"   ";
            std::cout<<'\n';
        }
    }
    int Next(int current, std::mt19937& rng) {
        if (coin(rng) < 0.5)
            return (current + 1) % N;
        else
            return (current - 1 + N) % N;
    }
};

class SelfLoopWalk {
    int N;
    std::vector<std::vector<double>> mas;
    std::uniform_real_distribution<> uni{0.0, 1.0};
public:
    SelfLoopWalk(int n) : N(n) {
        mas=std::vector<std::vector<double>>(N,std::vector<double>(N,0));
        for (int i=0;i<N;++i) {
            mas[i][i]=0.8;
            mas[i][(i-1+N)%N]=0.1;
            mas[i][(i+1)%N]=0.1;
        }
    }
    void PrintMas() const{
        for(int i=0;i<N;++i){
            for(int j=0;j<N;++j) std::cout<<std::fixed << std::setprecision(1)<<mas[i][j]<<"   ";
            std::cout<<'\n';
        }
    }
    int Next(const int& current, std::mt19937& rng) {
        double r = uni(rng);
        if (r < 0.8)
            return current;
        else if (r < 0.9)
            return (current + 1) % N;
        else
            return (current - 1 + N) % N;
    }
};


template <typename Walker>
std::vector<int> GenerateTrajectory(Walker& walker, const int& initial_state, const int& length, std::mt19937& rng) {
    std::vector<int> trajectory(length);
    trajectory[0] = initial_state;
    for (int i = 1; i < length; ++i)
        trajectory[i] = walker.Next(trajectory[i-1], rng);
    return trajectory;
}

std::vector<double> Histogram(const std::vector<int>& trajectory, const int& n_states) {
    std::vector<int> counts(n_states, 0);
    for (int s : trajectory) ++counts[s];
    std::vector<double> freq(n_states);
    int traj_size = trajectory.size();
    for (int i = 0; i < n_states; ++i)
        freq[i] = static_cast<double>(counts[i]) / traj_size;
    return freq;
}

std::vector<double> autocorrelation(const std::vector<int>& trajectory, const int& max_lag) {
    size_t n = trajectory.size();
    double mean = 0.0;
    for (int x : trajectory) mean += x;
    mean /= n;

    double var = 0.0;
    for (int x : trajectory) var += (x - mean) * (x - mean);
    var /= (n - 1);

    std::vector<double> acf(max_lag + 1, 0.0);
    for (int lag = 0; lag <= max_lag; ++lag) {
        double cov = 0.0;
        for (size_t t = 0; t < n - lag; ++t) {
            cov += (trajectory[t] - mean) * (trajectory[t + lag] - mean);
        }
        cov /= (n - lag);
        acf[lag] = cov / var;
    }
    return acf;
}


int main(int argc, char** argv) {
    if (argc < 2) return 4;
    try {
        int t = std::stoi(argv[1]);
        if (t < 3) throw std::exception();
    } catch (...) {
        return 4;
    }

    int N = std::stoi(argv[1]);
    int L = 100000;
    int MAX_LAG = 10;
    std::random_device rd;
    std::mt19937 rng(rd());
    RandomWalk walker1(N);
    SelfLoopWalk walker2(N);

    int init_state = 0;
    std::vector<int> traj1 = GenerateTrajectory(walker1, init_state, L, rng), traj2 = GenerateTrajectory(walker2, init_state, L, rng);

    std::cout<<"Matrix 1:\n";
    walker1.PrintMas();
    std::cout<<"\nMatrix 2:\n";
    walker2.PrintMas();
    std::cout<<'\n';

    std::vector<double> hist1 = Histogram(traj1, N);
    std::vector<double> hist2 = Histogram(traj2, N);
    std::cout << "Histogram:\n";
    for (int i = 0; i < N; ++i)
        std::cout << std::fixed << std::setprecision(4) << hist1[i] << " | " << hist2[i] << " | " << (1.0/N) << '\n';
    std::cout << '\n';


    std::vector<double> acf1 = autocorrelation(traj1, MAX_LAG);
    std::vector<double> acf2 = autocorrelation(traj2, MAX_LAG);
    std::cout << "Autokorelaz:\n";
    for (int lag = 0; lag <= MAX_LAG; ++lag)
        std::cout << "  " << std::setw(2) << lag << "  | " << std::fixed << std::setprecision(4) << acf1[lag] << " | " << acf2[lag] << '\n';

    return 0;
}