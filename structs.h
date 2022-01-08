#ifndef STRUCTS_H
#define STRUCTS_H

#include <QObject>

enum class ColorEnum {
    COLORLESS = 0,
    RED = 1,
    GREEN = 2,
    BLUE = 3,
    YELLOW = 4
};

struct Cell {
    ColorEnum color = ColorEnum::COLORLESS;
    bool isBusy     = false;

    QString getStrColor() const;
    int     getNumColor() const;
    void    setColor(int id);

};

#endif // STRUCTS_H
