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
	static const size_t DISK_SIZE = 1024;	  //Sta�y rozmiar dysku (bajty)

	//---------------- Definicje struktur i klas ----------------
	class Disk {
	public:
		//Tablica reprezentuj�ca przestrze� dyskow� (jeden indeks - jeden bajt)
		std::array<char, DISK_SIZE> space;

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
	//Struktura pliku (fizyczna)
	struct FileFAT {
		//-------------------- Definicje sta�ych --------------------
		const std::string DATA;		   //Dane typu string
		const unsigned int NEXT_FRAGM; //Wskazanie na kolejny fragment pliku
									   //w przestrzeni dyskowej

		//----------------------- Konstruktor -----------------------
		/**
			Konstruktor domy�lny. Inicjalizuje pola data i nextPart warto�ci� NULL.
		*/
		FileFAT() : DATA(NULL), NEXT_FRAGM(NULL) {}
		/**
			Konstruktor inicjalizuj�cy pola data i nextPart podanymi zmiennymi.

			@param data_ Zmienna typu string do przechowania tekstu.
			@param nextPart_ Zmienna typu unsigned int b�d�ca wskazaniem na indeks
			kolejnej cz�ci pliku w przestrzeni dyskowej. J
		*/
		FileFAT(const std::string &data, const unsigned int nextFragm) : DATA(data), NEXT_FRAGM(nextFragm) {}
	};
	//Struktura pliku (logiczna)
	struct File {
		std::string name;  //Nazwa pliku
		unsigned int id;   //Numer identyfikacyjny pliku
		unsigned int size; //Rozmiar pliku
		std::string data;

		/**
			Konstruktor domy�lny.
		*/
		File() {}
		/**
			Konstruktor inicjalizuj�cy pola name i id podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
		*/
		File(const std::string &name_, const unsigned int &id_) : name(name_), id(id_) {}
		/**
			Konstruktor inicjalizuj�cy pola name, id i data podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
			@param data_ Dane typu string zapisane w pliku
		*/
		File(const std::string &name_, const unsigned int &id_, const std::string &data_) : name(name_), id(id_), data(data_) {}
	};

	//------------------- Definicje zmiennych -------------------
	std::bitset<DISK_SIZE / BLOCK_SIZE> clusterMap; //Tablica bitowa blok�w (0 - wolny, 1 - zaj�ty)

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domy�lny. Rezerwuje bloki potrzebne do zapisania
		tablicy bitowej blok�w oraz zapisuje t� tablic� na dysku
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------
	//Tworzy plik
	void CreateFile(const std::string &name, const unsigned int &id);
	void CreateFile(const std::string &name, const unsigned int &id, const std::string &data);
	//Otwiera plik
	const std::string OpenFile(const unsigned int &id);
	//Usuwa plik (ca�kowicie wymazuje)
	void DeleteFile();
	//Usuwa plik (usuwa go z tablicy FAT)
	void TruncateFile();

	//------------------ Metody do wy�wietlania -----------------
	//Wy�wietla zawarto�� dysku w formie binarnej
	void DisplayDiskContentBinary();
	//Wy�wietla zawarto�� dysku w formie znak�w
	void DisplayDiskContentChar();
	//Wy�wietla tablic� blok�w
	void DisplayBlocks();
	//Wy�wietla fileFAT
	void DisplayFileFAT(const std::vector<FileFAT> &fileFAT);

private:
	//-------------------- Metody Pomocnicze --------------------
	//Zmienia warto�� w tablicy bitowej blok�w i zapisuje zmian� na dysku
	void ChangeClusterMapValue(const unsigned int &block, const bool &value);

	//Zapisuje wektor FileFAT na dysku
	void WriteFileFAT(const unsigned int &begin, const std::vector<FileFAT> &fileFAT);
	//Konwertuje kompletny plik na form� do zapisania na dysku
	std::vector<FileFAT> FileToFileFAT(const File &file);
	//Oblicza ile blok�w zajmie podany string (uwzgl�dniaj�c cz�� FAT)
	const unsigned int CalculateNeededBlocks(const std::string &data);
	//Znajduje nieu�ywane bloki do zapisania pliku;
	std::vector<unsigned int> FindUnallocatedBlocks(unsigned int blockCount);
};

#endif //SEXYOS_FILEMANAGER_H