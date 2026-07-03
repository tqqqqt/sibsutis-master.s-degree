#include <iostream>
#include <fstream>
#include <random>
#include <queue>
#include <vector>
#include <memory>
#include <cmath>
#include <limits>

std::mt19937 gen(std::random_device{}());

// Абстрактный базовый класс для потока требований
class Stream {
public:
    virtual double nextInterval() = 0;
    virtual ~Stream() = default;
};

// Регулярный поток (детерминированные интервалы)
class RegularStream : public Stream {
    double interval_;
public:
    RegularStream(double intensity) : interval_(1.0 / intensity) {}
    double nextInterval() override {
        return interval_;
    }
};

// Пуассоновский поток (экспоненциальные интервалы)
class PoissonStream : public Stream {
    std::exponential_distribution<double> dist_;
public:
    PoissonStream(double intensity) : dist_(intensity) {}
    double nextInterval() override {
        return dist_(gen);
    }
};

// Поток Эрланга k-го порядка (сумма k экспоненциальных)
class ErlangStream : public Stream {
    int k_;
    std::exponential_distribution<double> exp_dist_;
public:
    ErlangStream(double intensity, int k) : k_(k), exp_dist_(intensity * k) {
        // Интенсивность исходного простейшего потока = intensity * k,
        // тогда средний интервал результирующего потока = k / (intensity * k) = 1/intensity
    }
    double nextInterval() override {
        double sum = 0.0;
        for (int i = 0; i < k_; ++i) {
            sum += exp_dist_(gen);
        }
        return sum;
    }
};

// Результаты симуляции: вектор пар (время, длина очереди)
using Result = std::vector<std::pair<double, size_t>>;

// Класс симулятора
class Simulator {
    double mu_;                 // интенсивность обслуживания
    double maxTime_;            // максимальное время моделирования
    std::unique_ptr<Stream> stream_;
public:
    Simulator(std::unique_ptr<Stream> stream, double mu, double maxTime)
        : stream_(std::move(stream)), mu_(mu), maxTime_(maxTime) {}

    Result run() {
        Result result;
        size_t queueLength = 0;
        bool serverBusy = false;
        double currentTime = 0.0;
        std::exponential_distribution<double> serviceDist(mu_);

        // Планируем первое событие прихода
        double nextArrival = stream_->nextInterval();
        // Следующее событие завершения обслуживания (infinity если нет)
        double nextDeparture = std::numeric_limits<double>::infinity();

        // Записываем начальное состояние
        result.emplace_back(currentTime, queueLength);

        while (currentTime < maxTime_) {
            // Выбираем ближайшее событие
            if (nextArrival < nextDeparture) {
                // Событие: приход требования
                currentTime = nextArrival;

                // Генерируем следующее время прихода
                nextArrival = currentTime + stream_->nextInterval();

                if (!serverBusy) {
                    // Прибор свободен – начинаем обслуживание немедленно
                    serverBusy = true;
                    double serviceTime = serviceDist(gen);
                    nextDeparture = currentTime + serviceTime;
                    // Очередь не увеличивается, т.к. требование сразу пошло на прибор
                } else {
                    // Прибор занят – ставим в очередь
                    ++queueLength;
                }
                // Запись состояния после обработки прихода
                result.emplace_back(currentTime, queueLength);
            }
            else if (nextDeparture < nextArrival) {
                // Событие: завершение обслуживания
                currentTime = nextDeparture;

                if (queueLength > 0) {
                    // В очереди есть требования – начинаем обслуживание следующего
                    --queueLength;
                    double serviceTime = serviceDist(gen);
                    nextDeparture = currentTime + serviceTime;
                } else {
                    // Очередь пуста – прибор освобождается
                    serverBusy = false;
                    nextDeparture = std::numeric_limits<double>::infinity();
                }
                // Запись состояния после завершения обслуживания
                result.emplace_back(currentTime, queueLength);
            }
            else {
                // Если оба времени равны (практически невозможно при непрерывных распределениях)
                // или оба бесконечны, выходим
                break;
            }
        }
        return result;
    }
};

void saveResult(const std::string& filename, const Result& data) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error open file: " << filename << std::endl;
        return;
    }
    out.precision(10);
    for (const auto& p : data)
        out << p.first << " " << p.second << "\n";
    std::cout << "Result save in " << filename << "\n\n";
}

int main() {
    const double intensity = 1.0;      // интенсивность входящего потока
    const double serviceIntensity = 0.95; // интенсивность обслуживания
    const double maxTime = 1000.0;      // время моделирования

    int erlangOrder = 0;
    std::cout << "Poradok potoka Erlanga: ";
    std::cin >> erlangOrder;
    if (erlangOrder < 1) {
        std::cerr << "Poradok < 1, set 1." << std::endl;
        erlangOrder = 1;
    }

    // 1. Регулярный поток
    std::cout << "Regular potok..." << std::endl;
    {
        auto stream = std::make_unique<RegularStream>(intensity);
        Simulator sim(std::move(stream), serviceIntensity, maxTime);
        auto res = sim.run();
        saveResult("queue_regular.txt", res);
    }

    // 2. Пуассоновский поток
    std::cout << "Pyassonovski potok..." << std::endl;
    {
        auto stream = std::make_unique<PoissonStream>(intensity);
        Simulator sim(std::move(stream), serviceIntensity, maxTime);
        auto res = sim.run();
        saveResult("queue_poisson.txt", res);
    }

    // 3. Поток Эрланга заданного порядка
    std::cout << "Erlang potok " << erlangOrder << "-go poradka..." << std::endl;
    {
        auto stream = std::make_unique<ErlangStream>(intensity, erlangOrder);
        Simulator sim(std::move(stream), serviceIntensity, maxTime);
        auto res = sim.run();
        std::string filename = "queue_erlang_" + std::to_string(erlangOrder) + ".txt";
        saveResult(filename, res);
    }

    return 0;
}