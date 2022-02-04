#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <set>
#include <chrono>
#include <map>
#include <sstream>
#include <random>

#define INFINITO std::numeric_limits<int>::max()

// estrutura que define a composicao de uma coordenada de ponto
typedef struct Ponto
{

    int x, y, z, r, g, b;
    int c1, c2, c3;

    std::string chave;

    
    void formaChave(int divisor)
    {

        c1 = x / divisor;
        c2 = y / divisor;
        c3 = z / divisor;
    }

    void criaChave(std::string separador, std::string extensao)
    {

        chave.append(std::to_string(c1));
        chave.append(separador);
        chave.append(std::to_string(c2));
        chave.append(separador);
        chave.append(std::to_string(c3));
        chave.append(extensao);
    }

} ponto;

// funcao que pula linhas baseada num valor inteiro enviado como parametro
std::fstream &pulaLinha(std::fstream &file, int num)
{
    file.seekg(std::ios::beg);
    for (int i = 0; i < num - 1; i++)
    {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return file;
}

//void calculaDiferencial(std::map<std::string, std::pair<int, std::string>> &tabela, int &x, int &menorX, int &maiorX, int &y, int &z, std::string &separador, std::string &extensao)
//{
//}

int main(int argc, char *argv[])
{

    // inicializa o cronometro pra medicao de perda de eficiencia
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::string headerIni = "ply\nformat ascii 1.0\ncomment Version 2, Copyright 2017, 8i Labs, Inc.\ncomment frame_to_world_scale 0.179523\ncomment frame_to_world_translation -45.2095 7.18301 -54.3561\ncomment width 1023\nelement vertex ";
    std::string headerFim = "\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header\n";

    // abre arquivo segundo parametro informado no terminal
    std::fstream file(argv[2]);

    int j = 15;

    std::stringstream ss;
    ss << argv[1];

    int divisor;

    ss >> divisor;

    // condicao de saida de programa pra nao aceitar valores negativos
    if (divisor < 0)
    {
        std::cout << "Erro! insira um valor inteiro positivo no primeiro parammetro.\n";
        exit(0);
    }

    int menorX = INFINITO, menorY = INFINITO, menorZ = INFINITO;
    int maiorX = 0, maiorY = 0, maiorZ = 0;

    std::string linhaAtual;
    std::string extensao = ".ply";
    std::string separador = "-";

    std::map<std::string, std::pair<int, std::string>> tabela;

    pulaLinha(file, j);
    ponto pontoAtual;

    // loop que preenche o mapa enquanto houver conteudo no arquivo input
    while (file.good())
    {

        file >> linhaAtual;
        pontoAtual.x = std::stoi(linhaAtual);
        file >> linhaAtual;
        pontoAtual.y = std::stoi(linhaAtual);
        file >> linhaAtual;
        pontoAtual.z = std::stoi(linhaAtual);
        file >> linhaAtual;

        pontoAtual.r = std::stoi(linhaAtual);
        file >> linhaAtual;
        pontoAtual.g = std::stoi(linhaAtual);
        file >> linhaAtual;
        pontoAtual.b = std::stoi(linhaAtual);

        pontoAtual.chave = "";
        pontoAtual.formaChave(divisor);

        menorX = std::min(menorX, pontoAtual.c1);
        maiorX = std::max(maiorX, pontoAtual.c1);

        menorY = std::min(menorY, pontoAtual.c2);
        maiorY = std::max(maiorY, pontoAtual.c2);

        menorZ = std::min(menorZ, pontoAtual.c3);
        maiorZ = std::max(maiorZ, pontoAtual.c3);

        pontoAtual.criaChave(separador, extensao);

        auto it = tabela.find(pontoAtual.chave);

        if (it == tabela.end())
        {
            std::string dado = std::to_string(pontoAtual.x) + " " + std::to_string(pontoAtual.y) + " " + std::to_string(pontoAtual.z) + " " + std::to_string(pontoAtual.r) + " " + std::to_string(pontoAtual.g) + " " + std::to_string(pontoAtual.b) + "\n";
            tabela.insert(std::make_pair(pontoAtual.chave, std::make_pair(1, dado)));
        }
        else
        {
            std::string dado = std::to_string(pontoAtual.x) + " " + std::to_string(pontoAtual.y) + " " + std::to_string(pontoAtual.z) + " " + std::to_string(pontoAtual.r) + " " + std::to_string(pontoAtual.g) + " " + std::to_string(pontoAtual.b) + "\n";
            it->second.first++;
            it->second.second.append(dado);
        }
    }

    int x = menorX, y = menorY, yAtual = menorY, z = menorZ;

    // variavel booleana que torna-se falsa caso seja encontrado um nodo; volta a ser verdadeira caso troque y ou z
    bool first = true;

    // funcao de valor aleatorio, de alcance entre 0 - 1000
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, 1000);

    // variaveis que armazenam os valores para calculo de diferencial
    int randX, randY, randZ;
    int randXAnterior, randYAnterior, randZAnterior;

    // endereco destino do calculo de diferenciais (mesma pasta do output de blocagem)
    std::stringstream sss;
    sss << argv[3];
    sss << "diferenciais.txt";

    // abre o arquivo de texto em modo de substituicao (pra testar multiplos numeros aleatorios)
    std::ofstream outputDiferencial(sss.str(), std::ios_base::trunc);

    // loop de varredura completa que encontra nodos no map tabela
    for (;;)
    {

        auto it = tabela.find(std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao);
        if (it != tabela.end())
        {

            // std::cout << "achou! " << x << separador << y << separador << z << extensao << "\n"; // PARA DEBUGGING

            if (first)
            {
                first = false;
                randX = distribution(generator);
                randY = distribution(generator);
                randZ = distribution(generator);
                randXAnterior = randX;
                randYAnterior = randY;
                randZAnterior = randZ;

                outputDiferencial << std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao;
                outputDiferencial << "\n"
                                  << "Vetor de Movimento: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n";
                outputDiferencial << "Diferencial: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n\n";
            }
            else
            {
                randX = distribution(generator);
                randY = distribution(generator);
                randZ = distribution(generator);

                // abs: retorna valor absoluto (sem negativos)
                randXAnterior = std::abs(randX - randXAnterior);
                randYAnterior = std::abs(randY - randYAnterior);
                randZAnterior = std::abs(randZ - randZAnterior);

                outputDiferencial << std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao;
                outputDiferencial << "\n"
                                  << "Vetor de Movimento: " << randX << " " << randY << " " << randZ << "\n";
                outputDiferencial << "Diferencial: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n\n";
            }
        }

        // as condicionais a seguir determinam a ordem de incremento das coordenadas, funcionando como um relogio/calendario de prioridade (xx/yy/zz)
        // (isso nos permite manipular a ordem de busca dos nodos)
        if (x == maiorX)
        {
            first = true;
            x = menorX;
            y++;
        }

        if (y == maiorY)
        {
            first = true;
            y = menorY;
            yAtual = menorY;
            z++;
        }

        if (yAtual < y || (x == 0 && y == 0))
        {
            auto it = tabela.find(std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao);
            if (it != tabela.end())
            {
                // std::cout << "achou! " << x << separador << y << separador << z << extensao << "\n";      // SOMENTE PARA DEBUGGING

                if (first)
                {
                    first = false;
                    randX = distribution(generator);
                    randY = distribution(generator);
                    randZ = distribution(generator);
                    randXAnterior = randX;
                    randYAnterior = randY;
                    randZAnterior = randZ;

                    outputDiferencial << std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao;
                    outputDiferencial << "\n"
                                      << "Vetor de Movimento: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n";
                    outputDiferencial << "Diferencial: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n\n";
                }
                else
                {
                    randX = distribution(generator);
                    randY = distribution(generator);
                    randZ = distribution(generator);
                    randXAnterior = std::abs(randX - randXAnterior);
                    randYAnterior = std::abs(randY - randYAnterior);
                    randZAnterior = std::abs(randZ - randZAnterior);

                    outputDiferencial << std::to_string(x) + separador + std::to_string(y) + separador + std::to_string(z) + extensao;
                    outputDiferencial << "\n"
                                      << "Vetor de Movimento: " << randX << " " << randY << " " << randZ << "\n";
                    outputDiferencial << "Diferencial: " << randXAnterior << " " << randYAnterior << " " << randZAnterior << "\n\n";
                }
            }
            yAtual++;
        }

        // condicao de quebra do loop, indicando que nao ha mais pontos a serem buscados
        if (x == menorX && z == maiorZ + 1 && y == menorY)
        {
            break;
        }

        x++;
    }

    outputDiferencial.close();

    auto it = tabela.begin();

    // loop de impressao dos dados em arquivos de nome formado pela juncao das chaves calculadas atraves do metodo de blocagem
    while (it != tabela.end())
    {
        std::string headerCompleta = headerIni + std::to_string(it->second.first) + headerFim;
        std::string headerExport = headerCompleta + it->second.second;
        std::ofstream novoArquivo(argv[3] + it->first);
        novoArquivo << headerExport;
        novoArquivo.close();
        it++;
    }

    file.close();

    // calculo diferencial de tempo que define o intervalo de execucao total do programa, logo em seguida impresso no terminal
    // (usado somente pra teste de eficiencia)
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Tempo de execucao: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s" << std::endl;

    return 0;
}
