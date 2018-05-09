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


//myMap = tipo definido do unordered_map<string, size_t> para simplificação
typedef unordered_map<string, size_t> myMap;

//vetor de mapas = cada posição do vetor guardará uma mapa de um documento
vector <myMap> vMapTokens;

//myMin = tipo definido do typedef vector<size_t> para simplificação
typedef vector<size_t> myMin;

/*vetor de mapas = cada posição do vetor guardará uma vetor de valores mínimo do minHash  para
todas as funções minHash para cada documento */
vector <myMin> vMinHash;

//myBucket = tipo definido do unordered_map<string, vector<size_t> > para simplificação
typedef unordered_map<string, vector<size_t>> myBucket;

/*vetor de buckets = cada posição do vetor guardará um map, que contém a string da
 * assinatura do baldecomo chave e um vetor contendo as posições dos documentos que possuem
 * essa assinatura */
vector <myBucket> vMapBuckets;

//myColisoes = tipo definido do vector<size_t> para simplificação
typedef vector<size_t> myColisoes;

//lista de palavras que não devem fazer parte do map de Documentos
list<string> stopWords;

//armarzena o nome do arquivo digitado pelo usuário
string fileDoc = "";

//armazena a quantidade de minHashs que será calculada
//nB = faixas e nR = assinatura
int nFuncHash = 1;
int nB, nR = 1;

#define P 109297 //número primo grande
#define result "result.txt"

ofstream fileResult; // declara o arquivo onde armazenará os resultados 

/*----------------------------------------------*/

void createStopWords ();
void printStopWords(list<string> stopWords);
list<string> getStopWords ();
bool isStopWord(string word);

vector<myMap> getTokens(vector<myMap> &vMap, string nameFile) throw (logic_error);
void tokenize (string str, vector<myMap> &vMap);

void printTokens(vector<myMap> &vMapTokens);
void printHash(vector<myMap> &vMapTokens);
void printHashTokens(vector<myMap> &vMapTokens);

void getAllJaccard(vector<myMap> &vMapTokens, vector<myBucket> &vMapBuckets,
					vector<myColisoes> &vColisoes);
double jaccard (vector<myMap> &vMap, size_t pDoc1, size_t pDoc2);

void getAllMinHash(vector<myMap> &vMapTokens, vector<myMin> &vMinHash,
	size_t nFuncHash);
myMin minHash(vector<myMap> &vMapTokens, size_t a, size_t b);
void printAllMinHash(vector<myMin> &vMinHash, size_t nFuncHash);

void lsh(vector<myMin> &vMinHash,vector<myBucket> &vMapBuckets, int nR);
bool locateJaCalculouJaccard(size_t d1, size_t d2, vector<myColisoes> &vColisoes);
void printBuckets(vector<myBucket> &vMapBuckets);

string itoa(int a);

/*----------------------------------------------------------*/

int main(int argc, char *argv[])
{
	fileResult.open(result,ios::app);//abre arquivo que salvará o resultado
	
	while (fileDoc == "")
	{
		cout << "Digite o nome do arquivo: ";
		cin >> fileDoc;
	}
	
	cout << "Digite o valor de b (faixas): ";
	cin >> nB;
	
	cout << "Digite o valor de r (num. de assinaturas): ";
	cin >> nR;
	
	nFuncHash = nB * nR;
	
	fileResult << "Arquivo: " << fileDoc << " b=" <<nB << " r="<<nR <<
	" Formato: D1 D2 J"<<endl<<endl;
			
    createStopWords();

    vMapTokens = getTokens(vMapTokens, fileDoc);
	
	cout<<"Cálculo minHash para todos os documentos: " <<endl;
	getAllMinHash(vMapTokens, vMinHash, nFuncHash);

	cout << "Cálculo LSH: " <<endl;
    lsh(vMinHash,  vMapBuckets, nR);
    
    cout <<endl<<endl;
  //  printBuckets(vMapBuckets);	
    
    cout << "Similaridade de Jaccard: " <<endl;
	vector<myColisoes> vColisoes (vMapTokens.size());
	getAllJaccard(vMapTokens, vMapBuckets, vColisoes);


    return 1;

}

/** ----------------------------------------------------------------- */
/** Função que lê o arquivo e obtém os tokens
 * ------ */
vector<myMap> getTokens(vector<myMap> &vMap, string nameFile) throw (logic_error)
{
	string line;
	ifstream myFile;
	myFile.open (nameFile,ios::in);

	if ( myFile.is_open() )
	{
		while (myFile.good())
		{
			getline(myFile,line);
			tokenize(line, vMap);
		}
		myFile.close();
    }
	else
		throw logic_error("Não foi possível abrir o arquivo.");

	return vMap;
}

/** ----------------------------------------------------------------- */
/** Função que processa cada linha do arquivo, filtrando e convertendo
 * as palavras de acordo com os parâmetros definidos
 * ------ */
void tokenize (string str, vector<myMap> &vMap)
{
	char sep = ' ';
    istringstream line(str) ;
	string token;
	size_t found;
	myMap::const_iterator it;
	myMap mapDoc;

	while( getline( line, token, sep ) )
	{
		/*permitir somente palavras com mais de 2 caracteres,
		desde que não seja uma "palavra proibida" */
		if ( (token.length() > 2) && (isStopWord(token) == false) )
		{
			//converte para minúsculo
			transform(token.begin(), token.end(), token.begin(), ::tolower);

			//verifica se existe um sinal de pontuação no final do token
			if(ispunct(token[token.length()-1]))
			{
				//se existir, excluir a pontuação
				token.erase(token.begin()+token.length()-1,token.begin()+token.length());
			}

			//tenta encontrar o token no map
			it = mapDoc.find (token);
			if (it != mapDoc.end())
			{
				//se encontrar, acrescenta mais 1 no valor
				mapDoc[token]=it->second+1;
			}
			else
			{
				//senão, insere o novo token
				mapDoc.insert({token,1});
			}
		}
	}

	if (mapDoc.size() > 0 ) vMap.push_back(mapDoc);
}

	
/** ----------------------------------------------------------------- */
/** Função que obtém a similaridade de jaccard entre todos os pares
 * de documentos
 * ------ */
void getAllJaccard(vector<myMap> &vMapTokens, vector<myBucket> &vMapBuckets,
					vector<myColisoes> &vColisoes)
{
	myBucket map;
	vector<size_t> vDoc;

	bool calculou = false;
	bool start = true;
	
	size_t cont = 0;
	size_t contPrint = 0;
	double j, jTot, jMin, jMax, jMean, jVar, jDesvio;

	jTot = 0;
	vector<double> vJaccard;
	
	for(size_t i = 0;  i < vMapBuckets.size(); ++i)
	{
		map = vMapBuckets[i];
		
		for (auto& x: map)
		{
			vDoc = x.second;
			
			if (vDoc.size() > 1)
			{				
			 	for(size_t c = 0; c < vDoc.size()-1; ++c)
				{
					
					for (size_t x = c+1; x < vDoc.size(); ++x)
					{
						calculou = locateJaCalculouJaccard(vDoc[c], vDoc[x], vColisoes);
						
						if (calculou == false)
						{
							++cont;	
							//cálculo de jaccard entre par de documento sem minHash
							j = jaccard (vMapTokens, vDoc[c], vDoc[x]);
							vJaccard.push_back(j);
							jTot = jTot + j;
							
							if (start)
							{
								jMax = j;
								jMin = j;
							}	
							else
							{
								if(j > jMax) jMax = j;
								if(j < jMin) jMin = j;
							}	
							
							//só salvar resultado no arquivo o máximo do número de documentos
							++contPrint;
							
							if(contPrint < vMapTokens.size())
							{
								fileResult << (vDoc[c]+1) << " " << (vDoc[x]+1)<< " " << j <<endl;
							}
							start = false;
	
						}
						
					}	
				}
			}	
			
		}
	}
	
	jMean = jTot / cont;
	
	jTot = 0;
	for(size_t i = 0; i < vJaccard.size();++i)
	{
		jTot = jTot + pow ( (vJaccard[i] - jMean), 2);
	}	
	jVar=jTot/cont;
    jDesvio=sqrt(jVar);
    
	fileResult <<endl<< "Max: " << jMax << " Min: " << jMin <<
	" Mean: " <<jMean << " Desvio Padrão: " <<jDesvio <<
	" Total de pares: " << cont <<endl; 
	
	/*
	for (size_t i = 0; i<vColisoes.size(); ++i)
	{
	 cout << "Doc: " << (i+1) << " com pares: ";
	 myColisoes m = vColisoes.at(i);
	 for (size_t c = 0 ; c<m.size(); ++c)
	    {
		cout << (m[c]+1) <<", ";
		}	 
		cout <<endl;
	}*/	
	
	cout << " Total de pares que vai calcular jaccard: " << cont <<endl;	
}		
	

	
/** ----------------------------------------------------------------- */
/** Função que calcula a similaridade de jaccard entre dois documentos
 * ------ */
double jaccard (vector<myMap> &vMap, size_t pDoc1, size_t pDoc2)
{
	myMap::const_iterator it;
	size_t count = 0;
	double j = 0.0;

	myMap map1 = vMap[pDoc1];
	myMap map2 = vMap[pDoc2];

	if (map1.size() < map2.size())
	{
		//tentar encontrar os tokens do map1 no map2
		for (auto& item: map1)
		{
			it = map2.find (item.first);
			if (it != map2.end())
			{
				count +=1;
			}

		}
	}
	else
	{
		//tentar encontrar os tokens do map2 no map1
		for (auto& item: map2)
		{
			it = map1.find (item.first);
			if (it != map1.end())
			{
				count +=1;
			}

		}
	}

	double totWords = map1.size() + map2.size() - (count);
	//cout << "Total de palavras repetidas: " << count <<endl;
	//cout << "Total de palavras não repetidas: " << totWords <<endl;

	j = ( count /  totWords  );

	
	return j;
}


/** ----------------------------------------------------------------- */
/** Função que obtém o minHash para todos os documentos armazenados
 * no vetor de maps
 * ------ */
void getAllMinHash(vector<myMap> &vMapTokens, vector<myMin> &vMinHash, 
	size_t nFuncHash)
{
	size_t a, b;
	
	//gerar número aleatório de 1 até P-1
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	mt19937 generator (seed);
	uniform_int_distribution<int> distribution(1,(P-1));
		
	for (size_t i = 1; i <= nFuncHash; ++i )
	{
		a = distribution(generator);//número aleatório para a
		b = distribution(generator);//número aleatório para b
	
	//	cout << "H" << i <<" ";// " a=" << a << " e b=" << b <<endl ;
			
		myMin vMin;
		vMin = minHash(vMapTokens, a, b);
		
		//adicionar o valor mínimo da função hi de cada documento
		//no vMinHash	
		for (size_t c = 0; c < vMin.size(); ++c)
		{
			myMin v;

			if (i == 1)
			{
				v.push_back(vMin[c]);
				vMinHash.push_back(v);
			}	
			else
			{
				v = vMinHash[c];
				v.push_back(vMin[c]);
				vMinHash[c] = v;
			}
		}			
	}			
}

/** ----------------------------------------------------------------- */
/** Função que mostra na tela a matriz de similaridade dos minHashes
 * ------ */
void printAllMinHash(vector<myMin> &vMinHash, size_t nFuncHash)
{
	fileResult << "--------------------------------------------"<<endl<<endl;
	fileResult << "MinHash"<<endl;
	
	for (size_t i=0; i < vMinHash.size(); ++i)
	{
		cout << "D"<<(i+1)<<" ";
		fileResult << "D"<<(i+1)<<endl;
		
		myMin vMin;
		vMin = vMinHash[i];
		
		for(size_t c = 0; c < (nFuncHash - 1); ++c)
		{
			
			for (size_t cont = c + 1; cont <= (nFuncHash-1); ++cont)
			{
	//			cout <<"[H"<<(c+1)<<", H"<<(cont+1)<<"]=["<<vMin[c]
	//				<<", "<<vMin[cont]<<"]"<<endl;
				fileResult <<"[H"<<(c+1)<<", H"<<(cont+1)<<"]=["<<vMin[c]
					<<", "<<vMin[cont]<<"]"<<endl;	
				
			}	
		}	
	//	cout <<endl;
		fileResult<<endl;
	}	 
}
	
/** ----------------------------------------------------------------- */
/** Função que calcula o minHash para documento
 * ------ */
myMin minHash(vector<myMap> &vMapTokens, size_t a, size_t b)
{
	size_t h, c, min;

	myMap::hasher hf;
	
	myMin vMin;
	
	for (size_t i = 0; i < vMapTokens.size(); i++)
	{
		hf = vMapTokens[i].hash_function();
		c = 0;
		
		for (auto& item: vMapTokens[i])
		{
		    /* minHash: h(x) = (a*x + b) mod P ->
			onde x é o índice do atributo - hash armazenado */
			h = (a * hf(item.first) + b) % P;
			
			if  (c == 0 ) min = h;
			else if ( h < min ) min = h;
			c++;
		}
		vMin.push_back(min);
	}	

//	cout <<endl;
	
	//retorna  vetor com o valor min da função para cada documento
	return vMin;
}
/*Função lsh: divide os documentos em baldes, de acordo com a assinatura
 * de tamanho nR. Assinatura é a chave do mapa e o mapa contém um vetor
 * de inteiros que armazena a posição dos documentos
 * Assinatura será uma string, concatenada do valor do 
 * vetor de minhash(que é um inteiro)
 * Formato da assinatura: num1_num2_num3_num4_Num5*/
void lsh(vector<myMin> &vMinHash, vector<myBucket> &vMapBuckets, int nR)
{
	string sAssinatura = "";
	myBucket::const_iterator it;
	myBucket mapBuckets;
	
	for (size_t i=0; i < vMinHash.size(); ++i)
	{
		myMin vMin;
		vMin = vMinHash[i];
		
		size_t cont = 0; //conta o número da assintura, que deve ter tamanho nR
		for(size_t c=0; c< vMin.size(); ++c)
		{
			if (cont < nR)
			{
				if (cont == 0) sAssinatura = itoa(vMin[c]);
				else sAssinatura = sAssinatura + "_" + itoa(vMin[c]);
				++cont;
			}	
						
			if (cont == nR)
			{		
				//tenta encontrar a assinatura no map
				it = mapBuckets.find (sAssinatura);
				vector<size_t> vDoc;
				
			if (it != mapBuckets.end())
				{
					//se encontrar, acrescenta mais esse documento no vetor
					vDoc = it->second;
					vDoc.push_back(i);
					mapBuckets[sAssinatura]=vDoc;

				}
				else
				{
					//senão, insere a nova assinatura					
					vDoc.push_back(i);
					mapBuckets.insert({sAssinatura,vDoc});
				}
				
				//		
				
				sAssinatura="";
				cont = 0;
			}	
		}	
	}	
	
	vMapBuckets.push_back(mapBuckets);
}

bool locateJaCalculouJaccard (size_t d1, size_t d2, vector<myColisoes> &vColisoes)
{
	bool calculou = false;
	myColisoes vAux;
	
	if (vColisoes.size()> 0)
	{
		if (vColisoes.at(d1).size() > 0 )
		{
			vAux = vColisoes.at(d1);

			//verifica se existe o d2 no vetor de documentos que calcularam jaccard
			for (size_t i = 0; i<vAux.size(); i++)
			{
				if (vAux[i] == d2)
				{
					calculou = true;
					break;
				}	
			}
		}		
	}
	
	//caso ainda nao calculou o jaccard para o par de documentos,
	//atualiza o vetor de colisões informando que agora será calculado
	if (calculou == false)
	{
		vAux.push_back(d2);
		vColisoes.at(d1) = vAux;	
	}	
	
	return calculou;
}


/** ----------------------------------------------------------------- */
/** Função que cria uma lista de stop words
 * ------ */
void createStopWords ()
{
	string line;
	ifstream myFile;
	myFile.open ("stopWords.txt",ios::in);

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

/** ----------------------------------------------------------------- */
/** Função que imprime na tela todas as stop words
 * ------ */
void printStopWords(list<string> stopWords)
{
	for (auto item = stopWords.cbegin(); item != stopWords.cend(); ++item)
		cout << ' ' << *item;
	cout<<endl;
}

/** ----------------------------------------------------------------- */
/** Função que mostra na tela o valor de cada item gravado no map:
 * primeiro valor é a palavra e o segundo, a qtde de vezes em que aparece
 * a palavra no documento
 * ------ */
void printTokens(vector<myMap> &vMapTokens)
{
	size_t cont = 0;

	for(auto& vMap : vMapTokens)
	{
		++cont;
		cout << "Tokens do Doc" << cont << " armazenado no map:" << endl;
		for (auto& item: vMap)
			cout << item.first << ":" << item.second << endl;

		cout <<endl;
	}
}

/** ----------------------------------------------------------------- */
/** Função que mostra na tela o valor da função hash de cada palavra gravada
 * no map
 * ------ */
void printHash(vector<myMap> &MapTokens)
{
	myMap::hasher hf;
	size_t cont = 0;

	for(auto& vMap : vMapTokens)
	{
		++cont;
		cout << "Hash do Doc" << cont << " armazenado no map:" << endl;
		hf = vMap.hash_function();
		for (auto& item: vMap)
			cout << hf(item.first) << endl;

		cout <<endl;
	}
}


/** ----------------------------------------------------------------- */
/** Função que mostra na tela o token, a frequencia no documento
 *  e o valor da função hash de cada  palavra gravada no map
 * ------ */
void printHashTokens(vector<myMap> &vMapTokens)
{
	myMap::hasher hf;
	size_t cont = 0;

	for(auto& vMap : vMapTokens)
	{
		++cont;
		cout << "Doc" << cont << " armazenado no map:" << endl;
		hf = vMap.hash_function();
		for (auto& item: vMap)
			cout << item.first << ": " << item.second << " = " <<
			  hf(item.first) << endl;

		cout <<endl;
	}

	cout <<endl;
}


/** ----------------------------------------------------------------- */
/** Função que mostra na tela os buckets - assinatura e documentos
 * de cada bucket
 * ------ */
void printBuckets(vector<myBucket> &vMapBuckets)
{
	myBucket map;
	vector<size_t> vDoc;
	size_t total = 0;
	for(size_t i = 0;  i < vMapBuckets.size(); ++i)
	{
		map = vMapBuckets[i];
		for (auto& x: map)
		{
			vDoc = x.second;
			
			if (vDoc.size() > 1)
			{
				cout << "Assinatura: " << x.first << " - Docs: ";
			
			 	for(size_t c = 0; c < vDoc.size(); ++c)
				{
					cout << (vDoc[c]+1) << ", ";
				}
				cout<<endl;
				++total;
			}	
			
		}
	}	
	cout <<endl<<"Total: " << total<<endl;
}


int getStrToInt(string strConvert)
{
	//converte uma string em inteiro
	return ( atoi(strConvert.c_str() ) );
}

//converte um número inteiro em string
string itoa(int a)
{
    string ss="";   //create empty string
    while(a)
    {
        int x=a%10;
        a/=10;
        char i='0';
        i=i+x;
        ss=i+ss;      //append new character at the front of the string!
    }
    return ss;
}
