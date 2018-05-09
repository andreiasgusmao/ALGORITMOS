#include <string.h>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <time.h>
#include <unordered_map>
#include <chrono>
#include <cstddef>         // std::size_t
#include <sstream>
#include <cctype>
#include <iomanip>      // std::setprecision

using namespace std;


struct Data
{
	vector<string> database;
};

struct Value
{
	vector<size_t> x;
};

vector<Value> vNewValues;
vector<Data> vDataBase;
vector <string> vHead;


//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo e stop words digitado pelo usuário
string fileStopWords = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_transformed.csv"
string fileDocSaida;

//lista de palavras que não devem fazer parte dos documentos
list<string> stopWords;

//armazena o numero P digitado pelo usuário: número de atributos que quero gerar
size_t P;


#define information "information.txt"

ofstream fileResult; // declara o arquivo onde armazenará os resultados 
ofstream fileInformation; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/
void featureHashing();
void saveFileTransformation();

void createStopWords ();
void printStopWords(list<string> stopWords);
list<string> getStopWords ();
bool isStopWord(string word);

void getDatas(string nameFile) throw (logic_error);
void tokenize (string str);

void printDatas();

/*----------------------------------------------------------
Implementação do método de transformaçãoFeature Hashing
*  PARA BASE DE DADOS DE TEXTOS
* 
Arquivo database de entrada pode ser em qualquer extensão, e não tem
* cabeçalho. 
* Cada linha representa um documento texto
* 
* Exemplo de arquivo com 3 documentos
* este é um documento exemplo
* o exemplo de texto para documento
* o texto vai ser este aqui
* 
* 
* Arquivo de saída: nomeArquivoEntrada_transformed.csv
* Arquivo de informação: information.txt
* 
* Andreia Gusmão 03/05/2017

----------------------------------------------------------*/

int main(int argc, char *argv[])
{
	cout << "Transformacao Feature Hashing (Base de Textos) "<<endl;
	
	while (fileDoc == "")
	{
		cout << "Digite o nome do arquivo: ";
		cin >> fileDoc;
	}
	
	while (fileStopWords == "")
	{
		cout << "Digite o nome do arquivo de stop words: ";
		cin >> fileStopWords;
	}
	
	size_t index = fileDoc.find_first_of(".");
	fileDocSaida = fileDoc.substr (0,index);   
	fileDocSaida.append("_transformed.csv");
	
	//abre arquivo que salvará o resultado - texto é reescrito, se ja existir
	//arquivo com esse nome
	fileResult.open(fileDocSaida,ios::out);
	
	//abre arquivo que salvará as informações - texto é concatenado
	fileInformation.open(information,ios::app);
	
	createStopWords();
		
    getDatas(fileDoc);
        
    fileInformation << "Transformação Feature Hashing em Base de Textos."<<endl;
    fileInformation << "Arquivo de entrada: " << fileDoc << " com " << 
		vDataBase.size() << " documentos."<<endl;
	fileInformation << "Arquivo de saída: " << fileDocSaida<<endl;	

	cout<<endl;
	       
	//P é o número de atributos que serão criados
	do
	{
		cout << "Número de atributos que serão gerados: ";
		cin >> P;
	}while (P <= 1);
	
	fileInformation<<" Serão gerados "<< P << " novos atributos."<<endl;
	
	featureHashing();
	cout << " Tranformacao executada com sucesso."<<endl;
		
	saveFileTransformation();
	cout << " Arquivo com base texto transformada gravada com sucesso."<<endl;
	
	fileInformation <<"------------------------------------------------------------"<<endl;
	  
    return 1;

}

/** ----------------------------------------------------------------- */
/** Função que lê o arquivo e obtém os dados armazenados para 
 * a execução do algoritmo de transformação
 * ------ */
void getDatas(string nameFile) throw (logic_error)
{
	string line;
	ifstream myFile;
	myFile.open (nameFile,ios::in);

	if ( myFile.is_open() )
	{
		while (myFile.good())
		{
			getline(myFile,line);
			if (line != "")
			{
				tokenize(line);		
			}
		}
		myFile.close();	
    }
	else
		throw logic_error("Não foi possível abrir o arquivo.");

}

/** ----------------------------------------------------------------- */
/** Função que processa cada linha do arquivo, filtrando e convertendo
 * as palavras de acordo com os parâmetros definidos. Depois salva
 * cada token no vetor de database
 * ------ */
void tokenize (string str)
{
	char sep = ' ';//caracter de separação é o espaço
    istringstream line(str) ;
	string token;
	bool found;
	
	Data d;
		
	while( getline( line, token, sep ) )
	{
		//converte para minúsculo
		transform(token.begin(), token.end(), token.begin(), ::tolower);
			
		/*permitir somente palavras com mais de 2 caracteres,
		desde que não seja uma "palavra proibida" */
		if ( (token.length() > 2) && (isStopWord(token) == false) )
		{
			
			//verifica se existe um sinal de pontuação no final do token
			if(ispunct(token[token.length()-1]))
			{
				//se existir, excluir a pontuação
				token.erase(token.begin()+token.length()-1,token.begin()+token.length());
			}

			found = false;
			for(size_t i = 0; i < d.database.size(); ++i)
			{
				if(d.database[i] == token)
				{
					found = true;
					break;
				}	
			}	
			//se não encontrar, adiciona o token	
			if(found == false)
			{
				d.database.push_back(token);
			}	
		}
	}
		
	vDataBase.push_back(d);
}

/**
 * Funçao que realiza a transformação do texto
 * em contínuo
 *  
 * 
 * */
void featureHashing()
{
	size_t a, b, x, h;
	hash<string> indexHash;

	//gerar número aleatório de 1 até P
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	mt19937 generator (seed);
	uniform_int_distribution<int> distribution(1,P);
		
	do
	{	
		a = distribution(generator);//número aleatório para a. Obs: a <> P
	}while (a == P );
	b = distribution(generator);//número aleatório para b
	
	fileInformation<<"Valores aleatórios de a="<<a<< " e b="<<b<<endl;
	cout << " a=" << a << " , b=" << b << " e P="<<P<<endl;
				
	//inicializar o vetor de novos valores com 0 para todas as P posições			
	for (size_t i = 0; i < vDataBase.size(); ++i)
	{
		Value v;
		
		for(size_t c = 0; c < P; ++c)
		{
			v.x.push_back(0);//padrão tudo 0
		}	
	
		vNewValues.push_back(v);
	}
	
	
	for(size_t i = 0; i<vDataBase.size(); ++i)
	{
		Data d = vDataBase[i];
 		
 		for (size_t c = 0; c < d.database.size(); ++c )
		{
			
			//	onde x é o índice do atributo - hash armazenado 
			x = indexHash(d.database[c]);
				
			// h(x) = (a*x + b) mod P
			h = (a * x + b) % P;
						
			fileInformation<<"h("<<d.database[c]<<")="<<h<< " onde index x = " <<x<<endl;
			vNewValues[i].x[h] +=1;
		}		
	}
			 	
}	
	


/**
 * Funçao que salva em um arquivo de saída 
 * os novos valores da base de texto transformada.
 * Acrescenta uma primeira coluna atributo id,
 * que é composto de d+numero_da_linha
 * */
void saveFileTransformation()
{
	//salvar no arquivo o cabeçalho modificado
	size_t auxI;
	fileResult << "id";
	
	for(size_t c = 0; c < P; ++c)
	{	
		
		fileResult <<",h"<<c;	
			
	}
	fileResult<<endl;
	
	
	//salvar no arquivo os dados (database) transformados 
	for (size_t i = 0; i < vNewValues.size(); ++i)
	{
		fileResult <<"d"<<(i+1);
		
		vector<size_t> values = vNewValues[i].x;
		for (size_t c = 0; c < values.size(); ++c)
		{
			fileResult <<","<< values[c];
		}
		fileResult<<endl;	
	}	
}

	
/**
 * Funçao que mostra na tela todos os dados lidos do arquivo e
 * armazenados no vetor de database
 * */
 
void printDatas()
{
	cout << " Total linhas do database: " << vDataBase.size() <<endl;
		
	for (size_t i = 0; i < vDataBase.size(); ++i)
	{
		Data d = vDataBase[i];
		cout << "d" << (i+1)<<": com size: " << d.database.size() << " - ";	
		for (size_t c = 0; c < d.database.size(); ++c )
		{
			cout << d.database[c] << " ";
		}
		cout<<endl;	
	}	
}	
	
/** ----------------------------------------------------------------- */
/** Função que cria uma lista de stop words
 * ------ */
void createStopWords ()
{
	string line;
	ifstream myFile;
	myFile.open (fileStopWords,ios::in);

	if ( myFile.is_open() )
	{
		while (myFile.good())
		{
			getline(myFile,line);
			stopWords.push_back(line);
		}
		myFile.close();
    }
	else
		throw logic_error("Não foi possível abrir o arquivo de stop words.");

}

/** ----------------------------------------------------------------- */
/** Função que retorna a lista de stop words criada
 * ------ */
list<string> getStopWords ()
{
	return stopWords;
}

/** ----------------------------------------------------------------- */
/** Função que verifica se uma palavra está na lista de stopWords ou não
 * ------ */
bool isStopWord(string word)
{
	//PROVISÓRIO = alterar o arquivo de stopWords para deixar as
	//palavras em ordem crescente  e fazer uma busca "mais inteligente"

	for (auto item = stopWords.cbegin(); item != stopWords.cend(); ++item)
	{
		if (*item == word) return true;
	}

	return false;
}

	
