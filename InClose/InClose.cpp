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
#include <chrono>
#include <cstddef>         // std::size_t
#include <sstream>
#include <cctype>
#include <iomanip>      // std::setprecision
#include <iterator>


using namespace std;

struct Data
{
	string object;
	string attribute;		
};

struct Value
{
    vector<bool> values;
};

struct Concepts
{
    vector<size_t> values;
};

vector <Data> vDados;
vector <Value> vI;

vector <string> vAttributes;
vector <string> vObjects;

vector <Concepts> vA;
vector <Concepts> vB;

size_t rNew = 0; //utilizado na função inClose - índice do proximo conceito
int startCol = 0; //coluna para iniciar o inClose

//armazena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena o nome do arquivo de saída: arquivoEntrada+"_concepts.data"
string fileDocSaida;

ofstream fileResult; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/

void lineHead (string str);
void getDatas(string nameFile) throw (logic_error);
void saveDatas (string str) ;

void printDatas();
void printMatrixBoolean();

bool isExistsObject(string o);
bool isExistsAttribute(string o);

void computingAllFormalConcepts();
void createMatrixBoolean();
void setExtendA();
void inClose(int r, int y);
bool isCannonical(int r, int y);
void verifyIntendB();
void saveFileConcepts();

int getStrToInt(string strConvert);

/*----------------------------------------------------------
 * Algoritmo InClose: um algoritmo rápido para computar os conceitos
 * formais.
 * 
 * Padrão do arquivo de entrada: duas colunas separadas por tab,
 * onde a primeira coluna é referente ao objeto e a segunda ao atributo. Ex:
 * 
 * coelho	animal
 * coelho	terrestre
 * tucano	animal
 * tucano	terrestre
 * oliveira	planta
 * oliveira	terrestre
 * 
 * a qual armazenará os atributos ordenados: <animal, planta, terrestre>
 * e os objetos na ordem que aparecerem: <coelho, tucano, oliveira>
 * 
 * Matriz booleana
 * 1 0 1
 * 1 0 1
 * 0 1 1

/*----------------------------------------------------------*/

int main(int argc, char *argv[])
{		
	while (fileDoc == "")
	{
		cout << "Digite o nome do arquivo: ";
		cin >> fileDoc;
	}
	
	size_t index = fileDoc.find_first_of(".");
	fileDocSaida = fileDoc.substr (0,index);   
	fileDocSaida.append("_concepts.data");
	
	//abre arquivo que salvará o resultado - texto é reescrito, se ja existir
	//arquivo com esse nome
	fileResult.open(fileDocSaida,ios::out);
	
	cout << "Lendo arquivo de entrada. " <<endl;	
	getDatas(fileDoc);
	cout << "Total: " <<vAttributes.size() << " atributos e " << vObjects.size()<< " objetos."<<endl;
	
//	printDatas();
	
	computingAllFormalConcepts();
		
    return 1;
}

/** ----------------------------------------------------------------- */
/** Função que lê o arquivo e obtém os dados armazenados para 
 * a execução do algoritmo para computar todos os conceitos formais
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
			saveDatas(line);	 
		}
		myFile.close();
		
    }
	else
		throw logic_error("Não foi possível abrir o arquivo.");

}


/** ----------------------------------------------------------------- */
/** Função que processa cada linha do arquivo.
 * Padrão da linha: 
 * coluna 1 = objeto
 * coluna 2 = atributo
 * as colunas são separadas por um "tab"
 *  
 * ------ */
void saveDatas (string str) 
{
	char sep = '	';
    istringstream line(str) ;
	string value="";
	Data d;
	size_t col = 1;
	
	while( getline( line, value, sep ) )
	{
	    if(value != "")
	    {
		if (col == 1 )
		{
			d.object = value;
			if (isExistsObject(value) == false)
			{
				vObjects.push_back(value);
			}	
		}
		else
		{
			d.attribute = value;
			if (isExistsAttribute(value) == false)
			{
				//insere os atributos ordenando-os
				auto it = upper_bound(vAttributes.begin(), vAttributes.end(), value);
				vAttributes.insert(it, value);
			}
		}	
		++col;
		}	
	}
	
	vDados.push_back(d);
	
}

/**
 * Funçao que verifica se o objeto já existe no vetor que armazena os 
 * objetos
 * Retorna true se já existir e false se não existir
 * */
bool isExistsObject(string o)
{
	for(size_t i = 0; i < vObjects.size(); ++i)
	{
		if (vObjects[i] == o)
		{
			return true;
		}	
	}
	return false;	
}

/**
 * Funçao que verifica se o atributo já existe no vetor que armazena os 
 * atributos
 * Retorna true se já existir e false se não existir
 * */
bool isExistsAttribute(string a)
{
	for(size_t i = 0; i < vAttributes.size(); ++i)
	{
		if (vAttributes[i] == a)
		{
			return true;
		}	
	}
	return false;	
}

/**
 * Funçao principal que chama todos as outras funções que computa 
 * os conceitos formais 
 * e salva-os em arquivo
 * 
 * */
void computingAllFormalConcepts()
{
	createMatrixBoolean();
	cout << "Matriz booleana criada."<<endl;
//	printMatrixBoolean();
	
	/* A[0] = {0,1,...,m-1} */
	setExtendA();

	/* B[0] = {} - inicializa vazio */
	Concepts cVazio;
	vB.push_back(cVazio);
			
	rNew = 0;
	
	cout << "Iniciando o InClose..."<<endl;
	inClose(0, startCol);
	cout << "InClose concluido."<<endl;
	
	verifyIntendB();
	
	saveFileConcepts();
	cout <<"Arquivo com conceitos formais gravado com sucesso."<<endl;
}
	
/**
 * Funçao que a partir dos dados lidos do arquivo, cria a matriz booleana
 * que será utilizada na função inClose
 * 
 * */ 
void createMatrixBoolean()
{
	//inicializar toda a matriz com padrão false
	for(size_t i = 0; i < vObjects.size(); ++i)
	{
		Value v;
		for(size_t c = 0 ; c < vAttributes.size(); ++c)
		{
			v.values.push_back(false);
		}	
		vI.push_back(v);
	}	
	
	//startCol vai armazenar o índice do primeiro atributo que tiver pelo
	//menos um valor true	
	startCol = -1;
	
	//vI[i][a] = true caso o objeto i ter o atributo a
	for(size_t i = 0; i < vObjects.size(); i++)
	{
		for(size_t c = 0; c < vDados.size(); c++)
		{
			if (vObjects[i] == vDados[c].object)
			{
				for(size_t a = 0; a < vAttributes.size(); a++)
				{
					if (vAttributes[a] == vDados[c].attribute)
					{
						vI[i].values[a] = true;
						if (startCol == -1) startCol = a;
						
					}		
				}
			}	
		}	
	}

}

/**
 * Funçao que a inicializa vA[0] com todos os índices dos atributos
 * ordenados
 * */
void setExtendA()
{
	Concepts c;
	//o codigo está assim porque vAttributes já está ordenado
	for(size_t i = 0; i < vAttributes.size(); ++i)
	{
		c.values.push_back(i);
	}	
	vA.push_back(c);
}


/**
 * Funçao que a cria o conceito <{}, G>, caso não existir nenhum conceito
 * onde vB contém todos os objetos
 * */
void verifyIntendB()
{
	bool found = false;
	for(size_t i = 0; i < vB.size(); ++i)
	{
		if (vB[i].values.size() == vObjects.size())
		{
				found = true;
				break;
		}		
	}	
	
	Concepts c, cVazio;
	if (found == false)
	{
	//	cout << " verify;"<<endl;
		for(size_t i = 0; i < vObjects.size(); ++i)
		{
			c.values.push_back(i);
		}	
		vB.push_back(c);
		vA.push_back(cVazio);
		
	}	
}

/**
 * Funçao que executa o algoritmo InClose para computar todos 
 * os conceitos formais 
 * */
void inClose(int r, int y)
{
	++rNew;
					
	for(int j = y; j < vObjects.size(); ++j)
	{
	//	vA[rNew] = {};//vazio
		Concepts cVazio;
		if(vA.size() > rNew)
		{
			vA[rNew] = cVazio;
		}
		else vA.push_back(cVazio);
		
		for(size_t item = 0; item < vA[r].values.size(); ++item)
		{ 
			size_t i = vA[r].values[item]; 
			if(vI[j].values[i]) 
			{	
				//vA[rNew] = vA[rNew] U i;						
			 	vA[rNew].values.push_back(i);
			}
			
		}
		
		if(vA[rNew].values.size() > 0)
		{
			if(vA[rNew].values.size() == vA[r].values.size())
			{
			//	vB[r] = vB[r] U j;
				if (vB.size() == 0 || vB.size() <= r)
				{
					Concepts c;
					vB.push_back(c);
				}	 
				vB[r].values.push_back(j);
			}	
			else
			{
				if (isCannonical(r, j-1 ) )
				{
					//	vB[rNew] = vB[r] U j;
					if (vB.size() <= rNew)
					{
						Concepts c;
						vB.push_back(c);
					}	
					vB[rNew] = vB[r];
					vB[rNew].values.push_back(j);
					inClose(rNew, j+1);
				}	
			}	
		}		
	}		
}

/**
 * Funçao que retorna false se A[rnew] é encontrado, e   
 *                    true se não foi encontrado
 * */ 
bool isCannonical(const int r, int y)
{
	int h = 0;
	for(int k = vB[r].values.size()-1; k >= 0; --k)
	{	
		int pos = vB[r].values[k];
		for(int j = y; j >= (pos+1); --j)
		{
			for(h = 0; h < vA[rNew].values.size(); ++h)
			{
				if(vI[j].values[vA[rNew].values[h]] == false)
				{
					break;
				}	
			}
			if (h == vA[rNew].values.size())
			{
				return false;
			}		
		}
		y = vB[r].values[k] - 1;
	}
	
	for(int j = y; j >= 0; --j)
	{
		for(h = 0; h < vA[rNew].values.size(); ++h)
		{
			if(vI[j].values[vA[rNew].values[h]] == false)	
			{
				break;
			}	
		}
		if (h == vA[rNew].values.size())
		{
			return false;
		}		
	}
	return true;
}
	
/**
 * Funçao que mostra na tela todos os dados lidos do arquivo.
 * vetor de Atributos
 * vetor de objetos
 * Instâncias
 * */
void printDatas()
{
	cout << " Atributos: " << vAttributes.size() <<endl;
	
	for (size_t i = 0; i < vAttributes.size(); ++i)
	{
		cout << vAttributes[i] << ",";
	}		
	
	cout <<endl;
	
	cout << " Objetos: " <<endl;
	for (size_t i = 0; i < vObjects.size(); ++i)
	{
		cout << vObjects[i] << ",";
	}	
	cout <<endl;	
	
	cout << " Dados: " <<endl;
	for (size_t i = 0; i < vDados.size(); ++i)
	{
		cout << vDados[i].object << " - " << vDados[i].attribute <<endl;
	}		

}

/**
 * Funçao que salva em arquivo todos os conceitos formais encontrados
 * */
void saveFileConcepts()
{
	size_t count = vA.size();
    if (vB.size() > vA.size()) count = vB.size();
    
	fileResult << "Formal Concepts: " << count << " Atributos: " << vAttributes.size();
	fileResult << "Objetos: " << vObjects.size() <<endl;
       
	for (size_t i = 0; i < count; ++i)
	{
		fileResult <<"<{";
		
		if (i >= vB.size())
		{
			fileResult << " ";
		}			
		else
		{
			for (size_t c = 0; c < vB[i].values.size(); ++c)
			{
				if (c > 0) fileResult <<",";
				fileResult << vObjects[ vB[i].values[c] ];
		
			}
		}	
		fileResult <<"}, {";
				
		if (i >= vA.size())
		{
			fileResult << " ";
		}			
		else
		{
			for (size_t c = 0; c < vA[i].values.size(); ++c)
			{
				if (c > 0) fileResult <<",";
				fileResult <<vAttributes[ vA[i].values[c] ];
		
			}
		}	
		fileResult <<"}>" <<endl;
		
	}		
	
}

/**
 * Funçao que mostra na tela a matriz booleana
 * */

void printMatrixBoolean()
{
	cout << " Matriz I: " << vI.size() << endl;
	
	for (size_t i = 0; i < vI.size(); ++i)
	{
		
		for (size_t c = 0; c < vI[i].values.size(); ++c)
	    {
			cout << vI[i].values[c] << ",";	

	    }
	    cout <<endl;		
	}	

}		
	

				
/**
 * Função que converte um valor em string para valor inteiro
 * */
int getStrToInt(string strConvert)
{
	return ( atoi(strConvert.c_str() ) );
}
