#include<Windows.h>
#include<iostream>
#include<fstream>
#include <cstdlib>
#include <ctime>

using namespace std;


ifstream f_in("G:\\A.txt");
ofstream f_out("G:\\B.txt");
const int k_length = 5;
char *buf = new char[k_length + 1];
char c;
char k[] = "qwert";
bool file_ended = false;
bool last_string = false;
int rowNumber, threadNumber, numberNumber;
int **hashTable;

int* masRandom;
int numberRandom = 0;

HANDLE *hRow;
HANDLE *hThread;

class CMonitor {

    int *occupated_rows;

public:

    int *getOccupated_rows() const {
        return occupated_rows;
    }

    void setOccupated_rows(int n) {
        CMonitor::occupated_rows = new int[n];
        for (int i = 0; i < n; ++i) {
            occupated_rows[i]=0;
        }
    }

    void OcupateRow(int row_number) {
        WaitForSingleObject(hRow[row_number], INFINITE);
    };

    void FreeRow(int row_number) {
        ++occupated_rows[row_number];
        ReleaseMutex(hRow[row_number]);
    };
};

CMonitor cMonitor;

HANDLE hStringRead, // мьютекс "строка считана"
        hStringEncryp, // мьютекс "строка зашифрована"
        hStringWrite, // мьютекс "строка записана"
        hBuffer; // мьютекс доступа к буферу


DWORD WINAPI encrypt_string(LPVOID param);

DWORD WINAPI write_string(LPVOID param);

DWORD WINAPI read_string(LPVOID param) {
    c = f_in.get();
    int i;
//    ReleaseMutex(hStringWrite);
    while (c != EOF) {
        i = 0;
        WaitForSingleObject(hStringWrite, INFINITE);
        cout << "Reader has started\n";
//        WaitForSingleObject(hBuffer, INFINITE);
        buf[i] = c;
        i++;
        while ((c = f_in.get()) != EOF && i < k_length - 1) {
            buf[i] = c;
            i++;
        }
        if (c != EOF) {
            buf[i] = c;
            i++;
        }
        buf[i] = '\0';
        c = f_in.get();
        if (c == EOF)
            last_string = true;
        cout << ReleaseMutex(hStringRead) << " readed\n";
        cout << GetLastError() << " error\n";
    }

    return 0;
}

DWORD WINAPI encrypt_string(LPVOID param) {
    int j;
    while (true) // kostil
    {
        j = 0;
        WaitForSingleObject(hStringRead, INFINITE);
        cout << "Encrypter has started\n";
        while (buf[j] != '\0') {
            buf[j] = (buf[j] ^ k[j]);
            j++;
        }
        cout << ReleaseMutex(hStringEncryp) << " encrypted\n";
        if (last_string)
            return 0;
    }
}

DWORD WINAPI write_string(LPVOID param) {
    int t;
    while (true) // kostil
    {
        t = 0;
        WaitForSingleObject(hStringEncryp, INFINITE);
        cout << "Writer has started\n";
//        WaitForSingleObject(hBuffer, INFINITE);
        while (buf[t] != '\0') {
            f_out << buf[t];
            t++;
        }
//        ReleaseMutex(hBuffer);
        cout << ReleaseMutex(hStringWrite) << " writen\n";
        if (last_string)
            return 0;
    }
}

DWORD WINAPI digitGeneration(LPVOID param) {
    cout << "Запустился поток\n";
    for (int i = 0; i < numberNumber; ++i) {
        int digit = masRandom[numberRandom++];
        int row = digit % rowNumber;
        cMonitor.OcupateRow(row);
        cout << "["<<(int)param<<"]Закинулось число "<<digit<<" в ряд " << row << " столбец " << cMonitor.getOccupated_rows()[row] << "\n";
        hashTable[row][cMonitor.getOccupated_rows()[row]] = digit;
        cMonitor.FreeRow(row);
    }
    return 0;
}

int main() {
    srand(time(NULL));
    // добавить хэндлеров потоков, открыть нужжные мьютексы заранее, запустить потоки, дождаться завершения потоков, закрыть потоки
    setlocale(LC_ALL, "rus");
//    hStringRead = CreateMutex(NULL, false, "read");
//    hStringEncryp = CreateMutex(NULL, true, "encrypt");
//    hStringWrite = CreateMutex(NULL, true, "write");
    //cout<<ReleaseMutex(hStringWrite)<<" writen\rowNumber";
    //CloseHandle(hStringEncryp);
    //CloseHandle(hStringRead);

    cout << "Введите количество строк в хеш-таблице: ";
    cin >> rowNumber;
    cout << endl;
    cout << "Введите количество потоков: ";
    cin >> threadNumber;
    cout << endl;
    cout << "Введите количество чисел, генерируемых каждым потоком:";
    cin >> numberNumber;
    cout << endl;



    cMonitor.setOccupated_rows(rowNumber);
    const int maxValue = rowNumber * threadNumber;
    hashTable = new int *[rowNumber];
    for (int i = 0; i < rowNumber; ++i) {
        hashTable[i] = new int[maxValue];
    }
    masRandom = new int[maxValue];
    for (int i=0;i<maxValue;++i)
        masRandom[i]=rand();


    hRow = new HANDLE[rowNumber];
    hThread = new HANDLE[threadNumber];
    for (int i = 0; i < rowNumber; ++i)
        hRow[i] = CreateMutex(NULL, false, NULL);
    for (int i = 0; i < threadNumber; ++i) {
        hThread[i] = CreateThread(NULL, 0, digitGeneration, (LPVOID)i, 0, NULL);
    }


    for (int i = 0; i < threadNumber; ++i) {
        WaitForSingleObject(hThread[i], INFINITE);
    }

    for (int i = 0; i < rowNumber; ++i) {
        cout<<"------- ";
        for (int j = 0; j < cMonitor.getOccupated_rows()[i]; ++j) {
            cout << hashTable[i][j] << "\t";
        }
        cout << endl;
    }

    for (int i = 0; i < threadNumber; ++i)
        CloseHandle(hThread[i]);
    for (int i = 0; i < rowNumber; ++i)
        CloseHandle(hRow[i]);


    cout << "Goodbye!" << endl;
    return 0;
}