#include "structs.h"

int Cell::getNumColor() const {
    int idColor = -1;
    switch (color) {
    case ColorEnum::COLORLESS :
        idColor = 0;
        break;
    case ColorEnum::RED :
        idColor = 1;
        break;
    case ColorEnum::GREEN :
        idColor = 2;
        break;
    case ColorEnum::BLUE :
        idColor = 3;
        break;
    case ColorEnum::YELLOW :
        idColor = 4;
        break;
    }
    return idColor;
}

QString Cell::getStrColor() const {
    QString str;
    switch (color) {
    case ColorEnum::COLORLESS :
        str = "transparent";
        break;
    case ColorEnum::RED :
        str = "#A57C56";
        break;
    case ColorEnum::GREEN :
        str = "#A4B787";
        break;
    case ColorEnum::BLUE :
        str = "#B3D6D4";
        break;
    case ColorEnum::YELLOW :
        str = "#FCD582";
        break;
    }
    return str;
}

void Cell::setColor(int id) {
    switch (id) {
    case 0 :
        color = ColorEnum::COLORLESS;
        break;
    case 1 :
        color = ColorEnum::RED;
        break;
    case 2 :
        color = ColorEnum::GREEN;
        break;
    case 3 :
        color = ColorEnum::BLUE;
        break;
    case 4 :
        color = ColorEnum::YELLOW;
        break;
    }
}
