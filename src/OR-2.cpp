#include "ux.h"

int main()
{
    setlocale(LC_ALL, "ru_RU.UTF-8");
    setlocale(LC_ALL, "Russian");

    std::vector<std::string> block = {
        " ~ ORDER RELATION TOOL ~",
        "+--------------------------------+",
        "| Hasse diagram                  |",
        "| Create order relation          |",
        "| Add vertex                     |",
        "| Delete vertices                |",
        "| Matrices                       |",
        "| Matrix calculations            |",
        "| Exit                           |",
        "+--------------------------------+"
    };
    choosePrintCenteredBlock(block, 2);
}