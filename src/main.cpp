#include <iostream>
#include <string>
#include <tuple>
#include <limits>
#include "Module.h"
#include "Table.h" 

int main() {
    setlocale(LC_ALL, "ru");
    initialize();
    /*for (int i=0; i < road.size(); i++) {
        std::cout << base[string[i]] << std::endl;
    }*/
    Table table(base, road);

    while (true) {
        std::string chose = textStart();

        if (chose == "1") {
            system("clear"); // For Windows "cls", use "clear" for Linux/Mac
            table.printTable('+');
        }
        else if (chose == "2") {
            table.printTable('-');
        }
        else if (chose == "3") {
            table.printTable('*');
        }
        else if (chose == "4") {
            std::tuple<std::string, std::string, char> trio;
            trio = textGetVal();
            std::string op1 = std::get<0>(trio);
            std::string op2 = std::get<1>(trio);
            char sign = std::get<2>(trio);
            std::string ans = table.pomogi(op1, op2, sign);
            std::cout << "Ответ: " << ans << std::endl;
        }
        else {
            break;
        }

        std::cout << "(Нажмите Enter для продолжения)" << std::endl;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return 0;
}
