/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktor�w dla klas z FileManager.h

	@author Tomasz Kilja�czyk
	@version 24/10/18
*/

#include "FileManager.h"
#include<algorithm>
#include<iomanip>

//Operator do wy�wietlania czasu z dat�
std::ostream& operator << (std::ostream &os, const tm &time) {
	os << time.tm_hour << ':' << time.tm_min << ' ' << time.tm_mday << '.' << time.tm_mon << '.' << time.tm_year;
	return os;
}

//--------------------------- Dysk --------------------------

FileManager::Disk::Disk() {
	//Zape�nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	fill(space.begin(), space.end(), NULL);
}

FileManager::Disk::FAT::FAT() {
	std::fill(FileAllocationTable.begin(), FileAllocationTable.end(), -1);
}

void FileManager::Disk::write(const unsigned int &begin, const unsigned int &end, const std::string &data) {
	//Indeks kt�ry b�dzie s�u�y� do wskazywania na kom�rki pami�ci
	unsigned int index = begin;
	//Iterowanie po danych typu string i zapisywanie znak�w na dysku
	for (unsigned int i = 0; i < data.size() && i <= end - begin; i++) {
		space[index] = data[i];
		index++;
	}
	//Zapisywanie NULL, je�li dane nie wype�ni�y ostatniego bloku
	for (; index <= end; index++) {
		space[index] = NULL;
	}
}

void FileManager::Disk::write(const unsigned int &index, const unsigned int &data) {
	//Zapisz liczb� pod danym indeksem
	space[index] = data;
}

template<typename T>
const T FileManager::Disk::read(const unsigned int &begin, const unsigned int &end) {
	//Dane
	T data;

	//Je�li typ danych to string
	if (typeid(T) == typeid(std::string)) {
		//Odczytaj przestrze� dyskow� od indeksu begin do indeksu end
		for (unsigned int index = begin; index <= end; index++) {
			//Dodaj znak zapisany na dysku do danych
			data += space[index];
		}
	}

	return data;
}

//----------------------- FileManager  ----------------------

FileManager::FileManager() {
	//Przypisanie katalogu g��wnego do obecnego katalogu 
	currentDirectory = &DISK.FAT.rootDirectory;
}

//-------------------- Podstawowe Metody --------------------

void FileManager::CreateFile(const std::string &name, const std::string &data) {
	//Rozmiar pliku obliczony na podstawie podanych danych
	const unsigned int fileSize = CalculateNeededBlocks(data)*BLOCK_SIZE;

	//Je�li plik si� zmie�ci i nazwa nie u�yta
	if (CheckIfEnoughSpace(fileSize) && CheckIfNameUnused(*currentDirectory, name)) {
		//Stw�rz plik o podanej nazwie
		File file = File(name);
		//Zapisz w pliku jego rozmiar
		file.size = fileSize;

		//Zapisywanie daty stworzenia pliku
		time_t tt;
		time(&tt);
		file.creationTime = *localtime(&tt);
		file.creationTime.tm_year += 1900;
		file.creationTime.tm_mon += 1;
		file.modificationTime = file.creationTime;

		//Lista indeks�w blok�w, kt�re zostan� zaalokowane na potrzeby pliku
		const std::vector<unsigned int> blocks = FindUnallocatedBlocks(file.size / BLOCK_SIZE);

		//Wpisanie blok�w do tablicy FAT
		for (unsigned int i = 0; i < blocks.size() - 1; i++) {
			DISK.FAT.FileAllocationTable[blocks[i]] = blocks[i + 1];
		}

		//Dodanie do pliku indeksu pierwszego bloku na kt�rym jest zapisany
		file.FATindex = blocks[0];

		//Dodanie pliku do obecnego katalogu
		currentDirectory->files[file.name] = file;

		//Zapisanie danych pliku na dysku
		WriteFile(file, data);

		std::cout << "Stworzono plik o nazwie '" << file.name << "' w �cie�ce '" << GetCurrentPath() << "'.\n";
		return;
	}
	//Je�li plik si� nie mie�ci
	if (!CheckIfEnoughSpace(fileSize)) {
		std::cout << "Za ma�o miejsca!\n";
	}
	//Je�li nazwa u�yta
	if (!CheckIfNameUnused(*currentDirectory, name)) {
		std::cout << "Nazwa pliku '" << name << "' ju� zaj�ta!\n";
	}

}

//!!!!!!!!!! NIEDOKO�CZONE !!!!!!!!!!
const std::string FileManager::OpenFile(const std::string &name) {
	return DISK.read<std::string>(0 * 8, 4 * 8 - 1);
}
//!!!!!!!!!! NIEDOKO�CZONE !!!!!!!!!!

const std::string FileManager::GetFileData(const File &file) {
	//Dane
	std::string data;
	//Indeks do wczytywania danych z dysku
	unsigned int index = file.FATindex;
	//Dop�ki nie natrafimy na koniec pliku
	while (index != -1) {
		//Dodaje do danych fragment pliku pod wskazanym indeksem
		data += DISK.read<std::string>(index*BLOCK_SIZE, (index + 1)*BLOCK_SIZE - 1);
		//Przypisuje indeksowi kolejny indeks w tablicy FAT
		index = DISK.FAT.FileAllocationTable[index];
	}
	return data;
}

void FileManager::DeleteFile(const std::string &name) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);

	//Je�li znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		//Zmienna do tymczasowego przechowywania kolejnego indeksu
		unsigned int tempIndex;
		//Obecny indeks
		unsigned index = fileIterator->second.FATindex;
		//Je�li indeks na co� wskazuje
		while (index != -1) {
			//Spisz kolejny indeks
			tempIndex = DISK.FAT.FileAllocationTable[index];
			//Oznacz obecny indeks jako wolny
			DISK.FAT.bitVector[index] = 0;
			//Obecny indeks w tablicy FAT wskazuje na nic
			DISK.FAT.FileAllocationTable[index] = -1;
			//Przypisz do obecnego indeksu kolejny indeks
			index = tempIndex;
		}
		//Usu� plik z obecnego katalogu
		currentDirectory->files.erase(fileIterator);

		std::cout << "Usuni�to plik o nazwie '" << name << "' znajduj�cy si� w �cie�ce '" + GetCurrentPath() + "'.\n";
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w �cie�ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::TruncateFile(const std::string &name, const unsigned int &size) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);
	//Je�li znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		if (size <= fileIterator->second.size - BLOCK_SIZE) {
			//Zmienna do tymczasowego przechowywania kolejnego indeksu
			unsigned int tempIndex;
			//Zmienna do analizowania, ju� mo�na usuwa� cz�� pliku
			unsigned int currentSize = 0;
			//Obecny indeks
			unsigned index = fileIterator->second.FATindex;
			//Je�li indeks na co� wskazuje
			while (index != -1) {
				//Zwi�ksz obecny rozmiar o rozmiar jednostki alokacji
				currentSize += BLOCK_SIZE;
				//Spisz kolejny indeks
				tempIndex = DISK.FAT.FileAllocationTable[index];

				if (currentSize > size) {
					//Oznacz obecny indeks jako wolny
					DISK.FAT.bitVector[index] = 0;
					//Obecny indeks w tablicy FAT wskazuje na nic
					DISK.FAT.FileAllocationTable[index] = -1;
				}
				//Przypisz do obecnego indeksu kolejny indeks
				index = tempIndex;
			}

			std::cout << "Zmniejszono plik o nazwie '" << name << "' do rozmiaru " << size << ".\n";
		}
		else { std::cout << "Podano niepoprawny rozmiar!\n"; }
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w �cie�ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::CreateDirectory(const std::string &name) {
	//Je�li w katalogu nie istnieje podkatalog o podanej nazwie
	if (currentDirectory->subDirectories.find(name) == currentDirectory->subDirectories.end()) {
		//Do podkatalog�w obecnego katalogu dodaj nowy katalog o podanej nazwie
		currentDirectory->subDirectories[name] = Directory(name, &(*currentDirectory));
		std::cout << "Stworzono katalog o nazwie '" << currentDirectory->subDirectories[name].name
			<< "' w �cie�ce '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Nazwa katalogu '" << name << "' zaj�ta!\n"; }
}

void FileManager::CurrentDirectoryUp() {
	//Je�li istnieje katalog nadrz�dny
	if (currentDirectory->parentDirectory != NULL) {
		//Przej�cie do katalogu nadrz�dnego
		currentDirectory = currentDirectory->parentDirectory;
		std::cout << "Obecna �cie�ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Jeste� w katalogu g��wnym!\n"; }
}

void FileManager::CurrentDirectoryDown(const std::string &name) {
	//Je�li w obecnym katalogu znajduj� si� podkatalogi
	if (currentDirectory->subDirectories.find(name) != currentDirectory->subDirectories.end()) {
		//Przej�cie do katalogu o wskazanej nazwie
		currentDirectory = &(currentDirectory->subDirectories.find(name)->second);
		std::cout << "Obecna �cie�ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Brak katalogu o podanej nazwie!\n"; }
}

//--------------------- Dodatkowe metody --------------------

void FileManager::CurrentDirectoryRoot() {
	while (currentDirectory->parentDirectory != NULL) {
		CurrentDirectoryUp();
	}
}

//------------------ Metody do wy�wietlania -----------------

void FileManager::DisplayFileInfo(const std::string &name) {
	auto fileIterator = currentDirectory->files.find(name);
	if (fileIterator != currentDirectory->files.end()) {
		File file = fileIterator->second;
		std::cout << "Name: " << file.name << '\n';
		std::cout << "Size: " << file.size << '\n';
		std::cout << "Created: " << file.creationTime << '\n';
		std::cout << "FAT index: " << file.FATindex << '\n';
		std::cout << "Saved data: " << GetFileData(file) << '\n';
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w �cie�ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DisplayDirectoryStructure() {
	DisplayDirectory(DISK.FAT.rootDirectory, 1);
}
void FileManager::DisplayDirectory(const Directory &directory, unsigned int level) {
	std::cout << std::string(level, ' ') << directory.name << "\\\n";
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		std::cout << std::string(level + 1, ' ') << "- " << i->first << '\n';
	}
	level++;
	for (auto i = directory.subDirectories.begin(); i != directory.subDirectories.end(); i++) {
		DisplayDirectory(i->second, level);
	}
}

void FileManager::DisplayDiskContentBinary() {
	unsigned int index = 0;
	for (const char &c : DISK.space) {
		//bitset - tablica bitowa
		std::cout << std::bitset<8>(c) << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 16 == 15 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayDiskContentChar() {
	unsigned int index = 0;
	for (const char &c : DISK.space) {
		if (c == ' ') { std::cout << ' '; }
		else if (c >= 0 && c <= 32) std::cout << ".";
		else std::cout << c;
		std::cout << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 32 == 31 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayFileAllocationTable() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < DISK.FAT.FileAllocationTable.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << std::setfill('0') << std::setw(3) << (DISK.FAT.FileAllocationTable[i] != -1 ? std::to_string(DISK.FAT.FileAllocationTable[i]) : "NUL")
			<< (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayBitVector() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << DISK.FAT.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayFileFragments(const std::vector<std::string> &fileFragments) {
	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		std::cout << fileFragments[i] << std::string(BLOCK_SIZE - 1 - fileFragments[i].size(), ' ') << '\n';
	}
}

//-------------------- Metody Pomocnicze --------------------

const std::string FileManager::GetCurrentPath() {
	//�cie�ka
	std::string path;
	//Tymczasowa zmienna przechowuj�ca wska�nik na katalog
	Directory* tempDir = currentDirectory;
	//Dop�ki nie doszli�my do pustego katalogu
	while (tempDir != NULL) {
		//Dodaj do �cie�ki od przodu nazw� obecnego katalogu
		path.insert(0, "/" + tempDir->name);
		//Przypisanie tymczasowej zmiennej katalog wy�szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return path;
}

const bool FileManager::CheckIfNameUnused(const Directory &directory, const std::string &name) {
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		//Je�li nazwa ta sama
		if (i->first == name) { return false; }
	}
	return true;
}

const bool FileManager::CheckIfEnoughSpace(const unsigned int &dataSize) {
	//Je�li dane si� mieszcz�
	if (dataSize <= DISK.FAT.freeSpace) { return true; }
	//Je�li dane si� nie mieszcz�
	else { return false; }
}

void FileManager::ChangeBitVectorValue(const unsigned int &block, const bool &value) {
	//Je�li warto�� zaj�ty to wolne miejsce - BLOCK_SIZE
	if (value == 1) { DISK.FAT.freeSpace -= BLOCK_SIZE; }
	//Je�li warto�� wolny to wolne miejsce + BLOCK_SIZE
	else if (value == 0) { DISK.FAT.freeSpace += BLOCK_SIZE; }
	//Przypisanie blokowi podanej warto�ci
	DISK.FAT.bitVector[block] = value;
}

void FileManager::WriteFile(const File &file, const std::string &data) {
	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>fileFragments = DataToDataFragments(data);
	//Index pod kt�rym maj� zapisywane by� dane
	unsigned int index = file.FATindex;

	//Zapisuje wszystkie dane na dysku
	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		//Zapisuje fragment na dysku
		DISK.write(index * BLOCK_SIZE, index * BLOCK_SIZE + fileFragments[i].size() - 1, fileFragments[i]);
		//Zmienia warto�� bloku w wektorze bitowym na zaj�ty
		ChangeBitVectorValue(index, 1);
		//Przypisuje do indeksu numer kolejnego bloku
		index = DISK.FAT.FileAllocationTable[index];
	}
}

const std::vector<std::string> FileManager::DataToDataFragments(const std::string &data) {
	//Tablica fragment�w podanych danych
	std::vector<std::string>fileFragments;
	//Pocz�tek cz�ci danych, u�ywany podczas dzielenia danych
	unsigned int substrBegin = 0;

	//Przetworzenie ca�ych danych
	for (unsigned int i = 0; i < CalculateNeededBlocks(data); i++) {
		//Oblicza pocz�tek kolejnej cz�ci fragmentu danych.
		substrBegin = i * BLOCK_SIZE;
		//Dodaje do tablicy fragment�w kolejny fragment o d�ugo�ci BLOCK_SIZE
		fileFragments.push_back(data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

const unsigned int FileManager::CalculateNeededBlocks(const std::string &data) {
	/*
	Przybli�enie w g�r� rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa�, je�li zape�nia chocia� o jeden bajt
	wi�cej przy zaj�tym bloku, to trzeba zaalokowa� wtedy kolejny blok.
	*/
	return (int)ceil((double)data.size() / (double)BLOCK_SIZE);
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocksFragmented(unsigned int blockCount) {
	//Lista wolnych blok�w
	std::vector<unsigned int> blockList;

	//Szuka wolnych blok�w
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		//Je�li blok wolny
		if (DISK.FAT.bitVector[i] == 0) {
			//Dodaje indeks bloku
			blockList.push_back(i);
			//Potrzeba teraz jeden blok mniej
			blockCount--;
			//Je�li potrzeba 0 blok�w, przerwij
			if (blockCount == 0) { break; }
		}
	}
	blockList.push_back(-1);
	return blockList;
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocksBestFit(const unsigned int &blockCount) {
	//Lista indeks�w blok�w (dopasowanie)
	std::vector<unsigned int> blockList;
	//Najlepsze dopasowanie
	std::vector<unsigned int> bestBlockList(DISK.FAT.bitVector.size() + 1);
	//Lista dopasowa�
	std::vector<std::vector<unsigned int>> blockLists;

	//Szukanie wolnych blok�w spe�niaj�cych minimum miejsca
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		//Je�li blok wolny
		if (DISK.FAT.bitVector[i] == 0) {
			//Dodaj indeks bloku do listy blok�w
			blockList.push_back(i);
		}
		//Je�li blok zaj�ty
		else {
			//Je�li uzyskana lista blok�w jest wi�ksza od ilo�ci blok�w jak� chcemy uzyska�
			//to dodaj uzyskane dopasowanie do listy dopasowa�;
			if (blockList.size() >= blockCount) { blockLists.push_back(blockList); }

			//Czy�ci list� blok�w, aby mo�na przygotowa� kolejne dopasowanie
			blockList.clear();
		}
	}

	/*
	Je�li zdarzy si�, �e ostatni blok w wektorze bitowym jest wolny, to
	ostatnie dopasownie nie zostanie dodane do listy dopasowa�, dlatego
	trzeba wykona� poni�szy kod. Je�li ostatni blok w wektorze bitowym
	b�dzie zaj�ty to blockList b�dzie pusty i nie spie�ni warunku
	*/
	if (blockList.size() >= blockCount) { blockLists.push_back(blockList); }
	blockList.clear();

	//Je�li znaleziono dopasowania (rozmiar > 0)
	if (blockLists.size() > 0) {
		//Szuka najlepszego dopasowania z znalezionych dopasowa�
		for (const std::vector<unsigned int> &v : blockLists) {
			//Je�li obecne dopasowanie jest lepsze od poprzedniego
			if (v.size() < bestBlockList.size()) {
				bestBlockList = v;
			}
		}

		//Ucina najlepsze dopasowanie do ilo�ci blok�w jakie chcemy zaalokowa�
		bestBlockList.resize(blockCount);
	}

	return bestBlockList;
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocks(const unsigned int &blockCount) {
	//Szuka blok�w funkcj� z metod� best-fit
	std::vector<unsigned int> blockList = FindUnallocatedBlocksBestFit(blockCount);

	//Je�li funkcja z metod� best-fit nie znajdzie dopasowa�
	if (blockList.size() == 0) {
		//Szuka niezaalokowanych blok�w, wybieraj�c pierwsze wolne
		blockList = FindUnallocatedBlocksFragmented(blockCount);
	}

	//Dodaje -1, poniewa� przy zapisie w tablicy FAT ostatnia pozycja wskazuje na nic (czyli -1)
	blockList.push_back(-1);
	return blockList;
}
