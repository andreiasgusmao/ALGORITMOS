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
	vector<int> value;
};

struct NewAttribute
{
	string nameOriginal; //nome original, ex: cor
	string nameLabel; //nome do novo atributo, exemplo para cor azul: cor_azul
};

struct Head
{
	string name;
    bool isCategorical;
    vector<NewAttribute> newAttributes;
    vector<Binario> newValues;
    
};

struct Data
{
	vector<string> database;
};


vector <Head> vHead;
vector <Data> vDataBase;

//armazena o tipo de transformação q será executada
char cTransformation;

//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_transformed.csv"
string fileDocSaida;

#define information "information.txt"

#define ONE_HOT '1'
#define DUMMY '2'
#define EFFECT '3'

ofstream fileResult; // declara o arquivo onde armazenará os resultados 
ofstream fileInformation; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/
void coding();
void oneHotEncoding();
void dummyCoding();
void effectCoding();
void saveFileTransformation();

void readTypeAttribute();
void verifyAttributeIsCategoric();
bool isExistsAttribute(string atributo, Head h);

void getDatas(string nameFile) throw (logic_error);
void lineHead (string str);
void saveDatas (string str);

void printHead();
void printDatas();

/*----------------------------------------------------------
Implementação dos métodos de transformação de atributos categóricos em
* contínuos
* 1- one-hot encoding
* 2- dummy coding
* 3- effect coding

Arquivo database de entrada pode ser em qualquer extensão, porém a
* primeira linha deve ser os atributos e as demais linhas, os valores
* dos atributos.
* Cada atributo ou valor é separado por vírgula
* Obs: não está considerando um campo com mais de um valor
* 
* Arquivo de saída: result.csv
* Arquivo de informação: information.txt
* 
* Andreia Gusmão 30/04/2017

----------------------------------------------------------*/

int main(int argc, char *argv[])
{
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
	
	cout << "Digite o numero corresponde a transformação desejada: "<<endl;
	cout <<"1 - One-hot Encoding"<<endl;
	cout <<"2 - Dummy Coding"<<endl;
	cout <<"3 - Effect Coding"<<endl;
	cin >> cTransformation;
	
	if(cTransformation != '1' && cTransformation != '2' &&
		cTransformation != '3')	throw logic_error("Numero de transformacao digitado invalido.");
		
    getDatas(fileDoc);
    
    fileInformation << "Arquivo de entrada: " << fileDoc << " com " << 
		vDataBase.size() << " linhas."<<endl;
	fileInformation << "Arquivo de saída: " << fileDocSaida<<endl;	

	cout <<endl;
	
    readTypeAttribute();
    
    verifyAttributeIsCategoric();
	cout << " Verificacao concluida dos atributos que serao transformados."<<endl;
	
	//printDatas();
	
	if (cTransformation == ONE_HOT) oneHotEncoding();
	else if (cTransformation == DUMMY) dummyCoding();
	else effectCoding();
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
}

/**
 * Funçao que verifica para cada atributo que o usuário digitou
 * que será transformado em contínuo, os valores do atributo no
 * arquivo de database, para a composição dos "novos atributos" 
 * 
 * */
void verifyAttributeIsCategoric()
{
	for (size_t i = 0; i < vHead.size(); ++i)
	{
		Head h = vHead[i];
		
		if(h.isCategorical == true)
		{
			bool found = false;
			for(size_t c = 0; c<vDataBase.size(); ++c)
			{
				Data d = vDataBase[c];
				
				//só adicionar, caso atributo ainda não foi adicionado
				found = isExistsAttribute(d.database[i], h);
				if (found == false)
				{
					NewAttribute a;
					a.nameOriginal = d.database[i];
					a.nameLabel = h.name + "_" + a.nameOriginal;
					h.newAttributes.push_back(a);
				}	
			}	
	
			vHead[i] = h;
		}
	}	
		
}

/**
 * Funçao que verifica se o valor do arquivo de database já
 * foi adicionado como novo atributo (para que não seja add
 * mais de uma vez)
 * 
 * */
bool isExistsAttribute(string atributo, Head h)
{
	for (size_t i = 0; i < h.newAttributes.size(); ++i)
	{
		if(h.newAttributes[i].nameOriginal == atributo)
		{
			return true; //ja existe esse atributo
		}	
	}	
	
	return false;
}

/**
 * Funçao que realiza a transformação do atributo categórico
 * em contínuo, de acordo com o parâmetro passado pela
 * transformação escolhida pelo usuário
 * 
 * Valores de auxC: ver na função de cada transformação.
 * 
 * */
void coding(size_t auxC)
{
	for (size_t j = 0; j < vHead.size(); ++j)
	{
		Head h = vHead[j];
		
		if(h.isCategorical == true)
		{
			for(size_t i = 0; i<vDataBase.size(); ++i)
			{
				Data d = vDataBase[i];
		 		
				vector<NewAttribute> attributes = vHead[j].newAttributes;
		
				Binario v;
						
				for(size_t c = auxC; c < attributes.size(); ++c)
				{
					//somente na transformação effect coding a primeira linha = -1
					if (cTransformation == EFFECT && d.database[j] == attributes[0].nameOriginal)
					{
						v.value.push_back(-1);
					}
					else
					if (d.database[j] == attributes[c].nameOriginal)
					{
						v.value.push_back(1);	//valor = atributo
					}
					else
					{
						v.value.push_back(0);   //valor != atributo 
					}
							
				}
				h.newValues.push_back(v); //adiciona os novos valores
		
			}
			vHead[j] = h;
	
		}	
	}
		
}	
	
/**
 * Funçao de transformação one-hot encoding
 * Cada valor de atributo do atributo que será transformado será uma novo atributo.
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * 
 * Chama a função coding passando o valor de auxC=0, pois todos os novos 
 * atributos serão verificados
 * 
 * */
void oneHotEncoding()
{
	fileInformation << "Transformacao executada: one-hot encoding"<<endl;
	coding(0);
	
}	


/**
 * Funçao de transformação dummy coding
 * Cada valor de atributo do atributo que será transformado será uma novo atributo,
 * exceto o primeiro valor que é utilizado como referência e não como
 * novo atributo
 * Para k valores de atributo, k-1 novos atributos
 * 
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * A primeira linha será todos 0
 * 
 * Chama a função coding passando o valor de auxC=1, pois o
 * atributo 0 é o "atributo referência" e portanto, 
 * não será considerado novo atributo
 * 
 * */
void dummyCoding()
{
	fileInformation << "Transformacao executada: dummy coding"<<endl;
	coding(1);	
}	

/**
 * Funçao de transformação effect coding
 * Cada valor de atributo do atributo que será transformado será uma novo atributo,
 * exceto o primeiro valor que é utilizado como referência e não como
 * novo atributo
 * Para k valores de atributo, k-1 novos atributos
 * 
 * É feita a comparação: 
 * valor do atributo = novo atributo : 1
 * valor do atributo != novo atributo : 0
 * A única diferença com a transformação "dummy", é que a primeira
 * linha será todos -1
 * 
 * Chama a função coding passando o valor de auxC=1, pois o
 * atributo 0 é o "atributo referência" e portanto, 
 * não será considerado novo atributo
 * 
 * */
void effectCoding()
{
	fileInformation << "Transformacao executada: effect coding"<<endl;
	coding(1);
}	


/**
 * Funçao que salva em um arquivo de saída (result.csv)
 * os valores do database de entrada, com os novos valores
 * do(s) atributo(s) que foram transformados
 * */
void saveFileTransformation()
{
	//salvar no arquivo o cabeçalho (atributos) modificados
	size_t cont, auxI;
	
	for(size_t c = 0; c < vHead.size(); ++c)
	{	
		if(vHead[c].isCategorical == true)
		{
			if (cTransformation == ONE_HOT)
			{
				cont = 1;
				auxI = 0;
			}	
			else
			{
				//dummy e effect não grava o primeiro valor de atributo
				cont = 2;
				auxI = 1;
			}	

			for(size_t i = auxI; i < vHead[c].newAttributes.size(); ++i )
			{
				fileResult << vHead[c].newAttributes[i].nameLabel;
				if ( (cont < vHead[c].newAttributes.size()) ||
					(c < (vHead.size()-1)) )
				{	
					fileResult<<",";
				}	
				++cont;	
			}	
		}
		else
		{
			fileResult << vHead[c].name;
			if (c < vHead.size()-1 ) fileResult<<",";
		}	
	}
	fileResult<<endl;
	
	//salvar no arquivo os dados (database) 
	//atributos que foram transformados, gravar o novo valor
	for (size_t i = 0; i < vDataBase.size(); ++i)
	{
		//dados da linha i
		Data d = vDataBase[i];
			
		for (size_t c = 0; c < d.database.size(); ++c )
		{			
			//verificar no vetor de cabeçaho, se o atributo da coluna c
			//da linha i foi transformado ou não
			if(vHead[c].isCategorical == true)
			{
				cont = 1;
				vector<int> values = vHead[c].newValues[i].value;
					
					for (size_t x = 0; x < values.size(); ++x)
					{
						fileResult << values[x];
						if ( (cont < values.size()) || (c < (d.database.size()-1))  )
							fileResult<<",";
						++cont;
					}
				
			}
			else
			{
				//caso não foi transformado, gravar o valor original
				fileResult << d.database[c];
				if (c < (d.database.size()-1) )
					fileResult<<",";
						
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
		cout << vHead[i].name << " - " << vHead[i].isCategorical;
		for(size_t c = 0; c < vHead[i].newAttributes.size(); ++c)
		{
			NewAttribute a = vHead[i].newAttributes[c];
			
			cout << " - " << vHead[i].newAttributes[c].nameOriginal;
			cout << "("<<vHead[i].newAttributes[c].nameLabel<<")";
		}	
		cout <<endl;
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
	
