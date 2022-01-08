#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QObject>
#include <QAbstractListModel>

#include "structs.h"
#include "database.h"

class GameBoard : public QAbstractListModel {
    Q_OBJECT
public:
    Q_PROPERTY(int     currentScore READ currentScore WRITE setCurrentScore NOTIFY currentScoreChanged)
    Q_PROPERTY(bool    isFinal      READ isFinal      WRITE setIsFinal      NOTIFY isFinalChanged)

    enum circleRoles {
        cellColor = Qt::UserRole + 1,
        cellIsBusy
    };

    GameBoard(QObject *parent = 0);
    ~GameBoard();
    DataBase* appDb = nullptr;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    int     currentScore() const;
    bool    isFinal() const;

public slots:
    void refresh();
    void newGame();
    void clearBoard();
    bool tryFillBoardFromDB();
    bool tryFillInformationFromDB();
    void fillBoardEmptyCells();

    bool tryToMakeAFirstMove(int index);
    bool tryToMakeASecondMove(int index);
    void endASecondMove(int index);

    void setCurrentScore(int newScore);
    void setIsFinal(bool newIsFinal);

signals:
    void currentScoreChanged(int currentScore);
    void isFinalChanged(bool isFinal);

private:
    QList<Cell> cells;
    QSet<int> freeCells;
    QHash<int, QByteArray> roles;

    int     m_currentScore {0};
    bool    m_isFinal      {false};

    size_t pointsForWin = 10;
    size_t lehgthWin    = 5;
    size_t maxRow       = 9;
    size_t maxColumn    = 9;
    size_t boardSize    = maxRow * maxColumn;

    int firstClickCellId = -1;

    bool checkCellIsFree(int index) const;

    void clearCell(int indexCell);
    void placeCell(int indexCell, int idColor);
    void moveCell(int indexFrom, int indexTo);

    bool isValidPosition(int index) const;
    bool checkPossibilityWay(int from, int to, QSet<int>& seen) const;
    void checkAndApplyWinLines(int index);
    int  checkHorLine(int index, ColorEnum needColor) const;
    int  checkVerLine(int index, ColorEnum needColor) const;
    void applyHorLine(int index);
    void applyVerLine(int index);
    void makeComputerMove();
    bool checkIsFinal() const;

    QList<int> getFreeNeighbourCells(int index) const;
    int getIndexRightNeighbour(int index) const;
    int getIndexLeftNeighbour(int index) const;
    int getIndexTopNeighbour(int index) const;
    int getIndexBottomNeighbour(int index) const;
};

#endif // GAMEBOARD_H
