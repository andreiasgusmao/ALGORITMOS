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

struct Head
{
	string name;
    bool isCategorical;   
};

struct Data
{
	vector<string> database;
};

struct Value
{
	vector<size_t> x;
};

vector<Value> vNewValues;
vector <Head> vHead;
vector<Data> vDataBase;

//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_transformed.csv"
string fileDocSaida;

//armazena o numero P digitado pelo usuário: número de atributos que quero gerar
size_t P;

#define information "information.txt"

ofstream fileResult; // declara o arquivo onde armazenará os resultados 
ofstream fileInformation; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/
void featureHashing();
void saveFileTransformation();

void readTypeAttribute();

void getDatas(string nameFile) throw (logic_error);
void lineHead (string str);
void saveDatas (string str);

void printHead();
void printDatas();

/*----------------------------------------------------------
Implementação do método de transformação de atributos categóricos em
* contínuos: Feature Hashing

Arquivo database de entrada pode ser em qualquer extensão, porém a
* primeira linha deve ser os atributos e as demais linhas, os valores
* dos atributos.
* Cada atributo ou valor é separado por vírgula
* Obs: não está considerando um campo com mais de um valor
* 
* Arquivo de saída: arquivoEntrada_transformed.csv
* Arquivo de informação: information.txt
* 
* Andreia Gusmão 07/05/2017

----------------------------------------------------------*/

int main(int argc, char *argv[])
{
	cout << "Transformação Feature Hashing: atributos categóricos em continuos."<<endl;
	
	while (fileDoc == "")
	{
		cout << "Digite o nome do arquivo: ";
		cin >> fileDoc;
	}
	
	size_t index = fileDoc.find_first_of(".");
	fileDocSaida = fileDoc.substr (0,index);   
	fileDocSaida.append("_transformed.csv");
	
	//abre arquivo que salvará o resultado - texto é reescrito, se ja existir
	//arquivo com esse nome
	fileResult.open(fileDocSaida,ios::out);
	
	//abre arquivo que salvará as informações - texto é concatenado
	fileInformation.open(information,ios::app);
			
    getDatas(fileDoc);
    
    fileInformation << "Transformação Feature Hashing: atributos categóricos em continuos."<<endl;
    fileInformation << "Arquivo de entrada: " << fileDoc << " com " << 
		vDataBase.size() << " linhas."<<endl;
	fileInformation << "Arquivo de saída: " << fileDocSaida<<endl;	

	cout <<endl;
	
    readTypeAttribute();
 	
	featureHashing();
	cout << " Tranformacao executada com sucesso."<<endl;
		
	saveFileTransformation();
	cout << " Arquivo com atributos transformados gravado com sucesso."<<endl;
	
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

	size_t iLine = 1;
	if ( myFile.is_open() )
	{
		while (myFile.good())
		{
			getline(myFile,line);
			if (line != "")
			{
				if (iLine == 1)
				{
					lineHead(line); //1ª linha do arquivo é o cabeçalho(atributos)
				}
				else
				{
					saveDatas(line); //demais linhas do arquivo contém os dados
				}	
			}
			++iLine;	
			
		}
		myFile.close();
		
    }
	else
		throw logic_error("Não foi possível abrir o arquivo.");

}

/** ----------------------------------------------------------------- */
/** Função que lê a primeira linha do arquivo (cabeçalho) e salva
 * os nomes dos atributos no vetor de cabeçalho)
 * ------ */
void lineHead (string str) 
{
	char sep = ','; //caracter de separação é a vírgula
    istringstream line(str) ;
	string value;
	
	size_t cont = 1;	
	while( getline( line, value, sep ) )
	{
		Head h;
		h.name = value;
		h.isCategorical = false; //padrão
		vHead.push_back(h);
	}	
}

/** ----------------------------------------------------------------- */
/** Função que processa cada linha do arquivo, separando os valores
 * através do caracter "," e adicionando cada valor no vetor de database
 * ------ */
void saveDatas (string str)
{
	char sep = ',';//caracter de separação dos valores dos atributos é a vírgula
    istringstream line(str) ;
	string value;
	
	Data d;
	
	while( getline( line, value, sep ) )
	{
		d.database.push_back(value);	
	}
		
	vDataBase.push_back(d);

}

/**
 * Funçao que mostra  na tela o nome de cada atributo do arquivo, para que
 * o usuário digite se esse atributo vai ser transformado em contínuo
 * ou não. O padrão na leitura do arquivo de entrada, é false (0)
 * 
 *  1 = transforma
 *  0 = nao transforma
 * */
 void readTypeAttribute()
{
	char isCategorical;
	cout << "Para cada atributo, digite:"<<endl;
	cout<< "1=transforma em contínuo / 0=nao transforma em continuo:"<<endl;
	
	fileInformation << "Atributos categóricos que serão transformados em continuos:"<<endl;
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		cout << vHead[i].name << ":";
		cin >> isCategorical;
		
		if (isCategorical=='1')
		{
			vHead[i].isCategorical = true;
			fileInformation << vHead[i].name<<",";	
		}	
			
		
	}	
	cout <<endl;
	fileInformation<<endl;

	//P é o número de atributos que serão criados
	do
	{
		cout << "Número de atributos que serão gerados: ";
		cin >> P;
	}while (P <= 1);
	
	fileInformation<<" Serão gerados "<< P << " novos atributos."<<endl;
		
}


/**
 * Funçao que realiza a transformação do atributo categórico
 * em contínuo
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
	
										
	for (size_t j = 0; j < vHead.size(); ++j)
	{			
		if(vHead[j].isCategorical == true)
		{
			
			for(size_t i = 0; i<vDataBase.size(); ++i)
			{
				Data d = vDataBase[i];
		 		
		 		//	onde x é o índice do atributo - hash armazenado 
				x = indexHash(d.database[j]);
				
				// h(x) = (a*x + b) mod P
				h = (a * x + b) % P;
						
				fileInformation<<"h("<<d.database[j]<<")="<<h<< " onde index x = " <<x<<endl;
				vNewValues[i].x[h] +=1;
				
			}
			
		}	
	}	
}	
	

/**
 * Funçao que salva em um arquivo de saída 
 * os valores do database de entrada, com os novos valores
 * do(s) atributo(s) que foram transformados
 * */
void saveFileTransformation()
{
	//salvar no arquivo o cabeçalho (atributos) modificados
	size_t cont = 0;
	
	for(size_t c = 0; c < vHead.size(); ++c)
	{	
		if(vHead[c].isCategorical == true)
		{

			if (cont == 0)
			{
				cont = 1;
				for(size_t i = 0; i < P; ++i )
				{
					if (c == 0 && i == 0)
					{
						fileResult <<"h"<<i;
					}
					else	
					{
						fileResult << ","<<"h"<<i;
					}			
				}
			}		
		}
		else
		{
			if (c == 0 )
			{
				fileResult << vHead[c].name;
			}
			else
			{
				fileResult << ","<<vHead[c].name;
			}
		}		
	}
	fileResult<<endl;
	
	//salvar no arquivo os dados (database) 
	//atributos que foram transformados, gravar o novo valor
	for (size_t i = 0; i < vDataBase.size(); ++i)
	{
		//dados da linha i
		Data d = vDataBase[i];
			
		cont = 0;
	
		for (size_t c = 0; c < d.database.size(); ++c )
		{			
			if(vHead[c].isCategorical == true)
			{
				if(cont==0)
				{
					cont = 1;
				
					vector<size_t> values = vNewValues[i].x;
					for (size_t x = 0; x < values.size(); ++x)
					{
						if (c == 0 && x == 0)
						{
							fileResult << values[x];
						}
						else
						{
							fileResult <<","<< values[x];
						}
					}		
				}
			}
			else
			{
				//caso não foi transformado, gravar o valor original
				if ( c == 0 )
				{
					fileResult << d.database[c];
				}
				else
				{
					fileResult <<","<< d.database[c];
				}	
						
			} 
		}
		fileResult<<endl;	
	}
}

	

/**
 * Funçao que mostra na tela os atributos do arquivo de entrada
 * e os novos atributos, caso esse atributo for transformado em contínuo
 * */
 
void printHead()
{
	cout << "Cabeçalho do arquivo de entrada com "<< vHead.size() << " atributos." <<endl;
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		cout << vHead[i].name << " - " << vHead[i].isCategorical <<endl;
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
		cout << "linha " << (i+1)<<":";	
		for (size_t c = 0; c < d.database.size(); ++c )
		{
			cout << d.database[c] << ", ";
		}
		cout<<endl;	
	}	
}	
	
