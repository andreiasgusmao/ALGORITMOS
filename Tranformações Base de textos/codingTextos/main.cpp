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

struct Binario
{
	//vector era do tipo bool, mas alterei para int, pois no effect coding
	//grava tbem -1, além de 0 e 1.
	vector<int> values;
};

struct Data
{
	vector<string> database;
};

vector <string> vHead;
vector <Binario> vNewValues;
vector <Data> vDataBase;

//armazena o tipo de transformação q será executada
char cTransformation;

//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo e stop words digitado pelo usuário
string fileStopWords = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_transformed.csv"
string fileDocSaida;

//lista de palavras que não devem fazer parte do map de Documentos
list<string> stopWords;

#define information "information.txt"

#define ONE_HOT '1'
#define DUMMY '2'
#define EFFECT '3'

ofstream fileResult; // declara o arquivo onde armazenará os resultados 
ofstream fileInformation; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/
void coding(char &cTransformation);
void oneHotEncoding();
void dummyCoding();
void effectCoding();
void saveFileTransformation();

void verifyAttributeText();
bool isExistsAttribute(string atributo);

void createStopWords ();
void printStopWords(list<string> stopWords);
list<string> getStopWords ();
bool isStopWord(string word);

void getDatas(string nameFile) throw (logic_error);
void tokenize (string str);

void printDatas();

/*----------------------------------------------------------
Implementação dos métodos de transformação de atributos categóricos em
* contínuos - PARA BASE DE DADOS DE TEXTOS
* 1- one-hot encoding
* 2- dummy coding
* 3- effect coding

Arquivo database de entrada pode ser em qualquer extensão, e não tem
* cabeçalho. 
* Cada linha representa um documento texto
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
	while (fileDoc == "")
	{
		cout << "Digite o nome do arquivo: ";
		cin >> fileDoc;
	}
	
	while (fileStopWords == "")
	{
		cout << "Digite o nome de stop words: ";
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
	
	cout << "Digite o numero corresponde a transformação (Base de Textos) desejada: "<<endl;
	cout <<"1 - One-hot Encoding"<<endl;
	cout <<"2 - Dummy Coding"<<endl;
	cout <<"3 - Effect Coding"<<endl;
	cin >> cTransformation;
	
	if(cTransformation != '1' && cTransformation != '2' &&
		cTransformation != '3')	throw logic_error("Numero de transformacao digitado invalido.");
	
	createStopWords();
		
    getDatas(fileDoc);
        
    fileInformation << "Arquivo de entrada: " << fileDoc << " com " << 
		vDataBase.size() << " documentos."<<endl;
	fileInformation << "Arquivo de saída: " << fileDocSaida<<endl;	

	cout<<endl;
	       
    verifyAttributeText();
	cout << " Verificacao concluida dos atributos da base de textos."<<endl;

	
	if (cTransformation == ONE_HOT) oneHotEncoding();
	else if (cTransformation == DUMMY) dummyCoding();
	else effectCoding();
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
 * Funçao que verifica para cada palavra de todos os documents,
 * se já foi adicionado como novo atributo. SE não foi, é adicionado.
 * 
 * */
void verifyAttributeText()
{
	bool found;
	for (size_t i = 0; i < vDataBase.size(); ++i)
	{
		Data d = vDataBase[i];
		
		for (size_t c = 0; c < d.database.size(); ++c )
		{
			//só adicionar, caso palavra ainda não foi adicionada como atributo
			found = isExistsAttribute(d.database[c]);
			if (found == false)
			{
				vHead.push_back(d.database[c]);
			}	
		}	
	}	
}

/**
 * Funçao que verifica se o valor do arquivo de database já
 * foi adicionado como novo atributo (para que não seja add
 * mais de uma vez)
 * 
 * */
bool isExistsAttribute(string atributo)
{
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		if(vHead[i] == atributo)
		{
			return true; //ja existe esse atributo
		}	
	}	
	
	return false;
}

/**
 * Funçao que realiza a transformação do texto
 * em contínuo, de acordo com o parâmetro passado pela
 * transformação escolhida pelo usuário
 * 
 * 
 * */
void coding(char &cTransformation)
{
	bool found = false;
	
	for(size_t i = 0; i<vDataBase.size(); ++i)
	{
		Data d = vDataBase[i];
		
		Binario b;
		
		for(size_t c = 0; c < vHead.size(); ++c)
		{
			found = false;
			
			if ((cTransformation == EFFECT || cTransformation == DUMMY)
				&& c == 0)
			{
				found = false;
			}
			else
			{		
				for (size_t j = 0; j < d.database.size(); ++j)
				{
					if (d.database[j] == vHead[c])
					{
						found = true;
						break;
					}
				}
			}
			
			//somente na transformação effect coding a coluna = -1
			if (cTransformation == EFFECT && c == 0)
			{
				b.values.push_back(-1);
			}
			else
			if (cTransformation == DUMMY && c == 0)
			{
				b.values.push_back(0);
			}
			else
			if (found)
			{
				b.values.push_back(1);	//valor = atributo
			}
			else
			{
				b.values.push_back(0);	//valor ! atributo
			}				
				
				
							
		}
		vNewValues.push_back(b);
	}	 	
}	
	
/**
 * Funçao de transformação one-hot encoding
 * Cada palavra será transformada será uma novo atributo.
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * 
 * */
void oneHotEncoding()
{
	fileInformation << "Transformacao executada: one-hot encoding"<<endl;
	coding(cTransformation);
	
}	


/**
 * Funçao de transformação dummy coding
 * Cada palavra do texto será um novo atributo.
 * O primeiro novo atributo será "atributo referência"
 * 
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * A primeira coluna será todos 0
 * 
 * 
 * */
void dummyCoding()
{
	fileInformation << "Transformacao executada: dummy coding"<<endl;
	coding(cTransformation);	
}	

/**
 * Funçao de transformação dummy coding
 * Cada palavra do texto será um novo atributo.
 * O primeiro novo atributo será "atributo referência"
 * 
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * A primeira coluna será todos -1
 * 
 * 
 * */void effectCoding()
{
	fileInformation << "Transformacao executada: effect coding"<<endl;
	coding(cTransformation);
}	


/**
 * Funçao que salva em um arquivo de saída 
 * os novos valores da base de texto transformada.
 * Acrescenta uma primeira coluna atributo id,
 * que é composto de d+numero_da_linha
 * */
void saveFileTransformation()
{
	//salvar no arquivo o cabeçalho (atributos) modificados
	size_t auxI;
	fileResult << "id,";
	
	for(size_t c = 0; c < vHead.size(); ++c)
	{	
		
		fileResult << vHead[c];
		if ( c < (vHead.size()-1))
		{	
			fileResult<<",";
		}	
			
	}
	fileResult<<endl;
	
	//salvar no arquivo os dados (database) transformados 
	for (size_t i = 0; i < vNewValues.size(); ++i)
	{
		fileResult <<"d"<<(i+1)<<",";
		
		Binario b = vNewValues[i];
		
		for(size_t c = 0; c < b.values.size(); ++c)
		{
			fileResult << b.values[c];
			if (c < (b.values.size()-1))  
				fileResult<<",";			
		}
		fileResult<<endl;	
		
	}	
}

	

/**
 * Funçao que mostra na tela os atributos do arquivo de entrada
 * e os novos atributos, caso esse atributo for transformado em contínuo
 * */
 
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

	
