/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktor�w

	@author Tomasz Kilja�czyk
	@version 22/10/18
*/

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#include <math.h>
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>

/*
	!!!UWAGA!!!
	Cz�� funkcji jest zaimplementowana tylko po to, �eby
	testowa� inne funkcje i maj� np sta�e lokalizacje blok�w
	albo tylko tworz� obiekt z danymi i zapisuj�
*/

class FileManager {
private:
	//--------------- Definicje sta�ych statycznych -------------
	static const unsigned int BLOCK_SIZE = 8; //Sta�y rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024;	  //Sta�a pojemno�� dysku (bajty)

	//---------------- Definicje struktur i klas ----------------
	struct File {
		std::string name;  //Nazwa pliku
		unsigned int size; //Rozmiar pliku
		unsigned int FATindex; //Indeks pozycji pocz�tku pliku w tablicy FAT

		/**
			Konstruktor domy�lny.
		*/
		File() {}

		/**
			Konstruktor inicjalizuj�cy pola name i id podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
		*/
		File(const std::string &name_) : name(name_) {};
	};
	struct Directory {
		std::string name; //Nazwa katalogu
		std::unordered_map<std::string, File> files; //Tablica hashowa plik�w w katalogu
		std::unordered_map<std::string, Directory>subDirectories; //Tablica hashowa podkatalog�w
		Directory* parentDirectory; //Wska�nik na katalog nadrz�dny

		Directory() {}
		Directory(const std::string &name_) : name(name_), parentDirectory(NULL) {}
	};

	class Disk {
	public:
		struct FAT {
			std::bitset<DISK_CAPACITY / BLOCK_SIZE> bitVector; //Wektor bitowy blok�w (0 - wolny blok, 1 - zaj�ty blok)
			/*
			Zawiera indeksy blok�w dysku na dysku, na kt�rych znajduj� si� pofragmentowane dane pliku.
			Indeks odpowiada rzeczywistemu blokowi dyskowemu, a jego zawarto�ci� jest indeks nast�pnego bloku lub NULL
			*/
			std::array<unsigned int, DISK_CAPACITY / BLOCK_SIZE>FileAllocationTable = { NULL };
			Directory rootDirectory{ Directory("root") }; //Katalog g��wny
			unsigned int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o ilo�ci wolnego miejsca na dysku (bajty)
		} FAT; //System plik�w
		//Tablica reprezentuj�ca przestrze� dyskow� (jeden indeks - jeden bajt)
		std::array<char, DISK_CAPACITY> space;

		//----------------------- Konstruktor -----------------------
		/**
			Konstruktor domy�lny. Wykonuje zape�nienie przestrzeni dyskowej warto�ci� NULL
		*/
		Disk();

		//-------------------------- Metody -------------------------
		/**
			Zapisuje dane (string) na dysku od indeksu 'begin' do indeksu 'end' w��cznie.

			@param begin Indeks od kt�rego dane maj� by� zapisywane.
			@param end Indeks na kt�rym zapisywanie danych ma si� zako�czy�.
			@param data Dane typu string.
			@return void.
		*/
		void write(const unsigned int &begin, const unsigned int &end, const std::string &data);

		/**
			Zapisuje dane (unsigned int) pod wskazanym indeksem.

			@param index Indeks na kt�rym zapisana zostanie liczba.
			@param data Liczba typu unsigned int.
			@return void.
		*/
		void write(const unsigned int &index, const unsigned int &data);

		/**
			Zapisuje dane typu bitset od wskazanego indeksu.

			@param index Indeks od kt�rego dane maj� by� zapisywane.
			@param data Bitset o dowolnym.
			@return void.
		*/
		template<unsigned int size>
		void write(const unsigned int &begin, const std::bitset<size> &data);

		/**
			Odczytuje dane typu dowolnego w wskazanym przedziale.

			@param begin Indeks od kt�rego dane maj� by� odczytywane.
			@param end Indeks do kt�rego dane maj� by� odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const unsigned int &begin, const unsigned int &end);
	} DISK; //Prosta klasa dysku (fizycznego)
	//Struktura pliku
	//Struktura katalogu
	

	//------------------- Definicje zmiennych -------------------
	Directory* currentDirectory; //Obecnie u�ytkowany katalog

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domy�lny. Przypisuje do obecnego katalogu katalog g��wny.
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------

	//Tworzy plik
	void CreateFile(const std::string &name, const std::string &data);

	//Otwiera plik
	const std::string OpenFile(const unsigned int &id);

	//Usuwa plik (ca�kowicie wymazuje)
	void DeleteFile(const std::string &name);

	//Usuwa plik (usuwa go z tablicy FAT)
	void TruncateFile();

	//Tworzy nowy katalog
	void CreateDirectory(const std::string &name);

	//Zmienia obecny katalog na katalog nadrz�dny
	void CurrentDirectoryUp();

	//Zmienia obecny katalog na katalog podrz�dny o podanej nazwie
	void CurrentDirectoryDown(const std::string &name);

	//--------------------- Dodatkowe metody --------------------

	//Zmienia obecny katalog na katalog g��wny
	void CurrentDirectoryRoot();

	//------------------ Metody do wy�wietlania -----------------

	//Wy�wietla struktur� katalog�w
	void DisplayDirectoryStructure();
	//Wy�wietla rekurencyjnie katalog i jego podkatalogi
	void DisplayDirectory(const Directory &directory, unsigned int level);

	//Wy�wietla zawarto�� dysku w formie binarnej
	void DisplayDiskContentBinary();

	//Wy�wietla zawarto�� dysku w formie znak�w
	void DisplayDiskContentChar();

	//Wy�wietla Tablic� Alokacji Plik�w
	void DisplayFileAllocationTable();

	//Wy�wietla wektor bitowy
	void DisplayBitVector();

	//Wy�wietla fragmenty pliku
	void DisplayFileFragments(const std::vector<std::string> &fileFragments);

private:
	//-------------------- Metody Pomocnicze --------------------

	//Zwraca obecnie u�ywan� �cie�k�
	const std::string GetCurrentPath();

	//Sprawdza czy nazwa pliku jest u�yta w danym katalogu
	const bool CheckIfNameUnused(const Directory &directory, const std::string &name);

	//Sprawdza czy jest miejsce na dane o zadaniej wielko�ci
	const bool CheckIfEnoughSpace(const unsigned int &dataSize);

	//Zmienia warto�� w tablicy bitowej blok�w i zapisuje zmian� na dysku
	void ChangeBlockMapValue(const unsigned int &block, const bool &value);

	//Zapisuje wektor fragment�w File.data na dysku
	void WriteFile(const File &file, const std::string &data);

	//Konwertuje string na form� gotow� do zapisania na dysku
	const std::vector<std::string> DataToDataFragments(const std::string &str);

	//Oblicza ile blok�w zajmie podany string
	const unsigned int CalculateNeededBlocks(const std::string &data);

	//Znajduje nieu�ywane bloki do zapisania pliku;
	const std::vector<unsigned int> FindUnallocatedBlocks(unsigned int blockCount);
};

#endif //SEXYOS_FILEMANAGER_H
