#include <iostream>
#include <vector>
#include <memory>

// (Вариант 8)

class PrintStrategy
{
public:
    virtual ~PrintStrategy() {}
    virtual void print(const std::vector<int>& arr) const = 0;
};

// Печать по N элементов в строке через пробел
class PrintNPerLine : public PrintStrategy
{
private:
    int n_;
public:
    PrintNPerLine(int n) : n_(n) {}

    void print(const std::vector<int>& arr) const override
    {
        std::cout << "Печать по " << n_ << " элементов в строке" << std::endl;
        for (size_t i = 0; i < arr.size(); ++i)
        {
            std::cout << arr[i] << " ";
            if ((i + 1) % n_ == 0)
            {
                std::cout << std::endl;
            }
        }
        if (arr.size() % n_ != 0)
        {
            std::cout << std::endl;
        }
    }
};

// Печать по одному элементу в строке
class PrintOnePerLine : public PrintStrategy
{
public:
    void print(const std::vector<int>& arr) const override
    {
        std::cout << "Печать по одному элементу в строке" << std::endl;
        for (int val : arr)
        {
            std::cout << val << std::endl;
        }
    }
};

class Array
{
private:
    std::vector<int> data_;
    PrintStrategy* strategy_; // Указатель на текущую стратегию

public:
    Array(const std::vector<int>& data, PrintStrategy* strategy = nullptr)
        : data_(data), strategy_(strategy) {}

    // Метод для смены стратегии
    void setStrategy(PrintStrategy* strategy)
    {
        strategy_ = strategy;
    }

    // Выполнение печати
    void print() const
    {
        if (strategy_)
        {
            strategy_->print(data_);
        }
        else
        {
            std::cout << "Стратегия не задана!" << std::endl;
        }
    }
};

int main()
{
    setlocale(LC_ALL, "Russian");

    // Исходные данные
    std::vector<int> numbers = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

    PrintNPerLine print3(3);       // Печать по 3 элемента в строке
    PrintOnePerLine printSingle;   // Печать по 1 элементу в строке

    // Создаем массив с первой стратегией
    Array myArray(numbers, &print3);

    // Печать по 3 элемента
    myArray.print();
    std::cout << std::endl;

    // Меняем стратегию
    myArray.setStrategy(&printSingle);
    myArray.print();

    return 0;
}