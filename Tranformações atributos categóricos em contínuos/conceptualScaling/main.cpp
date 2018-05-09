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
    bool isTransformed;
    size_t idNewHead;
};

struct Value
{
	vector<string> values;
};
	
struct NewHead
{
	string name;
    bool isTransformed;
    size_t posDatabase;
	vector<Value> newValues;
};

struct Data
{
	vector<string> database;
};

	
vector <Head> vHead;
vector <NewHead> vNewHead;
vector <Data> vDataBase;


//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_transformed.csv"
string fileDocSaida;

//armazena as posições do par de atributos que serão transformados
size_t posAtCategorical;
size_t posAtNumeric;

#define information "information.txt"

ofstream fileResult; // declara o arquivo onde armazenará os resultados 
ofstream fileInformation; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/
void conceptualScaling();
void saveFileTransformation();

void setNewHead();
void readPairAttribute()throw (logic_error);

void getDatas(string nameFile) throw (logic_error);
void lineHead (string str);
void saveDatas (string str);

void printDatas();

/*----------------------------------------------------------
Implementação dos método de transformação de atributos categóricos em
* contínuos: Conceptual Scaling
* 
Arquivo database de entrada pode ser em qualquer extensão, porém a
* primeira linha deve ser os atributos e as demais linhas, os valores
* dos atributos.
* Cada atributo ou valor é separado por vírgula
* Obs: não está considerando um campo com mais de um valor
* 
* Arquivo pode ter qualquer número de atributos, porém
* só um par de atributo será transformado, ou seja, o usuário
* irá digitar o código de um atributo categórico e outro numérico.
* 
* Arquivo de saída: arquivoEntrada_transformed.csv
* Arquivo de informação: information.txt
* 
* Andreia Gusmão 08/05/2017

----------------------------------------------------------*/

int main(int argc, char *argv[])
{
	cout <<"Transformação conceptual scaling"<<endl;
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
    
    fileInformation << "Arquivo de entrada: " << fileDoc << " com " << 
		vDataBase.size() << " linhas."<<endl;
	fileInformation << "Arquivo de saída: " << fileDocSaida<<endl;	
	fileInformation << "Transformação Conceptual Scaling"<<endl;	

    readPairAttribute();
    setNewHead();
 
	conceptualScaling();
	cout << endl<<"Tranformacao executada com sucesso."<<endl;
		
	saveFileTransformation();
	cout << "Arquivo com atributos transformados gravado com sucesso."<<endl;
	
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
		h.isTransformed = false; //padrão
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
 * Funçao que mostra  na tela os nomes dos atributos gravados
 * do arquivo de entrada e o código correspondente.
 * O usuário deve digitar dois códigos: um para atributo categórico
 * e outro para o atributo numérico, que serão trnsformados
 * 
 * */
 void readPairAttribute()  throw (logic_error)
{
	cout << "--------------------------------------------------"<<endl;
	cout << "Atributos do arquivo de entrada:"<<endl;
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		cout << i << "=" <<vHead[i].name << " , ";		
	}
	cout <<endl<<endl;
	
	cout << "Digite o codigo corresponde ao atributo escolhido." <<endl; 
	cout << "Codigo do atributo categorico: ";
	cin >> posAtCategorical;	
	cout << "Codigo do atributo numerico: ";
	cin >> posAtNumeric;	
	
	bool achouC = false;
	bool achouN = false;
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		if(i == posAtCategorical) achouC = true;
		if(i == posAtNumeric) achouN = true;
		
		if(achouC && achouN) break;
			
	}
	
	if 	(achouC == false || achouN == false)
		throw logic_error("Verifique o codigo dos atributos digitados.");

	
	vHead[posAtCategorical].isTransformed = true;
	vHead[posAtNumeric].isTransformed = true;
		
}


/**
 * Funçao que cria o novo cabeçalho de atributos, de acordo
 * com o par de atributos que será transformado.
 * 
 * */
void setNewHead()
{
	NewHead nh;
		
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		if(vHead[i].isTransformed)
		{
			
			if(posAtCategorical == i) 
			{
				nh.name = vHead[posAtCategorical].name + "_" + vHead[posAtNumeric].name;
				nh.isTransformed = true;
				fileInformation << nh.name<<", ";
					
				vNewHead.push_back(nh);
					
				vHead[i].idNewHead = vNewHead.size()-1;
			}
		}
		else
		{
			nh.name = vHead[i].name;
			nh.isTransformed = false;
			nh.posDatabase = i;
						
			vNewHead.push_back(nh);
			
			vHead[i].idNewHead = vNewHead.size()-1;
		}	
			
	}
	fileInformation<<endl;
}
	

/**
 * Funçao que realiza a transformação do atributo categórico
 * em contínuo Conceptual Scaling
 * */
void conceptualScaling()
{
	int value;
	
	for(size_t i = 0; i < vDataBase.size(); ++i)
	{
		Data d = vDataBase[i]; //cada linha de dados do arquivo de entrada
		
					
		for(size_t j = 0; j < vHead.size(); ++j) //cada coluna de atributo
		{
			if(vHead[j].isTransformed)
			{
				if(posAtCategorical == j)
				{
						
					//então a posição j é a coluna do atributo categórico				
					value = stod (d.database[posAtNumeric]);
					Value v;
					for(int c = 1; c <= value; ++c)
					{
						stringstream intToStr;
						intToStr << c;
						string name = d.database[posAtCategorical] + "_>=" + intToStr.str();
					    v.values.push_back(name);
					}	
					vNewHead[ vHead[j].idNewHead ].newValues.push_back( v);					
				
				}	
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
	for(size_t c = 0; c < vNewHead.size(); ++c)
	{	
		if(c > 0)
		{
			fileResult <<",";
		}	
		fileResult << vNewHead[c].name;
	
	}
	fileResult<<endl;
	
	//salvar no arquivo os dados (database) 
	//atributos que foram transformados, gravar o novo valor
	for(size_t i = 0; i < vDataBase.size(); ++i)
	{
		Data d = vDataBase[i]; //dados da linha i
		
		for(size_t c = 0; c < vNewHead.size(); ++c)
		{
			if(vNewHead[c].isTransformed)
			{
				Value v = vNewHead[c].newValues[i];
				
				for(size_t j = 0; j < v.values.size(); ++j)
				{
					
					if (j == 0) //primeiro valor da linha i da coluna c transformada
					{
						if (c > 0)
						{
							fileResult<<",";
						}	
						
						fileResult<<v.values[j];
					}	
					else
					{
						
						//valores das colunas anteriores a coluna c
						for(size_t x = 0; x < c; ++x)
						{
							if (x > 0)
							{
								fileResult<<",";
							}	
							fileResult<<d.database [vNewHead[x].posDatabase];
						}	
						
						//
						fileResult<<","<<v.values[j];
						
					}
					//valores das colunas posteriores a coluna c transformada
					for(size_t x = c+1; x < vNewHead.size(); ++x)
					{
						fileResult<<","<<d.database [vNewHead[x].posDatabase];
					}	
					
					fileResult<<endl;		
				}	
				
				break;
			
			}
			else
			{
				if(c > 0)
				{
					fileResult <<",";
				}	
				fileResult<<d.database [vNewHead[c].posDatabase];
			}		
		
		}
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
	
