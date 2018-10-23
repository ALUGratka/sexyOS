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
	class Disk {
	public:
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
	struct FileFAT {
		std::string name;  //Nazwa pliku
		unsigned int size; //Rozmiar pliku
		std::vector<unsigned int>occupiedBlocks; //Bloki zajmowane przez plik
		std::string data;

		/**
			Konstruktor domy�lny.
		*/
		FileFAT() {}

		/**
			Konstruktor inicjalizuj�cy pola name i id podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
		*/
		FileFAT(const std::string &name_) : name(name_) {};

		/**
			Konstruktor inicjalizuj�cy pola name, id i data podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
			@param data_ Dane typu string zapisane w pliku
		*/
		FileFAT(const std::string &name_, const std::string &data_) : name(name_), data(data_) {}
	};
	//Struktura katalogu
	struct Directory {
		std::string name; //Nazwa katalogu
		std::unordered_map<std::string, FileFAT> FAT; //Tablica hashowa plik�w w katalogu
		std::unordered_map<std::string, Directory>subDirectories; //Tablica hashowa podkatalog�w

		Directory() {}
		Directory(const std::string &name_) : name(name_) {}
	};

	//------------------- Definicje zmiennych -------------------
	std::bitset<DISK_CAPACITY / BLOCK_SIZE> blockMap; //Tablica bitowa blok�w (0 - wolny, 1 - zaj�ty)
	Directory rootDirectory{ Directory("root") }; //Katalog g��wny
	unsigned int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o ilo�ci wolnego miejsca na dysku (bajty)

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domy�lny. Rezerwuje bloki potrzebne do zapisania
		tablicy bitowej blok�w oraz zapisuje t� tablic� na dysku
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------

	//Tworzy plik
	void CreateFile(const std::string &name);
	void CreateFile(const std::string &name, const std::string &data);

	//Otwiera plik
	const std::string OpenFile(const unsigned int &id);

	//Usuwa plik (ca�kowicie wymazuje)
	void DeleteFile();

	//Usuwa plik (usuwa go z tablicy FAT)
	void TruncateFile();

	//------------------ Metody do wy�wietlania -----------------

	//Wy�wietla struktur� katalog�w
	void DisplayDirectoryStructure();

	//Wy�wietla zawarto�� dysku w formie binarnej
	void DisplayDiskContentBinary();

	//Wy�wietla zawarto�� dysku w formie znak�w
	void DisplayDiskContentChar();

	//Wy�wietla tablic� blok�w
	void DisplayBlocks();

	//Wy�wietla fragmenty pliku
	void DisplayFileFragments(const std::vector<std::string> &fileFragments);

private:
	//-------------------- Metody Pomocnicze --------------------

	//Sprawdza czy nazwa pliku jest u�yta w danym katalogu
	const bool CheckIfNameUnused(const Directory &directory, const std::string &name);

	//Sprawdza czy jest miejsce na dane o zadaniej wielko�ci
	const bool CheckIfEnoughSpace(const unsigned int &dataSize);

	//Zmienia warto�� w tablicy bitowej blok�w i zapisuje zmian� na dysku
	void ChangeBlockMapValue(const unsigned int &block, const bool &value);

	//Zapisuje wektor FileFAT na dysku
	void WriteFile(const FileFAT &file);

	//Konwertuje kompletny plik na form� do zapisania na dysku
	const std::vector<std::string> FileFATToFileFragments(const FileFAT &fileFAT);

	//Oblicza ile blok�w zajmie podany string
	const unsigned int CalculateNeededBlocks(const std::string &data);

	//Znajduje nieu�ywane bloki do zapisania pliku;
	std::vector<unsigned int> FindUnallocatedBlocks(unsigned int blockCount);
};

#endif //SEXYOS_FILEMANAGER_H
