#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 4

typedef struct {
  char nome[50];
  double valores[N];
} Flor;

typedef struct {
  int *vertices;
  int quantVertices;
  int ordem;
  double centro;
  char nome[50];
} Grupo;

int truePositivos(Grupo grupo, Flor *vertices);
int falsePositivos(Grupo grupo, Flor *vertices);
int falseNegativos(Grupo *grupos, int quantGrupos, Flor*vertice, Grupo grupo);
int trueNegativos(Grupo *grupos, int quantGrupos, Flor*vertice, Grupo grupo);
void defineNomeGrupos(Grupo *grupos, int quantGrupos, Flor *vertices);
Grupo *ajustaEm3Grupos(Grupo *grupos, int quantGrupos);
void ordenaGruposDecescente(Grupo *grupos, int quantGrupos);
Grupo *dfsInicia(double **matriz, int numVertices, Grupo *grupos, int *quantGrupos, Flor *vertices);
void dfs(double **matriz, int *visitados, int vertice, int numVertices, Grupo *grupos, int quantGrupos, Flor *vertices);
Flor *leEntrada(int *linhas);
double **alocaMatriz(int linhas, int colunas);
void distanciaEuclidiana(double **matrizVertices, double **matriz, int linhas, int colunas);
double maiorValor(double **matriz, int linhas, int colunas);
void normalizaMatriz(double **matriz, double denominador, int linhas, int colunas);
void liberaMatriz(double **matriz, int linhas);
int defineArestas(double **matrizNormalizada, double valor, int linhas, int colunas);
void escreveMatriz(double **matriz, char *nome, int linhas, int colunas);

int main()
{
    int m = 0;

    Flor * matrizVertices = leEntrada(&m);

    if(matrizVertices == NULL)
    {
        printf("Não foi possível abrir o arquivo!\n");
        return 1;
    }

    double **matrizAux = alocaMatriz(m, N);

    for(int i=0;i<m;i++)
      for(int j=0;j<N;j++)
        matrizAux[i][j] = matrizVertices[i].valores[j];
        
    escreveMatriz(matrizAux, "matrizDeVertices.txt", m, N);

    double **matrizDistancia = alocaMatriz(m, m);

    distanciaEuclidiana(matrizAux, matrizDistancia, m, m);

    escreveMatriz(matrizDistancia, "matrizDistanciaEuclidiana.txt", m, m);

    double maior = maiorValor(matrizDistancia, m, m);

    normalizaMatriz(matrizDistancia, maior, m, m);

    maior = maiorValor(matrizDistancia, m, m);

    escreveMatriz(matrizDistancia, "matrizNormalizada.txt", m, m);

    int quantArestas = defineArestas(matrizDistancia, 0.08, m, m);

    //printf("%d", quantArestas);

    escreveMatriz(matrizDistancia, "matrizDeAdjacencia.txt", m, m);
    
    FILE *arquivo;

    arquivo = fopen("arestas.txt", "w");

    //fprintf(arquivo, "%d", quantArestas);

    for(int i=0;i<m;i++)
    {
        int aux = 0;
        for(int j=0;j<m;j++)
            if(matrizDistancia[i][j] == 1)
            {
                fprintf(arquivo, "%d %d\n", i+1, j+1);
                aux = 1;
            }
        if(!aux)
            fprintf(arquivo, "%d\n", i+1);
    }

    fclose(arquivo);

    Grupo * grupos = NULL;
    int quantGrupos = 0;
    grupos = dfsInicia(matrizDistancia, m, grupos, &quantGrupos, matrizVertices);

    for(int i=0;i<quantGrupos;i++)
    {
        //printf("%d", grupos[i].quantVertices);
        printf("\nVertices Grupo %d: Centro = %lf - QuantVertices = %d\n", grupos[i].ordem, grupos[i].centro, grupos[i].quantVertices);
        for(int j=0;j<grupos[i].quantVertices;j++)
            printf("%d\n", grupos[i].vertices[j]);
    }

    ordenaGruposDecescente(grupos, quantGrupos);

    Grupo * grupos_3 = ajustaEm3Grupos(grupos, quantGrupos);
  
    defineNomeGrupos(grupos_3, 3, matrizVertices);
  
    printf("\n-----\n");

    for(int i=0;i<3;i++)
    {
        //printf("%d", grupos[i].quantVertices);
        printf("\nVertices Grupo %s: Centro = %lf - QuantVertices = %d\n", grupos_3[i].nome, grupos_3[i].centro, grupos_3[i].quantVertices);
        for(int j=0;j<grupos_3[i].quantVertices;j++)
            printf("%d\n", grupos_3[i].vertices[j]);
    }
    
    for(int i=0;i<3;i++)
    {
        int tp = truePositivos(grupos_3[i], matrizVertices);
        int fp = falsePositivos(grupos_3[i], matrizVertices);
        int tn = trueNegativos(grupos_3, 3, matrizVertices, grupos_3[i]);
        int fn = falseNegativos(grupos_3, 3, matrizVertices, grupos_3[i]);

        printf("\nGrupo %s:\n\n", grupos_3[i].nome);
        printf("True Positivos: %d\n", tp);
        printf("False Positivos: %d\n", fp);
        printf("True Negativos: %d\n", tn);
        printf("False Negativos: %d\n", fn);
        float acuracia = (tp + tn) / ((tp + fp + tn + fn) * 1.0);
        float recall = tp / ((tp + fn) * 1.0);
        float precision = tp / ((tp + fp) * 1.0);
        float f1 = 2 * precision * recall / (precision + recall);
        printf("\nAcuracia: %f\n", acuracia);
        printf("Recall: %f\n", recall);
        printf("Precision: %f\n", precision);
        printf("F1 Score: %f\n", f1);
    }

    liberaMatriz(matrizAux, m);
    liberaMatriz(matrizDistancia, m);
  
    return 0;
}

Flor *leEntrada(int *linhas)
{
    FILE *arquivo;
    Flor * matrizVertices = NULL;
    char linha[50];

    arquivo = fopen("IrisDataset.csv", "r");
  
    if (arquivo == NULL) 
        return NULL;
  
    while (fgets(linha, 50, arquivo) != NULL) 
    {
        matrizVertices = realloc(matrizVertices, sizeof(Flor) * (linhas[0]+1));

        Flor flor;

        char *token = strtok(linha, ",");
        int aux = 0;

        while (token != NULL)
        {
            if(aux == 4)
            {
              strcpy(flor.nome, token);
            }
            else
            {
              flor.valores[aux] = atof(token);
              aux++;
            }
            token = strtok(NULL, ",");
        }
        matrizVertices[linhas[0]] = flor;
        linhas[0]++;
    }

    fclose(arquivo);

    return matrizVertices;
}

void distanciaEuclidiana(double **matrizVertices, double **matriz, int linhas, int colunas)
{

    for(int i=0;i<linhas;i++)
        for(int j=0;j<colunas;j++)
        {
            double aux = 0;
            for(int c=0;c<N;c++)
                aux += (pow(matrizVertices[i][c] - matrizVertices[j][c], 2));

            matriz[i][j] = sqrt(aux);
        }
}

double maiorValor(double **matriz, int linhas, int colunas)
{
    double maior = matriz[0][0];

    for(int i=0;i<linhas;i++)
        for(int j=0;j<colunas;j++)
            if(matriz[i][j] > maior)
                maior = matriz[i][j];
  
    return maior;
}

void normalizaMatriz(double **matriz, double denominador, int linhas, int colunas)
{
    for(int i=0;i<linhas;i++)
        for(int j=0;j<colunas;j++)
            matriz[i][j] = matriz[i][j] / denominador;
}

void liberaMatriz(double **matriz, int linhas)
{
    for (int i = 0; i < linhas; i++)
    {
        free(matriz[i]);
    }

    free(matriz);
}

int defineArestas(double **matrizNormalizada, double valor, int linhas, int colunas)
{
    int quant = 0;

    for(int i=0;i<linhas;i++)
        for(int j=0;j<colunas;j++)
        {
            if(matrizNormalizada[i][j] <= valor && i != j)
            {
                matrizNormalizada[i][j] = 1;
                quant++;
            }
            else
                matrizNormalizada[i][j] = 0;
        }

    return quant;
}

double **alocaMatriz(int linhas, int colunas)
{
    double **matriz = malloc(sizeof(double *) * linhas);

    for(int i=0;i<linhas;i++)
    matriz[i] = malloc(sizeof(double) * colunas);

    return matriz;
}

void escreveMatriz(double **matriz, char *nome, int linhas, int colunas)
{
    FILE *arquivo;

    arquivo = fopen(nome, "w");

    for(int i=0;i<linhas;i++)
    {
        for(int j=0;j<colunas;j++)
            fprintf(arquivo, "%lf ", matriz[i][j]);
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
}

void dfs(double **matriz, int *visitados, int vertice, int numVertices, Grupo *grupos, int quantGrupos, Flor *vertices)
{
    visitados[vertice] = 1;
  
    for (int i = 0; i < numVertices; i++)
      if(matriz[vertice][i] && visitados[i] == 0)
      {
        //printf("\n%d com %d\n", vertice, i);
        for(int j=0;j<N;j++)
            grupos[quantGrupos-1].centro += vertices[i].valores[j];

        grupos[quantGrupos-1].quantVertices++;
        int aux = grupos[quantGrupos-1].quantVertices;
        grupos[quantGrupos-1].vertices = realloc(grupos[quantGrupos-1].vertices, sizeof(int) * aux);
        grupos[quantGrupos-1].vertices[aux-1] = i;
        dfs(matriz, visitados, i, numVertices, grupos, quantGrupos, vertices);
      }
}

Grupo * dfsInicia(double **matriz, int numVertices, Grupo *grupos, int *quantGrupos, Flor *vertices)
{
    int *verticesVisitados = malloc(sizeof(int) * numVertices);

    for (int i = 0; i < numVertices; i++) 
    {
        verticesVisitados[i] = 0;
    }
    
    int grupoAux = 0;

    for (int i = 0; i < numVertices; i++)
    {
        if (verticesVisitados[i] == 0)
        {   
            //printf("\ngrupo %d\n", grupoAux);
            Grupo grupo;
            grupo.ordem = grupoAux++;
            grupo.quantVertices = 1;
            grupo.vertices = malloc(sizeof(int));
            grupo.vertices[0] = i;

            double soma = 0;
            for(int j=0;j<N;j++)
                soma += vertices[i].valores[j];
            grupo.centro = soma;

            quantGrupos[0]++;
            grupos = realloc(grupos, sizeof(Grupo) * quantGrupos[0]);
            grupos[quantGrupos[0] - 1] = grupo;
        
            dfs(matriz, verticesVisitados, i, numVertices, grupos, quantGrupos[0], vertices);
            grupos[quantGrupos[0] - 1].centro = grupos[quantGrupos[0] - 1].centro / (grupos[quantGrupos[0] - 1].quantVertices * N);
        }
    }

    free(verticesVisitados);

    return grupos;
}

void ordenaGruposDecescente(Grupo *grupos, int quantGrupos)
{
    for (int i = 0; i < quantGrupos - 1; i++) 
    {
        int indiceMaior = i;
    
        for (int j = i + 1; j < quantGrupos; j++) 
        {
            if (grupos[j].quantVertices > grupos[indiceMaior].quantVertices)
            {
                indiceMaior = j;
            }
        }
    
        Grupo temp = grupos[i];
        grupos[i] = grupos[indiceMaior];
        grupos[indiceMaior] = temp;
    }
}

Grupo *ajustaEm3Grupos(Grupo *grupos, int quantGrupos)
{
    Grupo *gruposAux = malloc(sizeof(Grupo) * 3);

    for (int i = 0; i < 3; i++) 
    {
        gruposAux[i] = grupos[i];

        gruposAux[i].vertices = malloc(sizeof(int) * grupos[i].quantVertices);
        for (int j = 0; j < grupos[i].quantVertices; j++) 
        {
            gruposAux[i].vertices[j] = grupos[i].vertices[j];
        }
    }

    for (int i = 3; i < quantGrupos; i++) 
    {
        double menor = abs(grupos[i].centro - gruposAux[0].centro);
        int indice = 0;
        for (int j = 1; j < 3; j++) 
        {
            if (abs(grupos[i].centro - gruposAux[j].centro) < menor) 
            {
                menor = abs(grupos[i].centro - gruposAux[j].centro);
                indice = j;
            }
        }

        gruposAux[indice].vertices = realloc(gruposAux[indice].vertices, sizeof(int) * (gruposAux[indice].quantVertices + grupos[i].quantVertices));

        for (int c = 0; c < grupos[i].quantVertices; c++) 
        {
            gruposAux[indice].vertices[gruposAux[indice].quantVertices + c] = grupos[i].vertices[c];
        }
        gruposAux[indice].quantVertices += grupos[i].quantVertices;
    }

    return gruposAux;
}

void defineNomeGrupos(Grupo *grupos, int quantGrupos, Flor *vertices)
{
    for(int i=0;i<quantGrupos;i++)
    {
        int setosa = 0;
        int virginica = 0;
        int versicolor = 0;

        for(int j=0;j<grupos[i].quantVertices;j++)
        {
            if(strstr(vertices[grupos[i].vertices[j]].nome, "Virginica") != NULL)
            virginica++;
            if(strstr(vertices[grupos[i].vertices[j]].nome, "Setosa") != NULL)
            setosa++;
            if(strstr(vertices[grupos[i].vertices[j]].nome, "Versicolor") != NULL)
            versicolor++;
        }

        if(setosa > virginica && setosa > versicolor)
        strcpy(grupos[i].nome, "Setosa");
        else
        {
        if(virginica > setosa && virginica > versicolor)
            strcpy(grupos[i].nome, "Virginica");
        else
            strcpy(grupos[i].nome, "Versicolor");
        }
    }
}

int truePositivos(Grupo grupo, Flor *vertices)
{
    int soma = 0;
    for(int i=0;i<grupo.quantVertices;i++)
        if(strstr(vertices[grupo.vertices[i]].nome, grupo.nome) != NULL)
            soma++;

    return soma;
}

int falsePositivos(Grupo grupo, Flor *vertices) 
{
    int soma = 0;
    for(int i=0;i<grupo.quantVertices;i++)
        if(strstr(vertices[grupo.vertices[i]].nome, grupo.nome) == NULL)
            soma++;

    return soma;
}

int falseNegativos(Grupo *grupos, int quantGrupos, Flor*vertice, Grupo grupo)
{
    int soma = 0;

    for(int i=0;i<quantGrupos;i++)
    {
        if(grupos[i].ordem == grupo.ordem)
            continue;

        for(int j=0;j<grupos[i].quantVertices;j++)
            if(strstr(vertice[grupos[i].vertices[j]].nome, grupo.nome) != NULL)
                soma++;
    }

    return soma;
}

int trueNegativos(Grupo *grupos, int quantGrupos, Flor*vertice, Grupo grupo)
{
    int soma = 0;

    for(int i=0;i<quantGrupos;i++)
    {
        if(grupos[i].ordem == grupo.ordem)
            continue;

        for(int j=0;j<grupos[i].quantVertices;j++)
            if(strstr(vertice[grupos[i].vertices[j]].nome, grupo.nome) == NULL)
                soma++;
    }

    return soma;
}