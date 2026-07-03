#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <cmath>

#define LAMBDA 1.0          // интенсивность входного потока
#define MU1 0.9            // интенсивность обслуживания для 1 прибора
#define MU2 0.3             // интенсивность обслуживания для каждого из 3 приборов
#define DT 0.01             // шаг дискретизации времени
#define SIM_TIME 1000.0     // общее время моделирования
#define N_STEPS static_cast<long long>(SIM_TIME / DT) // количество шагов
#define COUNT_APP 3

std::mt19937 rng(std::random_device{}());
std::uniform_real_distribution<double> uni(0.0, 1.0);

double genServiceTime(double mu) {
    std::exponential_distribution<double> exp_dist(mu);
    return exp_dist(rng);
}

void simulateSingleChannel(const std::string& filename) {
    double current_time = 0.0;
    double busy_until = 0.0;
    int queue_length = 0;
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Open file error " << filename << std::endl;
        return;
    }

    for (long long step = 0; step < N_STEPS; ++step) {
        current_time += DT;

        // 1. Проверка окончания обслуживания
        if (busy_until <= current_time) {
            if (queue_length > 0) {
                double service_time = genServiceTime(MU1);
                busy_until = current_time + service_time;
                --queue_length;
            } else 
                busy_until = 0.0;
        }

        // 2. Разыгрывание прихода нового требования
        if (uni(rng) < LAMBDA * DT) {
            if (busy_until <= current_time) {
                double service_time = genServiceTime(MU1);
                busy_until = current_time + service_time;
            } else
                ++queue_length;
        }

        out << current_time << " " << queue_length << "\n";
    }
    out.close();
}

void simulateThreeChannels(const std::string& filename) {
    double current_time = 0.0;
    std::vector<double> busy_until(COUNT_APP, 0.0);
    std::vector<int> queues(COUNT_APP, 0);
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error open file " << filename << std::endl;
        return;
    }

    std::uniform_int_distribution<int> dev_choice(0, COUNT_APP-1);

    for (long long step = 0; step < N_STEPS; ++step) {
        current_time += DT;

        // 1. Проверка окончания обслуживания для каждого прибора
        for (int i = 0; i < COUNT_APP; ++i) {
            if (busy_until[i] <= current_time) {
                if (queues[i] > 0) {
                    double service_time = genServiceTime(MU2);
                    busy_until[i] = current_time + service_time;
                    --queues[i];
                } else
                    busy_until[i] = 0.0;
            }
        }

        // 2. Разыгрывание прихода нового требования
        if (uni(rng) < LAMBDA * DT) {
            int chosen = dev_choice(rng);
            if (busy_until[chosen] <= current_time) {
                double service_time = genServiceTime(MU2);
                busy_until[chosen] = current_time + service_time;
            } else
                ++queues[chosen];
        }

        int total_queue = queues[0] + queues[1] + queues[2];
        out << current_time << " " << total_queue << "\n";
    }
    out.close();
}

int main() {
    std::cout << "Params:\n";
    std::cout << "A = " << LAMBDA << ", shag dt = " << DT << ", time = " << SIM_TIME << "\n";
    std::cout << "System 1: 1 aparat, u = " << MU1 << "\n";
    std::cout << "System 2: " << COUNT_APP << " aparat, u = " << MU2 << " each\n";
    std::cout << "Modelirovanie...\n\n";

    simulateSingleChannel("queue1.txt");
    simulateThreeChannels("queue2.txt");

    std::cout << "Two columns: time, queue size.\n";
    std::cout << "  - queue1.txt (one chennel SMO)\n";
    std::cout << "  - queue2.txt (three channel SMO)\n";

    return 0;
}