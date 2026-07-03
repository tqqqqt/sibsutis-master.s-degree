#include <chrono>
#include <cmath>
#include <iostream>
#include <random>

int main() {
    const double h = 1.0;  // толщина пластинки
    const double Sigma = 1.0;  // полное макроскопическое сечение
    const double Sigma_a = 0.2;  // сечение поглощения
    const unsigned long long N = 100000;  // число моделируемых траекторий
    const double p_abs =
        Sigma_a / Sigma;  // Вероятность поглощения при столкновении

    std::mt19937 gen(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    unsigned long long N_plus = 0;   // прошли сквозь
    unsigned long long N_minus = 0;  // отразились
    unsigned long long N0 = 0;       // поглощены

    for (unsigned long long n = 0; n < N; ++n) {
        double x = 0.0;  // начальная координата (вход в пластинку)
        double cos_phi =
            1.0;  // начальное направление: нормальное падение (cos=1)
        // cos_phi = dist(gen) * 2.0 - 1.0; // равномерно на [-1, 1]

        bool trajectory_finished = false;
        while (!trajectory_finished) {
            // Розыгрыш длины свободного пробега
            double xi;
            do {
                xi = dist(gen);
            } while (xi == 0.0);  // избегаем -log(0)
            double lambda = -std::log(xi) / Sigma;

            double x_next = x + lambda * cos_phi;

            if (x_next > h) {
                ++N_plus;  // прохождение
                break;
            }
            if (x_next < 0.0) {
                ++N_minus;  // отражение
                break;
            }

            double gamma = dist(gen);
            if (gamma <= p_abs) {
                ++N0;  // поглощение
                break;
            }

            x = x_next;
            cos_phi = dist(gen) * 2.0 - 1.0;  // диапазон [-1, 1]
        }
    }

    double P_plus = static_cast<double>(N_plus) / N;
    double P_minus = static_cast<double>(N_minus) / N;
    double P0 = static_cast<double>(N0) / N;

    std::cout << "Результаты моделирования " << N << " траекторий:\n";
    std::cout << "Прошедшие:   " << N_plus << "  (вероятность = " << P_plus
              << ")\n";
    std::cout << "Отразившиеся: " << N_minus << "  (вероятность = " << P_minus
              << ")\n";
    std::cout << "Поглощённые:  " << N0 << "  (вероятность = " << P0 << ")\n";

    return 0;
}