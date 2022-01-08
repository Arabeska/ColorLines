#include "gameboard.h"

#include <QRandomGenerator>

GameBoard::~GameBoard(){
}

GameBoard::GameBoard(QObject *parent)
    : QAbstractListModel (parent){
    roles[cellColor]  = "cellColor";
    roles[cellIsBusy] = "cellIsBusy";
}

QVariant GameBoard::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    QVariant retVal= "";
    Cell cell = cells.at(index.row());
    switch (role){
    case cellColor:
        retVal = cell.getStrColor();
        break;
    case cellIsBusy:
        retVal = cell.isBusy;
        break;
    }

    return retVal;
}

int GameBoard::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return cells.count();
}

QHash<int, QByteArray> GameBoard::roleNames() const{
    return roles;
}

void GameBoard::clearBoard() {
    beginResetModel();
    for (int i = 0; i < cells.size(); ++i) {
        Cell& cell = cells[i];
        if (cell.isBusy)
            freeCells.insert(i);
        cell.color = ColorEnum::COLORLESS;
        cell.isBusy = false;
    }
    setCurrentScore(0);
    makeComputerMove();
    endResetModel();
}

void GameBoard::refresh() {
    if (tryFillBoardFromDB() && tryFillInformationFromDB()) {
        setIsFinal(checkIsFinal());
        return;
    }
    newGame();
}

void GameBoard::newGame() {
    appDb->clearTableInformation();
    appDb->clearTablePositions();
    appDb->insertNewBasicInfo(0);
    fillBoardEmptyCells();
    setCurrentScore(0);
    setIsFinal(false);
    makeComputerMove();
}

bool GameBoard::tryFillInformationFromDB() {
    QJsonArray arr = appDb->getLastInformation();
    if (arr.empty())
        return false;
    auto jObj = arr.at(0).toObject();
    setCurrentScore(jObj.value("score").toString().trimmed().toInt());
    return true;
}

bool GameBoard::tryFillBoardFromDB() {
    QJsonArray arr = appDb->getLastProgressPosition();
    if (arr.empty())
        return false;
    beginResetModel();
    cells.clear();
    freeCells.clear();
    for (int i = 0; i < arr.count(); ++i) {
        auto jObj = arr.at(i).toObject();
        Cell cell;
        cell.isBusy = jObj.value("is_busy_cell").toString().trimmed().toInt();
        if(cell.isBusy)
            cell.setColor(jObj.value("color_cell").toString().trimmed().toInt());
        cells.append(cell);
        if(!cell.isBusy)
            freeCells.insert(i);
    }
    endResetModel();
    return true;
}

void GameBoard::fillBoardEmptyCells() {
    beginResetModel();
    cells.clear();
    freeCells.clear();
    for (int i = 0; i < boardSize; ++i) {
        Cell cell;
        cell.color = ColorEnum::COLORLESS;
        cell.isBusy = false;
        cells.append(cell);
        freeCells.insert(i);
        appDb->insertNewPosition(i, cell);
    }
    endResetModel();
}

int GameBoard::currentScore() const {
    return m_currentScore;
}

bool GameBoard::isFinal() const {
    return m_isFinal;
}

void GameBoard::setCurrentScore(int newScore) {
    if (m_currentScore == newScore)
        return;

    m_currentScore = newScore;
    appDb->updateScore(0, newScore);
    emit currentScoreChanged(m_currentScore);
}

void GameBoard::setIsFinal(bool newIsFinal) {
    if (m_isFinal == newIsFinal)
        return;

    m_isFinal = newIsFinal;
    emit isFinalChanged(m_isFinal);
}

/**
 * @brief GameBoard::tryToMakeAFirstMove
 * Проверяет на доступность ячейку, откуда игрок хочет переместить круг и запоминает ее,
 * в случае ее доступности
 * * @param index - индекс ячейки, откуда игрок хочет переместить круг
 * * @return true - если ячейка доступна и запомнена
 */
bool GameBoard::tryToMakeAFirstMove(int index) {
    if (firstClickCellId == -1 && !checkCellIsFree(index)) {
        firstClickCellId = index;
        return true;
    }
    return false;
}

/**
 * @brief GameBoard::tryToMakeASecondMove
 * Проверяет на доступность ячейку, куда хочет переместить круг игрок,
 * и перемещает, в случае доступности, либо сбрасывает первый ход, если ячейка недоступна
 * * @param index - индекс ячейки, куда хочет сходить игрок
 * * @return true - если ход доступен и совершен
 */
bool GameBoard::tryToMakeASecondMove(int index) {
    if (firstClickCellId == -1) {
        return false;
    }
    QSet<int> cellsSeen;
    if (!checkCellIsFree(index) ||
            !checkPossibilityWay(firstClickCellId, index, cellsSeen)) {
        firstClickCellId = -1;
        return false;
    }
    moveCell(firstClickCellId, index);
    firstClickCellId = -1;
    return true;
}

/**
 * @brief GameBoard::endASecondMove
 * Заканчивает ход игрока, проверяя на наличие победных линий и совершает ход компьютера
 * * @param index - индекс ячейки, последнего хода
 */
void GameBoard::endASecondMove(int index) {
    checkAndApplyWinLines(index);
    makeComputerMove();
}

/**
 * @brief GameBoard::isValidPosition
 * Проверяет, что индекс не выходит за границы допустимого диапазона
 * * @param index - индекс ячейки для проверки
 * * @return true - если индекс валидный
 */
bool GameBoard::isValidPosition(int index) const {
    return index >= 0 && index < cells.size();
}

/**
 * @brief GameBoard::checkVerLine
 * Проверяет вертикальную линию на наличие 5 одинаковых ячеек подряд, с заданным цветом
 * * @param index - индекс ячейки, относительно которой проверять
 * * @param needColor - цвет, который необходимо искать
 * * @return int - индекс первой ячейки из победной линии, либо -1
 */
int GameBoard::checkVerLine(int index, ColorEnum needColor) const {
    int indexStartVert = index;
    int resV = indexStartVert/maxRow;
    if (resV > 0)
        indexStartVert -= (indexStartVert/maxColumn) * maxColumn;
    int maxLengthLine = 0;
    int indexFirstLine = -1;
    for (int i = indexStartVert; i < cells.size(); i+=maxColumn) {
        if (cells.at(i).isBusy && cells.at(i).color == needColor) {
            if (indexFirstLine == -1)
                indexFirstLine = i;
            maxLengthLine++;
        }
        else {
            maxLengthLine = 0;
            indexFirstLine = -1;
        }

        if (maxLengthLine == lehgthWin)
            break;
    }
    if (maxLengthLine == lehgthWin)
        return indexFirstLine;
    else
        return -1;
}

/**
 * @brief GameBoard::checkHorLine
 * Проверяет горизонтальную линию на наличие 5 одинаковых ячеек подряд, с заданным цветом
 * * @param index - индекс ячейки, относительно которой проверять
 * * @param needColor - цвет, который необходимо искать
 * * @return int - индекс первой ячейки из победной линии, либо -1
 */
int GameBoard::checkHorLine(int index, ColorEnum needColor) const {
    int indexStartHor = index;
    int resH = indexStartHor%maxColumn;
    if (resH != 0 || resH != maxColumn)
        indexStartHor -= resH;
    int maxLengthLine = 0;
    int indexFirstLine = -1;
    for (int i = indexStartHor; i < indexStartHor+maxColumn; ++i) {
        if (cells.at(i).isBusy && cells.at(i).color == needColor) {
            if (indexFirstLine == -1)
                indexFirstLine = i;
            maxLengthLine++;
        }
        else {
            maxLengthLine = 0;
            indexFirstLine = -1;
        }

        if (maxLengthLine == lehgthWin)
            break;
    }
    if (maxLengthLine == lehgthWin)
        return indexFirstLine;
    else
        return -1;
}

/**
 * @brief GameBoard::applyHorLine
 * Применяет горизонтальную победную линию, убирая необходимые ячейки и прибавляя счет
 * * @param index - индекс первой ячейки, с которой необходимо начать применение
 */
void GameBoard::applyHorLine(int index) {
    setCurrentScore(m_currentScore+pointsForWin);
    for (int i = index; i < index+lehgthWin; ++i) {
        clearCell(i);
    }
}

/**
 * @brief GameBoard::applyVerLine
 * Применяет вертикальную победную линию, убирая необходимые ячейки и прибавляя счет
 * * @param index - индекс первой ячейки, с которой необходимо начать применение
 */
void GameBoard::applyVerLine(int index) {
    setCurrentScore(m_currentScore+pointsForWin);
    for (int i = 0, ind = index; i < lehgthWin; ++i, ind+=maxColumn) {
        clearCell(ind);
    }
}

/**
 * @brief GameBoard::checkAndApplyWinLines
 * Проверяет и применяет победные линии, относительно заданного индекса
 * * @param index - индекс ячейки
 */
void GameBoard::checkAndApplyWinLines(int index) {
    ColorEnum needColor = cells.at(index).color;
    int indexFirstVert = checkVerLine(index, needColor);
    if (indexFirstVert != -1)
        applyVerLine(indexFirstVert);
    int indexFirstHor = checkHorLine(index, needColor);
    if (indexFirstHor != -1)
        applyHorLine(indexFirstHor);
}

/**
 * @brief GameBoard::checkCellIsFree
 * Проверяет свободна ли ячейка
 * * @param index - индекс, по которому расположена ячейка
 * * @return true - если ячейка свободна
 */
bool GameBoard::checkCellIsFree(int index) const {
    return !cells.at(index).isBusy;
}

/**
 * @brief GameBoard::placeCell
 * Размещает новую фигуру но поле
 * * @param index - индекс, где разместить фигуру,
 *          idColor - id цвета фигуры
 */
void GameBoard::placeCell(int indexCell, int idColor) {
    Cell cell;
    cell.setColor(idColor);
    cell.isBusy = true;
    cells.replace(indexCell, cell);
    freeCells.remove(indexCell);
    appDb->updateStatusPosition(indexCell, cell);
    QModelIndex idx = index(indexCell, 0);
    emit dataChanged(idx, idx);
}

/**
 * @brief GameBoard::clearCell
 * Меняет состояние ячейки на свободна
 * * @param indexCell - индекс ячейки
 */
void GameBoard::clearCell(int indexCell) {
    Cell cell = cells.at(indexCell);
    cell.isBusy = false;
    cells.replace(indexCell, cell);
    freeCells.insert(indexCell);
    appDb->updateStatusPosition(indexCell, cell);
    QModelIndex idx = index(indexCell, 0);
    emit dataChanged(idx, idx);
}

/**
 * @brief GameBoard::moveCell
 * Перемещает ячейку
 * * @param indexFrom - индекс, откуда переместить ячейку
 * * @param indexTo - индекс, куда разместить ячейку
 */
void GameBoard::moveCell(int indexFrom, int indexTo) {
    cells.replace(indexTo, cells.at(indexFrom));
    clearCell(indexFrom);
    freeCells.remove(indexTo);
    QModelIndex idx = index(indexTo, 0);
    emit dataChanged(idx, idx);
    appDb->updateStatusPosition(indexTo, cells.at(indexTo));
}

/**
 * @brief GameBoard::makeComputerMove
 * Выполняет ход компьютера, размещая в 3 случайные ячейки
 * по фигуре случайного цвета
 */
void GameBoard::makeComputerMove() {
    QList<int> cellsCompMove;
    for (size_t i = 0; i < 3; ++i) {
        int step = QRandomGenerator::global()->bounded(freeCells.size());
        int idColor = QRandomGenerator::global()->bounded(4) + 1;
        int idCell = *(freeCells.begin()+step);
        placeCell(idCell, idColor);
        cellsCompMove.append(idCell);
    }
    for (int idCell : cellsCompMove) {
        checkAndApplyWinLines(idCell);
    }
    setIsFinal(checkIsFinal());
}

/**
 * @brief GameBoard::checkIsFinal
 * Проверяет - окончена ли игра
 * @return true, если свободных ячеек не осталось
 */
bool GameBoard::checkIsFinal() const {
    return freeCells.count() <= 0;
}

/**
 * @brief GameBoard::checkPossibilityWay
 * Рекурсивная функция определяет, существует ли свободный путь от from до to
 * * @param from - индекс, от которого необхоимо найти путь
 * * @param to - индекс, к которому необходимо найти путь
 * * @param cellsSeen - ссылка, на список, уже посещенных ячеек
 * @return список индексов, соседних, для заданного
 */
bool GameBoard::checkPossibilityWay(int from, int to, QSet<int>& cellsSeen) const {
    if (from == to) {
        return true;
    }
    bool isPossibility = false;
    cellsSeen.insert(from);
    for (int ind : getFreeNeighbourCells(from)) {
        if (!cellsSeen.contains(ind))
            isPossibility = checkPossibilityWay(ind, to, cellsSeen);
        if (isPossibility)
            break;
    }
    return isPossibility;
}

/**
 * @brief GameBoard::getFreeNeighbourCells
 * Возвращает индексы свободных клеток справа, слева, вверху и внизу от заданного индекса
 * * @param index - индекс, соседние клетки которого, необходимо узнать
 * * @return список индексов, соседних, для заданного
 */
QList<int> GameBoard::getFreeNeighbourCells(int index) const {
    QList<int> cellsRes;
    int rInd = getIndexRightNeighbour(index);
    if (rInd != -1 && !cells.at(rInd).isBusy)
        cellsRes.append(rInd);
    int lInd = getIndexLeftNeighbour(index);
    if (lInd != -1 && !cells.at(lInd).isBusy)
        cellsRes.append(lInd);
    int tInd = getIndexTopNeighbour(index);
    if (tInd != -1 && !cells.at(tInd).isBusy)
        cellsRes.append(tInd);
    int bInd = getIndexBottomNeighbour(index);
    if (bInd != -1 && !cells.at(bInd).isBusy)
        cellsRes.append(bInd);
    return cellsRes;
}

int GameBoard::getIndexRightNeighbour(int index) const {
    int ind = index;
    ind += 1;
    if (!isValidPosition(ind) || ind % maxColumn == 0)
        ind = -1;
    return ind;
}

int GameBoard::getIndexLeftNeighbour(int index) const {
    int ind = index;
    ind -= 1;
    if (!isValidPosition(ind) || index % maxColumn == 0)
        ind = -1;
    return ind;
}

int GameBoard::getIndexTopNeighbour(int index) const {
    int ind = index;
    ind -= maxColumn;
    if (!isValidPosition(ind))
        ind = -1;
    return ind;
}

int GameBoard::getIndexBottomNeighbour(int index) const {
    int ind = index;
    ind += maxColumn;
    if (!isValidPosition(ind))
        ind = -1;
    return ind;
}
