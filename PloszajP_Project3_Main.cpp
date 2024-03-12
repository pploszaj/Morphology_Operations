#include <iostream>
#include <fstream>
#include <string>

class Morphology {
public:
    int numImgRows;
    int numImgCols;
    int imgMin;
    int imgMax;
    int numStructRows;
    int numStructCols;
    int structMin;
    int structMax;
    int rowOrigin;
    int colOrigin;
    int rowFrameSize;
    int colFrameSize;
    int extraRows;
    int extraCols;
    int rowSize;
    int colSize;
    int** zeroFramedAry;
    int** morphAry;
    int** tempAry;
    int** structAry;

    Morphology(int imgRows, int imgCols, int imgMinVal, int imgMaxVal, int structRows, int structCols, int structMinVal, int structMaxVal, int originRow, int originCol)
            : numImgRows(imgRows), numImgCols(imgCols), imgMin(imgMinVal), imgMax(imgMaxVal),
              numStructRows(structRows), numStructCols(structCols), structMin(structMinVal), structMax(structMaxVal),
              rowOrigin(originRow), colOrigin(originCol) {


        rowFrameSize = numStructRows / 2;
        colFrameSize = numStructCols / 2;
        extraRows = rowFrameSize * 2;
        extraCols = colFrameSize * 2;
        rowSize = numImgRows + extraRows;
        colSize = numImgCols + extraCols;

        zeroFramedAry = createArray(rowSize, colSize);
        morphAry = createArray(rowSize, colSize);
        tempAry = createArray(rowSize, colSize);
        structAry = createArray(numStructRows, numStructCols);

    }

    ~Morphology() {
        deleteArray(zeroFramedAry, rowSize);
        deleteArray(morphAry, rowSize);
        deleteArray(tempAry, rowSize);
        deleteArray(structAry, numStructRows);
    }

    static int** createArray(int numRows, int numCols) {
        int** array = new int*[numRows + 1];
        for(int i = 0; i <= numRows; i++) {
            array[i] = new int[numCols+1]{};
        }
        return array;
    }

    void deleteArray(int** array, int numRows) {
        for(int i = 0; i < numRows; i++) {
            delete[] array[i];
        }
        delete[] array;
    }

    void zero2DAry(int** Ary, int nRows, int nCols) {
        for(int i = 0; i < nRows; i++){
            for(int j = 0; j < nCols; j++){
                Ary[i][j] = 0;
            }
        }

    }

    void loadImg(std::ifstream& inFile) {
        if(!inFile){
            std::cerr << "File is not open" << std::endl;
            return;
        }

        int pixelVal;

        for (int i = rowOrigin; i < (numImgRows + rowOrigin); i++) {
            for (int j = colOrigin; j < (numImgCols + colOrigin); j++) {
                if (inFile >> pixelVal) {
                    zeroFramedAry[i][j] = pixelVal;
                }
            }
        }

    }

    void loadStruct(std::ifstream& structFile, int** Ary) {
        if (!structFile) {
            std::cerr << "Struct file is not open or valid." << std::endl;
            return;
        }

        int pixelVal;
        for (int i = 0; i < numStructRows; ++i) {
            for (int j = 0; j < numStructCols; ++j) {
                if (structFile >> pixelVal) {
                    Ary[i][j] = pixelVal;
                } else {
                    std::cerr << "Error reading structuring element data." << std::endl;
                    return;
                }
            }
        }
    }

    void computeDilation(int** inAry, int** outAry) {
        for (int i = rowFrameSize; i < numImgRows + rowFrameSize; i++) {
            for (int j = colFrameSize; j < numImgCols + colFrameSize; j++) {
                if (inAry[i][j] > 0) {
                    onePixelDilation(i, j, inAry, outAry);
                }
            }
        }
    }

    void computeErosion(int** inAry, int** outAry) {
        for (int i = rowFrameSize; i < numImgRows + rowFrameSize; i++) {
            for (int j = colFrameSize; j < numImgCols + colFrameSize; j++) {
                if (inAry[i][j] > 0) {
                    onePixelErosion(i, j, inAry, outAry);
                }
            }
        }
    }

    void computeOpening(int** inAry, int** outAry, int** tmpAry) {
        computeErosion(inAry, tmpAry);
        computeDilation(tmpAry, outAry);

    }

    void computeClosing(int** inAry, int** outAry, int** tmpAry) {
        computeDilation(inAry, tmpAry);
        computeErosion(tmpAry, outAry);
    }

    void onePixelDilation(int i, int j, int** inAry, int** outAry) {
        int iOffset = i - rowOrigin;
        int jOffset = j - colOrigin;
        for (int rIndex = 0; rIndex < numStructRows; ++rIndex) {
            for (int cIndex = 0; cIndex < numStructCols; ++cIndex) {
                if(structAry[rIndex][cIndex] > 0){
                    outAry[iOffset + rIndex][jOffset + cIndex] = 1;
                }
            }
        }
    }

    void onePixelErosion(int i, int j, int** inAry, int** outAry) {
        bool matchFlag = true;
        int iOffset = i - rowOrigin;
        int jOffset = j - colOrigin;

        for (int rIndex = 0; rIndex < numStructRows && matchFlag; ++rIndex) {
            for (int cIndex = 0; cIndex < numStructCols && matchFlag; ++cIndex) {
                if (structAry[rIndex][cIndex] > 0 && inAry[iOffset + rIndex][jOffset + cIndex] <= 0) {
                    matchFlag = false;
                }
            }
        }
        if (matchFlag) {
            outAry[i][j] = 1;
        } else {
            outAry[i][j] = 0;
        }
    }

    void aryToFile(int** Ary, std::ofstream& outFile) {
        outFile << numImgRows << " " << numImgCols << " " << imgMin << " " << imgMax << std::endl;

        for (int i = rowFrameSize; i < numImgRows + rowFrameSize; i++) {
            for (int j = colFrameSize; j < numImgCols + colFrameSize; j++) {
                if(Ary[i][j] > 0) {
                    outFile << Ary[i][j] << " ";
                } else {
                    outFile << "." << " ";
                }
            }
            outFile << std::endl;
        }
    }

    void prettyPrint(int** Ary, std::ofstream& outFile, bool isStructArray = false) {
        int rows = isStructArray ? numStructRows : numImgRows + rowFrameSize;
        int cols = isStructArray ? numStructCols : numImgCols + colFrameSize;

        for (int i = isStructArray ? 0 : rowFrameSize; i < rows; i++) {
            for (int j = isStructArray ? 0 : colFrameSize; j < cols; j++) {
                outFile << (Ary[i][j] == 0 ? ". " : "1 ");
            }
            outFile << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 8) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        return 1;
    }

    std::ifstream imgFile(argv[1]);
    std::ifstream structFile(argv[2]);
    std::ofstream dilateOutFile(argv[3]);
    std::ofstream erodeOutFile(argv[4]);
    std::ofstream closingOutFile(argv[5]);
    std::ofstream openingOutFile(argv[6]);
    std::ofstream prettyPrintFile(argv[7], std::ios::app);


    if (!imgFile || !structFile || !dilateOutFile || !erodeOutFile || !closingOutFile || !openingOutFile || !prettyPrintFile) {
        std::cerr << "File open error." << std::endl;
        return 1;
    }

    int numImgRows, numImgCols, imgMin, imgMax;
    imgFile >> numImgRows >> numImgCols >> imgMin >> imgMax;

    int numStructRows, numStructCols, structMin, structMax, rowOrigin, colOrigin;;
    structFile >> numStructRows >> numStructCols >> structMin >> structMax;
    structFile >> rowOrigin >> colOrigin;


    Morphology morphology(numImgRows, numImgCols, imgMin, imgMax, numStructRows, numStructCols, structMin, structMax, rowOrigin, colOrigin);

    morphology.zero2DAry(morphology.zeroFramedAry, morphology.rowSize, morphology.colSize);
    morphology.loadImg(imgFile);
    morphology.prettyPrint(morphology.zeroFramedAry, prettyPrintFile);

    morphology.zero2DAry(morphology.structAry, morphology.numStructRows, morphology.numStructCols);
    morphology.loadStruct(structFile, morphology.structAry);
    morphology.prettyPrint(morphology.structAry, prettyPrintFile, true);

    morphology.zero2DAry(morphology.morphAry, morphology.rowSize, morphology.colSize);
    morphology.computeDilation(morphology.zeroFramedAry, morphology.morphAry);
    morphology.aryToFile(morphology.morphAry, dilateOutFile);
    morphology.prettyPrint(morphology.morphAry, prettyPrintFile);

    morphology.zero2DAry(morphology.morphAry, morphology.rowSize, morphology.colSize);
    morphology.computeErosion(morphology.zeroFramedAry, morphology.morphAry);
    morphology.aryToFile(morphology.morphAry, erodeOutFile);
    morphology.prettyPrint(morphology.morphAry, prettyPrintFile);

    morphology.zero2DAry(morphology.morphAry, morphology.rowSize, morphology.colSize);
    morphology.computeOpening(morphology.zeroFramedAry, morphology.morphAry, morphology.tempAry);
    morphology.aryToFile(morphology.morphAry, openingOutFile);
    morphology.prettyPrint(morphology.morphAry, prettyPrintFile);

    morphology.zero2DAry(morphology.morphAry, morphology.rowSize, morphology.colSize);
    morphology.computeClosing(morphology.zeroFramedAry, morphology.morphAry, morphology.tempAry);
    morphology.aryToFile(morphology.morphAry, closingOutFile);
    morphology.prettyPrint(morphology.morphAry, prettyPrintFile);

    imgFile.close();
    structFile.close();
    dilateOutFile.close();
    erodeOutFile.close();
    closingOutFile.close();
    openingOutFile.close();
    prettyPrintFile.close();

}


