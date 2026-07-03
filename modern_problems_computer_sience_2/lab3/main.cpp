#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

class AsyncPipeline {
    int S;      // число ступеней
    int N;      // число элементов
    double mu;  // математическое ожидание времени
    double sigma;  // среднеквадратическое отклонение
    mt19937 rng;
    normal_distribution<> dist;

    struct Stage {
        bool busy;
        int remaining;
    };
    vector<Stage> stages;
    vector<bool> buffer;  // буферы между ступенями (S-1 штук)
    int next_input;  // следующий элемент для подачи на вход
    int processed;  // сколько элементов обработано

   public:
    AsyncPipeline(int s, int n, double m, double sig)
        : S(s), N(n), mu(m), sigma(sig), rng(random_device{}()), dist(m, sig) {
        stages.resize(S, {false, 0});
        if (S > 1) buffer.resize(S - 1, false);
        next_input = 0;
        processed = 0;
    }

    // Запуск модели, возвращает число тактов
    int run() {
        int ticks = 0, time;
        bool has_input;
        double t;
        while (processed < N) {
            // 1. Уменьшаем счётчики для занятых ступеней
            for (int i = 0; i < S; ++i) {
                if (stages[i].busy && stages[i].remaining > 0)
                    stages[i].remaining--;
            }

            // 2. Обрабатываем завершившиеся ступени
            for (int i = 0; i < S; ++i) {
                if (!stages[i].busy || stages[i].remaining != 0) continue;

                if (i == S - 1) {  // последняя ступень
                    processed++;
                    stages[i].busy = false;
                } else {  // промежуточная ступень
                    if (!buffer[i]) {      // буфер свободен
                        buffer[i] = true;  // помещаем элемент
                        stages[i].busy = false;
                    }
                    // иначе ждём освобождения буфера
                }
            }

            // 3. Запускаем новые обработки
            for (int i = 0; i < S; ++i) {
                if (stages[i].busy) continue;

                has_input = false;
                if (i == 0)
                    has_input = (next_input < N);
                else
                    has_input = (buffer[i - 1]);
                
                if (has_input) {
                    // Генерация времени обработки
                    t = dist(rng);
                    time = max(1, (int)round(t));
                    stages[i].busy = true;
                    stages[i].remaining = time;

                    // Забираем элемент из входного буфера
                    if (i == 0)
                        next_input++;
                    else
                        buffer[i - 1] = false;
                }
            }
            ticks++;
        }
        return ticks;
    }
};

int main() {
    const int N = 10000;  // количество элементов
    const double mu = 10.0;  // мат. ожидание времени обработки
    const double sigma = 2.0;  // среднеквадратическое отклонение
    const int num_runs = 10;  // число прогонов для усреднения
    const int max_stages = 20;  // максимальное число ступеней

    cout << fixed << setprecision(3);
    cout << "Stages\tT_async_avg\t\tT_sync\t\tRatio (async/sync)\n";

    double total_ticks, T_async, T_sync, ratio;
    for (int S = 1; S <= max_stages; ++S) {
        total_ticks = 0.0;
        for (int run = 0; run < num_runs; ++run) {
            AsyncPipeline pipeline(S, N, mu, sigma);
            total_ticks += pipeline.run();
        }
        T_async = total_ticks / num_runs;
        T_sync = (S + N - 1) * mu;
        ratio = T_async / T_sync;

        cout << S << "\t" << T_async << "\t\t" << T_sync << "\t\t" << ratio
             << "\n";
    }

    return 0;
}